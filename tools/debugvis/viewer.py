#!/usr/bin/env python3
"""
debugvis_viewer.py  —  Live visualization viewer for your raytracer

Runs as a completely separate process.  Connects to your C++ program over a
Unix socket and displays:
  • The image being rendered pixel-by-pixel in real time
  • Hover any pixel to see its RGB, ray direction, angles, and hit info
  • Click a pixel to pin its info panel
  • Side-panel with vector diagram and angle breakdown

Launch:  python3 tools/debugvis/viewer.py
  (or use the debugvis.sh launcher script)
"""

import socket
import struct
import os
import sys
import math
import threading
import time
import tkinter as tk
from tkinter import ttk
from collections import defaultdict

# ── Protocol constants (must match DebugVisualizer.h) ──────────────────────
MSG_INIT       = 0x01
MSG_PIXEL      = 0x02
MSG_RAY        = 0x03
MSG_HIT        = 0x04
MSG_LABEL      = 0x05
MSG_FRAME_DONE = 0x06
MSG_ROW_DONE   = 0x07

SOCK_PATH = os.environ.get("DEBUGVIS_SOCK", "/tmp/debugvis.sock")

# ── Pixel data store ───────────────────────────────────────────────────────
class PixelStore:
    """Thread-safe store for all per-pixel debug data."""
    def __init__(self):
        self.width  = 0
        self.height = 0
        self.pixels = {}          # (x,y) -> (r,g,b)
        self.rays   = {}          # (x,y) -> (ox,oy,oz, dx,dy,dz)
        self.hits   = {}          # (x,y) -> (t, nx,ny,nz, hx,hy,hz)
        self.labels = {}          # (x,y) -> str
        self.frame_done = False
        self.dirty_pixels = []    # batch of newly set pixels
        self.lock = threading.Lock()
        self.pixels_received = 0
        self.rows_done = set()

    def set_size(self, w, h):
        with self.lock:
            self.width  = w
            self.height = h
            self.pixels.clear()
            self.rays.clear()
            self.hits.clear()
            self.labels.clear()
            self.dirty_pixels.clear()
            self.pixels_received = 0
            self.rows_done.clear()
            self.frame_done = False

    def set_pixel(self, x, y, r, g, b):
        with self.lock:
            self.pixels[(x, y)] = (r, g, b)
            self.dirty_pixels.append((x, y, r, g, b))
            self.pixels_received += 1

    def set_ray(self, px, py, ox, oy, oz, dx, dy, dz):
        with self.lock:
            self.rays[(px, py)] = (ox, oy, oz, dx, dy, dz)

    def set_hit(self, px, py, t, nx, ny, nz, hx, hy, hz):
        with self.lock:
            self.hits[(px, py)] = (t, nx, ny, nz, hx, hy, hz)

    def set_label(self, px, py, text):
        with self.lock:
            self.labels[(px, py)] = text

    def mark_row(self, y):
        with self.lock:
            self.rows_done.add(y)

    def mark_frame_done(self):
        with self.lock:
            self.frame_done = True

    def drain_dirty(self):
        """Return and clear the dirty pixel list."""
        with self.lock:
            batch = self.dirty_pixels[:]
            self.dirty_pixels.clear()
            return batch

    def get_info(self, x, y):
        with self.lock:
            return {
                "pixel": self.pixels.get((x, y)),
                "ray":   self.rays.get((x, y)),
                "hit":   self.hits.get((x, y)),
                "label": self.labels.get((x, y)),
            }

