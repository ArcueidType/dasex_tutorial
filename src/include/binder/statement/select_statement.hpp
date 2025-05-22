/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-13 20:07:31
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-03 13:30:35
 * @FilePath: /task_sche_demo/src/include/binder/statement/select_statement.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../bound_expression.h"
#include "../bound_order_by.h"
#include "../bound_statement.hpp"
#include "../bound_table_ref.h"
#include "../table_ref/bound_subquery_ref.h"

namespace DaseX {

class CataLog;

class SelectStatement : public BoundStatement {
public:
  explicit SelectStatement(
      std::unique_ptr<BoundTableRef> table,
      std::vector<std::unique_ptr<BoundExpression>> select_list,
      std::unique_ptr<BoundExpression> where,
      std::vector<std::unique_ptr<BoundExpression>> group_by,
      std::unique_ptr<BoundExpression> having,
      std::unique_ptr<BoundExpression> limit_count,
      std::unique_ptr<BoundExpression> limit_offset,
      std::vector<std::unique_ptr<BoundOrderBy>> sort, CTEList ctes,
      bool is_distinct)
      : BoundStatement(StatementType::SELECT_STATEMENT),
        table_(std::move(table)), select_list_(std::move(select_list)),
        where_(std::move(where)), group_by_(std::move(group_by)),
        having_(std::move(having)), limit_count_(std::move(limit_count)),
        limit_offset_(std::move(limit_offset)), sort_(std::move(sort)),
        ctes_(std::move(ctes)), is_distinct_(is_distinct) {}

  /** Bound FROM clause. */
  std::unique_ptr<BoundTableRef> table_;

  /** Bound SELECT list. */
  std::vector<std::unique_ptr<BoundExpression>> select_list_;

  /** Bound WHERE clause. */
  std::unique_ptr<BoundExpression> where_;

  /** Bound GROUP BY clause. */
  std::vector<std::unique_ptr<BoundExpression>> group_by_;

  /** Bound HAVING clause. */
  std::unique_ptr<BoundExpression> having_;

  /** Bound LIMIT clause. */
  std::unique_ptr<BoundExpression> limit_count_;

  /** Bound OFFSET clause. */
  std::unique_ptr<BoundExpression> limit_offset_;

  /** Bound ORDER BY clause. */
  std::vector<std::unique_ptr<BoundOrderBy>> sort_;

  CTEList ctes_;
  /** Is SELECT DISTINCT */
  bool is_distinct_;
  std::string ToString() const;
};

} // namespace DaseX
