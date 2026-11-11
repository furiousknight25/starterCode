
#LOADING STUFF<br>
-w (int) width<br>
-h (int) height<br>
-o (string) output file name<br>
-aa (int) anti aliasing<br>
./buildVCPkg/examples/JsonLoaderScene examples/scenes/scenes_A/threeTriangles.json -w 300 -h 200 -o output.png


## vectest(testing the raycast vectors from the camera)

cmake --build buildVCPkg --target vectest --- builds the tools to run the tests
./buildVCPkg/examples/vectest

## Sphere / Ray-Sphere Intersection Unit Tests

cmake --build buildVCPkg --target utest_sphere --- builds the unit tests for Sphere and ray-sphere intersections
./buildVCPkg/utests/utest_sphere -s --- runs the sphere tests and outputs all passing assertions

## Sphere Render (Ray-Sphere Flag Generation)

cmake --build buildVCPkg --target SphereRender --- builds the sphere renderer
./buildVCPkg/examples/SphereRender --- renders Bangladeshi flag to output.png 

## Triangle & Multi-Shape Occlusion Unit Tests

cmake --build buildVCPkg --target utest_triangle --- builds the unit tests for Triangle intersection
./buildVCPkg/utests/utest_triangle -s --- runs triangle unit tests

cmake --build buildVCPkg --target utest_multishape --- builds the unit tests for multiple objects and depth occlusion
./buildVCPkg/utests/utest_multishape -s --- runs multi-shape occlusion unit tests

## Scene Render (Weekend Homework - Triangles, Multiple Shapes & Antialiasing at 200x200)

cmake --build buildVCPkg --target SceneRender --- builds the multi-shape scene renderer
./buildVCPkg/examples/SceneRender --- renders 200x200 images:
  - scene_multishape_200x200.png (normal visualization on spheres, flat colors on triangles)
  - triangle_render_200x200.png (flat-colored standalone triangle)
  - scene_aliased_1spp_200x200.png (1 sample per pixel comparison)

## Shader Render (Lab - Lambertian and Blinn-Phong)

cmake --build buildVCPkg --target ShaderRender --- builds the shader renderer
./buildVCPkg/examples/ShaderRender --- renders lambertian and blinn-phong spheres


## Bounding Volume Hierarchy (BVH) Extension - Bronze Level

A decoupled, high-performance spatial acceleration structure implementing Axis-Aligned Bounding Boxes (AABB) and a recursive binary Bounding Volume Hierarchy (BVH). Accelerates primary ray and shadow ray queries from $O(N)$ linear scans to $O(\log N)$ tree traversals.

### Architecture & Decoupling
The BVH implementation is cleanly isolated in dedicated files:
- `src/shapes/AABB.h`: Standalone AABB class with slab ray intersection and box union.
- `src/shapes/BVHNode.h`: Self-contained `BVHNode` class implementing the `Shape` interface.
- `src/shapes/ShapeList.h`: Optional `bvh_root` pointer. If null, automatically falls back to standard linear search.
- `src/SceneLoader/SceneContainer.h`: `use_bvh` boolean toggle.

To rewrite or delete the BVH code, simply replace or clear `src/shapes/BVHNode.h` without breaking the core renderer.

### Building & Running Unit Tests
```bash
cmake --build buildVCPkg --target utest_bvh
./buildVCPkg/utests/utest_bvh
```
Runs 5 test suites covering AABB construction, slab intersection (hits, misses, parallel rays), shape bounding boxes, and full equivalence between BVH and linear search across 2,900+ ray tests.

### Running the BVH Benchmark
```bash
cmake --build buildVCPkg --target BVHBenchmark
./buildVCPkg/examples/BVHBenchmark examples/scenes/scenes_A/spheres_1K.json -w 250 -h 250
```

### Rendering with / without BVH via JsonLoaderScene
```bash
# Render with BVH (default)
./buildVCPkg/examples/JsonLoaderScene examples/scenes/scenes_A/spheres_1K.json -w 500 -h 500 -o output_bvh.png

# Render without BVH (linear search comparison)
./buildVCPkg/examples/JsonLoaderScene examples/scenes/scenes_A/spheres_1K.json -w 500 -h 500 -no-bvh -o output_linear.png
```

### Benchmark Timings & Speedup Results

| Scene                  | Resolution      | Shape Count   | Light Count    | Linear Time (No BVH) | BVH Time           | Tree Build | Speedup    | Exact Match       |
| ---------------------- | --------------- | ------------- | -------------- | -------------------- | ------------------ | ---------- | ---------- | ----------------- |
| `spheres_1K.json`      | 250x250 (1 spp) | 1,000 spheres | 7 point lights | 6,728.0 ms (6.7 s)   | 657.6 ms (0.7 s)   | 11.8 ms    | **10.23x** | 62,500 / 62,500   |
| `spheres_1K.json`      | 500x500 (1 spp) | 1,000 spheres | 7 point lights | 30,005.8 ms (30.0 s) | 2,981.7 ms (3.0 s) | 13.2 ms    | **10.06x** | 250,000 / 250,000 |
| `boxSphereTest.json`   | 250x250 (1 spp) | 368 shapes    | 2 point lights | 959.6 ms (1.0 s)     | 60.0 ms (0.1 s)    | 3.5 ms     | **16.00x** | 62,500 / 62,500   |
| `boxSphereTest.json`   | 500x500 (4 spp) | 368 shapes    | 2 point lights | 18,892.1 ms (18.9 s) | 1,389.7 ms (1.4 s) | 4.0 ms     | **13.59x** | 4 rays/pixel (AA) |
| `effingerSpheres.json` | 250x250 (1 spp) | 7 spheres     | 1 point light  | 95.8 ms (0.1 s)      | 94.1 ms (0.1 s)    | 0.0 ms     | **1.02x**  | 62,500 / 62,500   |



