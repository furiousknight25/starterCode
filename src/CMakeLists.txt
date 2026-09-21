add_library (cs4212-util
  ArgumentParsing.cpp ArgumentParsing.h
  handleGraphicsArgs.cpp handleGraphicsArgs.h
  model_obj.cpp model_obj.h
  Framebuffer.cpp Framebuffer.h
)
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_include_directories(cs4212-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cs4212-util PRIVATE Boost::program_options)
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG ZLIB::ZLIB)

