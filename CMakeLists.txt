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
        target_compile_options(sim_ds INTERFACE -msse4.2)
    endif()
endif()

set(CLANG_WARNING_OPTIONS -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic)
set(GCC_WARNING_OPTIONS -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused)
set(MSVC_WARNING_OPTIONS /W4)
target_compile_options(sim_ds PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang>:${CLANG_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:GCC>:${GCC_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNING_OPTIONS}>
    )

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
