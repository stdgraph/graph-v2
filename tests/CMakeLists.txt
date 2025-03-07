option(ENABLE_COVERAGE "Enable coverage build." OFF)

if(ENABLE_COVERAGE)
  # set compiler flags
  set(CMAKE_CXX_FLAGS "-O0 -coverage")

  # find required tools
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)

  # add coverage target
  add_custom_target(coverage
    # gather data
    COMMAND ${LCOV} --directory . --capture --output-file coverage.txt
    # generate report
    COMMAND ${GENHTML} --demangle-cpp -o coverage coverage.txt
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_compile_definitions(TEST_DATA_ROOT_DIR="${TEST_DATA_ROOT_DIR}")
add_compile_definitions(TEST_OUTPUT_ROOT_DIR="${TEST_EXAMPLE_ROOT_DIR}/output/")


add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE project_options)
target_link_options(catch_main INTERFACE $<$<CXX_COMPILER_ID:GNU>:-pthread -fconcepts-diagnostics-depth=2>)
target_include_directories(catch_main
    INTERFACE ${CSVPARSER_INCLUDE_DIR}
)

# begin /* Add unit tests */
set(UNITTEST_SOURCES
    ### Examples
    #"examples.cpp" 
    #"examples_tests.cpp"

    ### Basic tests
    "descriptor_tests.cpp"
    "tests.cpp"

    ### Views
    #"vertexlist_tests.cpp" 
    #"incidence_tests.cpp"  
    #"neighbors_tests.cpp"
    #"edgelist_view_tests.cpp" 
    #"dfs_tests.cpp" 
    #"bfs_tests.cpp"

    ### Algorithms
    #"mis_tests.cpp" 
    #"mst_tests.cpp"
    #"tc_tests.cpp"
    #"cc_tests.cpp"

    #"dijkstra_shortest_paths_tests.cpp" 
    #"bellman_shortest_paths_tests.cpp" 

    #"breadth_first_search_tests.cpp"
    #"depth_first_search_tests.cpp"
    #"topological_sort_tests.cpp"

    #"transitive_closure_tests.cpp" # removed from P1709 proposal

    ### Coroutine & Visitor Experiments
    #   "co_bfs_tests.cpp"
    #   "co_dijkstra_tests.cpp"
    #   "visitor_dijkstra_tests.cpp"

    ### Other
    "csv_routes.cpp" 

    ### Adjacency List Containers
    "csv_routes_vofl_tests.cpp" 
    #"csv_routes_dov_tests.cpp" 
    #"csv_routes_csr_tests.cpp" 

    ### Edge List Containers
    #"edgelist_tests.cpp" 
)




foreach(SOURCE IN LISTS UNITTEST_SOURCES)
	get_filename_component(TEST_NAME ${SOURCE} NAME_WLE)
	add_executable(${TEST_NAME} ${SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE project_warnings project_options catch_main Catch2::Catch2WithMain graph)
	target_include_directories(${TEST_NAME} 
				INTERFACE ${Catch2_INCLUDE_DIRS_DEBUG} 
				INTERFACE ${CSVPARSER_INCLUDE_DIR}
	)
	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
# end /* Add unit tests */
