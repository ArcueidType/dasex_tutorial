//
// Created by root on 24-3-13.
//

#pragma  once
#include "abstract_expression.h"
#include "pipeline.hpp"
#include "pipeline_group.hpp"
#include "physical_operator.hpp"
#include "physical_operator_type.hpp"
#include "planner.hpp"

namespace DaseX
{

class PhysicalOperator;
class ExecuteContext;

class Pipe {
public:

    explicit Pipe(int parallel_num)
            : parallel_num_(parallel_num) {}

    std::shared_ptr<PhysicalOperator>  root;

    std::shared_ptr<PhysicalOperator>  child;

    Pipe *next;

    std::vector<std::shared_ptr<PhysicalOperator>>  parents;

    std::vector<std::shared_ptr<PhysicalOperator>>  physical_ops;

    int parallel_num_ = 72;

    // return std::shared_ptr<const Pipe>
    auto BuildPipe(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;

    auto BuildPhysicalProjection(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;
    auto BuildPhysicalNestedLoopJoin(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;
    auto BuildPhysicalSeqScan(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;
    auto BuildPhysicalFilter(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;
    auto BuildPhysicalAggregation(const AbstractPlanNodeRef& plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;
    auto BuildPhysicalSort(const AbstractPlanNodeRef &plan) -> std::vector<std::shared_ptr<PhysicalOperator>>;

private:
    auto IsCrossProduct(const AbstractExpressionRef &predicate) -> bool;
};


}

