# Simple benmarking for the graph library, and more specifically the dijkstra variants

add_executable(graph_bench graph_bench.cpp "mm_simple.cpp" "mm_load_example.cpp")
target_link_libraries(graph_bench fast_matrix_market::fast_matrix_market)

add_executable(sort_matrix_market "sort_matrix_market.cpp")
target_link_libraries(sort_matrix_market fast_matrix_market::fast_matrix_market)
