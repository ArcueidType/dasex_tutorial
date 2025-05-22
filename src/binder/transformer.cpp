/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 13:47:46
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-19 13:11:18
 * @FilePath: /task_sche/src/binder/transformer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/binder/binder.h"
#include "../include/binder/statement/create_statement.hpp"
#include "../include/binder/table_ref/bound_expression_list_ref.h"
#include "nodes/nodes.hpp"
#include "nodes/parsenodes.hpp"
namespace DaseX {

void Binder::SaveParseTree(duckdb_libpgquery::PGList *tree) {
  std::vector<std::unique_ptr<StatementType>> statements;
  for (auto entry = tree->head; entry != nullptr; entry = entry->next) {

    statement_nodes_.push_back(
        reinterpret_cast<duckdb_libpgquery::PGNode *>(entry->data.ptr_value));
  }
}

auto Binder::BindStatement(duckdb_libpgquery::PGNode *stmt)
    -> std::unique_ptr<BoundStatement> {

    // TODO(扩展): 后续可以用 bustub 那套接其他 statement 的解析
//  printf("Node Type : %s\n", NodeTagToString(stmt->type).c_str());
  switch (stmt->type) {
  case duckdb_libpgquery::T_PGRawStmt:
    return BindStatement(
        reinterpret_cast<duckdb_libpgquery::PGRawStmt *>(stmt)->stmt);
  case duckdb_libpgquery::T_PGCreateStmt:
    return BindCreate(
        reinterpret_cast<duckdb_libpgquery::PGCreateStmt *>(stmt));
  case duckdb_libpgquery::T_PGSelectStmt:
    return BindSelect(
        reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(stmt));
    // case duckdb_libpgquery::T_PGInsertStmt:
    // return BindInsert(reinterpret_cast<duckdb_libpgquery::PGInsertStmt
    // *>(stmt));
    default:
        Exception(NodeTagToString(stmt->type));

  }
}

} // namespace DaseX
