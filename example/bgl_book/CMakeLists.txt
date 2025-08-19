# example/bgl_book/CMakeLists.txt

add_compile_definitions(BGL_DATA_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_compile_definitions(BGL_OUTPUT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output/")

add_executable(ch3_toposort "ch3_toposort.cpp")
