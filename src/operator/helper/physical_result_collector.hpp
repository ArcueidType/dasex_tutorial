#pragma once

#include "physical_operator.hpp"
#include <vector>
#include <memory>
#include <arrow/api.h>

namespace DaseX{

using RecordBatchPtr = std::shared_ptr<arrow::RecordBatch>;

// sink_state
class PhysicalResultCollectorLocalSinkState : public LocalSinkState {
public:
    PhysicalResultCollectorLocalSinkState() {}
public:
    std::vector<RecordBatchPtr> local_result;
    int64_t num_processed;
    int64_t size = 0;
};

// source_state
class PhysicalResultCollectorLocalSourceState : public LocalSourceState {
public:
    PhysicalResultCollectorLocalSourceState(PhysicalResultCollectorLocalSinkState &input) : local_result(&(input.local_result)) {}
public:
    std::vector<RecordBatchPtr> *local_result;
    int64_t size = 0;
};

class PhysicalResultCollector : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::RESULT_COLLECTOR;
    std::shared_ptr<PhysicalOperator> plan;
    PhysicalResultCollector(std::shared_ptr<PhysicalOperator> plan) : PhysicalOperator(PhysicalOperatorType::RESULT_COLLECTOR), plan(plan) {}
    // source
    SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const override;
    std::shared_ptr<LocalSourceState> GetLocalSourceState() const override;
    bool is_source() const override { return true; }
    bool ParallelSource() const override { return true; }

    // sink
    SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output);
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
};


} // DaseX


