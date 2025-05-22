#pragma once

#include <arrow/api.h>
#include <vector>

namespace DaseX
{

    class VectorOperations
    {
    public:
        VectorOperations() {}

    public:
        // A = B
        static int Equals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);

        // A != B
        static int NotEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);

        // A > B
        static int GreaterThan(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);

        // A >= B
        static int GreaterThanEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);

        // A < B
        static int LessThan(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);

        // A <= B
        static int LessThanEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel);
    };

} // DaseX