cmake_minimum_required(VERSION 3.10)

project(ddbs LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Release)

set(SQLParser_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/sql-parser/libsqlparser.so)

add_custom_target(build_SQLParser ALL
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sql-parser/
    COMMENT "Original sql-parser makefile target."
)

add_library(SQLParser SHARED IMPORTED)

set_property(TARGET SQLParser APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(SQLParser PROPERTIES IMPORTED_LOCATION_NOCONFIG "${SQLParser_LIBRARY}")

add_dependencies(SQLParser build_SQLParser)

file(GLOB sources src/*.cpp src/*.hpp)

add_executable(main 
    ${sources}
)
target_include_directories(main PUBLIC sql-parser/src)
target_link_libraries(main PUBLIC SQLParser)
