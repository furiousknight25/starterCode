add_library (cs4212-util
  model_obj.cpp model_obj.h
  Framebuffer.cpp Framebuffer.h
  shaders/Shader.h
  shapes/shape.h
  shapes/hit_record.h
  shapes/Sphere.h
  shapes/ShapeList.h
  math/interval.h
)
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_include_directories(cs4212-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG ZLIB::ZLIB)

