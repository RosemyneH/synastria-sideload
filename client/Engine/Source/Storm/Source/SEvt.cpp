#include <cstdint>
#include "SEvt.h"
#include "Engine/Source/Storm/H/SCritSect.h"
#include "Engine/Source/Storm/H/storm.h"

// STATIC: 0x00CB74F8
static int32_t s_modified;

// STATIC: 0x00CB7500
static _TYPEHASHENTRY** s_typehashtable;

// STATIC: 0x00CB7504
static uint32_t s_typehashtablesize;

// STATIC: 0x00CB7508
static uint32_t s_typehashtableused;

// STATIC: 0x00CB750C
static CCritSect s_critsect;

// VERIFY: 77691d28 manual
// OFFSET: 0x0077E9F0
static void DeleteIdHashTable(_IDHASHTABLE* idhashtable) {
    for (uint32_t i = 0; i < idhashtable->size; i++) {
        while (_IDHASHENTRY* entry = idhashtable->data[i]) {
            idhashtable->data[i] = entry->next;
            delete entry;
        }
    }

    SMemFree(idhashtable->data, __FILE__, __LINE__, 0);
    delete idhashtable;
}

// INLINED: [0x0077EB21->0x0077EB7C]
static _TYPEHASHENTRY* FindTypeHashEntry(uint32_t type, uintptr_t subtype) {
    if (!s_typehashtable || !s_typehashtablesize) {
        return nullptr;
    }

    for (_TYPEHASHENTRY* entry = s_typehashtable[(s_typehashtablesize - 1) & (subtype ^ type)]; entry; entry = entry->next) {
        if (entry->type == type && entry->subtype == subtype) {
            return entry;
        }
    }

    return nullptr;
}

// VERIFY: 740271ea manual
// OFFSET: 0x0077EA50
int32_t STORMAPI SEvtDestroy() {
    s_critsect.Enter();

    for (uint32_t i = 0; i < s_typehashtablesize; i++) {
        while (_TYPEHASHENTRY* entry = s_typehashtable[i]) {
            while (_IDHASHTABLE* idhashtable = entry->idhashtable) {
                entry->idhashtable = idhashtable->next;
                DeleteIdHashTable(idhashtable);
            }

            s_typehashtable[i] = entry->next;
            delete entry;
        }
    }

    delete s_typehashtable;
    s_typehashtable = nullptr;
    s_typehashtablesize = 0;
    s_typehashtableused = 0;
    s_modified = 1;

    s_critsect.Leave();
    return 1;
}

// VERIFY: 6de37e6b manual
// OFFSET: 0x0077EB10
int32_t STORMAPI SEvtUnregisterType(uint32_t type, uintptr_t subtype) {
    int32_t result = 0;

    s_critsect.Enter();

    _TYPEHASHENTRY* entry = FindTypeHashEntry(type, subtype);

    if (entry) {
        while (_IDHASHTABLE* idhashtable = entry->idhashtable) {
            entry->idhashtable = idhashtable->next;
            DeleteIdHashTable(idhashtable);
        }

        _TYPEHASHENTRY** link = &s_typehashtable[(s_typehashtablesize - 1) & (subtype ^ type)];

        while (*link) {
            if (*link == entry) {
                _TYPEHASHENTRY* curr = *link;
                *link = curr->next;
                delete curr;
                s_typehashtableused--;
            } else {
                link = &(*link)->next;
            }
        }

        result = 1;
        s_modified = 1;
    }

    s_critsect.Leave();
    return result;
}
