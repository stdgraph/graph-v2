include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: range-v3")

FetchContent_Declare(
    range
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.11.0
)

FetchContent_GetProperties(range)
if(NOT range_POPULATED)
  FetchContent_Populate(
    range
  )
endif()

set(RANGE_SOURCE_DIR "${range_SOURCE_DIR}")
set(RANGE_INCLUDE_DIR "${range_INCLUDE_DIR}")

FetchContent_MakeAvailable(range)