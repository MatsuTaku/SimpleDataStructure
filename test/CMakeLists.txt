cmake_minimum_required(VERSION 3.10)

add_subdirectory(lib/googletest/googletest)

file(GLOB TEST_SOURCES *_test.cpp)
foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_SOURCE_NAME ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_SOURCE_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_SOURCE_NAME} gtest_main sim_ds)
  add_test(${TEST_SOURCE_NAME} ${TEST_SOURCE_NAME})
endforeach()

add_executable(sim_ds_test ${TEST_SOURCES})
target_link_libraries(sim_ds_test gtest_main sim_ds)
add_test(sim_ds_test sim_ds_test)


