# 
# Adds the src directory to the include search
# path. ${CMAKE_SOURCE_DIR} is prefixed to allow the files in the
# utest directory to correctly include the relative reference to the
# src files.
#
include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(
  vectest
  vectest.cpp
  )
target_include_directories(vectest PRIVATE
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(vectest PRIVATE
	cs4212-util
)

add_executable(
  SphereRender
  SphereRender.cpp
  )
target_include_directories(SphereRender PRIVATE
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(SphereRender PRIVATE
	cs4212-util
	PNG::PNG
	ZLIB::ZLIB
)

add_executable(
  SceneRender
  SceneRender.cpp
  )
target_include_directories(SceneRender PRIVATE
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(SceneRender PRIVATE
	cs4212-util
	PNG::PNG
	ZLIB::ZLIB
)

# ShaderRender (uncomment when filled out)
# add_executable(
#   ShaderRender
#   ShaderRender.cpp
# )
# target_include_directories(ShaderRender PRIVATE
# 	${CMAKE_SOURCE_DIR}/src
# )
# target_link_libraries(ShaderRender PRIVATE
# 	cs4212-util
# 	PNG::PNG
# 	ZLIB::ZLIB
# )

add_executable(
  ScratchRender
  ScratchRender.cpp
  )
target_include_directories(ScratchRender PRIVATE
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(ScratchRender PRIVATE
	cs4212-util
	PNG::PNG
	ZLIB::ZLIB
)

add_executable(
  JsonLoaderScene
  JsonLoaderScene.cpp
  ${CMAKE_SOURCE_DIR}/src/SceneLoader/SceneParser_JSON.cpp
  )
target_include_directories(JsonLoaderScene PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/buildVCPkg/vcpkg_installed/x64-linux/include
)
target_link_libraries(JsonLoaderScene PRIVATE
	cs4212-util
	PNG::PNG
	ZLIB::ZLIB
)
# ── Debug Visualizer: automatically link to all example targets ──
if(DEBUG_VIS)
  # Demo program that showcases the debug visualizer
  add_executable(debugvis_demo debugvis_demo.cpp)
  target_include_directories(debugvis_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(debugvis_demo PRIVATE cs4212-util PNG::PNG ZLIB::ZLIB)

  foreach(tgt vectest SphereRender SceneRender ShaderRender ScratchRender JsonLoaderScene debugvis_demo)
    if(TARGET ${tgt})
      target_link_libraries(${tgt} PRIVATE debugvis)
    endif()
  endforeach()
endif()