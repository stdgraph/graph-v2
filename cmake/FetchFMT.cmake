include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: fmt")

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.2.1
)
FetchContent_MakeAvailable(fmt)
set(FMT_SOURCE_DIR "${fmt_SOURCE_DIR}")
set(FMT_INCLUDE_DIR "${fmt_INCLUDE_DIR}")


