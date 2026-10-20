# ============================================================================
# Debug Visualizer — CMake integration
#
# This creates a static library "debugvis" that you can link against.
# It's controlled by the DEBUG_VIS CMake option (OFF by default).
#
# When ON:
#   - Adds a compile definition DEBUGVIS_ENABLED to your targets
#   - Provides the DebugFramebuffer wrapper
#
# When OFF:
#   - Nothing happens.  Zero overhead.  No code changes needed.
# ============================================================================

# Header-only library (no .cpp files needed — everything is in the headers)
add_library(debugvis INTERFACE)

target_include_directories(debugvis INTERFACE
    ${CMAKE_SOURCE_DIR}/tools/debugvis
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(debugvis INTERFACE DEBUGVIS_ENABLED)

# Print a message so you know it's active
message(STATUS "🔬 Debug Visualizer ENABLED — link with 'debugvis' and run viewer.py")
