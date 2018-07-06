cmake_minimum_required(VERSION 3.1)

project(SimDS)

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


file(GLOB HEADERS include/sim_ds/*.hpp)
add_library(sim_ds STATIC include/sim_ds.cpp ${HEADERS})
include_directories(include)


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED thread)
if(Boost_FOUND)
    message(STATUS "!!! BOOST FOUND !!!")
else()
    message(STATUS "!!! BOOST NOT FOUND !!!")
endif()

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
target_link_libraries(sim_ds ${Boost_LIBRARIES})


enable_testing()
add_subdirectory(test)

