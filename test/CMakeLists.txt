# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_compile_definitions(TEST_DATA_ROOT_DIR="${TEST_DATA_ROOT_DIR}")


add_library(catch_main STATIC catch_main.cpp "csv_routes.hpp")
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(catch_main PRIVATE project_options)
target_link_options(catch_main INTERFACE $<$<CXX_COMPILER_ID:GNU>:-pthread>)

add_executable(tests tests.cpp "csv_routes_vofl_tests.cpp" "csv_routes.hpp"  "csv_routes.cpp"   "csv_routes_dov_tests.cpp" "examples.cpp" "csv_routes_csr_tests.cpp" "vertexlist_tests.cpp" "incidence_tests.cpp")
target_link_libraries(tests PRIVATE project_warnings project_options catch_main graph)
target_link_libraries(tests PRIVATE CONAN_PKG::fmt)
target_link_libraries(tests PRIVATE csv_parser)
#target_link_libraries(tests PRIVATE graph)
#target_link_libraries(tests PRIVATE graph_adaptor)

# automatically discover tests that are defined in catch based test files you
# can modify the unittests. TEST_PREFIX to whatever you want, or use different
# for different binaries. See 
# https://github.com/catchorg/Catch2/commit/3d7104c124e3af88eda573f867f79d0aa9ef6c72
# for more info about catch_discover_tests.
catch_discover_tests(
  tests
  TEST_PREFIX   "unittests."
  REPORTER      xml
  OUTPUT_DIR    .
  OUTPUT_PREFIX "unittests."
  OUTPUT_SUFFIX .xml)

# Add a file containing a set of constexpr tests
add_executable(constexpr_tests constexpr_tests.cpp "csv_routes.hpp"  "csv_routes.cpp")
target_link_libraries(constexpr_tests PRIVATE project_options project_warnings graph
                                              catch_main)
target_link_libraries(constexpr_tests PRIVATE csv_parser)

catch_discover_tests(
  constexpr_tests
  TEST_PREFIX   "constexpr."
  REPORTER      xml
  OUTPUT_DIR    .
  OUTPUT_PREFIX "constexpr."
  OUTPUT_SUFFIX .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
add_executable(relaxed_constexpr_tests constexpr_tests.cpp "csv_routes.hpp"  "csv_routes.cpp")
target_link_libraries(relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main graph)
target_link_libraries(relaxed_constexpr_tests PRIVATE csv_parser)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

catch_discover_tests(
  relaxed_constexpr_tests
  TEST_PREFIX   "relaxed_constexpr."
  REPORTER      xml
  OUTPUT_DIR    .
  OUTPUT_PREFIX "relaxed_constexpr."
  OUTPUT_SUFFIX .xml)
