#include "pipeline.hpp"
#include "physical_radix_join.hpp"
#include "physical_hash_join.hpp"
#include "physical_nested_loop_join.hpp"
#include "physical_table_scan.hpp"
#include "physical_hash_aggregation.hpp"
#include "physical_hash_aggregation_with_multifield.hpp"
#include <utility>

namespace DaseX
{

Pipeline::Pipeline() :
                   input(nullptr),
                   output(nullptr),
                   source(nullptr),
                   sink(nullptr),
                   temporary_chunk(nullptr),
                   is_final_chunk(false),
                   is_finish(false) {}

Pipeline::Pipeline(std::shared_ptr<LocalSourceState> input_, std::shared_ptr<LocalSinkState> output_) : input(std::move(input_)),
                                                                         output(std::move(output_)),
                                                                         source(nullptr),
                                                                         sink(nullptr),
                                                                         temporary_chunk(nullptr),
                                                                         is_final_chunk(false),
                                                                         is_finish(false) {}

void Pipeline::add_dependency(std::shared_ptr<Pipeline> parent)
{
    this->parent = parent;
}

void Pipeline::set_child(std::shared_ptr<Pipeline> child) {
    this->child = child;
}

PipelineResultType Pipeline::execute_pipeline()
{
	// Step 1: 通过状态来控制中间算子的执行，3类算子分别对应3种类型的状态，通过状态决定最终Pipeline的执行结果
    PipelineResultType result = PipelineResultType::FINISHED;
    SourceResultType source_result = SourceResultType::FINISHED;
    OperatorResultType operator_result = OperatorResultType::FINISHED;
    SinkResultType sink_result = SinkResultType::FINISHED;
	// Step 2: 内部为一个死循环（也可以用while），不断调用source获取数据块，然后调用operator进行处理，最终结果由sink产生
    for (uint64_t i = 0; i < max_chunks; i++)
    {
        if (source == nullptr)
        {
            break;
        }
		// Step 4: 调用Source的相关函数获取一个数据块，数据块保存在Pipeline的temporary_chunk中，同一个Pipeline中的算子均可访问temporary_chunk
		//         所以，本质上接下来的算子处理逻辑都是针对temporary_chunk。
		//         这里也可以进行优化，将temporary_chunk设计为一个能容纳多个数据块的缓存数组
        source_result = source->get_data(execute_context, *input);
		// Step 5: 根据Soure的执行结果决定下一步处理，
		//         Source执行通常有3中结果：1. HAVE_MORE_OUTPUT,正常逻辑，表示源头已经读取到一个数据块；
		//                              2. FINISHED, 表示源头已经没有数据
		//                              3. BLOCKED, 阻塞，用于更灵活的任务调度，暂时没有使用
        switch (source_result)
        {
		// Step 5.1: HAVE_MORE_OUTPUT表示Pipeline正常执行，循环继续
        case SourceResultType::HAVE_MORE_OUTPUT:
            result = PipelineResultType::HAVE_MORE_OUTPUT;
            break;
		// Step 5.2: FINISHED表示Pipeline执行完成，设置Pipeline结束标志以及是否最后一个数据块的标志，
		//           这些标志在后续算子或PipelineGroup中使用
        case SourceResultType::FINISHED:
            result = PipelineResultType::FINISHED;
            this->is_finish = true;
            this->is_final_chunk = true;
            break;
        case SourceResultType::BLOCKED:
            result = PipelineResultType::BLOCKED;
            break;
        default:
            break;
        }
		// Step 5.3: 执行Operator部分
        if (operators.size() != 0)
        {
            //  for (int i = 0; i < operators.size(); i++)
            for (int i = (operators.size() - 1); i >= 0; i--)
            {
                execute_context->operator_idx = i;
				//  Step 5.5: 调用Operator算子相关方法，
				//  Operator执行有4种状态：1. HAVE_MORE_OUTPUT: 正常处理，表示处理完一个数据块，并将结果写入temporary_chunk
				//                      2. NO_MORE_OUTPUT: 正常处理，表示处理完一个数据块，但是没有结果输出（例如Filter，过滤后没有数据满足条件，因此输出为NULL）
				//                      3. FINISHED: 结束处理
				//                      4. BLOCKED: 阻塞，同Source，暂无使用
				// TODO: 这里需要补充代码
                operator_result = operators[i]->execute_operator(execute_context);
                switch (operator_result) {
                    case OperatorResultType::HAVE_MORE_OUTPUT:
                        if (source_result == SourceResultType::FINISHED) {
                            result = PipelineResultType::FINISHED;
                        } else {
                            result = PipelineResultType::HAVE_MORE_OUTPUT;
                        }
                        break;
                    case OperatorResultType::NO_MORE_OUTPUT:
                        if (source_result == SourceResultType::FINISHED) {
                            result = PipelineResultType::FINISHED;
                        } else {
                            result = PipelineResultType::NO_MORE_OUTPUT;
                        }
                        break;
                    case OperatorResultType::FINISHED:
                        result = PipelineResultType::FINISHED;
                        break;
                    case OperatorResultType::BLOCKED:
                        result = PipelineResultType::BLOCKED;
                        break;
                }
				// Step 5.6: 根据Operator的状态可以提前中止后续的计算，如果状态为NO_MORE_OUTPUT，后续算子其实没必要再次执行
				// TODO: 这里需要补充代码
                if (operator_result == OperatorResultType::NO_MORE_OUTPUT) {
                    break;
                }
            } // for
        }     // operators
		// Step 5.7: 如果Source结束，Operator为NO_MORE_OUTPUT，说明已经没有数据，结束循环。Push模型必然会多处理一个空数据块，然后才知道结束数据处理。
		// TODO: 这里需要补充代码
        if (operator_result == OperatorResultType::NO_MORE_OUTPUT && source_result != SourceResultType::FINISHED) {
            continue;
        }
		// Step 5.8: 执行Sink算子，Pipeline可以没有Sink，因此要先判断Sink是否存在
        if(sink) {
			// Step 5.9: 调用Sink相关函数
			// TODO: 这里需要补充代码
            sink_result = sink->sink(execute_context, *output);
			// Step 5.10: 同样Sink的执行也对应3种状态：1. NEED_MORE_INPUT：正常执行；
			//                                    2. FINISHED: 正常结束；
			//									  3. BLOCKED: 阻塞，同Source，暂无使用
			// TODO: 这里需要补充代码
            switch (sink_result) {
                case SinkResultType::NEED_MORE_INPUT:
                    if (source_result == SourceResultType::FINISHED) {
                            result = PipelineResultType::FINISHED;
                        } else {
                            result = PipelineResultType::NEED_MORE_INPUT;
                        }
                        break;
                case SinkResultType::FINISHED:
                    result = PipelineResultType::FINISHED;
                    break;
                case SinkResultType::BLOCKED:
                    result = PipelineResultType::BLOCKED;
                    break;
            }
        }
		// Step 5.11: 判断Pipeline是否结束
        if (result == PipelineResultType::FINISHED)
        {
            auto pipeline_group = this->get_pipeline_group();
			// Step 5.12: 当前Pipeline结束，在PipelineGroup中增加统计结束Pipeline的计数
            if (auto pipe_group = pipeline_group.lock())
            {
                pipe_group->add_count();
            }
            else
            {
                throw std::runtime_error("Invalid PhysicalRadixJoin::pipeline");
            }
            break;
        }
    } // for
    return result;
} // execute_pipeline

void Pipeline::Initialize(int pipe_idx, int work_id) {
	// Step 1: 设置并行度
	// 如果是根pipeline，根据传入的work_id来设置
	// 如果不是，只需要从依赖的pipeline中获取work_id即可
	// 注意:目前所有pipeline_task有统一的并行度，如果要支持灵活的并行配置的话，这里的逻辑需要重新修改
    if(unlikely(work_id != -1)) {
        this->work_id = work_id;
    } else {
        this->work_id = this->parent->work_id;
    }
    if(auto weak_pipelines = pipelines.lock()) {
        is_root = weak_pipelines->is_root;
    }
	// Step 2: 为Pipeline配置Source，并初始化本地Source状态
	// 如果Source为TABLE_SCAN类型，初始化PhysicalTableScan算子，
	// 否则，将父Pipeline的输出设置为当前Pipeline的输入
    if(source->type == PhysicalOperatorType::TABLE_SCAN) {
        auto &lsource = (*source).Cast<PhysicalTableScan>();
        lsource.partition_id = work_id;
        input = lsource.GetLocalSourceState();
    } else {
        // Copy Pipeline主要初始化Source,在这里就可以设置lsink_state
        source->lsink_state = parent->output;
        input = source->GetLocalSourceState();
    }
	// Step 3: 初始化Join的状态，Join比较特殊，需要将Build的结果赋值给Probe
	// 记录算子在Pipeline中的位置信息
    int count_pg = 0;
    int operators_size = operators.size();
    for(int i = (operators_size - 1) ; i >=0 ; i--) {
        switch (operators[i]->type) {
			// Step 4: 为Join算子配置哈希表信息，Probe阶段需要访问Build阶段的哈希表
            case PhysicalOperatorType::HASH_JOIN:
            {
                auto &join_op = (*operators[i]).Cast<PhysicalHashJoin>();
				// Step 5: 为Join算子配置哈希表信息，Probe阶段需要访问Build阶段的哈希表，
				//         在PipelineGroup中，可能依赖两条PipelineGroup，因此要遍历所有PipelineGroup并找到Sink为Join类型的父Pipeline，
                if(auto weak_pipelines = pipelines.lock()) {
                    auto &depend_pg = weak_pipelines->dependencies;
                    auto &cur_pg = depend_pg[count_pg];
                    auto &cur_pipe = cur_pg->pipelines[pipe_idx];
                    if(cur_pipe->sink->type != PhysicalOperatorType::HASH_JOIN) {
                        count_pg++;
                        auto &next_pg = depend_pg[count_pg];
                        auto &next_pipe = next_pg->pipelines[pipe_idx];
                        join_op.lsink_state = next_pipe->output;
                    } else {
                        join_op.lsink_state = cur_pipe->output;
                    }
                }
                count_pg++;
                break;
            }
            case PhysicalOperatorType::NESTED_LOOP_JOIN:
            {
                auto &join_op = (*operators[i]).Cast<PhysicalNestedLoopJoin>();
                if(auto weak_pipelines = pipelines.lock()) {
                    auto &depend_pg = weak_pipelines->dependencies;
                    auto &cur_pg = depend_pg[count_pg];
                    auto &cur_pipe = cur_pg->pipelines[pipe_idx];
                    if(cur_pipe->sink->type != PhysicalOperatorType::NESTED_LOOP_JOIN) {
                        count_pg++;
                        auto &next_pg = depend_pg[count_pg];
                        auto &next_pipe = next_pg->pipelines[pipe_idx];
                        join_op.lsink_state = next_pipe->output;
                    } else {
                        join_op.lsink_state = cur_pipe->output;
                    }
                }
                count_pg++;
                break;
            }
            default:
                break;
        }
    }
	// Step 6: 为Pipeline配置Sink，并初始化本地Sink状态
	// 如果pipeline有sink，就需要为Pipeline设置输出
    if(sink) {
        switch (sink->type) {
            case PhysicalOperatorType::RADIX_JOIN:
            {
                if((*sink).Cast<PhysicalRadixJoin>().is_build) {
                    output = this->sink->GetLocalSinkState();
                    sink->lsink_state = output;
                } else {
                    output = this->parent->sink->lsink_state;
                }
                break;
            }
			// Step 7: 为无字段GROUP_BY类型Sink初始化状态
            case PhysicalOperatorType::HASH_GROUP_BY:
            {
				// Step 7.1: 获取当前Pipeline的Sink，并构建对应的状态
				// TODO: 这里需要补充代码
                output = this->sink->GetLocalSinkState();
                auto &output_p = (*output).Cast<AggLocalSinkState>();
				// Step 7.2: 初始化Sink算子的状态信息
				// TODO: 这里需要补充代码
                int partition_nums = 0;
                if (auto pipe_group = this->pipelines.lock()) {
                    partition_nums = pipe_group->parallelism;
                }
                output_p.partition_nums = partition_nums;
                output_p.partitioned_hash_tables.resize(partition_nums);
				// Step 7.3: 将输出结果记录到Sink中，在子Pipeline中作为Source使用
				// TODO: 这里需要补充代码
                sink->lsink_state = output;
                break;
            }
            case PhysicalOperatorType::MULTI_FIELD_HASH_GROUP_BY:
            {
                output = this->sink->GetLocalSinkState();
                auto &output_p = (*output).Cast<MultiFieldAggLocalSinkState>();
                int partition_nums = 0;
                if(auto pipe_group = this->pipelines.lock()) {
                    partition_nums = pipe_group->parallelism;
                }
                output_p.partition_nums = partition_nums;
                output_p.partition_table->Initialize(partition_nums);
                sink->lsink_state = output;
                break;
            }
            default:
            {
				// Step 8: 为一般情况下的Pipeline配置输出，即为Sink算子的输出状态
                output = this->sink->GetLocalSinkState();
				// Step 9: 将输出结果记录到Sink中，在子Pipeline中作为Source使用
                sink->lsink_state = output;
                break;
            }
        } // switch
    }
}  // Initialize

std::shared_ptr<Pipeline> Pipeline::Copy(int work_id) {
	// Step 1: 构建一个新Pipeline即执行上下文
	auto new_pipeline = std::make_shared<Pipeline>();
	std::shared_ptr<ExecuteContext> new_execute_context = std::make_shared<ExecuteContext>();
	// Step 2: 复制Pipeline所属PipelineGroup
	// TODO: 这里需要补充代码
    new_pipeline->pipelines = this->pipelines;
	// Step 3: 复制Pipeline的Source（调用对应的Operator::Copy()方法，无需同学们实现，下面同理）
	// TODO: 这里需要补充代码
    new_pipeline->source = this->source->Copy(work_id);
	// Step 4: 复制Pipeline的Operator
	// TODO: 这里需要补充代码
    for (auto &op : this->operators) {
        new_pipeline->operators.emplace_back(op->Copy(work_id));
    }
	// Step 5: 复制Pipeline的Sink
	// TODO: 这里需要补充代码
    if (this->sink) {
        new_pipeline->sink = this->sink->Copy(work_id);
    } else {
        new_pipeline->sink = nullptr;
    }
	// Step 6: 复制其它执行信息
	// TODO: 这里需要补充代码
    new_pipeline->work_id = work_id;
    new_pipeline->is_final_chunk = this->is_final_chunk;
    new_pipeline->is_finish = this->is_finish;
    new_pipeline->is_root = this->is_root;
	// Step 7: 关联执行上下文
    new_execute_context->set_pipeline(new_pipeline);
    return new_pipeline;
}

} // DaseX
