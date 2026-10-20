#!/usr/bin/env bash
# ============================================================================
# debugvis.sh — Launch the debug viewer + your raytracer together
#
# USAGE:
#   ./tools/debugvis/debugvis.sh ./buildVCPkg/examples/vectest
#   ./tools/debugvis/debugvis.sh ./buildVCPkg/examples/SphereRender
#
# The viewer opens first, waits for a connection, then your program runs.
# When your program finishes, the viewer stays open so you can inspect.
# Close the viewer window to exit.
# ============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VIEWER="$SCRIPT_DIR/viewer.py"
SOCK_PATH="${DEBUGVIS_SOCK:-/tmp/debugvis.sock}"

# Clean up stale socket
rm -f "$SOCK_PATH"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <your-program> [args...]"
    echo ""
    echo "Examples:"
    echo "  $0 ./buildVCPkg/examples/vectest"
    echo "  $0 ./buildVCPkg/examples/SphereRender"
    echo ""
    echo "Make sure you built with:  cmake -DDEBUG_VIS=ON .."
    exit 1
fi

echo "🔬 Starting debug viewer..."
python3 "$VIEWER" &
VIEWER_PID=$!

# Give the viewer a moment to start listening
sleep 0.5

echo "🚀 Running: $*"
"$@" || true

echo "✓ Program finished. Viewer is still open — inspect your pixels!"
echo "  Close the viewer window to exit."

# Wait for viewer to close
wait $VIEWER_PID 2>/dev/null || true
