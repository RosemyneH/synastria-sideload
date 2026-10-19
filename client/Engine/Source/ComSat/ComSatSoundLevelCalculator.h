#ifndef ENGINE_SOURCE_COMSAT_COMSATSOUNDLEVELCALCULATOR_H
#define ENGINE_SOURCE_COMSAT_COMSATSOUNDLEVELCALCULATOR_H

#include <cstddef>
#include <cstdint>

namespace ComSat {
namespace Sound {

class LevelCalculator16Bit {
    public:
    static void CalculateLevelForFrame(const uint8_t* data, uint32_t size, float& level, float& decibels);
    static float GetMaxDecibelLevel();
};

}
}

#endif
