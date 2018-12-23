cmake_minimum_required(VERSION 3.1)

project(SiDS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE          is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_STANDARD  is ${CMAKE_CXX_STANDARD}")
message(STATUS "CXX_FLAGS           are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG     are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE   are ${CMAKE_CXX_FLAGS_RELEASE}")

set(SDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADERS ${SDS_INCUDE_DIR}/sim_ds.hpp ${SDS_INCLUDE_DIR}/sim_ds/*.hpp)

# Build Public Library
add_library(sim_ds STATIC ${SDS_INCLUDE_DIR}/sim_ds.cpp ${HEADERS})
target_include_directories(sim_ds PUBLIC ${SDS_INCLUDE_DIR})
target_include_directories(sim_ds PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(sim_ds PRIVATE ${Boost_LIBRARIES})

# Build Private Static Library for Development on Xcode
include_directories(${SDS_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(STATUS "Enable test SimDS")
	enable_testing()
	add_subdirectory(test)
endif()

