#ifndef WOW_SOURCE_DB_DBCLIENT_GAMEOBJECTSTATS_C_H
#define WOW_SOURCE_DB_DBCLIENT_GAMEOBJECTSTATS_C_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/GameObjectStats.h"

class CDataStore;

class GameObjectStats_C : public GameObjectStats {
    public:
    static const uint32_t s_cacheVersion = 1;

    void Unpack(CDataStore* msg);
};

#endif
