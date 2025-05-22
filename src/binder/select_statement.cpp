/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-03 13:31:55
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-03 13:31:55
 * @FilePath: /task_sche-binder/src/binder/select_statement.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "../include/binder/statement/select_statement.hpp"

namespace DaseX {

// TODO(zyy): fix debug func: to string
auto SelectStatement::ToString() const -> std::string { return "BoundSelect "; }

} // namespace DaseX
