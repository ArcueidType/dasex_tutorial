/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-20 12:50:06
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-20 12:50:06
 * @FilePath: /task_sche_parser/src/binder/insert_statement.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "catalog.hpp"
#include "../table_ref/bound_base_table_ref.h"
#include "../bound_statement.hpp"

namespace DaseX {

class SelectStatement;

class InsertStatement : public BoundStatement {
public:
  explicit InsertStatement(std::unique_ptr<BoundBaseTableRef> table,
                           std::unique_ptr<SelectStatement> select);

  std::unique_ptr<BoundBaseTableRef> table_;

  std::unique_ptr<SelectStatement> select_;
};

} // namespace DaseX
