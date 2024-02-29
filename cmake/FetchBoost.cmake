include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: boost")

# See https://github.com/boostorg/cmake for documentation
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES cobalt) # Identify the libraries we're using to minimize build time
set(BOOST_ENABLE_MPI OFF)
set(BOOST_ENABLE_PYTHON OFF)
set(BUILD_SHARED_LIBS OFF) # We want static libraries

FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG        boost-1.84.0
)

FetchContent_GetProperties(boost)
if(NOT boost_POPULATED)
    FetchContent_Populate(boost)
    add_subdirectory(
        ${boost_SOURCE_DIR}
        ${boost_BINARY_DIR}
        EXCLUDE_FROM_ALL
    )
endif()

set(BOOST_SOURCE_DIR "${boost_SOURCE_DIR}")
# Defining BOOST_INCLUDE_DIR doesn't make sense because library in boost has its own include directory

FetchContent_MakeAvailable(boost)
