# Simple benmarking for the graph library, and more specifically the dijkstra variants

# C++23 needed for zip view
set (CMAKE_CXX_STANDARD 23)

add_executable(graph_bench graph_bench.cpp "mm_simple.cpp" "mm_load_example.cpp" "mm_bench_dijkstra.cpp" "timer.cpp" "mm_files.cpp")
target_link_libraries(graph_bench PRIVATE project_warnings project_options graph fmt::fmt fast_matrix_market::fast_matrix_market)
target_compile_definitions(graph_bench PRIVATE BENCHMARK_DATA_DIR="${BENCHMARK_DATA_DIR}")

# An executable to sort a matrix market file
add_executable(sort_matrix_market "sort_matrix_market.cpp" "timer.cpp")
target_link_libraries(sort_matrix_market PRIVATE fast_matrix_market::fast_matrix_market fmt::fmt)
