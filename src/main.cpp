// #include <iostream>
// #include "scheduler.hpp"
// #include <thread>
// #include "pipeline_task.hpp"

// using namespace DaseX;

// int main() {
//     //std::thread
//     t(&Scheduler::periodic_check_soft_task_queue_per_running_worker,
//     &scheduler);
//     //std::cout << "=======================================3" << std::endl;
//     //t.detach();
//     Scheduler scheduler(2);
//     std::thread
//     t(&Scheduler::periodic_check_soft_task_queue_per_running_worker,
//     &scheduler); t.detach(); PipelineTask pipeline_task; auto task_function =
//     [&pipeline_task]() { pipeline_task(); };
//     // scheduler.submit_task(task_function, 0, true);
//     scheduler.submit_task(task_function, 1, true);
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     scheduler.shutdown();
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     return 0;
// }
