#include "abstract_plan.h"
#include "arrow_help.hpp"
#include "binder.h"
#include "bound_in_expression.hpp"
#include "bound_project_function_expression.hpp"
#include "catalog.hpp"
#include "expression.hpp"
#include "expression_executor.hpp"
#include "like.hpp"
#include "logical_type.hpp"
#include "nest_loop_join_plan.h"
#include "numa_help.hpp"
#include <arrow/api.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "optimizer.hpp"
#include "physical_filter.hpp"
#include "physical_hash_aggregation.hpp"
#include "physical_hash_join.hpp"
#include "physical_order.hpp"
#include "physical_project.hpp"
#include "physical_result_collector.hpp"
#include "physical_table_scan.hpp"
#include "pipe.h"
#include "pipeline_group_execute.hpp"
#include "pipeline_task.hpp"
#include "planner.hpp"
#include "postgres_parser.hpp"
#include "projection_plan.h"
#include "rc.hpp"
#include "scheduler.hpp"
#include "seq_scan_plan.h"
#include "spdlog/fmt/bundled/core.h"
#include "string_functions.hpp"
#include "import_data.hpp"
#include "query_handler.hpp"

using namespace DaseX;

const int arr[] = {1, 2, 3, 4};
const std::string fileDir4 = "../test_data/4thread";
const std::string fileDir1 = "/opt/tutorial_dasex_data";

std::string fileDir = fileDir1;
std::vector<int> work_ids(arr, arr + sizeof(arr) / sizeof(arr[0]));
std::vector<int> partition_idxs(arr, arr + sizeof(arr) / sizeof(arr[0]));

// TEST(TPCHBySqlsTest, TPCHBySqlsTestQ14) {

// 	std::string test_sql_tpch = "select\n"
// 		"(sum(case\n"
// 		"             when p_type like 'PROMO%' \n"
// 		"             then l_extendedprice*(1.00-l_discount)\n"
// 		"             else 0.00\n"
// 		"             end) / sum(l_extendedprice * (1.00 - l_discount)) ) * 100 \n"
// 		"from\n"
// 		"lineitem,part \n"
// 		"where\n"
// 		"l_partkey = p_partkey\n"
// 		"and l_shipdate >= 809884800 \n"
// 		"and l_shipdate < 812476800;";

// 	std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(72);
// 	bool importFinish = false;

//     // 插入Part表数据
//     std::shared_ptr<Table> table_part;
//     std::vector<std::string> part_file_names;
//     for(int i = 0; i < work_ids.size(); i++) {
//         std::string file_name = fileDir + "/" + "part.tbl_" + std::to_string(i);
//         part_file_names.emplace_back(file_name);
//     }
//     InsertPartMul(table_part, scheduler, work_ids, partition_idxs, part_file_names, importFinish);
//     importFinish = false;
//     spdlog::info("[{} : {}] InsertPart Finish!!!", __FILE__, __LINE__);

//     // 插入Lineitem表数据
//     std::shared_ptr<Table> table_lineitem;
//     std::vector<std::string> lineitem_file_names;
//     for(int i = 0; i < work_ids.size(); i++) {
//         std::string file_name = fileDir + "/" + "lineitem.tbl_" + std::to_string(i);
//         lineitem_file_names.emplace_back(file_name);
//     }
//     InsertLineitemMul(table_lineitem, scheduler, work_ids, partition_idxs, lineitem_file_names, importFinish);
//     importFinish = false;

// 	scheduler->shutdown();

// 	QueryHandler query_handler(global_catalog);
	
// 	auto start_execute = std::chrono::steady_clock::now();
// 	RC rc = query_handler.Query(test_sql_tpch);

// 	ASSERT_EQ(RC::SUCCESS, rc);

// 	auto end_execute = std::chrono::steady_clock::now();

// 	spdlog::info("[{} : {}] Q14执行时间: {}", __FILE__, __LINE__, Util::time_difference(start_execute, end_execute));
// }

TEST(TPCHBySqlsTest, TPCHBySqlsTestQ4) {

	std::string test_sql_tpch = 
		"SELECT "
            "o_orderpriority, "
            "COUNT(*) AS order_count "
        "FROM "
            "orders "
        ",( "
            "SELECT l_orderkey, count(*) as l_cnt "
            "FROM lineitem "
            "WHERE l_commitdate < l_receiptdate "
			"GROUP BY l_orderkey"
        ") as q1 "
        "WHERE "
			"o_orderkey = q1.lineitem.l_orderkey "
			"AND q1.l_cnt > 0 "
            "AND o_orderdate >= 741456000 "
            "AND o_orderdate < 749404800 "
        "GROUP BY "
            "o_orderpriority "
        "ORDER BY "
            "o_orderpriority;";

	std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(72);
	bool importFinish = false;

    // 插入order表数据
    std::shared_ptr<Table> table_part;
    std::vector<std::string> part_file_names;
    for(int i = 0; i < work_ids.size(); i++) {
        std::string file_name = fileDir + "/" + "orders.tbl_" + std::to_string(i);
        part_file_names.emplace_back(file_name);
    }
    InsertOrdersMul(table_part, scheduler, work_ids, partition_idxs, part_file_names, importFinish);
    importFinish = false;

    // 插入Lineitem表数据
    std::shared_ptr<Table> table_lineitem;
    std::vector<std::string> lineitem_file_names;
    for(int i = 0; i < work_ids.size(); i++) {
        std::string file_name = fileDir + "/" + "lineitem.tbl_" + std::to_string(i);
        lineitem_file_names.emplace_back(file_name);
    }
    InsertLineitemMul(table_lineitem, scheduler, work_ids, partition_idxs, lineitem_file_names, importFinish);
    importFinish = false;
	spdlog::info("[{} : {}] InsertPart Finish!!!", __FILE__, __LINE__);

	scheduler->shutdown();

	QueryHandler query_handler(global_catalog);
	
	auto start_execute = std::chrono::steady_clock::now();
	RC rc = query_handler.Query(test_sql_tpch);

	ASSERT_EQ(RC::SUCCESS, rc);

	auto end_execute = std::chrono::steady_clock::now();

	spdlog::info("[{} : {}] Q4执行时间: {}", __FILE__, __LINE__, Util::time_difference(start_execute, end_execute));
}
