#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_TIME_TYPES_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_TIME_TYPES_H

#include <cstddef>
#include <cstdint>

namespace Blizzard {
namespace Time {

typedef int64_t Timestamp;

struct TimeRec {
    /* 0000 */ int32_t year;
    /* 0004 */ int32_t month;
    /* 0008 */ int32_t day;
    /* 000C */ int32_t hour;
    /* 0010 */ int32_t minute;
    /* 0014 */ int32_t second;
    /* 0018 */ int32_t nanosecond;
    /* 001C */ int32_t weekday;
    /* 0020 */ int32_t yearday;
};

}
}

#endif