# ── Network receiver thread ───────────────────────────────────────────────
class Receiver(threading.Thread):
    """Listens on Unix socket, parses binary messages, populates PixelStore."""

    def __init__(self, store: PixelStore, on_init_cb=None):
        super().__init__(daemon=True)
        self.store = store
        self.on_init_cb = on_init_cb  # called from this thread when INIT arrives
        self._running = True

    def run(self):
        # Clean up stale socket
        try:
            os.unlink(SOCK_PATH)
        except FileNotFoundError:
            pass

        srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind(SOCK_PATH)
        srv.listen(1)
        srv.settimeout(1.0)

        print(f"[debugvis] Waiting for connection on {SOCK_PATH} ...")

        while self._running:
            try:
                conn, _ = srv.accept()
            except socket.timeout:
                continue
            print("[debugvis] Connected!")
            self._handle(conn)
            print("[debugvis] Disconnected.  Waiting for next connection...")

        srv.close()
        try:
            os.unlink(SOCK_PATH)
        except FileNotFoundError:
            pass

    def _recv_exact(self, conn, n):
        """Read exactly n bytes or raise."""
        chunks = []
        remaining = n
        while remaining > 0:
            chunk = conn.recv(remaining)
            if not chunk:
                raise ConnectionError("closed")
            chunks.append(chunk)
            remaining -= len(chunk)
        return b"".join(chunks)

    def _handle(self, conn):
        try:
            while self._running:
                hdr = self._recv_exact(conn, 1)
                tag = hdr[0]

                if tag == MSG_INIT:
                    data = self._recv_exact(conn, 4)
                    w, h = struct.unpack("<HH", data)
                    self.store.set_size(w, h)
                    if self.on_init_cb:
                        self.on_init_cb(w, h)
                    print(f"[debugvis] Image: {w}×{h}")

                elif tag == MSG_PIXEL:
                    data = self._recv_exact(conn, 7)
                    x, y = struct.unpack_from("<HH", data, 0)
                    r, g, b = data[4], data[5], data[6]
                    self.store.set_pixel(x, y, r, g, b)

                elif tag == MSG_RAY:
                    data = self._recv_exact(conn, 28)
                    px, py = struct.unpack_from("<HH", data, 0)
                    ox, oy, oz, dx, dy, dz = struct.unpack_from("<6f", data, 4)
                    self.store.set_ray(px, py, ox, oy, oz, dx, dy, dz)

                elif tag == MSG_HIT:
                    data = self._recv_exact(conn, 32)
                    px, py = struct.unpack_from("<HH", data, 0)
                    t, nx, ny, nz, hx, hy, hz = struct.unpack_from("<7f", data, 4)
                    self.store.set_hit(px, py, t, nx, ny, nz, hx, hy, hz)

                elif tag == MSG_LABEL:
                    data = self._recv_exact(conn, 6)
                    px, py, tlen = struct.unpack("<HHH", data)
                    text = self._recv_exact(conn, tlen).decode("utf-8", errors="replace")
                    self.store.set_label(px, py, text)

                elif tag == MSG_ROW_DONE:
                    data = self._recv_exact(conn, 2)
                    y, = struct.unpack("<H", data)
                    self.store.mark_row(y)

                elif tag == MSG_FRAME_DONE:
                    self.store.mark_frame_done()
                    print("[debugvis] Frame complete.")

        except ConnectionError:
            pass
        finally:
            conn.close()

    def stop(self):
        self._running = False


# ── Helper: angle formatting ──────────────────────────────────────────────
def vec_angles(dx, dy, dz):
    """Return (azimuth_deg, elevation_deg, length) for a direction vector."""
    length = math.sqrt(dx*dx + dy*dy + dz*dz)
    if length < 1e-12:
        return 0.0, 0.0, 0.0
    ndx, ndy, ndz = dx/length, dy/length, dz/length
    azimuth   = math.degrees(math.atan2(ndx, -ndz))   # yaw around Y
    elevation = math.degrees(math.asin(max(-1, min(1, ndy))))
    return azimuth, elevation, length


