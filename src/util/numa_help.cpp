#include <numa_help.hpp>

namespace DaseX {
namespace Util {

// 检查NUMA节点的空闲内存
void print_socket_free_memory() {
  int max_node = numa_max_node();
  for (int i = 0; i <= max_node; i++) {
    long free_memory_size = 0;
    long node_memory_size = numa_node_size(i, &free_memory_size);
    // double node_memory_size_g = node_memory_size / 1024 / 1024 / 1024;
    double free_memory_size_g = free_memory_size / 1024 / 1024 / 1024;
    spdlog::info("[{} : {}] Free memory on NUMA node {} : {} GB.", __FILE__, __LINE__, i, free_memory_size_g);
  }
}

// 绑定核心
void bind_thread_to_cpu(int cpu_core) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_core, &cpuset);

  if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
    spdlog::info("Failed to set thread affinity to CPU {}.", cpu_core);
  }
}

// 绑定内存
void bind_memory_to_cpu() { numa_set_localalloc(); }

} // namespace Util
} // namespace DaseX
