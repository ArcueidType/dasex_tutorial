cmake_minimum_required(VERSION 3.22)
project(src)

#指定可执行文件输出位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
#设置libgcc_s.so.1目录
#注意 在134上该项不可被注释
# set(LIBGCC_PATH "/home/cwl/tool")

find_package(Parquet REQUIRED)


#指定头文件目录
list(APPEND EXTAR_INCLUDE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/binder
        ${CMAKE_CURRENT_SOURCE_DIR}/include/binder/expressions
        ${CMAKE_CURRENT_SOURCE_DIR}/include/binder/statement
        ${CMAKE_CURRENT_SOURCE_DIR}/include/binder/table_ref
        ${CMAKE_CURRENT_SOURCE_DIR}/binder
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/common/config
        ${CMAKE_CURRENT_SOURCE_DIR}/common/context
        ${CMAKE_CURRENT_SOURCE_DIR}/common/type
        ${CMAKE_CURRENT_SOURCE_DIR}/expression
        ${CMAKE_CURRENT_SOURCE_DIR}/function
        ${CMAKE_CURRENT_SOURCE_DIR}/operator
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/agg
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/filter
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/helper
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/hashjoin
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/nestedloopjoin
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/radixjoin
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/order
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/project
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/scan
        ${CMAKE_CURRENT_SOURCE_DIR}/optimizer
        ${CMAKE_CURRENT_SOURCE_DIR}/pipeline
        ${CMAKE_CURRENT_SOURCE_DIR}/pipeline/execute
        ${CMAKE_CURRENT_SOURCE_DIR}/planner
        ${CMAKE_CURRENT_SOURCE_DIR}/sche
        ${CMAKE_CURRENT_SOURCE_DIR}/sql
        ${CMAKE_CURRENT_SOURCE_DIR}/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/util
        ${CMAKE_CURRENT_SOURCE_DIR}/test)


#添加源文件
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/binder/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/config/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/context/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/type/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/expression/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/function/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/agg/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/filter/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/helper/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/hashjoin/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/nestedloopjoin/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/join/radixjoin/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/order/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/project/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/operator/scan/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/optimizer/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pipeline/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pipeline/execute/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/planner/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sche/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sql/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/storage/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)


add_library(${PROJECT_NAME} ${SRC_FILES}
        pipeline/pipe.h
        pipeline/pipe.cpp
        planner/logic_expression.h
        operator/physical_operator.cpp
        operator/physical_operator.cpp
        operator/physical_operator.cpp
        operator/physical_operator.cpp
        pipeline/execute/pipeline_group_execute.hpp
        pipeline/execute/pipeline_group_execute.cpp
        expression/expression_executor_state.cpp
        common/type/expression_type.cpp
        function/function.cpp
        function/scalar_function.hpp
        function/scalar_function.cpp
        function/string_functions.hpp
        expression/bound_function_expression.hpp
        function/unary_executor.hpp
        function/string_functions.cpp
        function/like.cpp
        function/like.hpp
        util/common.hpp
        util/typedefs.hpp
        expression/expression.cpp
        operator/helper/physical_result_collector.hpp
        operator/helper/physical_result_collector.cpp
        operator/order/physical_order.hpp
        common/type/order_type.hpp
        operator/order/physical_order.cpp
        common/type/task_type.hpp
        operator/order/merge_sort_task.hpp
        expression/bound_project_function_expression.hpp
        function/numeric.hpp
        function/binary_executor.hpp
        common/type/numeric_operator_type.hpp
        function/numeric.cpp
        function/binary_exector.cpp
        operator/join/hashjoin/physical_hash_join.hpp
        operator/join/hashjoin/physical_hash_join.cpp
        operator/join/hashjoin/join_hashtable.hpp
        operator/join/hashjoin/join_hashtable.cpp
        expression/bound_in_expression.hpp
        binder/node_tag_to_string.cpp
        include/binder/table_ref/bound_cte_ref.h
        planner/sort_plan.h
        include/binder/expressions/bound_case.h
        planner/like_expression.h
        include/binder/expressions/bound_func_call.h
        planner/calc_expression.h
        operator/join/bloom_filter.hpp
        operator/join/bloom_filter.cpp
        planner/case_expression.h
        planner/llm_expression.h
        function/llm_call.hpp
        function/llm_call.cpp

)


#添加头文件搜索目录
target_include_directories(${PROJECT_NAME} PUBLIC ${EXTAR_INCLUDE_HEADER})

########################################添加依赖库-start########################################
## 添加jemalloc
find_library(JEMALLOC_LIB jemalloc)
if (JEMALLOC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
    message(STATUS "jemalloc library found at: ${JEMALLOC_LIB}")
else ()
    message(FATAL_ERROR "jemalloc not found")
endif ()

find_package(leveldb REQUIRED)

########################################添加依赖库-end########################################
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE Arrow::arrow_shared libgcc_s.so.1)
target_link_libraries(${PROJECT_NAME} PRIVATE duckdb_pg_query)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE leveldb::leveldb)
target_link_libraries(${PROJECT_NAME} PRIVATE parquet_shared)
# 91环境下，在这里链接libnuma库，普通用户没有权限链接非常规目录下的库，因此把相关库考到可以访问的目录
find_library(LIB_NUMA numa REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_NUMA})
