/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-11 16:12:31
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-12 14:40:50
 * @FilePath: /task_sche/src/sche/barrier.hpp
 * @Description: 内存屏障，设置需要到达的线程数
 */
#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace DaseX {

class Barrier {
public:
    std::mutex m_lock;
    std::condition_variable m_cv;
    int16_t thread_count;
    int16_t arrive_thread_count;
    int8_t m_release;

public:
    Barrier(int thread_count_)
            : thread_count(thread_count_), arrive_thread_count(0), m_release(0) {}

    void wait() {
        std::unique_lock<std::mutex> lk(m_lock);
        if (arrive_thread_count == 0) {
            m_release = 0;
        }
        arrive_thread_count++;
        if (arrive_thread_count == thread_count) {
            arrive_thread_count = 0;
            m_release = 1;
            m_cv.notify_all();
        } else {
            m_cv.wait(lk, [&] { return m_release == 1; });
        }
    }
};

} // namespace DaseX
