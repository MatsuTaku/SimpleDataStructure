cmake_minimum_required(VERSION 3.1)
project(DistributedAC)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_STANDARD is ${CMAKE_CXX_STANDARD}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED thread)
if(Boost_FOUND)
    message(STATUS "!!! BOOST FOUND !!!")
else()
    message(STATUS "!!! BOOST NOT FOUND !!!")
endif()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})


include_directories(include)

file(GLOB HEADERS
	include/*.hpp
)

file(GLOB SOURCES
	src/*.cpp
)

add_library(SimDS STATIC
    ${SOURCES}
    ${HEADERS}
)
TARGET_LINK_LIBRARIES(v_codes ${Boost_LIBRARIES})
