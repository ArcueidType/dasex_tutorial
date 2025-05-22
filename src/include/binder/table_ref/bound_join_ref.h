/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-27 14:47:53
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-27 15:09:00
 * @FilePath: /task_sche-binder/src/binder/bound_join_ref.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <memory>
#include <string>
#include <utility>

#include "../bound_expression.h"
#include "../bound_table_ref.h"

namespace DaseX {

/**
 * Join types.
 */
enum class JoinType : uint8_t {
  INVALID = 0, /**< Invalid join type. */
  LEFT = 1,    /**< Left join. */
  RIGHT = 3,   /**< Right join. */
  INNER = 4,   /**< Inner join. */
  OUTER = 5,   /**< Outer join. */
  SEMI = 6,     /**< SEMI join. */
  ANTI = 7      /**< ANTI join. */
};

/**
 * A join. e.g., `SELECT * FROM x INNER JOIN y ON ...`, where `x INNER JOIN y ON
 * ...` is `BoundJoinRef`.
 */
class BoundJoinRef : public BoundTableRef {
public:
  explicit BoundJoinRef(JoinType join_type, std::unique_ptr<BoundTableRef> left,
                        std::unique_ptr<BoundTableRef> right,
                        std::unique_ptr<BoundExpression> condition)
      : BoundTableRef(TableReferenceType::JOIN), join_type_(join_type),
        left_(std::move(left)), right_(std::move(right)),
        condition_(std::move(condition)) {}

  JoinType join_type_;

  std::unique_ptr<BoundTableRef> left_;

  std::unique_ptr<BoundTableRef> right_;

  std::unique_ptr<BoundExpression> condition_;
};
} // namespace DaseX
