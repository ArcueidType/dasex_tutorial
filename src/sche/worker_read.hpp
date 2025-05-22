#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <sched.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>

namespace DaseX {

template<typename T>
class SafeQueue2 {
private:
    std::queue<T> m_queue; // 利用模板函数构造队列
    std::mutex m_mutex;

public:
    SafeQueue2() {}

    SafeQueue2(SafeQueue2 &&other) {}

    ~SafeQueue2() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(
                m_mutex); // 互斥信号变量加锁，防止m_queue被改变
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(
                m_mutex); // 互斥信号变量加锁，防止m_queue被改变
        return m_queue.size();
    }

    void enqueue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }

    bool dequeue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex); // 队列加锁

        if (m_queue.empty())
            return false;
        t = std::move(
                m_queue.front()); // 取出队首元素，返回队首元素值，并进行右值引用
        m_queue.pop(); // 弹出入队的第一个元素
        return true;
    }
};

class Scheduler;

enum class WorkerState2 {
    READY, RUNNING, IDLE, DEAD
};


class SchedulerRead;

// 一个Worker绑定一个Core，一个Core有且只有一个Worker，Worker负责执行实际的任务
class WorkerRead {
public:
    int worker_id;
    std::shared_ptr<SafeQueue2<std::function<void()>>> queue_hard;
    std::shared_ptr<SafeQueue2<std::function<void()>>> queue_soft;
    std::thread worker_thread;
    std::mutex worker_mutex;
    std::condition_variable cv;
    bool is_running;
    SchedulerRead *scheduler;
    WorkerState2 work_state;

public:
    WorkerRead(int worker_id, SchedulerRead *scheduler); // Scheduler* scheduler

    void shutdown() {
        this->is_running = false;
        cv.notify_all();
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }

    void work();

    // 提交任意返回值，任意参数的函数任务？接下来的问题就是如何将函数转成标准形式--->void()
    template<typename F, typename... Args>
    void submit_task(F &&f, bool is_hard_task, Args &&...args) {
        // 利用std::bind将函数和参数分离  ---> func  +  args
        std::function<decltype(f(args...))()> func =
                std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // 将 func  +  args ---> void()
        auto task_ptr =
                std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]() { (*task_ptr)(); };

        // 队列通用安全封包函数，并压入安全队列
        if (is_hard_task) {
            queue_hard->enqueue(warpper_func);
            cv.notify_all(); // 通知工作线程有新任务
        } else {
            queue_soft->enqueue(warpper_func);
            cv.notify_all(); // 通知工作线程有新任务
        }
    }
};

} // namespace DaseX
