# Examples

# Build errors with MSVC
set(EXAMPLE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/output/")

add_library(cppcon21_headers INTERFACE)
target_include_directories(cppcon21_headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/graphs/" )
target_include_directories(cppcon21_headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_subdirectory(examples)
