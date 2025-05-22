cmake_minimum_required(VERSION 3.22)

set(PROJECT_MAJOR_VERSION 2024)
set(PROJECT_MINOR_VERSION 07)
set(PROJECT_PATCH_VERSION 23)

project(dasex  VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION})

# 配置生成版本号头文件
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.h
        @ONLY
)
# 将生成的头文件目录添加到 include 路径中
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 打印测试信息
MESSAGE(STATUS "This is Project source dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "This is PROJECT_BINARY_DIR dir " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "This is CMAKE_SOURCE_DIR dir " ${CMAKE_SOURCE_DIR})

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 指定C++编译参数

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lnuma -O3")
# 调试使用下面切换为下面命令
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g -lnuma")

# 控制线程数
add_definitions(-DTHREAD_NUMS=4)


######################## 查找第三方库-start ########################

## googletest
include(FetchContent)
set(GTEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/gtest")
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://gitee.com/caiwanli/googletest.git
  GIT_TAG        release-1.12.0   # release-1.12.0
  SOURCE_DIR     ${GTEST_SOURCE_DIR}   # 指定本地路径        
)
# FetchContent_Declare(
#   googletest
#   SOURCE_DIR     ${GTEST_SOURCE_DIR}   # 指定本地路径        
# )
FetchContent_MakeAvailable(googletest)
find_package(GTest REQUIRED)
if(GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    message("GTest found : ${GTest_FOUND}")
    message(STATUS "GTEST_INCLUDE_DIR is :" ${GTEST_INCLUDE_DIR})
else()
    message(FATAL_ERROR "GTest library not found.")
endif()

## spdlog
set(SPDLOG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/spdlog")
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://gitee.com/caiwanli/spdlog.git
  GIT_TAG        v1.8.1   # v1.8.1
  SOURCE_DIR     ${SPDLOG_SOURCE_DIR}   # 指定本地路径        
)
# FetchContent_Declare(
#   spdlog
#   SOURCE_DIR     ${SPDLOG_SOURCE_DIR}   # 指定本地路径        
# )
FetchContent_MakeAvailable(spdlog)
include_directories(${SPDLOG_SOURCE_DIR}/include)


## libpg_query
set(LIBPG_QUERY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/libpg_query")
FetchContent_Declare(
  libpg_query
  GIT_REPOSITORY https://gitee.com/zhang_yuyun/libpg_query.git
  GIT_TAG        e3387391dab261a6a28bbe62c11ada4d7c598660  #lastest
  SOURCE_DIR     ${LIBPG_QUERY_SOURCE_DIR}   # 指定本地路径        
)
FetchContent_MakeAvailable(libpg_query)
include_directories(${LIBPG_QUERY_SOURCE_DIR}/include)

## fmt
set(FMT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/fmt")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://gitee.com/zhang_yuyun/fmt.git
  GIT_TAG        aed0cf5d8244f041c41a87e6a9aa590110291065  #lastest
  SOURCE_DIR     ${FMT_SOURCE_DIR}   # 指定本地路径        
)
FetchContent_MakeAvailable(fmt)
include_directories(${FMT_SOURCE_DIR}/include)

## pthread
find_package(Threads)
if(THREADS_FOUND)
    message("pthread library is found.")
else()
    message(FATAL_ERROR "pthread library not found.")
endif()

## Arrow
find_package(Arrow REQUIRED)
if (Arrow_FOUNG)
    message("Arrow FOUND:${Arrow_FOUND}")
    message("Arrow Version:${Arrow_VERSION}")
endif()

######################## 查找第三方库-end ########################

# 添加子目录
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
