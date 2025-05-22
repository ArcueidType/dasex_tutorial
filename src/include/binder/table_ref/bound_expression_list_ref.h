/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-13 19:22:34
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-14 13:19:25
 * @FilePath:
 * /task_sche_demo/src/include/binder/table_ref/bound_expression_list_ref.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "../bound_table_ref.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DaseX {

class BoundExpression;

/**
 * A bound table ref type for `values` clause.
 */
class BoundExpressionListRef : public BoundTableRef {
public:
  explicit BoundExpressionListRef(
      std::vector<std::vector<std::unique_ptr<BoundExpression>>> values,
      std::string identifier)
      : BoundTableRef(TableReferenceType::EXPRESSION_LIST),
        values_(std::move(values)), identifier_(std::move(identifier)) {}

  // need not to implement now
  std::vector<std::vector<std::unique_ptr<BoundExpression>>> values_;

  std::string identifier_;
};
} // namespace DaseX
