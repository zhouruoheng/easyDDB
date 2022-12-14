cmake_minimum_required(VERSION 3.10)

project(ddbs LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ??
# set (CMAKE_LIBRARY_PATH /usr/lib/x86_64-linux-gnu)

# ------------------------------ SET BRPC --------------------------------------------

option(LINK_SO "Whether examples are linked dynamically" OFF)

execute_process(
    COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/brpc -type d -regex \".*output/include$\" | head -n1 | xargs dirname | tr -d '\n'"
    OUTPUT_VARIABLE OUTPUT_PATH
)

set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})

include(FindThreads)
include(FindProtobuf)
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER src/db.proto)
# include PROTO_HEADER
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Search for libthrift* by best effort. If it is not found and brpc is
# compiled with thrift protocol enabled, a link error would be reported.
find_library(THRIFT_LIB NAMES thrift)
if (NOT THRIFT_LIB)
    set(THRIFT_LIB "")
endif()
find_library(THRIFTNB_LIB NAMES thriftnb)
if (NOT THRIFTNB_LIB)
    set(THRIFTNB_LIB "")
endif()

find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
if(LINK_SO)
    find_library(BRPC_LIB NAMES brpc)
else()
    find_library(BRPC_LIB NAMES libbrpc.a brpc)
endif()
if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
    message(FATAL_ERROR "Fail to find brpc")
endif()
include_directories(${BRPC_INCLUDE_PATH})

find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

execute_process(
    COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
    OUTPUT_VARIABLE GFLAGS_NS
)
if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
    execute_process(
        COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
        OUTPUT_VARIABLE GFLAGS_NS
    )
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
    if(NOT HAVE_CLOCK_GETTIME)
        set(DEFINE_CLOCK_GETTIME "-DNO_CLOCK_GETTIME_IN_MAC")
    endif()
endif()

set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__=__unused__ -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")

if(CMAKE_VERSION VERSION_LESS "3.1.3")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif()
include_directories(${LEVELDB_INCLUDE_PATH})

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OPENSSL_ROOT_DIR
        "/usr/local/opt/openssl"    # Homebrew installed OpenSSL
        )
endif()

find_package(OpenSSL)
include_directories(${OPENSSL_INCLUDE_DIR})

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    ${THRIFT_LIB}
    ${THRIFTNB_LIB}
    dl
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        pthread
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
        "-Wl,-U,_ProfilerStart"
        "-Wl,-U,_ProfilerStop")
endif()

# ------------------------------ SET SQLParser ---------------------------------------

add_custom_target(build_SQLParser ALL
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sql-parser/
    COMMENT "Original sql-parser makefile target."
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sql-parser/libsqlparser.so DESTINATION ${CMAKE_BINARY_DIR})

set(SQLParser_LIBRARY ${CMAKE_BINARY_DIR}/libsqlparser.so)

add_library(SQLParser SHARED IMPORTED)

set_property(TARGET SQLParser APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(SQLParser PROPERTIES IMPORTED_LOCATION_NOCONFIG "${SQLParser_LIBRARY}")

add_dependencies(SQLParser build_SQLParser)

# ------------------------------ Readline -----------------------------------------

find_path(READLINE_INCLUDE_PATH NAMES readline/readline.h)
find_library(READLINE_LIB NAMES readline)

include_directories(${READLINE_INCLUDE_PATH})

# ------------------------------ json ---------------------------------------------

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

# ------------------------------ curl ---------------------------------------------

find_package(CURL REQUIRED)

# ------------------------------ MySQL -------------------------------------------

find_path(MySQL_INCLUDE_PATH NAMES mysql/mysql.h)
find_library(MySQL_LIB NAMES mysqlclient)

include_directories(${MySQL_INCLUDE_PATH})

# ------------------------------ jsoncpp -----------------------------------------

find_path(JSONCPP_INCLUDE_PATH NAMES json/json.h)
find_library(JSONCPP_LIB NAMES jsoncpp)

include_directories(${JSONCPP_INCLUDE_PATH})

# ------------------------------ Copy Resources -----------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

# ------------------------------ Executable ---------------------------------------

file(GLOB client_sources 
    src/client/*.cpp src/client/*.hpp 
)

file(GLOB db_sources 
    src/db_server/*.cpp src/db_server/*.h 
    src/db_server/optimizer/*.h src/db_server/optimizer/*.cpp
    src/db_server/mysql_connector/*.h src/db_server/mysql_connector/*.cpp
    src/db_server/metadata/*.h src/db_server/metadata/*.cpp
)

file(GLOB kv_sources 
    src/kv_server/*.cpp src/kv_server/*.hpp 
)
file(GLOB optimizer_sources 
    src/optimizer/*.cpp src/optimizer/*.h 
)


add_executable(client ${client_sources} ${PROTO_SRC} ${PROTO_HEADER})
target_link_libraries(client ${BRPC_LIB} ${DYNAMIC_LIB} ${READLINE_LIB})

add_executable(db ${db_sources} ${PROTO_SRC} ${PROTO_HEADER} ${optimizer_sources})
target_include_directories(db PUBLIC  
    src/db_server
    sql-parser/src
    ${MySQL_INCLUDE_PATH}
    ${JSONCPP_INCLUDE_PATH}
)
target_link_libraries(db 
    ${BRPC_LIB} 
    ${DYNAMIC_LIB}
    ${MySQL_LIB}
    ${JSONCPP_LIB}
    SQLParser 
    nlohmann_json::nlohmann_json
    CURL::libcurl
)
