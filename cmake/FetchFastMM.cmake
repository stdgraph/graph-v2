include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: fast_matrix_market")

FetchContent_Declare(
        fast_matrix_market
        GIT_REPOSITORY https://github.com/alugowski/fast_matrix_market
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fast_matrix_market)
#set(FMT_SOURCE_DIR "${fmt_SOURCE_DIR}")
#set(FMT_INCLUDE_DIR "${fmt_INCLUDE_DIR}")

#target_link_libraries(YOUR_TARGET fast_matrix_market::fast_matrix_market)
