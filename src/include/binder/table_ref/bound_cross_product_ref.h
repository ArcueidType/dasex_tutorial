/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-27 18:52:21
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-27 19:07:54
 * @FilePath: /task_sche-binder/src/binder/bound_cross_product_ref.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <memory>
#include <string>
#include <utility>

#include "../bound_table_ref.h"

namespace DaseX {

/**
 * A cross product. e.g., `SELECT * FROM x, y`, where `x, y` is `CrossProduct`.
 */
class BoundCrossProductRef : public BoundTableRef {
public:
  explicit BoundCrossProductRef(std::unique_ptr<BoundTableRef> left,
                                std::unique_ptr<BoundTableRef> right)
      : BoundTableRef(TableReferenceType::CROSS_PRODUCT),
        left_(std::move(left)), right_(std::move(right)) {}

  std::unique_ptr<BoundTableRef> left_;

  std::unique_ptr<BoundTableRef> right_;
};
} // namespace DaseX
