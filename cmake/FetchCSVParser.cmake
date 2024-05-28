include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: csv-parser")

FetchContent_Declare(
  csv_parser
    GIT_REPOSITORY https://github.com/pratzl/csv-parser.git
    GIT_TAG        2.1.3
)
FetchContent_MakeAvailable(csv_parser)

set(CSVPARSER_INCLUDE_DIR "${csv_parser_SOURCE_DIR}/single_include")

