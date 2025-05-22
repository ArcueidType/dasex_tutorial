#include "create_table.hpp"
#include "query_handler.hpp"

namespace {

const int arr[] = {1, 2, 3, 4};
const std::string fileDir4 = "../test_data/4thread";
const std::string fileDir1 = "../test_data";

std::string fileDir = fileDir4;
std::vector<int> work_ids(arr, arr + sizeof(arr) / sizeof(arr[0]));
std::vector<int> partition_idxs(arr, arr + sizeof(arr) / sizeof(arr[0]));

}  // namespace

TableInfo CreateTableNationInfo() {
    std::vector<std::string> filed_names = {"n_nationkey", "n_name", "n_regionkey", "n_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    std::string table_name = "nation";
    TableInfo nation_info(table_name, filed_names, filed_types);
    return nation_info;

}

TableInfo CreateTableRegionInfo() {
    std::vector<std::string> filed_names = {"r_regionkey", "r_name", "r_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    std::string table_name = "region";
    TableInfo region_info(table_name, filed_names, filed_types);
    return region_info;
}

TableInfo CreateTablePartInfo() {
    std::vector<std::string> filed_names = {"p_partkey", "p_name", "p_mfgr", "p_brand", "p_type", "p_size", "p_container", "p_retailprice", "p_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::STRING;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::INTEGER;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "part";
    TableInfo part_info(table_name, filed_names, filed_types);
    return part_info;
}

TableInfo CreateTableSupplierInfo() {
    std::vector<std::string> filed_names = {"s_suppkey", "s_name", "s_address", "s_nationkey", "s_phone", "s_acctbal", "s_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    std::string table_name = "supplier";
    TableInfo supplier_info(table_name, filed_names, filed_types);
    return supplier_info;
}

TableInfo CreateTablePartsuppInfo() {
    std::vector<std::string> filed_names = {"ps_partkey", "ps_suppkey", "ps_availqty", "ps_supplycost", "ps_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    std::string table_name = "partsupp";
    TableInfo partsupp_info(table_name, filed_names, filed_types);
    return partsupp_info;
}

TableInfo CreateTableCustomerInfo() { 
    std::vector<std::string> filed_names = {"c_custkey", "c_name", "c_address", "c_nationkey", "c_phone", "c_acctbal", "c_mktsegment", "c_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    std::string table_name = "customer";
    TableInfo customer_info(table_name, filed_names, filed_types);
    return customer_info;
}

TableInfo CreateTableOrdersInfo() {
    std::vector<std::string> filed_names = {"o_orderkey", "o_custkey", "o_orderstatus", "o_totalprice", "o_orderdate", "o_orderpriority", "o_clerk", "o_shippriority", "o_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::INTEGER;
    LogicalType filed6  = LogicalType::STRING;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::INTEGER;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "orders";
    TableInfo orders_info(table_name, filed_names, filed_types);
    return orders_info;
}


TableInfo CreateTableLineitemInfo() {
    std::vector<std::string> filed_names = {"l_orderkey", "l_partkey", "l_suppkey", "l_linenumber", "l_quantity", "l_extendedprice", "l_discount", "l_tax", "l_returnflag", "l_linestatus", "l_shipdate", "l_commitdate", "l_receiptdate", "l_shipinstruct", "l_shipmode", "l_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::FLOAT;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::FLOAT;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    LogicalType filed10 = LogicalType::STRING;
    LogicalType filed11 = LogicalType::INTEGER;
    LogicalType filed12 = LogicalType::INTEGER;
    LogicalType filed13 = LogicalType::INTEGER;
    LogicalType filed14 = LogicalType::STRING;
    LogicalType filed15 = LogicalType::STRING;
    LogicalType filed16 = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    filed_types.push_back(filed10);
    filed_types.push_back(filed11);
    filed_types.push_back(filed12);
    filed_types.push_back(filed13);
    filed_types.push_back(filed14);
    filed_types.push_back(filed15);
    filed_types.push_back(filed16);
    std::string table_name = "lineitem";
    TableInfo lineitem_info(table_name, filed_names, filed_types);
    return lineitem_info;
}


TableInfo CreateTableRevenueInfo() {
    std::vector<std::string> filed_names = {"supplier_no", "total_revenue"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::DOUBLE;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    std::string table_name = "revenue";
    TableInfo revenue_info(table_name, filed_names, filed_types);
    return revenue_info;
}


// 在文件夹database_path创建数据库
RC LoadSinglePartitionAllTable(std::string& database_path, bool new_database) {
	auto load_single = [](std::string table_name,
						  std::function<DaseX::TableInfo()>& create_table_info,
						  std::shared_ptr<DaseX::CataLog>& catalog) {
		int partition_idx = 0;
		std::shared_ptr<Table> table;
		std::string file_name = fileDir1 + "/" + table_name + ".tbl";
        RC rc = RC::SUCCESS;

		// 创建表
		auto table_info = create_table_info();
        rc = catalog->register_table(table_info);
		if(rc != RC::SUCCESS) {
            return rc;
        }
		rc = catalog->get_table(table_info.table_name, table);
        if(rc != RC::SUCCESS) {
            return rc;
        }

		// 加载数据
		spdlog::info("[{}:{}] Start load '{}' from '{}'", __FILE__, __LINE__,
					 table_info.table_name, file_name);
		rc = catalog->load_from_single_file(table_info.table_name, table,
												 partition_idx, file_name,
												 CHUNK_SIZE, '|');

        if(rc != RC::SUCCESS) {
            return rc;
        }

		spdlog::info("[{}:{}] End load '{}'", __FILE__, __LINE__,
					 table_info.table_name);

		return rc;
	};

	std::vector<std::string> file_names = {
		"customer", "lineitem", "nation",  "orders",   "partsupp",
		"part",		"region",	"revenue", "supplier",
	};
	std::vector<std::function<DaseX::TableInfo()>> functionArray = {
		CreateTableCustomerInfo, CreateTableLineitemInfo,
		CreateTableNationInfo,	 CreateTableOrdersInfo,
		CreateTablePartsuppInfo, CreateTablePartInfo,
		CreateTableRegionInfo,	 CreateTableRevenueInfo,
		CreateTableSupplierInfo,
	};

	std::shared_ptr<DaseX::CataLog> catalog(nullptr);

	// 创建数据库
    if (new_database) {
        DeleteDatabase(database_path);
    }
	if (OpenDatabase(database_path, catalog, true) != RC::SUCCESS) {
        return RC::FAILED;
	}
	assert(catalog.get() != nullptr);

	for (int i = 0; i < file_names.size(); i++) {
		auto rc = load_single(file_names[i], functionArray[i], catalog);
        if(rc != RC::SUCCESS) {
            spdlog::error("[{}:{}] Failed to load '{}'", __FILE__, __LINE__,
						  file_names[i]);
            return rc;
        }
	}
    return RC::SUCCESS;
}

RC LoadMultiPartitionAllTable(std::string& database_path, bool new_database) {
	auto load_multi = [](std::string table_name,
						 std::function<DaseX::TableInfo()>& create_table_info,
						 std::shared_ptr<DaseX::CataLog>& catalog) {
		RC rc = RC::SUCCESS;
		std::shared_ptr<Table> table;
		std::vector<std::string> file_names;
		for (int i = 0; i < work_ids.size(); i++) {
			std::string file_name =
				fileDir4 + "/" + table_name + ".tbl_" + std::to_string(i);
			file_names.emplace_back(file_name);
		}

		// 创建表
		auto table_info = create_table_info();
		rc = catalog->register_table(table_info);
		if (rc != RC::SUCCESS) {
			return rc;
		}
		rc = catalog->get_table(table_info.table_name, table);
		if (rc != RC::SUCCESS) {
			return rc;
		}

		// 加载数据
		spdlog::info("[{}:{}] Start load '{}'", __FILE__, __LINE__,
					 table_info.table_name);
		rc = catalog->load_from_files(table_info.table_name, table,
										   work_ids, partition_idxs, file_names,
										   CHUNK_SIZE, '|');
		if (rc != RC::SUCCESS) {
			return rc;
		}
		spdlog::info("[{}:{}] End load '{}'", __FILE__, __LINE__,
					 table_info.table_name);
        return RC::SUCCESS;
	};

	std::vector<std::string> file_names = {
		"customer", "lineitem", "nation",  "orders",   "partsupp",
		"part",		"region",	"revenue", "supplier",
	};
	std::vector<std::function<DaseX::TableInfo()>> functionArray = {
		CreateTableCustomerInfo, CreateTableLineitemInfo,
		CreateTableNationInfo,	 CreateTableOrdersInfo,
		CreateTablePartsuppInfo, CreateTablePartInfo,
		CreateTableRegionInfo,	 CreateTableRevenueInfo,
		CreateTableSupplierInfo,
	};

	std::shared_ptr<DaseX::CataLog> catalog(nullptr);
	// 创建数据库
	if (new_database) {
		DeleteDatabase(database_path);
	}
	if (OpenDatabase(database_path, catalog, true) != RC::SUCCESS) {
		return RC::FAILED;
	}
	assert(catalog.get() != nullptr);

	for (int i = 0; i < file_names.size(); i++) {
		load_multi(file_names[i], functionArray[i], catalog);
	}
    return RC::SUCCESS;
}
