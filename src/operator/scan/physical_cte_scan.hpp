/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-07 20:34:12
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-07 20:34:12
 * @FilePath: /task_sche/src/operator/scan/physical_cte_scan.hpp
 * @Description:
 */
//#pragma once
//
//#include "physical_operator.hpp"
//#include "operator_result_type.hpp"
//#include "physical_operator_states.hpp"
//#include "catalog.hpp"
//#include "pipeline.hpp"
//#include "execute_context.hpp"
//
//namespace DaseX
//{
//
//    class CTEScanGlobalSourceState : public GlobalSourceState
//    {
//    };
//
//    // 这个算子的作用就是从中间结果中读取数据，例如JOIN后的结果
//    class PhysicalCTEScan : public PhysicalOperator
//    {
//    public:
//        static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::CTE_SCAN;
//
//    public:
//        //! Table scan that immediately projects out filter columns that are unused in the remainder of the query plan
//        PhysicalCTEScan() {}
//        SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, GlobalSourceState &input) const override;
//        bool is_source() const override { return true; }
//    };
//
//} // DaseX
