/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 15:52:00
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-14 20:10:30
 * @FilePath: /task_sche/src/binder/bound_star.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "../bound_expression.h"
#include "exception.h"
#include <string>
#include <utility>

namespace DaseX {

/**
 * The star in SELECT list, e.g. `SELECT * FROM x`.
 */
class BoundStar : public BoundExpression {
public:
  BoundStar() : BoundExpression(ExpressionType::STAR) {}
};
} // namespace DaseX