#include "../include/binder/binder.h"
#include "../include/binder/statement/create_statement.hpp"
#include "catalog.hpp"

namespace DaseX {

/**
 * 此函数用于执行 DDL 语句，目前仅用于处理 create 够，且仅仅接受 CreateStmt
 * @param create_stmt
 * @return RC 创建是否成功
 */
auto Binder::ExecuteDDL(const CreateStatement &create_stmt) -> RC {

  TableInfo tab = TableInfo();
  auto info = &tab;
  info->table_name = create_stmt.table_;

  // printf("table name : %s\n",info->table_name.c_str());

  for (Field temp_field : create_stmt.columns_) {

    // 仅 用于调试 =========================
    auto temp_info = temp_field.Tostring();
    printf("DEBUG: Fields 的 信息 %s\n", temp_info.c_str());
    // ====================================
    info->filed_names.push_back(temp_field.field_name);
    info->filed_types.push_back(temp_field.type);
    info->partition_num = Numa::CORES;
    info->column_nums = info->filed_names.size();
    // printf("field name : %s  ,filed_type: %d ,col_num: %d
    // \n",info->filed_names[0].c_str(),info->filed_types[0],info->column_nums);
  }

  auto rc = catalog_.lock()->register_table(*info);
  return rc;
}

} // namespace DaseX