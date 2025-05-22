/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 12:47:37
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-17 16:17:26
 * @FilePath: /task_sche/src/binder/bind_select.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/binder/binder.h"
#include "../include/binder/expressions/bound_agg_call.h"
#include "../include/binder/expressions/bound_binary_op.h"
#include "../include/binder/expressions/bound_column_ref.h"
#include "../include/binder/expressions/bound_constant.h"
#include "../include/binder/expressions/bound_star.h"
#include "../include/binder/expressions/bound_unary_op.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/binder/table_ref/bound_cross_product_ref.h"
#include "../include/binder/table_ref/bound_cte_ref.h"
#include "../include/binder/table_ref/bound_join_ref.h"
#include "../include/binder/table_ref/bound_subquery_ref.h"
#include "../include/binder/expressions/bound_case.h"
#include "../include/binder/expressions/bound_func_call.h"
#include "bound_expression.h"
#include "bound_subquery.h"
#include "catalog.hpp"
#include "compare_expression.h"
#include "exception.h"
#include "nodes/nodes.hpp"
#include "nodes/parsenodes.hpp"
#include "nodes/primnodes.hpp"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"

#include <cstdio>
#include <memory>

namespace DaseX {

/**
 * 对 Select 语句 进行处理的 主要函数
 * 以 CTE FROM target(select list) WHERE GROUPBY HAVING LIMIT ORDERBY
 * 的顺序进行解析,具体实现及文档参看对应 BindX 函数
 * @param pg_stmt
 * @return unique_ptr<SelectStatement>
 */
auto Binder::BindSelect(duckdb_libpgquery::PGSelectStmt *pg_stmt)
    -> std::unique_ptr<SelectStatement> {
        if (pg_stmt->valuesLists != nullptr) {
            auto values_list_name = fmt::format("__values#{}", universal_id_++);
            auto value_list = BindValuesList(pg_stmt->valuesLists);
            value_list->identifier_ = values_list_name;
            std::vector<std::unique_ptr<BoundExpression>> exprs;
            size_t expr_length = value_list->values_[0].size();
            for (size_t i = 0; i < expr_length; i++) {
                exprs.emplace_back(std::make_unique<BoundColumnRef>(std::vector{values_list_name, fmt::format("{}", i)}));
            }
            return std::make_unique<SelectStatement>(
                    std::move(value_list), std::move(exprs), std::make_unique<BoundExpression>(),
                    std::vector<std::unique_ptr<BoundExpression>>{}, std::make_unique<BoundExpression>(),
                    std::make_unique<BoundExpression>(), std::make_unique<BoundExpression>(),
                    std::vector<std::unique_ptr<BoundOrderBy>>{}, std::vector<std::unique_ptr<BoundSubqueryRef>>{}, false);
        }

  // CTES
  auto ctes = std::vector<std::unique_ptr<BoundSubqueryRef>>{};
  if (pg_stmt->withClause != nullptr) {
    ctes = BindCTE(pg_stmt->withClause);
    cte_scope_ = &ctes;
  }
  //  FROM clause.
  auto table = BindFrom(pg_stmt->fromClause);
  /// @FIXME: 此处直接修改 scope_ 会导致部分情况下子查询绑定错误 (Q16)
  const BoundTableRef *old_scope = scope_;
  scope_ = table.get();

  // DISTINCT
  bool is_distinct = false;
  if (pg_stmt->distinctClause != nullptr) {
    auto target = reinterpret_cast<duckdb_libpgquery::PGNode *>(
        pg_stmt->distinctClause->head->data.ptr_value);
    if (target != nullptr) {
      printf(
          "PGList *distinctClause;   /* NULL, list of DISTINCT ON exprs, or\n"
          "\t\t\t\t\t\t\t\t * lcons(NIL,NIL) for all (SELECT DISTINCT) */");
    }
    is_distinct = true;
  }


  if (pg_stmt->targetList == nullptr) {
    throw DaseX::Exception("no select list");
  }
  //  SELECT list. Between SELECT and FROM
  // TODO agg call failed, fix name
  auto select_list = BindSelectList(pg_stmt->targetList);

  //  WHERE clause.
  auto where = std::make_unique<BoundExpression>();
  if (pg_stmt->whereClause != nullptr) {
    where = BindWhere(pg_stmt->whereClause);
  }

  //  GROUP BY clause.
  auto group_by = std::vector<std::unique_ptr<BoundExpression>>{};
  if (pg_stmt->groupClause != nullptr) {
    group_by = BindGroupBy(pg_stmt->groupClause);
  }

  //  HAVING clause.
  auto having = std::make_unique<BoundExpression>();
  if (pg_stmt->havingClause != nullptr) {
    having = BindHaving(pg_stmt->havingClause);
  }

  auto limit_count = std::make_unique<BoundExpression>();
  //  LIMIT clause.
  if (pg_stmt->limitCount != nullptr) {
    limit_count = BindLimitCount(pg_stmt->limitCount);
  }

  //  OFFSET clause.
  auto limit_offset = std::make_unique<BoundExpression>();
  if (pg_stmt->limitOffset != nullptr) {
    limit_offset = BindLimitOffset(pg_stmt->limitOffset);
  }

  //  ORDER BY clause.
  auto sort = std::vector<std::unique_ptr<BoundOrderBy>>{};
  if (pg_stmt->sortClause != nullptr) {
    sort = BindSort(pg_stmt->sortClause);
  }

  scope_ = old_scope;
  
  return std::make_unique<SelectStatement>(
      std::move(table), std::move(select_list), std::move(where),
      std::move(group_by), std::move(having), std::move(limit_count),
      std::move(limit_offset), std::move(sort), std::move(ctes), is_distinct);
}

/**
 *  对 CTEs 进行处理
 * @param node
 * @return
 */
auto Binder::BindCTE(duckdb_libpgquery::PGWithClause *node)
    -> std::vector<std::unique_ptr<BoundSubqueryRef>> {
  std::vector<std::unique_ptr<BoundSubqueryRef>> ctes;
  for (auto cte_ele = node->ctes->head; cte_ele != nullptr;
       cte_ele = cte_ele->next) {
    auto cte = reinterpret_cast<duckdb_libpgquery::PGCommonTableExpr *>(
        cte_ele->data.ptr_value);

    auto subquery = BindSubquery(
        reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(cte->ctequery),
        cte->ctename);

    ctes.emplace_back(std::move(subquery));
  }

  return ctes;
}


auto Binder::BindFrom(duckdb_libpgquery::PGList *list)
    -> std::unique_ptr<BoundTableRef> {

  if (list == nullptr) {
    return std::make_unique<BoundTableRef>(TableReferenceType::EMPTY);
  }
  if (list->length > 1) {
    // select .. from x,y,z...

    // first node.
//    printf("The first node\n");
    auto c = list->head;
    auto lnode =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(c->data.ptr_value);
    auto ltable = BindTableRef(lnode);
    c = c->next;

    // second node.
    // printf("The second node\n");
    auto rnode =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(c->data.ptr_value);
    auto rtable = BindTableRef(rnode);
    c = c->next;

    auto result = std::make_unique<BoundCrossProductRef>(std::move(ltable),
                                                         std::move(rtable));
    // 递归解join
    // 左深树
    for (; c != nullptr; c = c->next) {
      auto node =
          reinterpret_cast<duckdb_libpgquery::PGNode *>(c->data.ptr_value);
      auto table = BindTableRef(node);
      result = std::make_unique<BoundCrossProductRef>(std::move(result),
                                                      std::move(table));
    }

    return result;
  }

  auto node =
      reinterpret_cast<duckdb_libpgquery::PGNode *>(list->head->data.ptr_value);
  return BindTableRef(node);
}

auto Binder::BindSelectList(duckdb_libpgquery::PGList *list)
    -> std::vector<std::unique_ptr<BoundExpression>> {
  std::vector<std::unique_ptr<BoundExpression>> exprs;
  auto select_list = std::vector<std::unique_ptr<BoundExpression>>{};
  bool is_select_star = false;
  bool has_agg = false;

  for (auto node = list->head; node != nullptr; node = node->next) {
    auto target =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(node->data.ptr_value);

    auto expr = BindExpression(target);

    if (expr->type_ == ExpressionType::STAR) {
      // "SELECT *".
      select_list = GetAllColumns(*scope_);
      is_select_star = true;
    } else {
      if (expr->HasAggregation()) {
        has_agg = true;
      }
      select_list.push_back(std::move(expr));
    }
  }
  return select_list;
}

auto Binder::BindWhere(duckdb_libpgquery::PGNode *root)
    -> std::unique_ptr<BoundExpression> {
  return BindExpression(root);
}

auto Binder::BindGroupBy(duckdb_libpgquery::PGList *list)
    -> std::vector<std::unique_ptr<BoundExpression>> {
  return BindExpressionList(list);
}

auto Binder::BindHaving(duckdb_libpgquery::PGNode *root)
    -> std::unique_ptr<BoundExpression> {
  return BindExpression(root);
}

auto Binder::BindLimitCount(duckdb_libpgquery::PGNode *root)
    -> std::unique_ptr<BoundExpression> {
  return BindExpression(root);
}

auto Binder::BindLimitOffset(duckdb_libpgquery::PGNode *root)
    -> std::unique_ptr<BoundExpression> {
  return BindExpression(root);
}

auto Binder::BindSort(duckdb_libpgquery::PGList *list)
    -> std::vector<std::unique_ptr<BoundOrderBy>> {
  auto order_by = std::vector<std::unique_ptr<BoundOrderBy>>{};

  for (auto node = list->head; node != nullptr; node = node->next) {
    auto temp =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(node->data.ptr_value);
    if (temp->type == duckdb_libpgquery::T_PGSortBy) {
      OrderByType type;
      auto sort = reinterpret_cast<duckdb_libpgquery::PGSortBy *>(temp);
      auto target = sort->node;
      if (sort->sortby_dir == duckdb_libpgquery::PG_SORTBY_DEFAULT) {
        type = OrderByType::DEFAULT;
      } else if (sort->sortby_dir == duckdb_libpgquery::PG_SORTBY_ASC) {
        type = OrderByType::ASC;
      } else if (sort->sortby_dir == duckdb_libpgquery::PG_SORTBY_DESC) {
        type = OrderByType::DESC;
      } else {
        throw NotImplementedException("unimplemented order by type");
      }
      auto order_expression = BindExpression(target);
      order_by.emplace_back(
          std::make_unique<BoundOrderBy>(type, std::move(order_expression)));
    } else {
      throw NotImplementedException("unsupported order by node");
    }
  }
  return order_by;
}

auto Binder::BindRangeVar(duckdb_libpgquery::PGRangeVar *table_ref)
    -> std::unique_ptr<BoundTableRef> {

  if (cte_scope_ != nullptr) {
    for (const auto &cte : *cte_scope_) {
      if (cte->alias_ == table_ref->relname) {
        std::string bound_name;
        if (table_ref->alias != nullptr) {
          bound_name = table_ref->alias->aliasname;
        } else {
          bound_name = table_ref->relname;
        }
        return std::make_unique<BoundCTERef>(cte->alias_,
                                             std::move(bound_name));
      }
    }
  }

  if (table_ref->alias != nullptr) {
    return BindBaseTableRef(table_ref->relname,
                            std::make_optional(table_ref->alias->aliasname));
  }

  return BindBaseTableRef(table_ref->relname, std::nullopt);
}

/**
 * join解析的工具类，赋予其 join_type
 * @param root
 * @return
 */
auto Binder::BindJoin(duckdb_libpgquery::PGJoinExpr *root)
    -> std::unique_ptr<BoundTableRef> {
  auto ctx_guard = NewContext();
  JoinType join_type;
  switch (root->jointype) {
  case duckdb_libpgquery::PG_JOIN_INNER: {
    join_type = JoinType::INNER;
    break;
  }
  case duckdb_libpgquery::PG_JOIN_LEFT: {
    join_type = JoinType::LEFT;
    break;
  }
  case duckdb_libpgquery::PG_JOIN_FULL: {
    join_type = JoinType::OUTER;
    break;
  }
  case duckdb_libpgquery::PG_JOIN_RIGHT: {
    join_type = JoinType::RIGHT;
    break;
  }
  case duckdb_libpgquery::PG_JOIN_SEMI: {
    join_type = JoinType::SEMI;
    break;
  }
  case duckdb_libpgquery::PG_JOIN_ANTI: {
    join_type = JoinType::ANTI;
    break;
  }
  default: {
    printf("Join type %d not supported", static_cast<int>(root->jointype));
  }
  }
  auto left_table = BindTableRef(root->larg);
  auto right_table = BindTableRef(root->rarg);
  auto join_ref = std::make_unique<BoundJoinRef>(
      join_type, std::move(left_table), std::move(right_table), nullptr);
  const BoundTableRef *old_scope = scope_;
  scope_ = join_ref.get();
  // 解析 condition 中的 表达式
  auto condition = BindExpression(root->quals);
  scope_ = old_scope;
  join_ref->condition_ = std::move(condition);
  return join_ref;
}

auto Binder::BindBaseTableRef(std::string table_name,
                              std::optional<std::string> alias)
    -> std::unique_ptr<BoundBaseTableRef> {
  // auto table = catalog_.lock()->tables.find(table_name);
  // if (table == catalog_.lock()->tables.end()) {
  //   printf("table not exists\n");
  // }
  std::shared_ptr<DaseX::Table> table;
  catalog_.lock()->get_table(table_name, table);
  return std::make_unique<BoundBaseTableRef>(
      std::move(table_name), std::move(alias), table->schema);
}

auto Binder::BindTableRef(duckdb_libpgquery::PGNode *node)
    -> std::unique_ptr<BoundTableRef> {
  switch (node->type) {
  case duckdb_libpgquery::T_PGRangeVar: {
    return BindRangeVar(
        reinterpret_cast<duckdb_libpgquery::PGRangeVar *>(node));
  }
  case duckdb_libpgquery::T_PGJoinExpr: {
    return BindJoin(reinterpret_cast<duckdb_libpgquery::PGJoinExpr *>(node));
  }
  case duckdb_libpgquery::T_PGRangeSubselect: {
    return BindRangeSubselect(
        reinterpret_cast<duckdb_libpgquery::PGRangeSubselect *>(node));
  }
  default:
    throw DaseX::Exception("unsupported node type: {}");
  }
}

auto Binder::BindRangeSubselect(duckdb_libpgquery::PGRangeSubselect *root)
    -> std::unique_ptr<BoundTableRef> {

  if (root->alias != nullptr) {
    return BindSubquery(
        reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(root->subquery),
        std::string(root->alias->aliasname));
  }
  return BindSubquery(
      reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(root->subquery),
      fmt::format("__subquery#{}", universal_id_++));
}

auto Binder::GetAllColumns(const BoundTableRef &scope)
    -> std::vector<std::unique_ptr<BoundExpression>> {
  switch (scope.type_) {
  case TableReferenceType::BASE_TABLE: {
    const auto &base_table_ref = dynamic_cast<const BoundBaseTableRef &>(scope);
    auto bound_table_name = base_table_ref.GetBoundTableName();
    const auto &schema = base_table_ref.schema_;
    auto columns = std::vector<std::unique_ptr<BoundExpression>>{};

    for (const auto &column : schema.fields()) {
      columns.push_back(std::make_unique<BoundColumnRef>(
          std::vector{bound_table_name, column->name()}));
    }
    return columns;
  }
  case TableReferenceType::CROSS_PRODUCT: {
    const auto &cross_product_ref =
        dynamic_cast<const BoundCrossProductRef &>(scope);
    auto columns = GetAllColumns(*cross_product_ref.left_);
    auto append_columns = GetAllColumns(*cross_product_ref.right_);
    std::copy(std::make_move_iterator(append_columns.begin()),
              std::make_move_iterator(append_columns.end()),
              std::back_inserter(columns));
    // to del
    for (auto &column : columns) {
      if (column->type_ == ExpressionType::COLUMN_REF) {
        auto &column_ref = dynamic_cast<BoundColumnRef &>(*column);
        auto temp_string = column_ref.ToString();
        // printf("cross cols :%s ",column_ref.col_name_[1].c_str());
        printf("cross cols :%s ", temp_string.c_str());
      }
    }
    printf("\n");
    return columns;
  }
  case TableReferenceType::JOIN: {
    const auto &join_ref = dynamic_cast<const BoundJoinRef &>(scope);
    auto columns = GetAllColumns(*join_ref.left_);
    auto append_columns = GetAllColumns(*join_ref.right_);
    std::copy(std::make_move_iterator(append_columns.begin()),
              std::make_move_iterator(append_columns.end()),
              std::back_inserter(columns));
    return columns;
  }
  case TableReferenceType::SUBQUERY: {
    const auto &subquery_ref = dynamic_cast<const BoundSubqueryRef &>(scope);
    auto columns = std::vector<std::unique_ptr<BoundExpression>>{};
    for (const auto &col_name : subquery_ref.select_list_name_) {
      columns.emplace_back(BoundColumnRef::Prepend(
          std::make_unique<BoundColumnRef>(col_name), subquery_ref.alias_));
    }
    return columns;
  }
  case TableReferenceType::CTE: {
    const auto &cte_ref = dynamic_cast<const BoundCTERef &>(scope);
    for (const auto &cte : *cte_scope_) {
      if (cte_ref.cte_name_ == cte->alias_) {
        auto columns = GetAllColumns(*cte);
        for (auto &column : columns) {
          auto &column_ref = dynamic_cast<BoundColumnRef &>(*column);
          column_ref.col_name_[0] = cte_ref.alias_;
        }
        return columns;
      }
    }
  }
  default:
    throw DaseX::Exception(
        "select * cannot be used with this TableReferenceType");
  }
}

auto Binder::BindExpressionList(duckdb_libpgquery::PGList *list)
    -> std::vector<std::unique_ptr<BoundExpression>> {
  std::vector<std::unique_ptr<BoundExpression>> exprs;
  auto select_list = std::vector<std::unique_ptr<BoundExpression>>{};

  for (auto node = list->head; node != nullptr; node = node->next) {
    auto target =
        reinterpret_cast<duckdb_libpgquery::PGNode *>(node->data.ptr_value);

    auto expr = BindExpression(target);

    select_list.push_back(std::move(expr));
  }

  return select_list;
}

auto Binder::BindConstant(duckdb_libpgquery::PGAConst *node)
    -> std::unique_ptr<BoundExpression> {

  const auto &val = node->val;
  switch (val.type) {
  case duckdb_libpgquery::T_PGInteger: {
    return std::make_unique<BoundConstant>(
        Value(static_cast<int32_t>(val.val.ival)));
  }
  case duckdb_libpgquery::T_PGFloat: {
    float parsed_val = std::stod(std::string(val.val.str));
    return std::make_unique<BoundConstant>(Value(parsed_val));
  }
  case duckdb_libpgquery::T_PGString: {
    return std::make_unique<BoundConstant>(Value(std::string(val.val.str)));
  }
  case duckdb_libpgquery::T_PGNull: {
    DaseX::Exception("TODO T_PGString\n");
  }
  default:
    break;
  }
}

auto Binder::BindColumnRef(duckdb_libpgquery::PGColumnRef *node)
    -> std::unique_ptr<BoundExpression> {
  auto fields = node->fields;
  auto head_node =
      static_cast<duckdb_libpgquery::PGNode *>(fields->head->data.ptr_value);
  switch (head_node->type) {
  case duckdb_libpgquery::T_PGString: {
    std::vector<std::string> column_names;
    for (auto node = fields->head; node != nullptr; node = node->next) {
      column_names.emplace_back(
          reinterpret_cast<duckdb_libpgquery::PGValue *>(node->data.ptr_value)
              ->val.str);
    }
    return ResolveColumn(*scope_, column_names);
  }
  case duckdb_libpgquery::T_PGAStar: {
    printf("bind Astar\n");
    return BindStar(reinterpret_cast<duckdb_libpgquery::PGAStar *>(head_node));
  }
  }
}

auto Binder::BindResTarget(duckdb_libpgquery::PGResTarget *root)
    -> std::unique_ptr<BoundExpression> {
  auto expr = BindExpression(root->val);
  if (!expr) {
    return nullptr;
  }
  if (root->name != nullptr) {
    return std::make_unique<BoundAlias>(root->name, std::move(expr));
  }
  return expr;
}

auto Binder::BindStar(duckdb_libpgquery::PGAStar *node)
    -> std::unique_ptr<BoundExpression> {
  return std::make_unique<BoundStar>();
}

auto Binder::BindFuncCall(duckdb_libpgquery::PGFuncCall *root)
    -> std::unique_ptr<BoundExpression> {
  auto name = root->funcname;
  auto function_char_name =
      reinterpret_cast<duckdb_libpgquery::PGValue *>(name->head->data.ptr_value)
          ->val.str;

  std::string function_name = function_char_name;
  std::vector<std::unique_ptr<BoundExpression>> children;
  if (root->args != nullptr) {
    for (auto node = root->args->head; node != nullptr; node = node->next) {
      auto child_expr = BindExpression(
          static_cast<duckdb_libpgquery::PGNode *>(node->data.ptr_value));
      children.push_back(std::move(child_expr));
    }
  }
  // printf("FuncCall func name is: %s\n", function_name.c_str());

  if (function_name == "min" || function_name == "max" ||
      function_name == "first" || function_name == "last" ||
      function_name == "sum" || function_name == "count"
      || function_name == "avg") {

    if (function_name == "count" && children[0]->type_ == ExpressionType::STAR) {
      function_name = "count_star";
    }

    // Just used in DEBUG
//    printf(" the agg func %s call \n", function_name.c_str());

    // Bind agg call.
    return std::make_unique<BoundAggCall>(function_name, root->agg_distinct,
                                          std::move(children));
  }


  // llm func call
  if (function_name == "llm" ) {
//	  // 可以在这再加一些辅助的参数
//	  if (root->args != nullptr) {
//		  for (auto node = root->args->head; node != nullptr; node = node->next) {
//			  auto child_expr =
//				  static_cast<duckdb_libpgquery::PGNode *>(node->data.ptr_value);
//			  printf("%d",child_expr->type);
//		  }
//	  }
	  return std::make_unique<BoundFuncCall>(function_name,std::move(children));
  }



  // TODO : 其他函数处理
    return std::make_unique<BoundFuncCall>(function_name, std::move(children));
}

static auto ResolveColumnRefFromSchema(const arrow::Schema &schema,
                                       const std::vector<std::string> &col_name)
    -> std::unique_ptr<BoundColumnRef> {
  if (col_name.size() != 1) {
    return nullptr;
  }
  std::unique_ptr<BoundColumnRef> column_ref = nullptr;
  const auto& fields = schema.field_names();
  for (const auto &column : schema.fields()) {
    if (column->name() == col_name[0]) {
      column_ref =
          std::make_unique<BoundColumnRef>(std::vector{column->name()});
    }
  }
  return column_ref;
}

auto Binder::ResolveColumnRefFromBaseTableRef(
    const BoundBaseTableRef &table_ref,
    const std::vector<std::string> &col_name)
    -> std::unique_ptr<BoundColumnRef> {
  auto bound_table_name = table_ref.GetBoundTableName();
  std::unique_ptr<BoundColumnRef> direct_resolved_expr =
      BoundColumnRef::Prepend(
          ResolveColumnRefFromSchema(table_ref.schema_, col_name),
          bound_table_name);

  // std::unique_ptr<BoundColumnRef> direct_resolved_expr =
  //         ResolveColumnRefFromSchema(table_ref.schema_, col_name);

  std::unique_ptr<BoundColumnRef> strip_resolved_expr = nullptr;

  if (col_name.size() > 1) {
    if (col_name[0] == bound_table_name) {
      auto strip_column_name = col_name;
      strip_column_name.erase(strip_column_name.begin());
      auto x = ResolveColumnRefFromSchema(table_ref.schema_, strip_column_name);
      x->table_name_.push_back(bound_table_name);
      strip_resolved_expr =
          BoundColumnRef::Prepend(std::move(x), bound_table_name);
    }
  }

  if (strip_resolved_expr != nullptr) {
    return strip_resolved_expr;
  }
  return direct_resolved_expr;
}

auto Binder::ResolveColumnRefFromSelectList(
    const std::vector<std::vector<std::string>> &subquery_select_list,
    const std::vector<std::string> &col_name)
    -> std::unique_ptr<BoundColumnRef> {
  std::unique_ptr<BoundColumnRef> column_ref = nullptr;
  for (const auto &column_full_name : subquery_select_list) {
    // colname ?
    if (col_name == column_full_name) {
      if (column_ref != nullptr) {
        throw Exception(fmt::format("{} is ambiguous in subquery select list",
                                    fmt::join(col_name, ".")));
      }
      column_ref = std::make_unique<BoundColumnRef>(column_full_name);
    }
  }
  return column_ref;
}

auto Binder::ResolveColumnRefFromSubqueryRef(
    const BoundSubqueryRef &subquery_ref, const std::string &alias,
    const std::vector<std::string> &col_name)
    -> std::unique_ptr<BoundColumnRef> {
  std::unique_ptr<BoundColumnRef> direct_resolved_expr =
      BoundColumnRef::Prepend(ResolveColumnRefFromSelectList(
                                  subquery_ref.select_list_name_, col_name),
                              subquery_ref.alias_);

  std::unique_ptr<BoundColumnRef> strip_resolved_expr = nullptr;

  if (col_name.size() > 1) {
    if (col_name[0] == alias) {
      auto strip_column_name = col_name;
      strip_column_name.erase(strip_column_name.begin());
      strip_resolved_expr = BoundColumnRef::Prepend(
          ResolveColumnRefFromSelectList(subquery_ref.select_list_name_,
                                         strip_column_name),
          subquery_ref.alias_);
    }
  }

  if (strip_resolved_expr != nullptr) {
    return strip_resolved_expr;
  }
  return direct_resolved_expr;
}

auto Binder::ResolveColumnInternal(const BoundTableRef &table_ref,
                                   const std::vector<std::string> &col_name)
    -> std::unique_ptr<BoundExpression> {
  switch (table_ref.type_) {
  case TableReferenceType::BASE_TABLE: {
    const auto &base_table_ref =
        dynamic_cast<const BoundBaseTableRef &>(table_ref);
    return ResolveColumnRefFromBaseTableRef(base_table_ref, col_name);
  }
  case TableReferenceType::CROSS_PRODUCT: {
    const auto &cross_product_ref =
        dynamic_cast<const BoundCrossProductRef &>(table_ref);
    auto left_column =
        ResolveColumnInternal(*cross_product_ref.left_, col_name);
    auto right_column =
        ResolveColumnInternal(*cross_product_ref.right_, col_name);

    if (left_column != nullptr) {
      return left_column;
    }
    return right_column;
  }
  case TableReferenceType::JOIN: {
    const auto &join_ref = dynamic_cast<const BoundJoinRef &>(table_ref);
    auto left_column = ResolveColumnInternal(*join_ref.left_, col_name);
    auto right_column = ResolveColumnInternal(*join_ref.right_, col_name);

    if (left_column != nullptr) {
      return left_column;
    }
    return right_column;
  }
  case TableReferenceType::SUBQUERY: {
    const auto &subquery_ref =
        dynamic_cast<const BoundSubqueryRef &>(table_ref);
    return ResolveColumnRefFromSubqueryRef(subquery_ref, subquery_ref.alias_,
                                           col_name);
  }
  case TableReferenceType::CTE: {
    const auto &cte_ref = dynamic_cast<const BoundCTERef &>(table_ref);
    for (const auto &cte : *cte_scope_) {
      if (cte_ref.cte_name_ == cte->alias_) {
        return ResolveColumnRefFromSubqueryRef(*cte, cte_ref.alias_, col_name);
      }
    }
  }
  default:
    throw NotImplementedException("Binder::ResolveColumnInternal table_ref.type_");
  }
}

auto Binder::ResolveColumn(const DaseX::BoundTableRef &scope,
                           const std::vector<std::string> &col_name)
    -> std::unique_ptr<DaseX::BoundExpression> {
  auto expr = ResolveColumnInternal(scope, col_name);
  return expr;
}

auto Binder::BindAExpr(duckdb_libpgquery::PGAExpr *root)
    -> std::unique_ptr<BoundExpression> {
  auto name = std::string((reinterpret_cast<duckdb_libpgquery::PGValue *>(
                               root->name->head->data.ptr_value))
                              ->val.str);

  std::unique_ptr<BoundExpression> left_expr = nullptr;
  std::unique_ptr<BoundExpression> right_expr = nullptr;

  if (root->lexpr != nullptr) {
    left_expr = BindExpression(root->lexpr);
  }
  if (root->rexpr != nullptr) {
    right_expr = BindExpression(root->rexpr);
  }

  // support > = <, but how to sup in/not in
  if (left_expr && right_expr) {
    return std::make_unique<BoundBinaryOp>(name, std::move(left_expr),
                                           std::move(right_expr));
  }
  if (!left_expr && right_expr) {
    return std::make_unique<BoundUnaryOp>(name, std::move(right_expr));
  }
  throw DaseX::Exception("unsupported AExpr: left == null while right != null");
}

auto Binder::BindBoolExpr(duckdb_libpgquery::PGBoolExpr *root)
    -> std::unique_ptr<BoundExpression> {
  switch (root->boolop) {
  case duckdb_libpgquery::PG_AND_EXPR:
  case duckdb_libpgquery::PG_OR_EXPR: {
    std::string op_name;
    if (root->boolop == duckdb_libpgquery::PG_AND_EXPR) {
      op_name = "and";
    } else if (root->boolop == duckdb_libpgquery::PG_OR_EXPR) {
      op_name = "or";
    }

    auto exprs = BindExpressionList(root->args);
    if (exprs.size() <= 1) {
      if (root->boolop == duckdb_libpgquery::PG_AND_EXPR) {
        throw DaseX::Exception("AND should have at least 1 arg");
      }
      throw DaseX::Exception("OR should have at least 1 arg");
    }
    auto expr = std::make_unique<BoundBinaryOp>(op_name, std::move(exprs[0]),
                                                std::move(exprs[1]));
    for (size_t i = 2; i < exprs.size(); i++) {
      expr = std::make_unique<BoundBinaryOp>(op_name, std::move(expr),
                                             std::move(exprs[i]));
    }
    return expr;
  }
  case duckdb_libpgquery::PG_NOT_EXPR: {
    auto exprs = BindExpressionList(root->args);
    if (exprs.size() != 1) {
      throw DaseX::Exception("NOT should have 1 arg");
    }
    return std::make_unique<BoundUnaryOp>("not", std::move(exprs[0]));
  }
  }
}

auto Binder::BindSubLink(duckdb_libpgquery::PGSubLink *root)
    -> std::unique_ptr<BoundExpression> {
  // 对 child BindExpression 必须在 BindSelect 之前执行，因为 BindSelect 修改了 scope_ 作用域 (从外层修改为内层)
  std::unique_ptr<BoundExpression> child;
  if (root->testexpr)
    child = BindExpression(root->testexpr);

  std::unique_ptr<SelectStatement> subquery = BindSelect(
    reinterpret_cast<duckdb_libpgquery::PGSelectStmt*>(root->subselect));
  SubqueryType subquery_type;
  ComparisonType comparison_type;
  
  switch (root->subLinkType) {
	case duckdb_libpgquery::PG_EXISTS_SUBLINK: {
		subquery_type = SubqueryType::EXISTS;
		break;
	}
	case duckdb_libpgquery::PG_ANY_SUBLINK:
	case duckdb_libpgquery::PG_ALL_SUBLINK: {
		// comparison with ANY() or ALL()
		subquery_type = SubqueryType::ANY;
		// get the operator name
		if (!root->operName) {
			// simple IN
			comparison_type = ComparisonType::Equal;
		} else {
			auto operator_name =
			    std::string((reinterpret_cast<duckdb_libpgquery::PGValue*>(root->operName->head->data.ptr_value))->val.str);
			comparison_type = OperatorToComparisonType(operator_name);
		}
		if (comparison_type != ComparisonType::Equal &&
      comparison_type != ComparisonType::NotEqual &&
      comparison_type != ComparisonType::GreaterThan &&
      comparison_type != ComparisonType::GreaterThanOrEqual &&
      comparison_type != ComparisonType::LessThan &&
      comparison_type != ComparisonType::LessThanOrEqual) {
			throw NotImplementedException("ANY and ALL operators require one of =,<>,>,<,>=,<= comparisons!");
		}
		if (root->subLinkType == duckdb_libpgquery::PG_ALL_SUBLINK) {
      // 暂时不支持 NOT 表达式
			// ALL sublink is equivalent to NOT(ANY) with inverted comparison
			// e.g. [= ALL()] is equivalent to [NOT(<> ANY())]
			// first invert the comparison type
      throw NotImplementedException("PG_ALL_SUBLINK NOT is not supported.");
			// comparison_type = NegateComparisonType(comparison_type);
      // auto subquery_expr = std::make_unique<BoundSubquery>(
      //   std::move(subquery), subquery_type, std::move(child), comparison_type);
			// return make_uniq<OperatorExpression>(ExpressionType::OPERATOR_NOT, std::move(subquery_expr));
		}
		break;
	}
	case duckdb_libpgquery::PG_EXPR_SUBLINK: {
		// return a single scalar value from the subquery
		// no child expression to compare to
		subquery_type = SubqueryType::SCALAR;
		break;
	}
  // duckdb 考虑了 ARRAY，TPC-H 应该碰不到，暂不支持
	// case duckdb_libpgquery::PG_ARRAY_SUBLINK: {
	// }
	default:
		throw NotImplementedException("Subquery of type not implemented.");
	}
  return std::make_unique<BoundSubquery>(
    std::move(subquery), subquery_type, std::move(child), comparison_type);
}

auto Binder::BindSubquery(duckdb_libpgquery::PGSelectStmt *node,
                          const std::string &alias)
    -> std::unique_ptr<BoundSubqueryRef> {
  std::vector<std::vector<std::string>> select_list_name;
  auto subquery = BindSelect(node);
  for (const auto &col : subquery->select_list_) {
    switch (col->type_) {
    case ExpressionType::COLUMN_REF: {
      const auto &column_ref_expr = dynamic_cast<const BoundColumnRef &>(*col);
      select_list_name.push_back(column_ref_expr.col_name_);
      continue;
    }
    case ExpressionType::ALIAS: {
      const auto &alias_expr = dynamic_cast<const BoundAlias &>(*col);
      select_list_name.push_back(std::vector{alias_expr.alias_});
      continue;
    }
    default:
      select_list_name.push_back(
          std::vector{fmt::format("__item#{}", universal_id_++)});
      continue;
    }
  }
  return std::make_unique<BoundSubqueryRef>(std::move(subquery),
                                            std::move(select_list_name), alias);
}

auto Binder::BindCase(duckdb_libpgquery::PGCaseExpr *root) -> std::unique_ptr<BoundExpression> {
    auto case_node = std::make_unique<BoundCase>();

    auto root_arg = BindExpression(reinterpret_cast<duckdb_libpgquery::PGNode *>(root->arg));
    for(auto cell = root->args->head; cell != nullptr; cell = cell->next) {
        CaseCheck case_check;

        auto w = reinterpret_cast<duckdb_libpgquery::PGCaseWhen *>(cell->data.ptr_value);
        auto test_raw = BindExpression(reinterpret_cast<duckdb_libpgquery::PGNode *>(w->expr));
        std::unique_ptr<BoundExpression> test;
        if (root_arg) {
            // root_arg 所有权变更
            case_check.when_expr =
                    std::make_unique<BoundBinaryOp>("==", std::move(root_arg), std::move(test_raw));
        } else {
            case_check.when_expr = std::move(test_raw);
        }
        case_check.then_expr = BindExpression(reinterpret_cast<duckdb_libpgquery::PGNode *>(w->result));
        case_node->case_checks.push_back(std::move(case_check));
    }
    if(root->defresult) {
        case_node->else_expr = BindExpression(reinterpret_cast<duckdb_libpgquery::PGNode *>(root->defresult));
    } else {
        //  直接放了空指针，可以考虑放个常量0之类的，不是表达式可能会报错。
        case_node->else_expr = nullptr;

    }
    return case_node;
}

auto Binder::BindExpression(duckdb_libpgquery::PGNode *node)
    -> std::unique_ptr<BoundExpression> {
    if(!node) {
        return nullptr;
    }
  switch (node->type) {
  case duckdb_libpgquery::T_PGColumnRef:
    return BindColumnRef(
        reinterpret_cast<duckdb_libpgquery::PGColumnRef *>(node));
  case duckdb_libpgquery::T_PGAConst:
    return BindConstant(reinterpret_cast<duckdb_libpgquery::PGAConst *>(node));
  case duckdb_libpgquery::T_PGResTarget:
    return BindResTarget(
        reinterpret_cast<duckdb_libpgquery::PGResTarget *>(node));
  case duckdb_libpgquery::T_PGAStar:
    return BindStar(reinterpret_cast<duckdb_libpgquery::PGAStar *>(node));
  case duckdb_libpgquery::T_PGFuncCall:
    return BindFuncCall(
        reinterpret_cast<duckdb_libpgquery::PGFuncCall *>(node));
  case duckdb_libpgquery::T_PGAExpr:
    return BindAExpr(reinterpret_cast<duckdb_libpgquery::PGAExpr *>(node));
  case duckdb_libpgquery::T_PGCaseExpr:
    return BindCase(reinterpret_cast<duckdb_libpgquery::PGCaseExpr *>(node));
  case duckdb_libpgquery::T_PGBoolExpr:
    return BindBoolExpr(
        reinterpret_cast<duckdb_libpgquery::PGBoolExpr *>(node));
  case duckdb_libpgquery::T_PGSubLink: {
    return BindSubLink(reinterpret_cast<duckdb_libpgquery::PGSubLink *>(node));
  }
  default:
    break;
  }
  throw NotImplementedException("Binder::BindExpression unsupported node type.");
}

    auto Binder::BindValuesList(duckdb_libpgquery::PGList *list) -> std::unique_ptr<BoundExpressionListRef> {
        std::vector<std::vector<std::unique_ptr<BoundExpression>>> all_values;

        for (auto value_list = list->head; value_list != nullptr; value_list = value_list->next) {
            auto target = static_cast<duckdb_libpgquery::PGList *>(value_list->data.ptr_value);

            auto values = BindExpressionList(target);
            all_values.push_back(std::move(values));
        }

        return std::make_unique<BoundExpressionListRef>(std::move(all_values), "<unnamed>");
    }

} // namespace DaseX