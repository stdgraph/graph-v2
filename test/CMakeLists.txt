add_compile_definitions(TEST_DATA_ROOT_DIR="${TEST_DATA_ROOT_DIR}")
add_compile_definitions(TEST_OUTPUT_ROOT_DIR="${TEST_EXAMPLE_ROOT_DIR}/output/")


add_library(catch_main STATIC catch_main.cpp "csv_routes.hpp")
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE project_options)
target_link_options(catch_main INTERFACE $<$<CXX_COMPILER_ID:GNU>:-pthread -fconcepts-diagnostics-depth=1>)
target_include_directories(catch_main
    INTERFACE ${CSVPARSER_INCLUDE_DIR}
)

add_executable(tests tests.cpp "examples.cpp" 
                               "csv_routes_vofl_tests.cpp" "csv_routes.hpp"  "csv_routes.cpp" "csv_routes_dov_tests.cpp" "csv_routes_csr_tests.cpp" 
                               "vertexlist_tests.cpp" "incidence_tests.cpp"  "neighbors_tests.cpp"  "edgelist_tests.cpp" 
                               "shortest_paths_tests.cpp" "transitive_closure_tests.cpp" "dfs_tests.cpp" "bfs_tests.cpp" "mis_tests.cpp"
                               )

target_link_libraries(tests PRIVATE project_warnings project_options catch_main Catch2::Catch2 graph)
target_link_libraries(tests PRIVATE fmt::fmt)
target_include_directories(tests 
    INTERFACE ${Catch2_INCLUDE_DIRS_DEBUG} 
    INTERFACE ${CSVPARSER_INCLUDE_DIR}
)

# automatically discover tests that are defined in catch based test files you
# can modify the unittests. TEST_PREFIX to whatever you want, or use different
# for different binaries. See 
# https://github.com/catchorg/Catch2/commit/3d7104c124e3af88eda573f867f79d0aa9ef6c72
# for more info about catch_discover_tests.
#catch_discover_tests(
#  tests
#  TEST_PREFIX   "unittests."
#  REPORTER      xml
#  OUTPUT_DIR    .
#  OUTPUT_PREFIX "unittests."
#  OUTPUT_SUFFIX .xml)

# Add a file containing a set of constexpr tests
#add_executable(constexpr_tests constexpr_tests.cpp "csv_routes.hpp"  "csv_routes.cpp")
#target_link_libraries(constexpr_tests PRIVATE project_options project_warnings graph
#                                              catch_main)
#target_link_libraries(constexpr_tests PRIVATE csv_parser)

#catch_discover_tests(
#  constexpr_tests
#  TEST_PREFIX   "constexpr."
#  REPORTER      xml
#  OUTPUT_DIR    .
#  OUTPUT_PREFIX "constexpr."
#  OUTPUT_SUFFIX .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
#add_executable(relaxed_constexpr_tests constexpr_tests.cpp "csv_routes.hpp"  "csv_routes.cpp")
#target_link_libraries(relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main graph)
#target_link_libraries(relaxed_constexpr_tests PRIVATE csv_parser)
#target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

#catch_discover_tests(
#  relaxed_constexpr_tests
#  TEST_PREFIX   "relaxed_constexpr."
#  REPORTER      xml
#  OUTPUT_DIR    .
#  OUTPUT_PREFIX "relaxed_constexpr."
#  OUTPUT_SUFFIX .xml)
