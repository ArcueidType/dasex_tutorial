
#pragma once

#include "value_type.hpp"

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>



namespace DaseX {

/*
把Chunk输出，输出格式：
+-----------+--------------------+--------------------------------+-----------------+-------------+
|c_custkey  |c_name              |c_address                       |c_phone          |c_acctbal    |
+-----------+--------------------+--------------------------------+-----------------+-------------+
|1          |Customer#000000001  |IVhzIApeRb ot,c,E               |25-989-741-2988  |711.56       |
|2          |Customer#000000002  |XSTf4,NCwDVaWNe6tEgvwfmRchLXak  |23-768-687-3665  |121.65       |
|3          |Customer#000000003  |MG9kdTD2WBHm                    |11-719-748-3364  |7498.12      |
|4          |Customer#000000004  |XxVSJsLAGtn                     |14-128-190-5944  |2866.83      |
|5          |Customer#000000005  |KvpyuHCplrB84WgAiGV6sYpZq7Tj    |13-750-942-6364  |794.47       |
+-----------+--------------------+--------------------------------+-----------------+-------------+
*/
void DisplayChunk(std::shared_ptr<arrow::RecordBatch> &record_batch, int limit = 30, bool tail = false);


void DisplayChunk(std::vector<std::shared_ptr<arrow::RecordBatch>> &record_batch_array, int limit = 30, bool tail = false);


} // DaseX
