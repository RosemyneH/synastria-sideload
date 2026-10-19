#include "Engine/Source/ComSat/ComSatRunningAverageCalculator.h"

// VERIFY: 08159cc8 auto
// OFFSET: 0x00886600
ComSat::RunningAverageCalculator::RunningAverageCalculator(ComSat::Allocator& allocator) {
    this->m_sum = 0.0;
    this->m_allocator = &allocator;
    this->m_values = nullptr;
    this->m_count = 0;
    this->m_index = 0;
}

// VERIFY: 94225611 auto
// OFFSET: [0x00886750, 0x00886790]
ComSat::RunningAverageCalculator::~RunningAverageCalculator() {
    if (this->m_values) {
        this->m_allocator->Free(this->m_values);
        this->m_values = nullptr;
    }
    this->m_count = 0;
    this->m_sum = 0.0;
    this->m_index = 0;
}

// VERIFY: fac249da manual
// OFFSET: 0x00886630
void ComSat::RunningAverageCalculator::Init(double value, uint32_t count) {
    if (this->m_values) {
        this->m_allocator->Free(this->m_values);
        this->m_values = nullptr;
    }
    this->m_sum = 0.0;
    this->m_count = 0;
    this->m_index = 0;
    this->m_values = static_cast<double*>(this->m_allocator->Alloc(sizeof(double) * count));
    this->m_count = count;
    for (uint32_t i = 0; i < count; i++) {
        this->m_values[i] = value;
        this->m_sum += value;
    }
}

// VERIFY: daa1cb75 auto
// OFFSET: 0x008866F0
void ComSat::RunningAverageCalculator::SubmitData(double value) {
    if (this->m_values) {
        this->m_sum = this->m_sum - this->m_values[this->m_index] + value;
        this->m_values[this->m_index] = value;
        this->m_index = (this->m_index + 1) % this->m_count;
    }
}

// VERIFY: e888bf5d auto
// OFFSET: 0x00886730
double ComSat::RunningAverageCalculator::GetCurrentAverage() {
    return this->m_sum / this->m_count;
}
