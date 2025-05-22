cmake_minimum_required(VERSION 3.22)
project(DaseX_Test)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# message(STATUS "CMAKE_BINARY_DIRR is :" ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(APPEND EXTAR_INCLUDE_HEADER ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Parquet REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${EXTAR_INCLUDE_HEADER})

target_link_libraries(${PROJECT_NAME} GTest::gtest_main src)


add_executable(test_load_from_file main.cpp import_data.cpp create_table.cpp)
target_link_libraries(test_load_from_file GTest::gtest_main src)

add_executable(test_persistence main.cpp)
target_link_libraries(test_persistence PRIVATE parquet_shared GTest::gtest_main src)

add_executable(tpch_by_sqls_loaded main.cpp create_table.cpp)
target_link_libraries(tpch_by_sqls_loaded PRIVATE GTest::gtest_main src)