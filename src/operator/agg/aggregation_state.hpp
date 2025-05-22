#pragma once

#include "value_type.hpp"

namespace DaseX {

enum AggFunctionType {
    COUNT,
    AVG,
    MIN,
    MAX,
    SUM,
};

class Aggstate {
public:
    virtual void add (double value) = 0;
    virtual void add (Value value) = 0;
    virtual double finalize() = 0;
    virtual void merge (const Aggstate &other) = 0;
    virtual ~Aggstate() {}
};

class SumState : public Aggstate
{
public:
    double sum = 0;

    inline void add (double value) override
    {
        sum += value;
    }

    inline void add (Value value) override
    {
        switch (value.type_) {
            case LogicalType::INTEGER:
            {
                sum += value.GetValueUnsafe<int>();
                break;
            }
            case LogicalType::FLOAT:
            {
                sum += value.GetValueUnsafe<float>();
                break;
            }
            case LogicalType::DOUBLE:
            {
                sum += value.GetValueUnsafe<double>();
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown Data Type!!!");
        }
    }

    inline double finalize() override
    {
        return sum;
    }

    inline void merge(const Aggstate &other) override
    {
        auto other_state = dynamic_cast<const SumState&>(other);
        sum += other_state.sum;
    }
};

class CountState : public Aggstate
{
public:
    int count = 0;

    inline void add(double value) override {
        count++;
    }

    inline void add (Value value) override
    {
        count++;
    }


    inline double finalize() override
    {
        return count;
    }

    inline void merge(const Aggstate &other) override
    {
        auto other_state = dynamic_cast<const CountState&>(other);
        count += other_state.count;
    }
};

class MaxState : public Aggstate
{
public:
    bool init = false;
    double max = 0;
    inline void add(double value) override
    {
        if (!init) {
            max = value;
            init = true;
        } else if (value > max)
            max = value;
    }

    inline void add(Value value) override
    {
        switch (value.type_) {
            case LogicalType::INTEGER:
            {
                int val = value.GetValueUnsafe<int>();
                if (!init) {
                    max = val;
                    init = true;
                } else if ( val > max)
                    max = val;
                break;
            }
            case LogicalType::FLOAT:
            {
                float val = value.GetValueUnsafe<float>();
                if (!init) {
                    max = val;
                    init = true;
                } else if ( val > max)
                    max = val;
                break;
            }
            case LogicalType::DOUBLE:
            {
                double val = value.GetValueUnsafe<double>();
                if (!init) {
                    max = val;
                    init = true;
                } else if ( val > max)
                    max = val;
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown Data Type!!!");
        } // switch
    }

    inline double finalize() override
    {
        return max;
    }

    inline void merge(const Aggstate &other) override
    {
        auto other_state = dynamic_cast<const MaxState&>(other);
        if (likely(other_state.init)) {
            if (likely(init)) {
                max = std::max(max, other_state.max);
            } else {
                max = other_state.max;
            }
        }
    }
};

class MinState : public Aggstate
{
public:
    bool init = false;
    double min = 0;

    inline void add(double value) override
    {
        if (!init) {
            min = value;
            init = true;
        } else if (value < min)
            min = value;
    }

    inline void add(Value value) override
    {
        switch (value.type_) {
            case LogicalType::INTEGER:
            {
                int val = value.GetValueUnsafe<int>();
                if (!init) {
                    min = val;
                    init = true;
                } else if ( val < min)
                    min = val;
                break;
            }
            case LogicalType::FLOAT:
            {
                float val = value.GetValueUnsafe<float>();
                if (!init) {
                    min = val;
                    init = true;
                } else if ( val < min)
                    min = val;
                break;
            }
            case LogicalType::DOUBLE:
            {
                double val = value.GetValueUnsafe<double>();
                if (!init) {
                    min = val;
                    init = true;
                } else if ( val < min)
                    min = val;
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown Data Type!!!");
        } // switch
    }

    inline double finalize() override
    {
        return min;
    }

    inline void merge(const Aggstate &other) override
    {
        auto other_state = dynamic_cast<const MinState&>(other);
        if (likely(other_state.init)) {
            if (likely(init)) {
                min = std::min(min, other_state.min);
            } else {
                min = other_state.min;
            }
        }
    }
};

class AvgState : public Aggstate
{
public:
    double sum = 0;
    int count = 0;

    inline void add(double value) override {
        sum += value;
        count++;
    }

    inline void add(Value value) override
    {
        switch (value.type_) {
            case LogicalType::INTEGER:
            {
                int val = value.GetValueUnsafe<int>();
                sum += val;
                count++;
                break;
            }
            case LogicalType::FLOAT:
            {
                float val = value.GetValueUnsafe<float>();
                sum += val;
                count++;
                break;
            }
            case LogicalType::DOUBLE:
            {
                double val = value.GetValueUnsafe<double>();
                sum += val;
                count++;
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown Data Type!!!");
        } // switch
    }

    inline double finalize() override { return 0; }

    inline double finalize_avg() {
        if (count == 0) {
            return 0;
        } else {
            return sum * (1.0) / count;
        }
    }

    inline void merge(const Aggstate &other) override
    {
        auto other_state = dynamic_cast<const AvgState&>(other);
        sum += other_state.sum;
        count += other_state.count;
    }
};

} // DaseX
