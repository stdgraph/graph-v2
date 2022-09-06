include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: catch2")
get_filename_component(FC_BASE "../externals"
                REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${FC_BASE})

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.8
)

FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
  FetchContent_Populate(
    catch2
  )
endif()

set(CATCH2_SOURCE_DIR "${catch2_SOURCE_DIR}")
set(CATCH2_INCLUDE_DIR "${catch2_INCLUDE_DIR}")

FetchContent_MakeAvailable(catch2)
add_subdirectory(${catch2_SOURCE_DIR})