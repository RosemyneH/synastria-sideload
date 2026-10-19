#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_BAG_C_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_BAG_C_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/Object_C.h"

class CGItem_C;

struct BAGSLOTRANGE {
    /* 0000 */ uint32_t m_first;
    /* 0004 */ uint32_t m_last;
};

struct ItemTypeCountParam {
    /* 0000 */ int32_t m_itemId;
    /* 0004 */ int32_t m_count;
};

class ItemStats_C;

class CGBag_C {
    public:
    /* 0000 */ int32_t m_numSlots;
    /* 0004 */ WOWGUID* m_slots;
    /* 0008 */ WOWGUID m_guid;
    /* 0010 */ uint8_t m_hasBank;

    int32_t FindItemSlot(const CGItem_C* item) const;
    int32_t FindItemSlot(WOWGUID guid) const;
    void UnlockItemsOfType(int32_t itemId, uint32_t flags);
    CGItem_C* FindItemOfBagFamily(uint32_t bagFamily, uint32_t flags);
    uint32_t GetAvgEquippedDurability() const;
    const CGItem_C* GetItemPointer(int32_t slot) const;
    int32_t GetNumFreeInvSlots();
    int32_t GetNumItems() const;
    uint64_t GetItemGuid(uint32_t slot) const;
    CGItem_C* FindItemOfType(int32_t itemId, uint32_t flags);
    CGItem_C* FindItemOfClass(int32_t classId, uint32_t subclassMask, uint32_t flags);
    CGItem_C* FindItem(int32_t (*callback)(const CGItem_C* item, void* param), void* param, uint32_t flags);
    CGItem_C* FindItemByName(const char* name, uint32_t flags);
    CGItem_C* FindItemByFullName(const char* name, uint32_t flags);
    CGItem_C* GetItemIdByName(const char* name, WOWGUID& container, uint32_t& slot, uint32_t flags);
    CGItem_C* GetItemPointer(int32_t slot);

    bool ContainsItem(WOWGUID guid, bool recurse, uint32_t flags) const;
    bool FindTotemCategory(int32_t category, uint32_t flags);
    bool FindTotemCategory(int32_t type, int32_t& mask, bool recurse, uint32_t flags);
    CGItem_C* FindItem(int32_t (*callback)(const CGItem_C* item, void* param), void* param, WOWGUID& container, uint32_t& slot, uint32_t flags);
    CGItem_C* FindItemOfType(int32_t itemId, WOWGUID& container, uint32_t& slot, uint32_t flags);
    int32_t GetItemTypeCount(int32_t itemId, uint32_t flags);
    int32_t GetItemTypeCharges(int32_t itemId, uint32_t flags);
    bool ItemCountValid(int32_t itemId, const ItemStats_C* stats, int32_t count, bool showError);
};

int32_t GetItemTypeCountCallback(const CGItem_C* item, void* param);
int32_t GetItemTypeChargesCallback(const CGItem_C* item, void* param);
int32_t FindItemIDCallback(const CGItem_C* item, void* param);
int32_t FindItemClassCallback(const CGItem_C* item, void* param);
int32_t GetItemLimitCategoryCallback(const CGItem_C* item, void* param);
int32_t GetDisplayErrorByInventoryError(int32_t error);

#endif
