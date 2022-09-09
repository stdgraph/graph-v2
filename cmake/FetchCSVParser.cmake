include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: csv-parser")
get_filename_component(FC_BASE "${PROJECT_SOURCE_DIR}/externals"
                REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${FC_BASE})

FetchContent_Declare(
  csv_parser
    GIT_REPOSITORY https://github.com/pratzl/csv-parser.git
    GIT_TAG        2.1.3
)

FetchContent_GetProperties(csv_parser)
if(NOT csv_parser_POPULATED)
  FetchContent_Populate(
    csv_parser
  )
endif()
set(CSVPARSER_INCLUDE_DIR "${csv_parser_SOURCE_DIR}/single_include")

FetchContent_MakeAvailable(csv_parser)