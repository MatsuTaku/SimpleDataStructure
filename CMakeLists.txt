cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(SiDS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(SiDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADERS ${SiDS_INCLUDE_DIR}/sim_ds.hpp ${SiDS_INCLUDE_DIR}/sim_ds/*.hpp)

# Build Public Library
add_library(sim_ds STATIC ${SiDS_INCLUDE_DIR}/sim_ds.cpp ${HEADERS})
target_include_directories(sim_ds INTERFACE ${SiDS_INCLUDE_DIR})

find_package(Boost 1.53.0
    COMPONENTS unit_test_framework iostreams system thread filesystem)

if (Boost_FOUND)
    target_compile_definitions(sim_ds INTERFACE -DSIDS_USE_BOOST)
    target_include_directories(sim_ds INTERFACE ${Boost_INCLUDE_DIRS})
endif()

include(CheckSSE4_2)
if(BUILTIN_POPCNT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang|Intel")
        target_compile_options(sim_ds INTERFACE
            -march=native
            $<$<CONFIG:Release>:-O3>
            )
    endif()
endif()

message(STATUS "BUILD_TYPE          is  ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_STANDARD        is  ${CMAKE_CXX_STANDARD}")
message(STATUS "CXX_FLAGS           are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG     are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE   are ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CXX_COMPILER_ID     is  ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "BUILTIN_POPCNT      is  ${BUILTIN_POPCNT}")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    enable_testing()
    add_subdirectory(test)
endif()
