/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-17 16:17:41
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-24 13:20:24
 * @FilePath: /task_sche-binder/src/planner/abstract_plan.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <arrow/type_fwd.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "catalog.hpp"

namespace DaseX {

#define PLAN_NODE_CLONE_WITH_CHILDREN(cname)                                   \
  auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const      \
      -> std::unique_ptr<AbstractPlanNode>                                     \
          override {                                                           \
    auto plan_node = cname(*this);                                             \
    plan_node.children_ = children;                                            \
    return std::make_unique<cname>(std::move(plan_node));                      \
  }

enum class PlanType : uint8_t {
  SeqScan,
  IndexScan,
  Insert,
  Update,
  Delete,
  Aggregation,
  Limit,
  NestedLoopJoin,
  NestedIndexJoin,
  HashJoin,
  Filter,
  Values,
  Projection,
  Sort,
  TopN,
  MockScan
};

class AbstractPlanNode;
using AbstractPlanNodeRef = std::shared_ptr<AbstractPlanNode>;

/** base class of plan node*/
class AbstractPlanNode {
public:
  AbstractPlanNode(std::shared_ptr<const arrow::Schema> output_schema,
                   std::vector<AbstractPlanNodeRef> children)
      : output_schema_(std::move(output_schema)),
        children_(std::move(children)) {}

  virtual ~AbstractPlanNode() = default;

  /** @return the schema for the output of this plan node */
  auto OutputSchema() const -> const arrow::Schema & { return *output_schema_; }

  /** @return the child of this plan node at index child_idx */
  auto GetChildAt(int child_idx) const -> AbstractPlanNodeRef {
    return children_[child_idx];
  }

  /** @return the children of this plan node */
  auto GetChildren() const -> const std::vector<AbstractPlanNodeRef> & {
    return children_;
  }

  /** @return the type of this plan node */
  virtual auto GetType() const -> PlanType = 0;

  /** @return the cloned plan node with new children */
  virtual auto
  CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const
      -> std::unique_ptr<AbstractPlanNode> = 0;

  /**
   * The schema for the output of this plan node. In the volcano model, every
   * plan node will spit out tuples, and this tells you what schema this plan
   * node's tuples will have.
   */
  std::shared_ptr<const arrow::Schema> output_schema_;

  /** children */
  std::vector<AbstractPlanNodeRef> children_;

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
  
  /** Debug */
  virtual auto ToString() -> std::string { return ""; }

  auto PrintTree() -> std::string {
    std::string result;
    return PrintTree(result, 0);
  }

  auto PrintTree(std::string &result, size_t padding = 0) -> std::string {
    std::string current;
    current = "->" + ToString();
    result += current;
    padding += 2;
    for (size_t i = 0; i < children_.size(); i++) {
      result += '\n';
      result += std::string(padding, ' ');
      children_[i]->PrintTree(result, padding);
    }
    return result;
  }
};

} // namespace DaseX
