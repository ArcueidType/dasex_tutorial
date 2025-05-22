/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-12 15:35:04
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-04-01 19:38:44
 * @FilePath: /task_sche/src/operator/scan/physical_table_scan.cpp
 * @Description:
 */
#include "physical_table_scan.hpp"

namespace DaseX
{

PhysicalTableScan::PhysicalTableScan(std::shared_ptr<Table> table,
                                     int partition_id,
                                     std::vector<int> projection_ids,
                                     std::vector<std::string> names) : PhysicalOperator(PhysicalOperatorType::TABLE_SCAN),
                                     table(table),
                                     partition_id(partition_id),
                                     projection_ids(std::move(projection_ids)),
                                     names(std::move(names)) {}

SourceResultType PhysicalTableScan::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const
{
    auto &lstate = input.Cast<TableScanLocalSourceState>();
    auto weak_pipeline = execute_context->get_pipeline();
    std::shared_ptr<PartitionedTable> partitioned_table = lstate.table->partition_tables[lstate.partition_id];
    if (auto pipeline = weak_pipeline.lock())
    {
        if (lstate.idx_chunk >= partitioned_table->chunk_nums || lstate.partition_id > lstate.table->partition_num)
        {
            pipeline->temporary_chunk = partitioned_table->get_data(lstate.idx_chunk, projection_ids);
            return SourceResultType::FINISHED;
        }
        pipeline->temporary_chunk = partitioned_table->get_data(lstate.idx_chunk, projection_ids);
        lstate.idx_chunk++;
        return pipeline->temporary_chunk->num_rows() == 0 ? SourceResultType::FINISHED : SourceResultType::HAVE_MORE_OUTPUT;
    }
    else
    {
        throw std::runtime_error("Invalid PhysicalTableScan::pipeline");
    }
}

std::shared_ptr<LocalSourceState> PhysicalTableScan::GetLocalSourceState() const {
    return std::make_shared<TableScanLocalSourceState>(table, partition_id);
}

std::shared_ptr<PhysicalOperator> PhysicalTableScan::Copy(int partition_id) {
    auto expression = std::make_shared<PhysicalTableScan>(this->table, partition_id, this->projection_ids, this->names);
    for(auto &en : this->children_operator) {
        expression->children_operator.emplace_back(en->Copy(partition_id));
    }
    expression->exp_name = this->exp_name;
    return expression;
}

} // DaseX
