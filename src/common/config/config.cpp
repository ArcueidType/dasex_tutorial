#include "config.hpp"

namespace DaseX {
namespace Numa {

int SOCKETS = numa_num_configured_nodes();

int CORES = numa_num_task_cpus();

int get_socket(int core) { return numa_node_of_cpu(core); }

} // namespace Numa

} // namespace DaseX
