#include "CKBIterator.h"
#include "Engine/Source/FrameXML/XMLTree.h"
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: 23adeffb auto
// OFFSET: 0x005509A0
void CKBIterator::GoToFirst() {
    if (!this->m_node) {
        return;
    }

    XMLNode* parent = this->m_node->m_parent;
    this->m_node = parent ? parent->m_child : nullptr;
    this->m_index = 0;
}

// VERIFY: d6f21bed auto
// OFFSET: 0x005509D0
void CKBIterator::GoToNext() {
    if (!this->m_node) {
        return;
    }

    XMLNode* next = this->m_node->m_next;
    this->m_index++;
    this->m_node = next;
}

// VERIFY: b80e814a auto
// OFFSET: 0x005509E0
void CKBIterator::GoTo(int32_t index) {
    XMLNode* node = this->m_node;

    if (!node) {
        return;
    }

    if (this->m_index == index - 1) {
        this->m_index++;
        this->m_node = node->m_next;
        return;
    }

    if (this->m_index == index) {
        return;
    }

    if (index < 0) {
        this->m_node = nullptr;
        this->m_index = 0;
        return;
    }

    if (this->m_index > index) {
        this->GoToFirst();
        node = this->m_node;
    }

    while (this->m_index < index) {
        if (!node) {
            break;
        }

        node = node->m_next;
        this->m_index++;
    }

    this->m_node = node;
}

// VERIFY: ca923db9 auto
// OFFSET: 0x00550A60
void CKBIterator::GoToFirstChild() {
    if (!this->m_node) {
        return;
    }

    this->m_node = this->m_node->m_child;
    this->m_index = 0;
}

// VERIFY: acf9db7c auto
// OFFSET: 0x00552060
void CKBIterator::GoToChild(const char* name) {
    if (!this->m_node) {
        return;
    }

    XMLNode* child = this->m_node->m_child;

    for (this->m_index = 0; child; this->m_index++) {
        if (!SStrCmpI(child->m_name.m_str, name, STORM_MAX_STR)) {
            break;
        }

        child = child->m_next;
    }

    this->m_node = child;
}
