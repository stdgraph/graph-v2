# example/CppCon2022/CMakeLists.txt

add_executable(graphs  "graphs.cpp")
target_link_libraries(graphs PRIVATE cppcon21_headers project_warnings project_options graph)

add_executable(bacon  "bacon.cpp")
target_link_libraries(bacon PRIVATE cppcon21_headers project_warnings project_options graph)

add_executable(ospf "ospf.cpp")
target_link_libraries(ospf PRIVATE cppcon21_headers project_warnings project_options graph fmt::fmt)

add_executable(imdb "imdb.cpp")
target_link_libraries(imdb PRIVATE cppcon21_headers project_warnings project_options graph)
