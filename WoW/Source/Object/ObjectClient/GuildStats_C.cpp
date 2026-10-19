#include "GuildStats_C.h"
#include "Engine/Source/Base/CDataStore.h"

// VERIFY: 9781c6fd auto
// OFFSET: 0x0098E3F0
void GuildStats_C::Pack(CDataStore* msg) {
    msg->Put(static_cast<uint32_t>(this->m_id));
    msg->PutString(this->m_name);

    for (uint32_t i = 0; i < 10; i++) {
        msg->PutString(this->m_rankNames[i]);
    }

    msg->Put(static_cast<uint32_t>(this->m_emblemStyle));
    msg->Put(static_cast<uint32_t>(this->m_emblemColor));
    msg->Put(static_cast<uint32_t>(this->m_borderStyle));
    msg->Put(static_cast<uint32_t>(this->m_borderColor));
    msg->Put(static_cast<uint32_t>(this->m_background));
    msg->Put(static_cast<uint32_t>(this->m_numRanks));
}

// VERIFY: 6df25489 auto
// OFFSET: 0x0075B3E0
void GuildStats_C::Unpack(CDataStore* msg) {
    msg->Get(reinterpret_cast<uint32_t&>(this->m_id));
    msg->GetString(this->m_name, sizeof(this->m_name));

    for (uint32_t i = 0; i < 10; i++) {
        msg->GetString(this->m_rankNames[i], sizeof(this->m_rankNames[i]));
    }

    msg->Get(reinterpret_cast<uint32_t&>(this->m_emblemStyle));
    msg->Get(reinterpret_cast<uint32_t&>(this->m_emblemColor));
    msg->Get(reinterpret_cast<uint32_t&>(this->m_borderStyle));
    msg->Get(reinterpret_cast<uint32_t&>(this->m_borderColor));
    msg->Get(reinterpret_cast<uint32_t&>(this->m_background));
    msg->Get(reinterpret_cast<uint32_t&>(this->m_numRanks));
}
