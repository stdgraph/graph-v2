include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: fmt")
get_filename_component(FC_BASE "${PROJECT_SOURCE_DIR}/externals"
                REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${FC_BASE})

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        8.1.1
)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
  FetchContent_Populate(
    fmt
  )
endif()

set(FMT_SOURCE_DIR "${fmt_SOURCE_DIR}")
set(FMT_INCLUDE_DIR "${fmt_INCLUDE_DIR}")

FetchContent_MakeAvailable(fmt)
add_subdirectory(${fmt_SOURCE_DIR})