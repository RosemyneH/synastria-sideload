#include "ItemTextCache.h"
#include "Engine/Source/Base/CDataStore.h"

// INLINED
ItemTextCache_C::ItemTextCache_C() {
    this->m_text[0] = '\0';
}

// INLINED
void ItemTextCache_C::Unpack(CDataStore* msg) {
    msg->GetString(this->m_text, sizeof(this->m_text));
}

// INLINED
void ItemTextCache_C::Pack(CDataStore* msg) {
    msg->PutString(this->m_text);
}
