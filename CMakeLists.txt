cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(SimDS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

include(CheckSSE4_2)
if(BUILTIN_POPCNT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang|Intel")
        set(CMAKE_CXX_FLAGS "-msse4.2 -march=native ${CMAKE_CXX_FLAGS}")
    endif()
endif()

message(STATUS "BUILD_TYPE          is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_STANDARD  is ${CMAKE_CXX_STANDARD}")
message(STATUS "CXX_FLAGS           are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG     are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE   are ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "BUILTIN_POPCNT       is ${BUILTIN_POPCNT}")
message(STATUS "CXX_COMPILER_ID       is ${CMAKE_CXX_COMPILER_ID}")

set(SDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADERS ${SDS_INCUDE_DIR}/sim_ds.hpp ${SDS_INCLUDE_DIR}/sim_ds/*.hpp)

# Build Public Library
add_library(sim_ds STATIC ${SDS_INCLUDE_DIR}/sim_ds.cpp ${HEADERS})
target_include_directories(sim_ds PUBLIC ${SDS_INCLUDE_DIR})

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    add_subdirectory(test)
endif()

