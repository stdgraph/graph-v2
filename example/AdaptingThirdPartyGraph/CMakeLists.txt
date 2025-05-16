# example/AdaptingThirdPartyGraph/CMakeLists.txt

set(EXAMPLE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/output/")
add_compile_definitions(EXAMPLE_OUTPUT_DIR="${EXAMPLE_OUTPUT_DIR}")

#add_library(catch_main STATIC catch_main.cpp)
#target_link_libraries(catch_main PUBLIC Catch2::Catch2)
#target_link_libraries(catch_main PRIVATE project_options)
#target_link_options(catch_main INTERFACE $<$<CXX_COMPILER_ID:GNU>:-pthread -fconcepts-diagnostics-depth=1>)

add_executable(AdaptingThirdPartyGraph  "adapting_a_third_party_graph.cpp")
target_link_libraries(AdaptingThirdPartyGraph PRIVATE project_warnings project_options catch_main Catch2::Catch2WithMain graph)
