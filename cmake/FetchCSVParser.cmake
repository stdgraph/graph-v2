include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: csv-parser")

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