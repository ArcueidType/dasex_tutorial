/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-13 14:59:08
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-07 22:09:59
 * @FilePath: /task_sche/src/util/common_macro.hpp
 * @Description: 全局宏定义，所有公共宏均定义在此处
 */
#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 */

#define NEXT_POW_2(V) \
    do                \
    {                 \
        V--;          \
        V |= V >> 1;  \
        V |= V >> 2;  \
        V |= V >> 4;  \
        V |= V >> 8;  \
        V |= V >> 16; \
        V++;          \
    } while (0)
#endif

#define CHUNK_SIZE 4096

// 分支预测
#define likely(x) __builtin_expect(!!(x), 1)   // x很可能为真
#define unlikely(x) __builtin_expect(!!(x), 0) // x很可能为假