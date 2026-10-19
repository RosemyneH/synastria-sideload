#ifndef WOW_SOURCE_UI_CKBITERATOR_H
#define WOW_SOURCE_UI_CKBITERATOR_H

#include <cstddef>
#include <cstdint>

class XMLNode;

class CKBIterator {
    public:
    /* 0000 */ XMLNode* m_node;
    /* 0004 */ int32_t m_index;

    void GoToFirst();
    void GoToNext();
    void GoTo(int32_t index);
    void GoToFirstChild();
    void GoToChild(const char* name);
};

#endif
