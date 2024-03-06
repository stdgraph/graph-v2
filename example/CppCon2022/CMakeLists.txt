# example/CppCon2022/CMakeLists.txt

set(EXAMPLE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/output/")
add_compile_definitions(EXAMPLE_OUTPUT_DIR="${EXAMPLE_OUTPUT_DIR}")

#add_library(catch_main STATIC catch_main.cpp)
#target_link_libraries(catch_main PUBLIC Catch2::Catch2)
#target_link_libraries(catch_main PRIVATE project_options)
#target_link_options(catch_main INTERFACE $<$<CXX_COMPILER_ID:GNU>:-pthread -fconcepts-diagnostics-depth=1>)

add_executable(CppCon2022  "rr_adaptor.hpp" "graphviz_output.hpp" "germany_routes_example.cpp")
target_link_libraries(CppCon2022 PRIVATE project_warnings project_options catch_main Catch2::Catch2WithMain graph)
