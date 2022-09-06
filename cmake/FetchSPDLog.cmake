include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: spdlog")
get_filename_component(FC_BASE "../externals"
                REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${FC_BASE})

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.9.2
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(
    spdlog
  )
endif()

set(SPDLOG_SOURCE_DIR "${spdlog_SOURCE_DIR}")
set(SPDLOG_INCLUDE_DIR "${spdlog_INCLUDE_DIR}")

FetchContent_MakeAvailable(spdlog)