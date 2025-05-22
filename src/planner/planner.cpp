/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-20 15:28:37
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-15 14:33:13
 * @FilePath: /task_sche_parser/src/planner/planner.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "planner.hpp"
#include "exception.h"

namespace DaseX {

void Planner::PlanQuery(const BoundStatement &statement) {
  switch (statement.type_) {
  // sleect handler
  case StatementType::SELECT_STATEMENT: {
    plan_ = PlanSelect(dynamic_cast<const SelectStatement &>(statement));
//    printf("children size : %d\n", plan_->children_.size());
    return;
  }
  // insert handler
  case StatementType::INSERT_STATEMENT: {
    // plan_ = PlanInsert(dynamic_cast<const InsertStatement &>(statement));
    plan_ = nullptr;
    return;
  }
  default:
    throw DaseX::Exception("the statement {} is not supported in planner yet");
  }
}

} // namespace DaseX