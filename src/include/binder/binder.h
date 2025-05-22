#pragma once

#include "bound_expression.h"
#include "bound_statement.hpp"
#include "bound_table_ref.h"
#include "catalog.hpp"
#include "expressions/bound_alias.h"
#include "expressions/bound_column_ref.h"
#include "expressions/bound_constant.h"
#include "nodes/nodes.hpp"
#include "nodes/parsenodes.hpp"
#include "nodes/pg_list.hpp"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"
#include "statement/create_statement.hpp"
#include "statement/insert_statement.hpp"
#include "statement/select_statement.hpp"
#include "statement_type.hpp"
#include "table_ref/bound_expression_list_ref.h"
#include "table_ref/bound_subquery_ref.h"
#include <memory>

namespace duckdb_libpgquery {
struct PGList;
struct PGSelectStmt;
struct PGAConst;
struct PGAStar;
struct PGFuncCall;
struct PGNode;
struct PGColumnRef;
struct PGResTarget;
struct PGAExpr;
struct PGJoinExpr;
} // namespace duckdb_libpgquery

namespace DaseX {

class CataLog;
class BoundColumnRef;
class BoundExpression;
class BoundTableRef;
class BoundBaseTableRef;
class BoundExpression;
class BoundExpressionListRef;
class BoundOrderBy;
class BoundSubqueryRef;
class CreateStatement;
class SelectStatement;

class Binder {

public:
  explicit Binder(std::weak_ptr<CataLog> catalog);

  // 配套使用, 将 pg parse tree 转换成 sql statement, 并保存在 statement_nodes_
  // 中.
  void ParseAndSave(const std::string &query);

  // 将pg与番薯转化到一个 sql statement的数组中
  void SaveParseTree(duckdb_libpgquery::PGList *tree);

  static auto IsKeyword(const std::string &text) -> bool;

  // 将 pg tag 转为 字符串
  static auto NodeTagToString(duckdb_libpgquery::PGNodeTag type) -> std::string;

  // Binder 入口, 解开 Rawstmt, 转入对应 Bind
  auto BindStatement(duckdb_libpgquery::PGNode *stmt)
      -> std::unique_ptr<BoundStatement>;

  // Create 语句相关
  auto BindCreate(duckdb_libpgquery::PGCreateStmt *pg_stmt)
      -> std::unique_ptr<CreateStatement>;
  auto BindColumnDefinition(duckdb_libpgquery::PGColumnDef *cdef) -> Field;

  // Select
  auto BindSelect(duckdb_libpgquery::PGSelectStmt *pg_stmt)
      -> std::unique_ptr<SelectStatement>;

  // ===================================================
  //  Bind 中，对于关系代数的解析顺序如下, 并在其中对所有的表达式进行解析
  //  From (BindFrom)
  //  Where(BindWhere)
  //  GroupBy(BindGroupBy)
  //  Having(BindHaving)
  //  OrderBy(BindSort)
  //  Limit(TODO)
  //  ===================================================

