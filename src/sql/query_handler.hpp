#pragma once

#include <memory>
#include "binder.h"
#include "catalog.hpp"
#include "scheduler.hpp"

namespace DaseX {

// TODO 输出结果
class QueryResult {
public:
    arrow::RecordBatchVector result_chunks;
    RC status;

    void print_result();

    int get_row_count();
};

class QueryHandler {
public:
    QueryHandler(std::weak_ptr<CataLog> catalog): catalog_(catalog) {
    }

    RC Query(std::string sql, bool verbose = true);

    QueryResult result_temp;
protected:
    std::weak_ptr<CataLog> catalog_;
    // std::shared_ptr<Scheduler> scheduler;
    // std::shared_ptr<Binder> binder;
};

}  // namespace DaseX