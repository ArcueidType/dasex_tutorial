/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-13 15:03:42
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-14 12:52:12
 * @FilePath: /task_sche_demo/src/include/binder/table_ref/bound_subquery_ref.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../statement/select_statement.hpp"
#include "../bound_expression.h"
#include "../bound_table_ref.h"

namespace DaseX {

class SelectStatement;

/**
 * A subquery. e.g., `SELECT * FROM (SELECT * FROM t1)`, where `(SELECT * FROM
 * t1)` is `BoundSubqueryRef`.
 */
class BoundSubqueryRef : public BoundTableRef {
public:
  explicit BoundSubqueryRef(
      std::unique_ptr<SelectStatement> subquery,
      std::vector<std::vector<std::string>> select_list_name, std::string alias)
      : BoundTableRef(TableReferenceType::SUBQUERY),
        subquery_(std::move(subquery)),
        select_list_name_(std::move(select_list_name)),
        alias_(std::move(alias)) {}

  std::unique_ptr<SelectStatement> subquery_;

  std::vector<std::vector<std::string>> select_list_name_;

  std::string alias_;
};

using CTEList = std::vector<std::unique_ptr<BoundSubqueryRef>>;

} // namespace DaseX
