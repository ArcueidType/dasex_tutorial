/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 10:47:59
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-14 10:47:59
 * @FilePath: /task_sche/src/util/numa_help.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <numa.h>
#include <sched.h>
#include <spdlog/spdlog.h>

namespace DaseX {

namespace Util {

// 检查NUMA节点的空闲内存
void print_socket_free_memory();

// 绑定核心
void bind_thread_to_cpu(int cpu_core);

// 绑定内存
void bind_memory_to_cpu();

} // namespace Util
} // namespace DaseX
