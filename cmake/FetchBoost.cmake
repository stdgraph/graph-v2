include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: boost")

FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG        boost-1.80.0
)

FetchContent_GetProperties(boost)
if(NOT boost_POPULATED)
  FetchContent_Populate(
    boost
  )
endif()

set(BOOST_SOURCE_DIR "${boost_SOURCE_DIR}")
set(BOOST_INCLUDE_DIR "${boost_INCLUDE_DIR}")

FetchContent_MakeAvailable(boost)