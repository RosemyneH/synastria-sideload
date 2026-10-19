#ifndef ENGINE_SOURCE_COMSAT_COMSATRUNNINGAVERAGECALCULATOR_H
#define ENGINE_SOURCE_COMSAT_COMSATRUNNINGAVERAGECALCULATOR_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/ComSatInterface.h"

namespace ComSat {

class RunningAverageCalculator {
    public:
    /* 0008 */ Allocator* m_allocator;
    /* 000C */ double* m_values;
    /* 0010 */ uint32_t m_count;
    /* 0014 */ uint32_t m_index;
    /* 0018 */ double m_sum;

    virtual ~RunningAverageCalculator();

    RunningAverageCalculator(Allocator& allocator);
    double GetCurrentAverage();
    void Init(double value, uint32_t count);
    void SubmitData(double value);
};

}

#endif
