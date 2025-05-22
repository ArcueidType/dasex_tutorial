
#include "../include/binder/binder.h"
#include "../include/binder/bound_statement.hpp"
#include "../include/binder/statement/create_statement.hpp"
#include "catalog.hpp"
#include "logical_type.hpp"
#include "nodes/nodes.hpp"
#include "nodes/pg_list.hpp"
#include "nodes/primnodes.hpp"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"
#include "spdlog/fmt/bundled/core.h"
#include <cstdio>
#include <string>

namespace DaseX {

/**
 * 此函数用于在 BindCreate 中，对列定义的部分进行解析
 * @param cdef
 * @return arrow::Field
 */
auto Binder::BindColumnDefinition(duckdb_libpgquery::PGColumnDef *cdef)
    -> Field {
  std::string col_name;
  if (cdef->colname != nullptr) {
    col_name = cdef->colname;
  }

  auto type_name = std::string((reinterpret_cast<duckdb_libpgquery::PGValue *>(
                                    cdef->typeName->names->tail->data.ptr_value)
                                    ->val.str));
  // 将 sql 句子中的列引用的类型转换为 catalog 中定义的类型 (此处利用了
  // arrow::Field)
  if (type_name == "int4") {
    return {col_name, LogicalType::INTEGER};
  }

  if (type_name == "double") {
    return {col_name, LogicalType::DOUBLE};
  }

  if (type_name == "float4") {
    return {col_name, LogicalType::FLOAT};
  }

  // bool
  if (type_name == "bool" || type_name == "boolean") {
    return {col_name, LogicalType::BOOL};
  }

  if (type_name == "string" || type_name == "varchar") {
    return {col_name, LogicalType::STRING};
  }
}

/**
 *  此函数用于对 Create 语句进行解析，并在后续可以通过 ExcuteDDL 函数进行执行
 *  主要对 PGCreateStmt 解析
 *  其中 relation -> table_name
 *  tableElts -> cols
 *  columnDef 中 constraints -> 约束条件
 * @param pg_stmt
 * @return unique_ptr<CreateStmt>
 */
auto Binder::BindCreate(duckdb_libpgquery::PGCreateStmt *pg_stmt)
    -> std::unique_ptr<CreateStatement> {

  auto table = std::string(pg_stmt->relation->relname);
  auto columns = std::vector<Field>();
  size_t column_count = 0;
  std::vector<std::string> primary_key;

  for (auto cell = pg_stmt->tableElts->head; cell != nullptr;
       cell = lnext(cell)) {
    auto node =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(cell->data.ptr_value);
    switch (node->type) {
    case duckdb_libpgquery::T_PGColumnDef: {
      auto cdef = reinterpret_cast<duckdb_libpgquery::PGColumnDef *>(
          cell->data.ptr_value);
      auto centry = BindColumnDefinition(cdef);

      // TODO: 处理对列有限制的时候
      if (cdef->constraints != nullptr) {
        for (auto constr = cdef->constraints->head; constr != nullptr;
             constr = constr->next) {

          auto constraint = reinterpret_cast<duckdb_libpgquery::PGConstraint *>(
              constr->data.ptr_value);
          switch (constraint->contype) {
            // 目前仅处理主键约束
          case duckdb_libpgquery::PG_CONSTR_PRIMARY: {
            if (!primary_key.empty()) {
              printf("TODO col constrait\n");
            }
            primary_key = {centry.field_name};
            break;
          }
          default:
            throw NotImplementedException("unsupported constraint");
          }
        }
      }

      columns.emplace_back(std::move(centry));
      column_count++;
      break;
    }
    case duckdb_libpgquery::T_PGConstraint: {
      for (auto con = cell; con != nullptr; con = con->next) {
        auto constraint = reinterpret_cast<duckdb_libpgquery::PGConstraint *>(
            con->data.ptr_value);
        switch (constraint->contype) {
        case duckdb_libpgquery::PG_CONSTR_PRIMARY: {
          std::vector<std::string> columns;
          for (auto kc = constraint->keys->head; kc != nullptr; kc = kc->next) {
            columns.emplace_back(reinterpret_cast<duckdb_libpgquery::PGValue *>(
                                     kc->data.ptr_value)
                                     ->val.str);
          }

          primary_key = std::move(columns);
          break;
        }
        default:
          throw NotImplementedException("unsupported constraint");
        }
      }
      break;
    }
    }
  }

  if (column_count == 0) {
    printf("at least 1 column!\n");
  }
  return std::make_unique<CreateStatement>(std::move(table), std::move(columns),
                                           std::move(primary_key));
}

} // namespace DaseX