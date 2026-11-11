
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


## BVH (Bounding Volume Hierarchy)

cmake --build buildVCPkg --target utest_bvh --- builds BVH unit tests
./buildVCPkg/utests/utest_bvh --- runs the tests

cmake --build buildVCPkg --target BVHBenchmark --- builds the benchmark tool
./buildVCPkg/examples/BVHBenchmark examples/scenes/scenes_A/spheres_1K.json -w 500 -h 500 --- runs comparison benchmark

Render with BVH (default):
./buildVCPkg/examples/JsonLoaderScene examples/scenes/scenes_A/spheres_1K.json -w 500 -h 500 -o output_bvh.png

Render without BVH:
./buildVCPkg/examples/JsonLoaderScene examples/scenes/scenes_A/spheres_1K.json -w 500 -h 500 -no-bvh -o output_linear.png

### Timings

spheres_1K.json (1000 spheres, 7 lights):
- 250x250: No BVH: 6.7s | BVH: 0.65s (~10x speedup)
- 500x500: No BVH: 30.0s | BVH: 2.98s (~10x speedup)

boxSphereTest.json (368 shapes, 2 lights):
- 250x250: No BVH: 0.96s | BVH: 0.06s (~16x speedup)
- 500x500 (4 aa): No BVH: 18.9s | BVH: 1.4s (~13.5x speedup)

effingerSpheres.json (7 spheres, 1 light):
- 250x250: No BVH: 0.09s | BVH: 0.09s (~1x speedup)