# ── Main GUI ──────────────────────────────────────────────────────────────
class ViewerApp:
    SCALE_OPTIONS = [1, 2, 3, 4, 6, 8]
    INFO_PANEL_W  = 340

    def __init__(self):
        self.store = PixelStore()
        self.scale = 3            # default zoom
        self.img_w = 100
        self.img_h = 100
        self.pinned = None        # (x,y) of pinned pixel, or None
        self.init_event = threading.Event()

        # Start receiver
        self.receiver = Receiver(self.store, on_init_cb=self._on_init)
        self.receiver.start()

        self._build_ui()

    def _on_init(self, w, h):
        """Called from receiver thread when INIT message arrives."""
        self.img_w = w
        self.img_h = h
        self.init_event.set()

    def _build_ui(self):
        self.root = tk.Tk()
        self.root.title("🔬 Raytracer Debug Viewer")
        self.root.configure(bg="#1e1e2e")
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        # ── Style ──
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("Dark.TFrame",       background="#1e1e2e")
        style.configure("Dark.TLabel",       background="#1e1e2e", foreground="#cdd6f4", font=("monospace", 10))
        style.configure("DarkBold.TLabel",   background="#1e1e2e", foreground="#89b4fa", font=("monospace", 11, "bold"))
        style.configure("Header.TLabel",     background="#1e1e2e", foreground="#f5c2e7", font=("monospace", 12, "bold"))
        style.configure("Dark.TLabelframe",  background="#1e1e2e", foreground="#a6adc8")
        style.configure("Dark.TLabelframe.Label", background="#1e1e2e", foreground="#a6adc8", font=("monospace", 10, "bold"))

        # ── Main layout ──
        main = ttk.Frame(self.root, style="Dark.TFrame")
        main.pack(fill=tk.BOTH, expand=True)

        # Left: canvas
        canvas_frame = ttk.Frame(main, style="Dark.TFrame")
        canvas_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=4, pady=4)

        # Status bar above canvas
        status_frame = ttk.Frame(canvas_frame, style="Dark.TFrame")
        status_frame.pack(fill=tk.X, pady=(0, 4))

        self.status_var = tk.StringVar(value="Waiting for connection...")
        ttk.Label(status_frame, textvariable=self.status_var, style="DarkBold.TLabel").pack(side=tk.LEFT)

        # Zoom control
        ttk.Label(status_frame, text="  Zoom:", style="Dark.TLabel").pack(side=tk.LEFT, padx=(20, 4))
        self.zoom_var = tk.IntVar(value=self.scale)
        for s in self.SCALE_OPTIONS:
            rb = tk.Radiobutton(status_frame, text=f"{s}×", variable=self.zoom_var, value=s,
                                command=self._on_zoom_change,
                                bg="#1e1e2e", fg="#cdd6f4", selectcolor="#313244",
                                activebackground="#1e1e2e", activeforeground="#89b4fa",
                                font=("monospace", 9))
            rb.pack(side=tk.LEFT, padx=1)

        cw = self.img_w * self.scale
        ch = self.img_h * self.scale
        self.canvas = tk.Canvas(canvas_frame, width=cw, height=ch,
                                bg="#11111b", highlightthickness=1, highlightbackground="#45475a")
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.canvas.bind("<Motion>", self._on_hover)
        self.canvas.bind("<Button-1>", self._on_click)
        self.canvas.bind("<Button-3>", self._on_right_click)

        # Create the PhotoImage for the rendered pixels
        self.photo = tk.PhotoImage(width=self.img_w, height=self.img_h)
        self.canvas_img = self.canvas.create_image(0, 0, anchor=tk.NW, image=self.photo)

        # Crosshair
        self.crosshair_h = self.canvas.create_line(0, 0, 0, 0, fill="#f38ba8", dash=(2, 2), state=tk.HIDDEN)
        self.crosshair_v = self.canvas.create_line(0, 0, 0, 0, fill="#f38ba8", dash=(2, 2), state=tk.HIDDEN)

        # Right: info panel
        info_frame = ttk.Frame(main, style="Dark.TFrame", width=self.INFO_PANEL_W)
        info_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=4, pady=4)
        info_frame.pack_propagate(False)

        ttk.Label(info_frame, text="🔍 Pixel Inspector", style="Header.TLabel").pack(anchor=tk.W, pady=(4, 8))

        # ── Coordinate section ──
        coord_frame = ttk.LabelFrame(info_frame, text="Coordinates", style="Dark.TLabelframe")
        coord_frame.pack(fill=tk.X, pady=4, padx=2)
        self.coord_var = tk.StringVar(value="  —")
        ttk.Label(coord_frame, textvariable=self.coord_var, style="Dark.TLabel").pack(anchor=tk.W, padx=4, pady=2)

        # ── Color section ──
        color_frame = ttk.LabelFrame(info_frame, text="Color", style="Dark.TLabelframe")
        color_frame.pack(fill=tk.X, pady=4, padx=2)

        self.color_swatch = tk.Canvas(color_frame, width=60, height=30, bg="#000000",
                                       highlightthickness=1, highlightbackground="#585b70")
        self.color_swatch.pack(anchor=tk.W, padx=4, pady=4)
        self.color_var = tk.StringVar(value="  —")
        ttk.Label(color_frame, textvariable=self.color_var, style="Dark.TLabel").pack(anchor=tk.W, padx=4, pady=2)

        # ── Ray section ──
        ray_frame = ttk.LabelFrame(info_frame, text="Ray", style="Dark.TLabelframe")
        ray_frame.pack(fill=tk.X, pady=4, padx=2)
        self.ray_var = tk.StringVar(value="  No ray data")
        ttk.Label(ray_frame, textvariable=self.ray_var, style="Dark.TLabel",
                  wraplength=self.INFO_PANEL_W - 30, justify=tk.LEFT).pack(anchor=tk.W, padx=4, pady=2)

        # ── Hit section ──
        hit_frame = ttk.LabelFrame(info_frame, text="Hit", style="Dark.TLabelframe")
        hit_frame.pack(fill=tk.X, pady=4, padx=2)
        self.hit_var = tk.StringVar(value="  No hit data")
        ttk.Label(hit_frame, textvariable=self.hit_var, style="Dark.TLabel",
                  wraplength=self.INFO_PANEL_W - 30, justify=tk.LEFT).pack(anchor=tk.W, padx=4, pady=2)

        # ── Vector diagram canvas ──
        vec_frame = ttk.LabelFrame(info_frame, text="Direction Diagram", style="Dark.TLabelframe")
        vec_frame.pack(fill=tk.X, pady=4, padx=2)
        self.vec_canvas = tk.Canvas(vec_frame, width=self.INFO_PANEL_W - 20, height=150,
                                     bg="#11111b", highlightthickness=0)
        self.vec_canvas.pack(padx=4, pady=4)

        # ── Label section ──
        label_frame = ttk.LabelFrame(info_frame, text="Label", style="Dark.TLabelframe")
        label_frame.pack(fill=tk.X, pady=4, padx=2)
        self.label_var = tk.StringVar(value="  —")
        ttk.Label(label_frame, textvariable=self.label_var, style="Dark.TLabel",
                  wraplength=self.INFO_PANEL_W - 30, justify=tk.LEFT).pack(anchor=tk.W, padx=4, pady=2)

        # ── Stats section ──
        stats_frame = ttk.LabelFrame(info_frame, text="Stats", style="Dark.TLabelframe")
        stats_frame.pack(fill=tk.X, pady=4, padx=2)
        self.stats_var = tk.StringVar(value="  0 pixels")
        ttk.Label(stats_frame, textvariable=self.stats_var, style="Dark.TLabel").pack(anchor=tk.W, padx=4, pady=2)

        # Pin indicator
        self.pin_var = tk.StringVar(value="")
        ttk.Label(info_frame, textvariable=self.pin_var, style="DarkBold.TLabel").pack(anchor=tk.W, pady=(8, 0))

        # ── Start refresh loop ──
        self._schedule_refresh()

    def _on_zoom_change(self):
        self.scale = self.zoom_var.get()
        self._rebuild_canvas_image()

    def _rebuild_canvas_image(self):
        """Recreate the PhotoImage at current scale and redraw everything."""
        s = self.scale
        w, h = self.img_w, self.img_h
        self.photo = tk.PhotoImage(width=w * s, height=h * s)
        self.canvas.delete(self.canvas_img)
        self.canvas_img = self.canvas.create_image(0, 0, anchor=tk.NW, image=self.photo)
        self.canvas.configure(scrollregion=(0, 0, w * s, h * s))

        # Redraw all existing pixels
        with self.store.lock:
            for (x, y), (r, g, b) in self.store.pixels.items():
                self._draw_pixel_scaled(x, y, r, g, b)

        # Recreate crosshairs on top
        self.crosshair_h = self.canvas.create_line(0, 0, 0, 0, fill="#f38ba8", dash=(2, 2), state=tk.HIDDEN)
        self.crosshair_v = self.canvas.create_line(0, 0, 0, 0, fill="#f38ba8", dash=(2, 2), state=tk.HIDDEN)

    def _draw_pixel_scaled(self, x, y, r, g, b):
        """Put a scaled pixel onto the PhotoImage."""
        s = self.scale
        color_str = f"#{r:02x}{g:02x}{b:02x}"
        # PhotoImage.put with a rectangular region
        if s == 1:
            self.photo.put(color_str, (x, y))
        else:
            # Fill an s×s block using a row of identical color strings
            row = "{" + " ".join([color_str] * s) + "}"
            block = " ".join([row] * s)
            self.photo.put(block, (x * s, y * s))

    def _schedule_refresh(self):
        """Periodically pull dirty pixels and update UI."""
        # Check for init resize
        if self.init_event.is_set():
            self.init_event.clear()
            self.img_w = self.store.width
            self.img_h = self.store.height
            self._rebuild_canvas_image()
            self.status_var.set(f"Rendering {self.img_w}×{self.img_h} ...")

        # Drain dirty pixels
        batch = self.store.drain_dirty()
        for x, y, r, g, b in batch:
            self._draw_pixel_scaled(x, y, r, g, b)

        # Stats
        with self.store.lock:
            total = self.store.pixels_received
            done  = self.store.frame_done
        total_px = self.img_w * self.img_h if self.img_w > 0 else 1
        pct = min(100.0, 100.0 * total / total_px) if total_px > 0 else 0
        self.stats_var.set(f"  {total:,} / {total_px:,} pixels ({pct:.1f}%)")
        if done:
            self.status_var.set(f"✓ Frame complete  ({self.img_w}×{self.img_h})")

        self.root.after(16, self._schedule_refresh)   # ~60 FPS

    def _pixel_from_canvas(self, cx, cy):
        """Convert canvas coords to pixel coords."""
        s = self.scale
        px = int(cx) // s
        py = int(cy) // s
        if 0 <= px < self.img_w and 0 <= py < self.img_h:
            return px, py
        return None, None

    def _on_hover(self, event):
        if self.pinned is not None:
            return  # don't update on hover when pinned
        px, py = self._pixel_from_canvas(event.x, event.y)
        if px is not None:
            self._update_info(px, py)
            self._update_crosshair(event.x, event.y)

    def _on_click(self, event):
        px, py = self._pixel_from_canvas(event.x, event.y)
        if px is not None:
            self.pinned = (px, py)
            self.pin_var.set(f"📌 Pinned ({px}, {py}) — right-click to unpin")
            self._update_info(px, py)
            self._update_crosshair(event.x, event.y)

    def _on_right_click(self, event):
        self.pinned = None
        self.pin_var.set("")

    def _update_crosshair(self, cx, cy):
        s = self.scale
        cw = self.img_w * s
        ch = self.img_h * s
        self.canvas.coords(self.crosshair_h, 0, cy, cw, cy)
        self.canvas.coords(self.crosshair_v, cx, 0, cx, ch)
        self.canvas.itemconfigure(self.crosshair_h, state=tk.NORMAL)
        self.canvas.itemconfigure(self.crosshair_v, state=tk.NORMAL)

    def _update_info(self, px, py):
        info = self.store.get_info(px, py)
        self.coord_var.set(f"  x={px}  y={py}")

        # Color
        pix = info["pixel"]
        if pix:
            r, g, b = pix
            hex_c = f"#{r:02x}{g:02x}{b:02x}"
            self.color_swatch.configure(bg=hex_c)
            self.color_var.set(
                f"  RGB: ({r}, {g}, {b})\n"
                f"  Hex: {hex_c}\n"
                f"  Float: ({r/255:.3f}, {g/255:.3f}, {b/255:.3f})"
            )
        else:
            self.color_swatch.configure(bg="#000000")
            self.color_var.set("  Not yet rendered")

        # Ray
        ray = info["ray"]
        if ray:
            ox, oy, oz, dx, dy, dz = ray
            az, el, mag = vec_angles(dx, dy, dz)
            self.ray_var.set(
                f"  Origin:    ({ox:.3f}, {oy:.3f}, {oz:.3f})\n"
                f"  Direction: ({dx:.3f}, {dy:.3f}, {dz:.3f})\n"
                f"  |dir|:     {mag:.4f}\n"
                f"  Azimuth:   {az:.1f}°  (yaw)\n"
                f"  Elevation: {el:.1f}°  (pitch)"
            )
            self._draw_direction_diagram(dx, dy, dz)
        else:
            self.ray_var.set("  No ray data for this pixel")
            self.vec_canvas.delete("all")

        # Hit
        hit = info["hit"]
        if hit:
            t, nx, ny, nz, hx, hy, hz = hit
            n_az, n_el, _ = vec_angles(nx, ny, nz)
            self.hit_var.set(
                f"  t = {t:.4f}\n"
                f"  Hit point:  ({hx:.3f}, {hy:.3f}, {hz:.3f})\n"
                f"  Normal:     ({nx:.3f}, {ny:.3f}, {nz:.3f})\n"
                f"  Normal az:  {n_az:.1f}°   el: {n_el:.1f}°"
            )
        else:
            self.hit_var.set("  No hit (miss or no data)")

        # Label
        lbl = info["label"]
        self.label_var.set(f"  {lbl}" if lbl else "  —")

    def _draw_direction_diagram(self, dx, dy, dz):
        """Draw a top-down (XZ) and side (YZ) vector diagram."""
        c = self.vec_canvas
        c.delete("all")
        cw = int(c.cget("width"))
        ch = int(c.cget("height"))

        # Normalize
        mag = math.sqrt(dx*dx + dy*dy + dz*dz)
        if mag < 1e-12:
            return
        ndx, ndy, ndz = dx/mag, dy/mag, dz/mag

        # --- Top-down view (XZ plane) - left half ---
        half_w = cw // 2
        cx1, cy1 = half_w // 2, ch // 2
        radius = min(half_w, ch) // 2 - 15

        # Circle outline
        c.create_oval(cx1 - radius, cy1 - radius, cx1 + radius, cy1 + radius,
                       outline="#585b70", width=1)
        # Axis lines
        c.create_line(cx1 - radius, cy1, cx1 + radius, cy1, fill="#45475a", dash=(1, 3))
        c.create_line(cx1, cy1 - radius, cx1, cy1 + radius, fill="#45475a", dash=(1, 3))
        # Labels
        c.create_text(cx1, cy1 - radius - 8, text="-Z", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx1, cy1 + radius + 8, text="+Z", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx1 + radius + 8, cy1, text="+X", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx1 - radius - 8, cy1, text="-X", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx1, cy1 - radius - 18, text="Top (XZ)", fill="#a6adc8", font=("monospace", 8))

        # Project onto XZ plane
        xz_mag = math.sqrt(ndx*ndx + ndz*ndz)
        if xz_mag > 1e-6:
            arrow_x = cx1 + int(ndx / xz_mag * radius * 0.85)
            arrow_y = cy1 + int(ndz / xz_mag * radius * 0.85)  # +Z goes down on screen
        else:
            arrow_x, arrow_y = cx1, cy1
        c.create_line(cx1, cy1, arrow_x, arrow_y, fill="#89b4fa", width=2, arrow=tk.LAST, arrowshape=(8, 10, 4))
        # Dot at center
        c.create_oval(cx1 - 3, cy1 - 3, cx1 + 3, cy1 + 3, fill="#f5c2e7", outline="")

        # --- Side view (YZ plane) - right half ---
        cx2 = half_w + half_w // 2
        cy2 = ch // 2

        c.create_oval(cx2 - radius, cy2 - radius, cx2 + radius, cy2 + radius,
                       outline="#585b70", width=1)
        c.create_line(cx2 - radius, cy2, cx2 + radius, cy2, fill="#45475a", dash=(1, 3))
        c.create_line(cx2, cy2 - radius, cx2, cy2 + radius, fill="#45475a", dash=(1, 3))
        c.create_text(cx2, cy2 - radius - 8, text="+Y", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx2, cy2 + radius + 8, text="-Y", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx2 + radius + 8, cy2, text="-Z", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx2 - radius - 8, cy2, text="+Z", fill="#6c7086", font=("monospace", 7))
        c.create_text(cx2, cy2 - radius - 18, text="Side (YZ)", fill="#a6adc8", font=("monospace", 8))

        # Project onto YZ plane: X = -Z (forward goes right), Y = -Y (up goes up on screen)
        yz_mag = math.sqrt(ndy*ndy + ndz*ndz)
        if yz_mag > 1e-6:
            arrow_x2 = cx2 + int(-ndz / yz_mag * radius * 0.85)
            arrow_y2 = cy2 + int(-ndy / yz_mag * radius * 0.85)
        else:
            arrow_x2, arrow_y2 = cx2, cy2
        c.create_line(cx2, cy2, arrow_x2, arrow_y2, fill="#a6e3a1", width=2, arrow=tk.LAST, arrowshape=(8, 10, 4))
        c.create_oval(cx2 - 3, cy2 - 3, cx2 + 3, cy2 + 3, fill="#f5c2e7", outline="")

    def _on_close(self):
        self.receiver.stop()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


# ── Entry point ───────────────────────────────────────────────────────────
if __name__ == "__main__":
    app = ViewerApp()
    app.run()
