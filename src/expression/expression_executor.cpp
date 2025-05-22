#include "expression_executor.hpp"
#include "logical_type.hpp"
#include "value_operations.hpp"
#include "common_macro.hpp"
#include "arrow_help.hpp"
#include "vector_operations.hpp"
#include <spdlog/spdlog.h>

namespace DaseX
{
    void ExpressionExecutor::SetExecutorState(std::shared_ptr<ExpressionExecutorState> &state_)
    {
        state_->executor = this->GetPtr();
        this->states.push_back(state_) ;
    }

    void ExpressionExecutor::SetExecutorStates(std::vector<std::shared_ptr<ExpressionExecutorState>> &states) {
        for(int i = 0; i < states.size(); i++) {
            SetExecutorState(states[i]);
            Initialize(*(expressions[i]), *(states[i]));
        }
    }

    void ExpressionExecutor::Initialize(Expression &expr, ExpressionExecutorState &state)
    {
        state.root_state = InitializeState(expr, state);
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(Expression &expr, ExpressionExecutorState &state)
    {
        switch (expr.expression_class)
        {
        case ExpressionClass::BOUND_COMPARISON:
            return InitializeState(expr.Cast<BoundComparisonExpression>(), state);
        case ExpressionClass::BOUND_CONJUNCTION:
            return InitializeState(expr.Cast<BoundConjunctionExpression>(), state);
        case ExpressionClass::BOUND_FUNCTION:
            return InitializeState(expr.Cast<BoundFunctionExpression>(), state);
        case ExpressionClass::BOUND_PROJECT_FUNCTION:
                return InitializeState(expr.Cast<BoundProjectFunctionExpression>(), state);
        case ExpressionClass::BOUND_CONSTANT:
            return InitializeState(expr.Cast<BoundConstantExpression>(), state);
        case ExpressionClass::BOUND_COLUMN_REF:
            return InitializeState(expr.Cast<BoundColumnRefExpression>(), state);
        case ExpressionClass::BOUND_IN:
            return InitializeState(expr.Cast<BoundInExpression>(), state);
        case ExpressionClass::BOUND_CASE:
            return InitializeState(expr.Cast<BoundCaseExpression>(), state);
        default:
            throw std::runtime_error("Attempting to initialize state of expression of unknown type!");
        }
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundComparisonExpression &expr, ExpressionExecutorState &state)
    {
        auto result = std::make_shared<ExpressionState>(std::make_shared<BoundComparisonExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        result->AddChild(*(expr.left), state);
        result->AddChild(*(expr.right), state);
        return result;
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundConjunctionExpression &expr, ExpressionExecutorState &state)
    {
        auto result = std::make_shared<ExpressionState>(std::make_shared<BoundConjunctionExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        for (auto &child : expr.children)
        {
            result->AddChild(*child, state);
        }
        return result;
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundConstantExpression &expr, ExpressionExecutorState &state)
    {
        return std::make_shared<ExpressionState>(std::make_shared<BoundConstantExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
    }
    
    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundColumnRefExpression &expr, ExpressionExecutorState &state)
    {
        return std::make_shared<ExpressionState>(std::make_shared<BoundColumnRefExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundFunctionExpression &expr, ExpressionExecutorState &state) {
        auto result = std::make_shared<ExpressionState>(std::make_shared<BoundFunctionExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        result->AddChild(*(expr.children[0]), state);
        result->AddChild(*(expr.children[1]), state);
        return result;
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundProjectFunctionExpression &expr, ExpressionExecutorState &state) {
        auto result = std::make_shared<ExpressionState>(std::make_shared<BoundProjectFunctionExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        result->AddChild(*(expr.children[0]), state);
        result->AddChild(*(expr.children[1]), state);
        return result;
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundInExpression &expr, ExpressionExecutorState &state) {
        auto result = std::make_shared<ExpressionState>(std::make_shared<BoundInExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        result->AddChild(*(expr.left), state);
        return result;
    }

    std::shared_ptr<ExpressionState> ExpressionExecutor::InitializeState(BoundCaseExpression &expr, ExpressionExecutorState &state) {
        auto result = std::make_shared<CaseExpressionState>(std::make_shared<BoundCaseExpression>(expr),std::make_shared<ExpressionExecutorState>(state));
        int size = expr.when_exprs.size();
        for(int i = 0; i < size; i++) {
            result->AddChild(*(expr.when_exprs[i]), state);
            result->AddChild(*(expr.then_exprs[i]), state);
        }
        result->AddChild(*(expr.else_expr), state);
        return result;
    }

    int ExpressionExecutor::SelectExpression(std::shared_ptr<arrow::RecordBatch> &input, std::vector<int> &sel)
    {
        SetChunk(input);
        if(auto state_weak = states[0].lock()) {
            int selected_tuples = Select(*expressions[0], state_weak->root_state, input->num_rows(), sel);
            return selected_tuples;
        } else {
            throw std::runtime_error("ExpressionExecutorState Object no longer exists");
        }
    } // SelectExpression

    int ExpressionExecutor::Select(Expression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel)
    {
        if (count == 0)
        {
            return 0;
        }
        switch (expr.expression_class)
        {
        case ExpressionClass::BOUND_COMPARISON:
            return Select(expr.Cast<BoundComparisonExpression>(), state, count, sel);
        case ExpressionClass::BOUND_CONJUNCTION:
            return Select(expr.Cast<BoundConjunctionExpression>(), state, count, sel);
        case ExpressionClass::BOUND_FUNCTION:
            return Select(expr.Cast<BoundFunctionExpression>(), state, count, sel);
        case ExpressionClass::BOUND_IN:
            return Select(expr.Cast<BoundInExpression>(), state, count, sel);
        default:
            throw std::runtime_error("Attempting to execute state of expression of unknown type!");
        }
    } // Select

    int ExpressionExecutor::Select(BoundComparisonExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel)
    {
        // 1.从state中取出左边的表达式值  对应列数据
        // 2.从state中取出右边的表达式值  比较值
        // 3.根据类型进行比较
        auto &left = state->left_column;
        auto &right = state->right_column;
        Execute(*expr.left, state->child_states[0], count, sel, left);
        Execute(*expr.right, state->child_states[1], count, sel, right);
        switch (expr.type)
        {
        case ExpressionTypes::COMPARE_EQUAL:
            return VectorOperations::Equals(left, right, count, sel);
        case ExpressionTypes::COMPARE_NOTEQUAL:
            return VectorOperations::NotEquals(left, right, count, sel);
        case ExpressionTypes::COMPARE_LESSTHAN:
            return VectorOperations::LessThan(left, right, count, sel);
        case ExpressionTypes::COMPARE_GREATERTHAN:
            return VectorOperations::GreaterThan(left, right, count, sel);
        case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
            return VectorOperations::LessThanEquals(left, right, count, sel);
        case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
            return VectorOperations::GreaterThanEquals(left, right, count, sel);
        default:
            throw std::runtime_error("Unknown comparison type!");
        }
    } // Select

    int ExpressionExecutor::Select(BoundFunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel) {
        auto &left = state->left_column;
        Execute(expr, state, count, sel, left);
        int cur_count = sel.size();
        return cur_count;
    }

    int ExpressionExecutor::Select(BoundConjunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel)
    {
        int current_count = count;
        if(expr.type == ExpressionTypes::CONJUNCTION_AND) {
            for (int i = 0; i < expr.children.size(); i++)
            {
                int tcount = Select(*expr.children[i], state->child_states[i], current_count, sel);
                current_count = tcount;
                if (current_count == 0)
                {
                    break;
                }
            }
        } else {
            std::vector<int> last_sel;
            std::vector<int> curr_sel;
            for (int i = 0; i < expr.children.size(); i++)
            {
                int tcount = Select(*expr.children[i], state->child_states[i], count, curr_sel);
                if(!last_sel.empty()) {
                    sel = Util::MergeTwoSortedArrays(last_sel, curr_sel);
                    current_count = sel.size();
                } else {
                    sel = curr_sel;
                    current_count = tcount;
                }
                last_sel = curr_sel;
                curr_sel.clear();
//                if (current_count == 0)
//                {
//                    break;
//                }
            }
        }
        return current_count;
    }

    // in表达式
    int ExpressionExecutor::Select(BoundInExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel) {
        auto &left = state->left_column;
        Execute(*expr.left, state->child_states[0], count, sel, left);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (expr.value_type)
        {
            case LogicalType::INTEGER:
            {
                auto int32array = std::static_pointer_cast<arrow::Int32Array>(left);
                if (!sel.empty()) {
                    for (int i = 0; i < count; i++)
                    {
                        Value tar_p(int32array->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0)
                        break;
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                } else {
                    for(int i = 0; i < count; i++) {
                        Value tar_p(int32array->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            } // case LogicalType::INTEGER:
            case LogicalType::FLOAT:
            {
                auto floatArray = std::static_pointer_cast<arrow::FloatArray>(left);
                if (!sel.empty()) {
                    for (int i = 0; i < count; i++)
                    {
                        Value tar_p(floatArray->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0)
                        break;
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                } else {
                    for(int i = 0; i < count; i++) {
                        Value tar_p(floatArray->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            } // case LogicalType::FLOAT:
            case LogicalType::DOUBLE:
            {
                auto doubleArray = std::static_pointer_cast<arrow::DoubleArray>(left);
                if (!sel.empty()) {
                    for (int i = 0; i < count; i++)
                    {
                        Value tar_p(doubleArray->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0)
                        break;
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                } else {
                    for(int i = 0; i < count; i++) {
                        Value tar_p(doubleArray->Value(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            } // case LogicalType::DOUBLE:
            case LogicalType::STRING:
            {
                auto stringArray = std::static_pointer_cast<arrow::StringArray>(left);
                if (!sel.empty()) {
                    for (int i = 0; i < count; i++)
                    {
                        Value tar_p(stringArray->GetString(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0) {
                        sel.clear();
                        break;
                    }
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                } else {
                    for(int i = 0; i < count; i++) {
                        Value tar_p(stringArray->GetString(i));
                        auto &vec = expr.value_set;
                        auto it = std::find(vec.begin(), vec.end(), tar_p);
                        if(it != vec.end()) {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            } // case LogicalType::STRING:
            default:
                throw std::runtime_error("Unsupport data type!");
        } // switch
        return res;
    }

    void ExpressionExecutor::Execute(Expression &expr,
                                     std::shared_ptr<ExpressionState> state,
                                     int count,
                                     std::vector<int> &sel,
                                     std::shared_ptr<arrow::Array> &result)
    {

        if (count == 0)
        {
            return;
        }
        switch (expr.expression_class)
        {
        case ExpressionClass::BOUND_COLUMN_REF:
            Execute(expr.Cast<BoundColumnRefExpression>(), state, count, sel, result);
            break;
        case ExpressionClass::BOUND_CONSTANT:
            Execute(expr.Cast<BoundConstantExpression>(), state, count, sel, result);
            break;
        case ExpressionClass::BOUND_FUNCTION:
            Execute(expr.Cast<BoundFunctionExpression>(), state, count, sel, result);
            break;
        default:
            throw std::runtime_error("Attempting to execute expression of unknown type!");
        }
    } // Execute

    void ExpressionExecutor::Execute(BoundColumnRefExpression &expr,
                                     std::shared_ptr<ExpressionState> state,
                                     int count,
                                     std::vector<int> &sel,
                                     std::shared_ptr<arrow::Array> &result)
    {
        if (!sel.empty())
        {
            // 根据sel选择对应数据，对列做切片操作
            auto target = chunk->column(expr.column_index);
            arrow::Status ok = DaseX::Util::ArraySlice(target, result, sel);
        }
        else
        {
//            spdlog::info("[{} : {}] Project chunk is : {}", __FILE__, __LINE__, chunk->ToString());
//			spdlog::info("[{} : {}] Project chunk size is : {}", __FILE__, __LINE__, chunk->num_columns());
            result = chunk->column(expr.column_index);
        }
    }

    void ExpressionExecutor::Execute(BoundConstantExpression &expr,
                                     std::shared_ptr<ExpressionState> state,
                                     int count,
                                     std::vector<int> &sel,
                                     std::shared_ptr<arrow::Array> &result)
    {
        auto &value = expr.value;
        switch (expr.return_type)
        {
        case LogicalType::INTEGER:
        {
            std::vector<int> arr(count);
            std::fill(arr.begin(), arr.end(), value.value_.int_);
            arrow::Status ok = DaseX::Util::AppendIntArray(result, arr);
            break;
        }
        case LogicalType::FLOAT:
        {
            std::vector<float> arr(count);
            std::fill(arr.begin(), arr.end(), value.value_.float_);
            arrow::Status ok = DaseX::Util::AppendFloatArray(result, arr);
            break;
        }
        case LogicalType::DOUBLE:
        {
            std::vector<double> arr(count);
            std::fill(arr.begin(), arr.end(), value.value_.double_);
            arrow::Status ok = DaseX::Util::AppendDoubleArray(result, arr);
            break;
        }
        case LogicalType::STRING:
        {
            std::vector<std::string> arr(count);
            std::string val = value.GetValueUnsafe<std::string>();
            std::fill(arr.begin(), arr.end(), val);
            arrow::Status ok = DaseX::Util::AppendStringArray(result, arr);
            break;
        }
        default:
            throw std::runtime_error("Unsupported type");
        }
    }

    void ExpressionExecutor::Execute(BoundFunctionExpression &expr,
                                     std::shared_ptr<ExpressionState> state,
                                     int count,
                                     std::vector<int> &sel,
                                     std::shared_ptr<arrow::Array> &result) {
        if(count == 0) {
            return;
        }
        auto &left = state->left_column;
        Execute(*(expr.children[0]), state->child_states[0], count, sel, left); // left为列
        expr.function.function(left, *state, sel);
    }

    // Project expression
    void ExpressionExecutor::Execute(std::shared_ptr<arrow::RecordBatch> &input, std::shared_ptr<arrow::RecordBatch> &result) {
        SetChunk(input);
        for(int i = 0; i < expressions.size(); i++) {
            auto &expr = *(expressions[i]);
            if(auto state_weak = states[i].lock()) {
                auto &state =  state_weak->root_state;
                std::shared_ptr<arrow::Array> res;
                ExecuteExpression(expr, state, i, result, res);
            }
        }
    }

    void ExpressionExecutor::ExecuteExpression(Expression &expr,
                                               std::shared_ptr<ExpressionState> state,
                                               int idx,
                                               std::shared_ptr<arrow::RecordBatch> &result,
                                               std::shared_ptr<arrow::Array> &res,
                                               bool recursion,
                                               std::vector<int> sel) {
        switch (expr.expression_class) {
            case ExpressionClass::BOUND_COLUMN_REF:
            {
                auto &expr_p = expr.Cast<BoundColumnRefExpression>();
                ExecuteExpression(expr_p, state, idx, result, res, recursion, sel);
                break;
            }
            case ExpressionClass::BOUND_CONSTANT:
            {
                auto &expr_p = expr.Cast<BoundConstantExpression>();
                ExecuteExpression(expr_p, state, idx, result, res, recursion, sel);
                break;
            }
            case ExpressionClass::BOUND_PROJECT_FUNCTION:
            {
                auto &expr_p = expr.Cast<BoundProjectFunctionExpression>();
                ExecuteExpression(expr_p, state, idx, result, res, recursion, sel);
                break;
            }
            case ExpressionClass::BOUND_CASE:
            {
                // TODO: 完成case_when表达式
                auto &expr_p = expr.Cast<BoundCaseExpression>();
                ExecuteExpression(expr_p, state, idx, result, res, recursion, sel);
                break;
            }
            default:
                break;
        } // switch
    }

    void ExpressionExecutor::ExecuteExpression(BoundColumnRefExpression &expr,
                                               std::shared_ptr<ExpressionState> state,
                                               int idx,
                                               std::shared_ptr<arrow::RecordBatch> &result,
                                               std::shared_ptr<arrow::Array> &res,
                                               bool recursion,
                                               std::vector<int> sel) {
        if(!recursion) {
            res = chunk->column(expr.column_index);
            std::string field_name = chunk->column_name(expr.column_index);
            if(result == nullptr) {
                std::shared_ptr<arrow::Field> TT_X = arrow::field(field_name, res->type());
                std::shared_ptr<arrow::Schema> schema_ttx = arrow::schema({TT_X});
                result = arrow::RecordBatch::Make(schema_ttx, res->length(), {res});
            } else {
                result = result->AddColumn(idx, field_name, res).ValueOrDie();
            }
        } else {
            Execute(expr, state, chunk->num_rows(), sel, res);
        }
    }

    void ExpressionExecutor::ExecuteExpression(BoundConstantExpression &expr,
                                               std::shared_ptr<ExpressionState> state,
                                               int idx,
                                               std::shared_ptr<arrow::RecordBatch> &result,
                                               std::shared_ptr<arrow::Array> &res,
                                               bool recursion,
                                               std::vector<int> sel) {
        Execute(expr, state, chunk->num_rows(), sel, res);
    }

    void ExpressionExecutor::ExecuteExpression(BoundProjectFunctionExpression &expr,
                                               std::shared_ptr<ExpressionState> state,
                                               int idx,
                                               std::shared_ptr<arrow::RecordBatch> &result,
                                               std::shared_ptr<arrow::Array> &res,
                                               bool recursion,
                                               std::vector<int> sel) {
        if(!recursion) {
            auto &left = state->left_column;
            auto &right = state->right_column;
            std::vector<int> tmp;
            auto &left_expr = *(expr.children[0]);
            auto &right_expr = *(expr.children[1]);
            ExecuteExpression(left_expr, state->child_states[0], idx, result, left, true, sel);
            ExecuteExpression(right_expr, state->child_states[1], idx, result, right, true, sel);
            std::vector<std::shared_ptr<arrow::Array>> res_p = {left, right};
            expr.function.function(res_p, *state, res);
            // spdlog::info("[{} : {}] res: {}", __FILE__, __LINE__, res->ToString());
            std::string field_name = expr.alias;
            if(result == nullptr) {
                std::shared_ptr<arrow::Field> TT_X = arrow::field(field_name, res->type());
                std::shared_ptr<arrow::Schema> schema_ttx = arrow::schema({TT_X});
                result = arrow::RecordBatch::Make(schema_ttx, res->length(), {res});
            } else {
                result = result->AddColumn(idx, field_name, res).ValueOrDie();
            }
        } else {
            auto &left = state->left_column;
            auto &right = state->right_column;
            std::vector<int> tmp;
            auto &left_expr = *(expr.children[0]);
            auto &right_expr = *(expr.children[1]);
            ExecuteExpression(left_expr, state->child_states[0], idx, result, left, true, sel);
            ExecuteExpression(right_expr, state->child_states[1], idx, result, right, true, sel);
            std::vector<std::shared_ptr<arrow::Array>> res_p = {left, right};
            expr.function.function(res_p, *state, res);
        }
    }

    void ExpressionExecutor::ExecuteExpression(BoundCaseExpression &expr,
                                               std::shared_ptr<ExpressionState> state,
                                               int idx,
                                               std::shared_ptr<arrow::RecordBatch> &result,
                                               std::shared_ptr<arrow::Array> &res,
                                               bool recursion,
                                               std::vector<int> sel) {
        auto &case_state = (*state).Cast<CaseExpressionState>();
        std::vector<int> last_true_sel;
        auto &current_true_sel = case_state.true_sel;
        auto &current_false_sel = case_state.false_sel;
        int count = chunk->num_rows();
        int expr_size = expr.when_exprs.size();
        std::vector<std::shared_ptr<Value>> result_value(count);
        for(int i = 0; i < expr_size; i++) {
            std::shared_ptr<arrow::Array> res_tmp;
            // 1.先执行when表达式
            auto when_expr = expr.when_exprs[i];
            auto then_expr = expr.then_exprs[i];
            auto when_state = state->child_states[i * 2];
            auto then_state = state->child_states[i * 2 + 1];
            // int Select(Expression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);
            int tcount = Select(*when_expr, when_state, count, current_true_sel);
            last_true_sel.insert(last_true_sel.end(), current_true_sel.begin(), current_true_sel.end());
            // 2.执行then表达式
            if(tcount == 0) {
                // everything is false: do nothing
                continue;
            }
            if(unlikely(tcount == count)) {
                // everything is true, we can skip the entire case and only execute the TRUE side
                ExecuteThenExpression(*then_expr, then_state, count, current_true_sel, result_value, res_tmp);
                Util::AppendValueArray(res, result_value);
                std::string field_name = expr.alias;
                if(result == nullptr) {
                    std::shared_ptr<arrow::Field> TT_X = arrow::field(field_name, res->type());
                    std::shared_ptr<arrow::Schema> schema_ttx = arrow::schema({TT_X});
                    result = arrow::RecordBatch::Make(schema_ttx, res->length(), {res});
                } else {
                    result = result->AddColumn(idx, field_name, res).ValueOrDie();
                }
                return;
            } else {
                ExecuteThenExpression(*then_expr, then_state, count, current_true_sel, result_value, res_tmp);
                current_true_sel.clear();
            }
        } // for
        // 执行else表达式
        auto else_expr = expr.else_expr;
        auto else_state = state->child_states[state->child_states.size()-1];
        Util::GetSetDifference(current_false_sel, count, last_true_sel);
        std::shared_ptr<arrow::Array> res_tmp;
        ExecuteThenExpression(*else_expr, else_state, count, current_false_sel, result_value, res_tmp);
        Util::AppendValueArray(res, result_value);
        std::string field_name = expr.alias;
        if(result == nullptr) {
            std::shared_ptr<arrow::Field> TT_X = arrow::field(field_name, res->type());
            std::shared_ptr<arrow::Schema> schema_ttx = arrow::schema({TT_X});
            result = arrow::RecordBatch::Make(schema_ttx, res->length(), {res});
        } else {
            result = result->AddColumn(idx, field_name, res).ValueOrDie();
        }
        current_true_sel.clear();
        current_false_sel.clear();
    }

    // ================================================Case表达式-start================================================
    void ExpressionExecutor::ExecuteThenExpression(Expression &expr,
                                                   std::shared_ptr<ExpressionState> state,
                                                   int count, std::vector<int> &sel,
                                                   std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res) {
        if (count == 0)
        {
            return;
        }
        switch (expr.expression_class)
        {
            case ExpressionClass::BOUND_COLUMN_REF:
                ExecuteThenExpression(expr.Cast<BoundColumnRefExpression>(), state, count, sel, result, array_res);
                break;
            case ExpressionClass::BOUND_CONSTANT:
                ExecuteThenExpression(expr.Cast<BoundConstantExpression>(), state, count, sel, result, array_res);
                break;
            case ExpressionClass::BOUND_PROJECT_FUNCTION:
                ExecuteThenExpression(expr.Cast<BoundProjectFunctionExpression>(), state, count, sel, result, array_res);
                break;
            default:
                throw std::runtime_error("Attempting to execute expression of unknown type!");
        }
    }

    void ExpressionExecutor::ExecuteThenExpression(BoundColumnRefExpression &expr,
                                                   std::shared_ptr<ExpressionState> state,
                                                   int count, std::vector<int> &sel,
                                                   std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res) {
        if (count == 0) {
            return;
        }
        std::shared_ptr<arrow::Array> result_array = chunk->column(expr.column_index);
        switch (result_array->type_id()) {
            case arrow::Type::INT32:
            {
                auto int32array = std::static_pointer_cast<arrow::Int32Array>(result_array);
                std::vector<int> tmp;
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    int tar = int32array->Value(idx);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                    tmp.push_back(tar);
                }
                Util::AppendIntArray(array_res, tmp);
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto floatArray = std::static_pointer_cast<arrow::FloatArray>(result_array);
                std::vector<float> tmp;
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    float tar = floatArray->Value(idx);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                    tmp.push_back(tar);
                }
                Util::AppendFloatArray(array_res, tmp);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto doubleArray = std::static_pointer_cast<arrow::DoubleArray>(result_array);
                std::vector<double> tmp;
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    double tar = doubleArray->Value(idx);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                    tmp.push_back(tar);
                }
                Util::AppendDoubleArray(array_res, tmp);
                break;
            }
            case arrow::Type::STRING:
            {
                auto stringArray = std::static_pointer_cast<arrow::StringArray>(result_array);
                std::vector<std::string> tmp;
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    std::string tar = stringArray->GetString(idx);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                    tmp.emplace_back(tar);
                }
                Util::AppendStringArray(array_res, tmp);
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    }

    void ExpressionExecutor::ExecuteThenExpression(BoundConstantExpression &expr,
                                                   std::shared_ptr<ExpressionState> state,
                                                   int count, std::vector<int> &sel,
                                                   std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res) {
        if (count == 0 || sel.empty()) {
            return;
        }
        Value &value_p = expr.value;
        switch (value_p.type_) {
            case LogicalType::INTEGER:
            {
                std::vector<int> tmp;
                int val = value_p.GetValueUnsafe<int>();
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    result[idx] = std::make_shared<Value>(value_p);
                    tmp.push_back(val);
                }
                Util::AppendIntArray(array_res, tmp);
                break;
            }
            case LogicalType::FLOAT:
            {
                std::vector<float> tmp;
                float val = value_p.GetValueUnsafe<float>();
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    result[idx] = std::make_shared<Value>(value_p);
                    tmp.push_back(val);
                }
                Util::AppendFloatArray(array_res, tmp);
                break;
            }
            case LogicalType::DOUBLE:
            {
                std::vector<double> tmp;
                double val = value_p.GetValueUnsafe<double>();
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    result[idx] = std::make_shared<Value>(value_p);
                    tmp.push_back(val);
                }
                Util::AppendDoubleArray(array_res, tmp);
                break;
            }
            case LogicalType::STRING:
            {
                std::vector<std::string> tmp;
                std::string val = value_p.GetValueUnsafe<std::string>();
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    result[idx] = std::make_shared<Value>(value_p);
                    tmp.emplace_back(val);
                }
                Util::AppendStringArray(array_res, tmp);
                break;
            }
            default:
                throw std::runtime_error("Invalid LogicalType::Type!!!");
        }
    }

    void ExpressionExecutor::ExecuteThenExpression(BoundProjectFunctionExpression &expr,
                                                   std::shared_ptr<ExpressionState> state,
                                                   int count, std::vector<int> &sel,
                                                   std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res) {
        if (count == 0 || sel.empty()) {
            return;
        }
        auto &left = state->left_column;
        auto &right = state->right_column;
        auto &left_expr = *(expr.children[0]);
        auto &right_expr = *(expr.children[1]);
        std::vector<std::shared_ptr<Value>> left_result(count);
        std::vector<std::shared_ptr<Value>> right_result(count);
        ExecuteThenExpression(left_expr, state->child_states[0], count, sel, left_result, left);
        ExecuteThenExpression(right_expr, state->child_states[1], count, sel, right_result, right);
        std::vector<std::shared_ptr<arrow::Array>> res_p = {left, right};
        expr.function.function(res_p, *state, array_res);
        switch (array_res->type_id()) {
            case arrow::Type::INT32:
            {
                auto int32array = std::static_pointer_cast<arrow::Int32Array>(array_res);
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    int tar = int32array->Value(i);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                }
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto floatArray = std::static_pointer_cast<arrow::FloatArray>(array_res);
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    float tar = floatArray->Value(i);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                }
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto doubleArray = std::static_pointer_cast<arrow::DoubleArray>(array_res);
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    double tar = doubleArray->Value(i);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                }
                break;
            }
            case arrow::Type::STRING:
            {
                auto stringArray = std::static_pointer_cast<arrow::StringArray>(array_res);
                for(int i = 0; i < sel.size(); i++) {
                    int idx = sel[i];
                    std::string tar = stringArray->GetString(i);
                    std::shared_ptr<Value> value_p = std::make_shared<Value>(tar);
                    result[idx] = value_p;
                }
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        }
    }
    
} // DaseX
