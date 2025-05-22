#pragma once

#include "worker.hpp"
#include <map>
#include <memory>
#include <queue>
#include <spdlog/spdlog.h>
#include <vector>

namespace DaseX {

typedef struct Workinfo {
    int soft_task_nums;

    Workinfo(int task_nums) { soft_task_nums = task_nums; }
} Workinfo;

typedef int WorkerID;

enum class WorkerState;

class Scheduler {
public:
    int num_workers;
    std::vector<std::shared_ptr<Worker>> workers;
    std::map<WorkerID, Workinfo> workers_info;
    std::queue<WorkerID> idle_workers;
    bool periodic;

public:
    Scheduler(int num_workers) : num_workers(num_workers), periodic(true) {
        // workers.resize(num_workers);
        // //牛逼，这里调用resize居然会导致段错误，想不通啊
        for (int i = 0; i < num_workers; i++) {
            auto worker = std::make_shared<Worker>(i, this);
            workers.push_back(worker);
            workers_info.emplace(i, Workinfo(0));
        }
    }

    template<typename F, typename... Args>
    void submit_task(F &&f, int worker_id, bool is_hard_task, Args &&...args) {
        workers[worker_id]->submit_task(std::forward<F>(f), is_hard_task,
                                        std::forward<Args>(args)...);
    }

    int get_num_workers() { return num_workers; }

    void shutdown() {
        periodic = false;
        for (int i = 0; i < num_workers; i++) {
            workers[i]->shutdown();
        }
    }

    // 定时任务，周期性检查各个忙的worker的soft任务队列，并将任务置换到空闲worker中
    void periodic_check_soft_task_queue_per_running_worker() {
        while (periodic) {
            std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)); // 线程休眠100毫秒
            for (int i = 0; i < num_workers; ++i) {
                switch (workers[i]->work_state) {
                    case WorkerState::READY:
                        continue;
                    case WorkerState::RUNNING:
                        // 检查soft队列任务数量，超过3的话置换到其它空闲线程，如果没有空闲线程，什么都不做
                        // TODO：这里其实需要一个调度策略
                        if (idle_workers.empty()) {
                            break;
                        }
                        if (workers[i]->queue_soft->size() > 3) {
                            std::function<void()> task;
                            bool dequeued = workers[i]->queue_soft->dequeue(task);
                            if (dequeued && task) {
                                WorkerID worker_id = std::move(idle_workers.front());
                                idle_workers.pop();
                                spdlog::info("steal task ========= ");
                                submit_task(task, worker_id, true);
                            }
                        }
                        break;
                    case WorkerState::IDLE:
                        // 记录workerID，留着被调度
                        idle_workers.emplace(workers[i]->worker_id);
                        break;

                    case WorkerState::DEAD:
                        // 线程关闭，暂时什么也不做
                    default:
                        break;
                }
            } // for
        }   // while
        spdlog::info("End the backround thread!!!");
    } //
};

} // namespace DaseX
