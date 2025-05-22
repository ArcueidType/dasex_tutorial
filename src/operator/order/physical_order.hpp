#pragma once

#include "execute_context.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "order_type.hpp"
#include <vector>
#include <memory>
#include <queue>
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <spdlog/spdlog.h>

namespace DaseX
{

// sink state
class OrderLocalSinkState : public LocalSinkState {
public:
    // 缓存所有需要排序的数据
    std::vector<std::shared_ptr<arrow::RecordBatch>> order_cache;
    // 将缓存的数据合并为一个RecordBatch，方便后续操作
    std::shared_ptr<arrow::RecordBatch> sort_data;
    // 排序后的数据索引标号，记录每行数据的局部位置
    std::shared_ptr<arrow::UInt64Array> sort_idx;
    // 排序后的实际数据
    // std::vector<std::shared_ptr<arrow::RecordBatch>> output_data;
public:
    OrderLocalSinkState() {};
};

class OrderGlobalSinkState : public GlobalSinkState {
public:
    // 缓存所有局部有序数据
    std::vector<std::shared_ptr<arrow::RecordBatch>> local_cache;
    // 局部数据索引
    std::vector<std::shared_ptr<arrow::UInt64Array>> local_sort_idx;
    std::vector<std::shared_ptr<arrow::RecordBatch>> global_sort_data;
    // orderby字段，采用列标号表示
    std::vector<int> *sort_keys;
    // 对应orderby字段的类型，与sort_keys一一对应
    std::vector<LogicalType> *key_types;
    // orderby字段排序规则，升序或降序
    std::vector<SortOrder> *orders;
public:
    OrderGlobalSinkState(std::vector<int> *sort_keys, std::vector<LogicalType> *key_types, std::vector<SortOrder> *orders) : sort_keys(sort_keys), key_types(key_types), orders(orders) {};
};

// source state
class OrderLocalSourceState : public LocalSourceState {
public:
    uint64_t idx_nums = 0; // 需要读取的RecordBatch
    // 这里必须使用指针保留位置，运行时可获取实际结果
    std::vector<std::shared_ptr<arrow::RecordBatch>> *result; // sort结果
public:
    OrderLocalSourceState(OrderGlobalSinkState &input) : result(&(input.global_sort_data)) {}
};

class PhysicalOrder : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::ORDER_BY;
    // orderby字段，采用列标号表示
    std::vector<int> *sort_keys;
    // 对应orderby字段的类型，与sort_keys一一对应
    std::vector<LogicalType> *key_types;
    // orderby字段排序规则，升序或降序
    std::vector<SortOrder> *orders;
public:
    PhysicalOrder(std::vector<int> *sort_keys, std::vector<LogicalType> *key_types, std::vector<SortOrder> *orders);
public:
    // Common interface
    // void build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) override;
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;

    // Source interface
    SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const override;
    std::shared_ptr<LocalSourceState> GetLocalSourceState() const override;
    bool is_source() const override { return true; }
    bool ParallelSource() const override { return true; }

    // Sink interface
    SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) override;
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
    std::shared_ptr<GlobalSinkState> GetGlobalSinkState() const override;
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
private:
    arrow::Status SortInLocal(OrderLocalSinkState &input);
};

} // namespace DaseX
