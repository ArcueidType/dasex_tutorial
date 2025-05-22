#include "query_handler.hpp"
#include <memory>
#include "binder.h"
#include "bound_statement.hpp"
#include "optimizer.hpp"
#include "physical_result_collector.hpp"
#include "pipe.h"
#include "planner.hpp"
#include "time_help.hpp"
#include "output_util.hpp"

namespace DaseX {

RC QueryHandler::Query(std::string sql, bool verbose /*true*/)
{
    Binder binder(open_catalog_ptr);
	binder.ParseAndSave(sql);
    auto scheduler = std::make_shared<Scheduler>(72);
    assert(binder.statement_nodes_.size() == 1);

    duckdb_libpgquery::PGNode* stmt = binder.statement_nodes_[0];

    std::unique_ptr<BoundStatement> statement = binder.BindStatement(stmt);

    RC rc = RC::SUCCESS;
    arrow::RecordBatchVector *res;

    switch (statement->type_) {
        case StatementType::CREATE_STATEMENT: {
            const auto &create_stmt =
                dynamic_cast<const CreateStatement &>(*statement);
            rc = binder.ExecuteDDL(create_stmt);
            break;
        }
        case StatementType::SELECT_STATEMENT: {
            Planner planner(open_catalog_ptr);
            const auto &select_stmt =
                dynamic_cast<const SelectStatement &>(*statement);

            planner.PlanQuery(select_stmt);
            AbstractPlanNodeRef plan = planner.plan_;
            if (verbose)
            {
                std::cerr << "Plan before optimized\n" << plan->PrintTree() << '\n';
            }
            Optimizer optimizer(binder);
            plan = optimizer.Optimize(std::move(plan));
            if (verbose)
            {
                std::cerr << "Plan after optimized\n" << plan->PrintTree() << '\n';
            }
            auto pipe = Pipe(72);
            auto physical_ops = pipe.BuildPipe(plan);

            // 如果 project 算子在最顶层，则加一个 ResultCollector 算子
            if (physical_ops[0]->type == PhysicalOperatorType::PROJECTION) {
                auto root = std::make_shared<PhysicalResultCollector>(
                    physical_ops[0]);
                root->children_operator = {physical_ops[0]};
                physical_ops[0] = std::move(root);
            }

            std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>();
            std::shared_ptr<PipelineGroup> pipeline_group = std::make_shared<PipelineGroup>();
            
            physical_ops[0]->build_pipelines(pipeline, pipeline_group);
            
            std::shared_ptr<PipelineGroupExecute> pipeline_group_executor = std::make_shared<PipelineGroupExecute>(scheduler, pipeline_group);
            
            pipeline_group_executor->traverse_plan();
            
            pipeline_group_executor->execute();
            scheduler->shutdown();
            // FIXME: 这里只能拿到最后一个chunk的数据，奇怪呀
            res = physical_ops[0]->GetLocalSourceState()->Cast<PhysicalResultCollectorLocalSourceState>().local_result;
            if (res) {
                 DisplayChunk(*res, 10);
                result_temp.result_chunks = *res;
                result_temp.status = RC::SUCCESS;
            } else {
                result_temp.status = RC::NO_RESULT;
            }
            
            rc = RC::SUCCESS;
            break;
        }
        default:
            rc = RC::INVALID_DATA;
    }
	binder.statement_nodes_.clear();
    return rc;
}

void QueryResult::print_result() {
    DisplayChunk(result_chunks, 5);
}

int QueryResult::get_row_count() { 
    int row_count = 0;
    for (auto &chunk : result_chunks) {
        row_count += chunk->num_rows();
    }
    return row_count; 
}

}  // namespace DaseX