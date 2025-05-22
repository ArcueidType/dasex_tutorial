/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-12 15:29:50
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-12 15:41:33
 * @FilePath: /task_sche/src/operator/physical_operator.hpp
 * @Description:
 */
#pragma once

#include "execute_context.hpp"
#include "logical_type.hpp"
#include "operator_result_type.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "pipeline_group.hpp"
#include <memory>
#include <vector>
#include <stdexcept>


namespace DaseX
{

class PipelineGroup;
//! PhysicalOperator is the base class of the physical operators present in the
//! execution plan
class PhysicalOperator : public std::enable_shared_from_this<PhysicalOperator>
{
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::INVALID;
    PhysicalOperatorType type;
    //! The set of children of the operator
    std::vector<std::shared_ptr<PhysicalOperator>> children_operator;
    // 算子返回类型
    std::vector<LogicalType> types;
    // 方便设置sink的输出状态，也方便pipeline与pipeline之间的连接
    std::shared_ptr<LocalSinkState> lsink_state;
    //! The global sink state of this operator
    std::shared_ptr<GlobalSinkState> gsink_state;
    std::string exp_name;

public:
    // common interface
    PhysicalOperator(PhysicalOperatorType type_) : type(type_) {}
    virtual std::string get_name() const { return physical_operator_to_string(type); }
    virtual std::vector<std::shared_ptr<PhysicalOperator>> get_children_operator() const { return children_operator; }
    virtual bool is_final_operator() const { return false; };
    virtual void build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup);
    virtual std::shared_ptr<PhysicalOperator> getptr() { return shared_from_this(); }
    virtual void AddChild(std::shared_ptr<PhysicalOperator> child);
    virtual std::shared_ptr<PhysicalOperator> Copy(int arg) = 0;
public:
    // Source interface
    virtual SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const
    {
        throw std::runtime_error("Calling GetData on a node that is not a source!");
    }

    virtual std::shared_ptr<LocalSourceState> GetLocalSourceState() const;

    virtual bool is_source() const { return false; }

    virtual bool ParallelSource() const { return false; }

public:
    // Operator interface
    virtual OperatorResultType execute_operator(std::shared_ptr<ExecuteContext> &execute_context) {
        throw std::runtime_error("Calling Execute on a node that is not an operator!");
    }
    virtual bool is_operator() const { return false; }
    virtual bool ParallelOperator() const { return false; }

public:
    // Sink interface
    virtual SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output)
    {
        throw std::runtime_error("Calling Sink on a node that is not a sink!");
    }
    virtual std::shared_ptr<LocalSinkState> GetLocalSinkState() const;
    virtual std::shared_ptr<GlobalSinkState> GetGlobalSinkState() const;
    virtual bool is_sink() const { return false; }
    virtual bool ParallelSink() const { return false; }

public:
    template <typename TARGET>
    TARGET &Cast()
    {
        if (TARGET::TYPE != PhysicalOperatorType::INVALID && type != TARGET::TYPE)
        {
            throw std::runtime_error("Failed to cast physical operator to type - physical operator type mismatch");
        }
        return reinterpret_cast<TARGET &>(*this);
    }

    template <typename TARGET>
    const TARGET &Cast() const
    {
        if (TARGET::TYPE != PhysicalOperatorType::INVALID && type != TARGET::TYPE)
        {
            throw std::runtime_error("Failed to cast physical operator to type - physical operator type mismatch");
        }
        return reinterpret_cast<const TARGET &>(*this);
    }
};

} // DaseX
