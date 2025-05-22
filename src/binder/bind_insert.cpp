/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 20:39:45
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-18 20:15:38
 * @FilePath: /task_sche/src/bind_insert.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iterator>
#include <memory>
#include <optional>
#include <string>

#include "../common/exception.h"
#include "../include/binder/binder.h"
#include "../include/binder/expressions/bound_column_ref.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/binder/statement/insert_statement.hpp"
#include "nodes/parsenodes.hpp"
#include "../include/binder/statement/select_statement.hpp"

namespace DaseX {

// auto Binder::BindInsert(duckdb_libpgquery::PGInsertStmt *pg_stmt) ->
// std::unique_ptr<InsertStatement> {

//   auto table = BindBaseTableRef(pg_stmt->relation->relname, std::nullopt);

//   auto select_statement =
//   BindSelect(reinterpret_cast<duckdb_libpgquery::PGSelectStmt
//   *>(pg_stmt->selectStmt));

//   return std::make_unique<InsertStatement>(std::move(table),
//   std::move(select_statement));
// }

} // namespace DaseX
