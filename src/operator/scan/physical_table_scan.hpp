#pragma once

#include "catalog.hpp"
#include "execute_context.hpp"
#include "operator_result_type.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "pipeline.hpp"

namespace DaseX
{

class TableScanLocalSourceState : public LocalSourceState {
public:
    uint64_t idx_chunk = 0;		   // 需要读取的Chunk块
    int partition_id = -1;			   // 表分区
    std::shared_ptr<Table> table; // 需要读取数据的表
public:
    TableScanLocalSourceState(std::shared_ptr<Table> table, int partition_id) : table(table), partition_id(partition_id) {}
};

class PhysicalTableScan : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::TABLE_SCAN;
    std::shared_ptr<Table> table; // 需要读取数据的表
    int partition_id;			   // 表分区
    std::vector<int> projection_ids;
    std::vector<std::string> names;
public:
    //! Table scan that immediately projects out filter columns that are unused in the remainder of the query plan
    PhysicalTableScan(std::shared_ptr<Table> table, int partition_id = -1, std::vector<int> projection_ids = {},std::vector<std::string> names = {});
    SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const override;
    bool is_source() const override { return true; }

    std::shared_ptr<LocalSourceState> GetLocalSourceState() const override;
    bool ParallelSource() const override { return true; }
    std::shared_ptr<PhysicalOperator> Copy(int partition_id) override;
};

} // DaseX
