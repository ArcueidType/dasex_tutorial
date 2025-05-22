/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-06 21:09:27
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-13 20:35:46
 * @FilePath: /task_sche/src/util/time_help.hpp
 * @Description: 计时相关函数
 */
#pragma once

#include <chrono>

namespace DaseX {
namespace Util {

/**
 * @description: 输出运行时长的函数
 * @param {time_point&} start 开始时间
 * @param {time_point&} end   结束时间
 * @return {int64_t}          持续时长
 * @author: Caiwanli
 * auto start = std::chrono::steady_clock::now();
 */
int64_t time_difference(const std::chrono::steady_clock::time_point &start,
                        const std::chrono::steady_clock::time_point &end);

} // namespace Util
} // namespace DaseX
