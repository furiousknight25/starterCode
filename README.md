Commands to build and run the unit tests for vec3's 

cmake --build buildVCPkg --target utest_vec3 --- builds the tools to run the tests

 ./buildVCPkg/utests/utest_vec3 -s --- this runs the tests, I believe -s looks for the successes

## vectest(testing the raycast vectors from the camera)

cmake --build buildVCPkg --target vectest --- builds the tools to run the tests
./buildVCPkg/examples/vectest