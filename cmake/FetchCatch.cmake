include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: catch2")

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(catch2)
set(CATCH2_SOURCE_DIR "${catch2_SOURCE_DIR}")
set(CATCH2_INCLUDE_DIR "${catch2_INCLUDE_DIR}")

