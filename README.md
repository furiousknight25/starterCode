
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


