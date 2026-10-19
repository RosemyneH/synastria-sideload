#ifndef ENGINE_SOURCE_FRAMEXML_XMLTREE_H
#define ENGINE_SOURCE_FRAMEXML_XMLTREE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Base/RCString.h"
#include "Engine/Source/Storm/H/stpl.h"

class CXMLAttribute;
class XMLNode;
class XMLNodeData;
struct XMLTree;

struct POSITION {
    /* 0000 */ uint32_t m_lineNumber;
    /* 0004 */ uint32_t m_columnNumber;
};

class CXMLAttribute {
    public:
    /* 0000 */ RCString m_name;
    /* 000C */ RCString m_value;
};

class XMLNodeData {
    public:
    virtual ~XMLNodeData();
    virtual void Release();
};

class XMLNode {
    public:
    /* 0000 */ XMLNodeData* m_userData;
    /* 0004 */ XMLNode* m_parent;
    /* 0008 */ XMLNode* m_child;
    /* 000C */ RCString m_name;
    /* 0018 */ char* m_body;
    /* 001C */ uint32_t m_bodyLen;
    /* 0020 */ TSGrowableArray<CXMLAttribute> m_attributes;
    /* 0030 */ uint32_t m_offset;
    /* 0034 */ XMLNode* m_next;

    static CDataAllocator s_allocator;

    static void* operator new(size_t bytes) noexcept;
    static void operator delete(void* ptr);

    XMLNode(XMLNode* parent, const char* name);
    ~XMLNode();
    void Detach();
    const char* GetAttributeByName(const char* name);
    XMLNode* GetChildByName(const char* name);
    void Init(XMLNode* parent, const char* name);
    void Recycle();
    void SetAttribute(const char* name, const char* value);
};

struct XMLTree {
    /* 0000 */ XMLNode* m_root;
    /* 0004 */ XMLNode* m_current;
};

void XMLTree_Cleanup();
void XMLTree_Free(XMLTree* tree);
XMLNode* XMLTree_GetRoot(XMLTree* tree);
XMLTree* XMLTree_Load(const char* data, uint32_t size);

#endif
