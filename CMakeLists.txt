cmake_minimum_required(VERSION 2.8.12)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Crypto: Found ccache package... Activating...")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(LIB_MAJOR_VERSION "2")
set(LIB_MINOR_VERSION "0")
set(LIB_PATCH_VERSION "0")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

if(CMAKE_VERSION VERSION_LESS 3.0)
    project(TurtleCoin CXX)
    enable_language(CXX)
else()
    cmake_policy(SET CMP0048 NEW)
    project(TurtleCoin VERSION "${LIB_VERSION_STRING}" LANGUAGES CXX)
endif()

## This section describes our general CMake setup options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SKIP_INSTALL_RULES OFF FORCE)
set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON FORCE)
set(CMAKE_SUPPRESS_REGENERATION ON)
set(CMAKE_POLICY_WARNING_CMP0048 OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## We only build static binaries -- this is left here for our dependencies
set(STATIC ON CACHE BOOL FORCE "Link libraries statically? Forced to ON")
add_definitions(-DSTATICLIB)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

option(BUILD_UNIT_TESTS "Build included tests" OFF)
if(DEFINED ENV{BUILD_UNIT_TESTS})
    set(BUILD_UNIT_TESTS $ENV{BUILD_UNIT_TESTS})
endif()
if(BUILD_UNIT_TESTS)
    message(STATUS "Tests added to targets list")
endif()

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O3 -static-libgcc -static -Wuninitialized")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -static-libstdc++ -static -Wuninitialized")
endif()

# makes the credits file includable in source code
file(READ CREDITS credits_content)
string(REPLACE "\"" "\\\"" credits_content "${credits_content}")
string(REPLACE "\n" "\\n\"\n\"" credits_content "${credits_content}")
set(credits_content "const auto program_credits = \n\"${credits_content}\";")
file(WRITE include/credits.h "${credits_content}")

add_subdirectory(${CMAKE_SOURCE_DIR}/external)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cppzmq)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cpphttplib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/ipv6-parse/include)

add_subdirectory(src)