  // 各种谓词处理
  auto BindSelectList(duckdb_libpgquery::PGList *list)
      -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindWhere(duckdb_libpgquery::PGNode *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindGroupBy(duckdb_libpgquery::PGList *list)
      -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindHaving(duckdb_libpgquery::PGNode *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindExpression(duckdb_libpgquery::PGNode *node)
      -> std::unique_ptr<BoundExpression>;

  auto BindExpressionList(duckdb_libpgquery::PGList *list)
      -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindConstant(duckdb_libpgquery::PGAConst *node)
      -> std::unique_ptr<BoundExpression>;

  auto BindColumnRef(duckdb_libpgquery::PGColumnRef *node)
      -> std::unique_ptr<BoundExpression>;

  auto BindResTarget(duckdb_libpgquery::PGResTarget *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindStar(duckdb_libpgquery::PGAStar *node)
      -> std::unique_ptr<BoundExpression>;

  auto BindFuncCall(duckdb_libpgquery::PGFuncCall *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindAExpr(duckdb_libpgquery::PGAExpr *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindBoolExpr(duckdb_libpgquery::PGBoolExpr *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindSubLink(duckdb_libpgquery::PGSubLink *root)
      -> std::unique_ptr<BoundExpression>;
  auto GetAllColumns(const BoundTableRef &scope)
      -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindCTE(duckdb_libpgquery::PGWithClause *node)
      -> std::vector<std::unique_ptr<BoundSubqueryRef>>;

  // About TableRef, but GetAllColumns always used
  auto
  BindFrom(duckdb_libpgquery::PGList *list) -> std::unique_ptr<BoundTableRef>;

  auto BindBaseTableRef(std::string table_name,
                        std::optional<std::string> alias)
      -> std::unique_ptr<BoundBaseTableRef>;

  auto BindRangeVar(duckdb_libpgquery::PGRangeVar *table_ref)
      -> std::unique_ptr<BoundTableRef>;

  auto BindTableRef(duckdb_libpgquery::PGNode *node)
      -> std::unique_ptr<BoundTableRef>;

  auto BindJoin(duckdb_libpgquery::PGJoinExpr *root)
      -> std::unique_ptr<BoundTableRef>;

  auto ResolveColumn(const BoundTableRef &scope,
                     const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundExpression>;

  auto ResolveColumnInternal(const BoundTableRef &table_ref,
                             const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundExpression>;

  auto
  ResolveColumnRefFromBaseTableRef(const BoundBaseTableRef &table_ref,
                                   const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundColumnRef>;

  // not sure correct
  auto BindSort(duckdb_libpgquery::PGList *list)
      -> std::vector<std::unique_ptr<BoundOrderBy>>;

  auto BindRangeSubselect(duckdb_libpgquery::PGRangeSubselect *root)
      -> std::unique_ptr<BoundTableRef>;

  auto
  BindSubquery(duckdb_libpgquery::PGSelectStmt *node,
               const std::string &alias) -> std::unique_ptr<BoundSubqueryRef>;

  auto ResolveColumnRefFromSelectList(
      const std::vector<std::vector<std::string>> &subquery_select_list,
      const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundColumnRef>;

  auto ResolveColumnRefFromSubqueryRef(const BoundSubqueryRef &subquery_ref,
                                       const std::string &alias,
                                       const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundColumnRef>;

  auto BindValuesList(duckdb_libpgquery::PGList *list)
      -> std::unique_ptr<BoundExpressionListRef>;

  auto BindLimitCount(duckdb_libpgquery::PGNode *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindLimitOffset(duckdb_libpgquery::PGNode *root)
      -> std::unique_ptr<BoundExpression>;

  auto BindCase(duckdb_libpgquery::PGCaseExpr *root) -> std::unique_ptr<BoundExpression>;

  // DDL 执行 TODO(放到合理的位置)
  auto ExecuteDDL(const CreateStatement &create_stmt) -> RC;

  // 保存所有 parsenode, 由此对应 bind 类型.
  std::vector<duckdb_libpgquery::PGNode *> statement_nodes_;

  class ContextGuard {
  public:
    explicit ContextGuard(const BoundTableRef **scope,
                          const CTEList **cte_scope) {
      old_scope_ = *scope;
      scope_ptr_ = scope;
      old_cte_scope_ = *cte_scope;
      cte_scope_ptr_ = cte_scope;
      *scope = nullptr;
    }
    ~ContextGuard() {
      *scope_ptr_ = old_scope_;
      *cte_scope_ptr_ = old_cte_scope_;
    }

  private:
    const BoundTableRef *old_scope_;
    const BoundTableRef **scope_ptr_;
    const CTEList *old_cte_scope_;
    const CTEList **cte_scope_ptr_;
  };

  // 用于在 BindFrom 以及 BindJoin 中，保护上下文。
  auto NewContext() -> ContextGuard {
    return ContextGuard(&scope_, &cte_scope_);
  }

private:
  std::weak_ptr<CataLog> catalog_;

  duckdb::PostgresParser parser_;

  // 在binding 表达式的过程中，确定域，当前的域
  const BoundTableRef *scope_{nullptr};

  //  CTE 域，用于 binding tables
  const CTEList *cte_scope_{nullptr};

  // 对一些未命名的列、元素，需要命名时，由此进行附名
  size_t universal_id_{0};
};

} // namespace DaseX