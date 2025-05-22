#pragma once

#include "include.hpp"

TableInfo CreateTableNationInfo();

TableInfo CreateTableRegionInfo();

TableInfo CreateTablePartInfo();

TableInfo CreateTableSupplierInfo();

TableInfo CreateTablePartsuppInfo();

TableInfo CreateTableCustomerInfo();

TableInfo CreateTableOrdersInfo();

TableInfo CreateTableLineitemInfo();

TableInfo CreateTableRevenueInfo();

RC LoadSinglePartitionAllTable(std::string& database_path, bool new_database = true);

RC LoadMultiPartitionAllTable(std::string& database_path, bool new_database = true);
