cmake_minimum_required(VERSION 3.13)

project(tm_examples C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    message(FATAL_ERROR "A C++17 compatible compiler is required. Please update to GCC version 7 or newer.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
    message(FATAL_ERROR "A C++17 compatible compiler is required. Please update to Clang version 6 or newer.")
  endif()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

#include(CMakeDependentOption)
#option(TM_BUILD_SHARED_LIBS "Build shared libraries instead of static libraries" OFF)
#cmake_dependent_option(TM_USE_SHARED_CRT "Build with dynamic CRT on Windows (/MD)" ON "TM_BUILD_SHARED_LIBS" OFF)

if(UNIX)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(MSVC)
  set(SYSTEM_ARCHITECTURE x86_64)
  enable_language(ASM_MASM)
else()
  execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE SYSTEM_ARCHITECTURE)
endif()

if("$ENV{VERBOSE}" STREQUAL "1")
  message(STATUS " Verbose Makefile ACTIVATED")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if(${COLORIZE_OUTPUT})
    string(APPEND CMAKE_CXX_FLAGS " -fcolor-diagnostics")
  endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
  if(${COLORIZE_OUTPUT})
    string(APPEND CMAKE_CXX_FLAGS " -fdiagnostics-color=always")
  endif()
endif()

if(MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WIN32_WINNT=0x0601)
  set(CMAKE_C_FLAGS "/MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} /bigobj")
else()
  include(CheckCXXCompilerFlag)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-sign-compare")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
  else()
    add_definitions(-DNDEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}")
endif()

# if(MSVC AND NOT TM_USE_SHARED_CRT)
#   foreach(flag_var
#         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#         CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#     if(${flag_var} MATCHES "/MD")
#       string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#     elseif(${flag_var} MATCHES "/MDd")
#       string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")    
#     endif()
#   endforeach(flag_var)
# endif()

if(NOT TM_EXAMPLES_LINKER_ITEMS)
  set(TM_EXAMPLES_LINKER_ITEMS "")
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS log log_setup system thread filesystem iostreams program_options)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND TM_EXAMPLES_LINKER_ITEMS ${Boost_LIBRARIES})

find_package(spdlog CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS spdlog::spdlog_header_only)

find_package(simdjson CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS simdjson::simdjson)

find_package(OpenSSL REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS OpenSSL::SSL OpenSSL::Crypto)

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)
find_package(nlohmann_json CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS nlohmann_json::nlohmann_json)

find_package(rabbitmq-c CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS rabbitmq::rabbitmq)

find_package(ZeroMQ REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS cppzmq libzmq)

find_package(hiredis CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS hiredis::hiredis)

find_package(nng CONFIG REQUIRED)
find_package(nngpp CONFIG REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS nng::nng nng::nngpp)

find_package(crossguid REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS crossguid)

find_package(Protobuf REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)

find_package(grpc REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS gRPC::address_sorting gRPC::gpr gRPC::grpc gRPC::grpc_unsecure gRPC::grpc++ gRPC::grpc++_alts gRPC::grpc++_error_details gRPC::grpc++_reflection gRPC::grpc++_unsecure gRPC::grpc_plugin_support gRPC::grpcpp_channelz)

find_package(etcdcpp REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS etcdcpp)

find_package(SOCI REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS SOCI::soci_core SOCI::soci_odbc SOCI::soci_empty SOCI::soci_mysql)

find_package(CURL REQUIRED)
find_package(unofficial-curlpp REQUIRED)
list(APPEND TM_EXAMPLES_LINKER_ITEMS CURL::libcurl unofficial::curlpp::curlpp)

find_package(TMInfra REQUIRED)
find_package(TMBasic REQUIRED)
find_package(TMTransport REQUIRED)
include_directories(${TMInfra_INCLUDE_DIR})
include_directories(${TMBasic_INCLUDE_DIR})
include_directories(${TMTransport_INCLUDE_DIR})
list(APPEND TM_EXAMPLES_LINKER_ITEMS ${TMTransport_LIBRARY} ${TMBasic_LIBRARY} ${TMInfra_LIBRARY})

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(transport_utils)
add_subdirectory(serialization_test)