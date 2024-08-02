# Simple benmarking for the graph library, and more specifically the dijkstra variants

# C++23 needed for zip view
set (CMAKE_CXX_STANDARD 23)

add_library(common_bench INTERFACE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(common_bench INTERFACE -Wno-sign-conversion)
endif()
target_link_libraries(common_bench INTERFACE project_warnings project_options fmt::fmt)

add_executable(graph_bench graph_bench.cpp "mm_simple.cpp" "mm_load_example.cpp" "mm_bench_dijkstra.cpp" "timer.cpp" "mm_files.cpp")
target_link_libraries(graph_bench PRIVATE  common_bench graph fast_matrix_market::fast_matrix_market)
target_compile_definitions(graph_bench PRIVATE BENCHMARK_DATA_DIR="${BENCHMARK_DATA_DIR}")
#if (MSVC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /PROFILE")
#    #target_link_options(graph_bench PRIVATE -pthread)
#endif()

# An executable to sort a matrix market file
add_executable(sort_matrix_market "sort_matrix_market.cpp" "timer.cpp")
target_link_libraries(sort_matrix_market PRIVATE common_bench fast_matrix_market::fast_matrix_market)
