#ifndef WOW_SOURCE_DB_DBCLIENT_CREATURESTATS_C_H
#define WOW_SOURCE_DB_DBCLIENT_CREATURESTATS_C_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/CreatureStats.h"

class CreatureStats_C : public CreatureStats {
    public:
    static const uint32_t s_cacheVersion = 1;

    virtual ~CreatureStats_C();
};

#endif
