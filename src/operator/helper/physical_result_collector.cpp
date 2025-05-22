#include "physical_result_collector.hpp"
#include "pipeline.hpp"
#include "arrow_help.hpp"

namespace DaseX{

// source
SourceResultType PhysicalResultCollector::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const {
    return SourceResultType::FINISHED;
}

std::shared_ptr<LocalSourceState> PhysicalResultCollector::GetLocalSourceState() const {
    return std::make_shared<PhysicalResultCollectorLocalSourceState>(this->lsink_state->Cast<PhysicalResultCollectorLocalSinkState>());
}

// sink
SinkResultType PhysicalResultCollector::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) {
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<PhysicalResultCollectorLocalSinkState>();
    int count_rows = 0;
    if (auto pipeline = weak_pipeline.lock()) {
        std::shared_ptr<arrow::RecordBatch> &data_chunk = pipeline->temporary_chunk;
        if (!pipeline->is_final_chunk)
        {
            count_rows = data_chunk->num_rows();
            if (count_rows != 0)
            {
                lstate.local_result.push_back(pipeline->temporary_chunk);
                lstate.size++;
                lstate.num_processed += data_chunk->num_rows();
            }
            // spdlog::info("[{} : {}] ResultCollector data_chunk is : {}", __FILE__, __LINE__, data_chunk->ToString());
            return SinkResultType::NEED_MORE_INPUT;
        } else {
            count_rows = data_chunk->num_rows();
            if (count_rows != 0)
            {
                lstate.local_result.push_back(pipeline->temporary_chunk);
                lstate.size++;
                lstate.num_processed += data_chunk->num_rows();
            }
            // spdlog::info("[{} : {}] lstate.num_processed is : {}", __FILE__, __LINE__, lstate.num_processed);
//            std::string fileName = "Q17_" + std::to_string(pipeline->pipeline_id) + ".csv";
//            std::string filePath = "/home/cwl/data/tpch_result/" + fileName;
//            std::vector<std::shared_ptr<arrow::RecordBatch>> tmp;
//            for(int k = 0; k < lstate.local_result.size(); k++) {
//                if(lstate.local_result[k] != nullptr && lstate.local_result[k]->num_rows() != 0) {
//                    tmp.emplace_back(lstate.local_result[k]);
//                }
//            }
//            if(!tmp.empty()) {
//                Util::WriteToCSVFromRecordBatch(tmp, filePath);
//            }
            return SinkResultType::FINISHED;
        }
    } else {
        throw std::runtime_error("Invalid PhysicalResultCollector::pipeline");
    }
} // sink

std::shared_ptr<LocalSinkState> PhysicalResultCollector::GetLocalSinkState() const {
    return std::make_shared<PhysicalResultCollectorLocalSinkState>();
} // GetLocalSinkState

std::shared_ptr<PhysicalOperator> PhysicalResultCollector::Copy(int arg) {
    return std::make_shared<PhysicalResultCollector>(plan->Copy(arg));
} // Copy

} // DaseX