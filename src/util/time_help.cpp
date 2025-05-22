/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-06 21:12:49
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-06 21:12:50
 * @FilePath: /task_sche/src/util/time_help.cpp
 * @Description:
 */
#include "time_help.hpp"

namespace DaseX {
namespace Util {

int64_t time_difference(const std::chrono::steady_clock::time_point &start,
                        const std::chrono::steady_clock::time_point &end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
      .count();
}

} // namespace Util
} // namespace DaseX
