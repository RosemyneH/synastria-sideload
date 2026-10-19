#ifndef WOW_SOURCE_BATTLENET_BSN_BSNENCODEINTERFACE_H
#define WOW_SOURCE_BATTLENET_BSN_BSNENCODEINTERFACE_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Battlenet/BSN/Protocol.h"
#include "WoW/Source/Battlenet/BSN/Types.h"
#include "Engine/Source/Storm/H/storm.h"

namespace BSN {
struct EncodeContext {
    /* 0000 */ const Protocol::Protocol* m_protocol;
    /* 0004 */ uint32_t m_typeId;
    /* 0008 */ uint32_t m_header;
    /* 000C */ void* m_data;

    void ResolveType();
};

class PrimitiveEncoder {
    public:
    virtual ~PrimitiveEncoder();
    virtual uint32_t GetContextSize() const = 0;
    virtual EncodeContext* ConstructContext(void* memory) const = 0;
    virtual void DestroyContext(EncodeContext* context) const = 0;
    virtual uint64_t GetUInt(const EncodeContext* context) const = 0;
    virtual int64_t GetInt(const EncodeContext* context) const = 0;
    virtual bool GetBool(const EncodeContext* context) const = 0;
    virtual double GetReal(const EncodeContext* context) const = 0;
    virtual uint32_t GetFourCC(const EncodeContext* context) const = 0;
    virtual uint32_t GetStringLength(const EncodeContext* context) const = 0;
    virtual bool GetString(const EncodeContext* context, char* buffer, uint32_t size) const = 0;
    virtual uint32_t GetBitArrayLength(const EncodeContext* context) const = 0;
    virtual bool GetBitArray(const EncodeContext* context, uint8_t* buffer, uint32_t bits) const = 0;
    virtual uint32_t GetBlobLength(const EncodeContext* context) const = 0;
    virtual bool GetBlob(const EncodeContext* context, uint8_t* buffer, uint32_t size) const = 0;
    virtual bool GetMemberContext(const EncodeContext* context, const char* name, EncodeContext* member) const = 0;
    virtual bool GetElementContext(const EncodeContext* context, uint32_t index, EncodeContext* element) const = 0;
    virtual bool GetChoiceName(const EncodeContext* context, char* buffer, uint32_t size) const = 0;
    virtual uint32_t GetArrayLength(const EncodeContext* context) const = 0;
    virtual bool GetOptional(const EncodeContext* context) const = 0;
    virtual bool SetUInt(EncodeContext* context, uint64_t value) const = 0;
    virtual bool SetInt(EncodeContext* context, int64_t value) const = 0;
    virtual bool SetBool(EncodeContext* context, bool value) const = 0;
    virtual bool SetReal(EncodeContext* context, double value) const = 0;
    virtual bool SetFourCC(EncodeContext* context, uint32_t value) const = 0;
    virtual bool SetString(EncodeContext* context, const char* value) const = 0;
    virtual bool SetBitArray(EncodeContext* context, const uint8_t* data, uint32_t bits) const = 0;
    virtual bool SetBlob(EncodeContext* context, const uint8_t* data, uint32_t size) const = 0;
    virtual bool SetStruct(EncodeContext* context, const char* name) const = 0;
    virtual bool SetChoice(EncodeContext* context, const char* typeName, const char* choiceName) const = 0;
    virtual bool SetArray(EncodeContext* context, uint32_t length) const = 0;
    virtual bool SetOptional(EncodeContext* context, bool exists) const = 0;
    virtual bool WalkAndSet(const PrimitiveEncoder* encoder, EncodeContext* context, const EncodeContext* source) const = 0;
    virtual bool WalkAndGet(const PrimitiveEncoder* encoder, const EncodeContext* source, EncodeContext* context) const = 0;
};

class BSNPrimitiveEncoder : public PrimitiveEncoder {
    public:
    uint32_t GetContextSize() const override;
    EncodeContext* ConstructContext(void* memory) const override;
    void DestroyContext(EncodeContext* context) const override;
    uint64_t GetUInt(const EncodeContext* context) const override;
    int64_t GetInt(const EncodeContext* context) const override;
    bool GetBool(const EncodeContext* context) const override;
    double GetReal(const EncodeContext* context) const override;
    uint32_t GetFourCC(const EncodeContext* context) const override;
    uint32_t GetStringLength(const EncodeContext* context) const override;
    bool GetString(const EncodeContext* context, char* buffer, uint32_t size) const override;
    uint32_t GetBitArrayLength(const EncodeContext* context) const override;
    bool GetBitArray(const EncodeContext* context, uint8_t* buffer, uint32_t bits) const override;
    uint32_t GetBlobLength(const EncodeContext* context) const override;
    bool GetBlob(const EncodeContext* context, uint8_t* buffer, uint32_t size) const override;
    bool GetMemberContext(const EncodeContext* context, const char* name, EncodeContext* member) const override;
    bool GetElementContext(const EncodeContext* context, uint32_t index, EncodeContext* element) const override;
    bool GetChoiceName(const EncodeContext* context, char* buffer, uint32_t size) const override;
    uint32_t GetArrayLength(const EncodeContext* context) const override;
    bool GetOptional(const EncodeContext* context) const override;
    bool SetUInt(EncodeContext* context, uint64_t value) const override;
    bool SetInt(EncodeContext* context, int64_t value) const override;
    bool SetBool(EncodeContext* context, bool value) const override;
    bool SetReal(EncodeContext* context, double value) const override;
    bool SetFourCC(EncodeContext* context, uint32_t value) const override;
    bool SetString(EncodeContext* context, const char* value) const override;
    bool SetBitArray(EncodeContext* context, const uint8_t* data, uint32_t bits) const override;
    bool SetBlob(EncodeContext* context, const uint8_t* data, uint32_t size) const override;
    bool SetStruct(EncodeContext* context, const char* name) const override;
    bool SetChoice(EncodeContext* context, const char* typeName, const char* choiceName) const override;
    bool SetArray(EncodeContext* context, uint32_t length) const override;
    bool SetOptional(EncodeContext* context, bool exists) const override;
    bool WalkAndSet(const PrimitiveEncoder* encoder, EncodeContext* context, const EncodeContext* source) const override;
    bool WalkAndGet(const PrimitiveEncoder* encoder, const EncodeContext* source, EncodeContext* context) const override;
};

class BSNPrimitiveWalker {
    public:
    struct Context {
        /* 0000 */ const Protocol::Protocol* m_protocol;
        /* 0004 */ uint32_t m_typeId;
        /* 0008 */ const PrimitiveEncoder* m_sourceEncoder;
        /* 000C */ const EncodeContext* m_sourceContext;
        /* 0010 */ const PrimitiveEncoder* m_targetEncoder;
        /* 0014 */ EncodeContext* m_targetContext;
    };

    static int32_t Walk(BSNPrimitiveWalker& walker, const Context& context);

    virtual ~BSNPrimitiveWalker();

    int32_t Copy(const Context& context);
    int32_t Array(const Protocol::Meta::Array& meta, const Context& context);
    int32_t BitArray(const Protocol::Meta::BitArray& meta, const Context& context);
    int32_t Blob(const Protocol::Meta::Blob& meta, const Context& context);
    int32_t Bool(const Protocol::Meta::Bool& meta, const Context& context);
    int32_t Choice(const Protocol::Meta::Choice& meta, const Context& context);
    int32_t FourCC(const Protocol::Meta::FourCC& meta, const Context& context);
    int32_t Int(const Protocol::Meta::Int& meta, const Context& context);
    int32_t Enum(const Protocol::Meta::Enum& meta, const Context& context);
    int32_t Optional(const Protocol::Meta::Optional& meta, const Context& context);
    int32_t Real32(const Protocol::Meta::Real32& meta, const Context& context);
    int32_t Real64(const Protocol::Meta::Real64& meta, const Context& context);
    int32_t Struct(const Protocol::Meta::Struct& meta, const Context& context);
    int32_t AsciiString(const Protocol::Meta::AsciiString& meta, const Context& context);
    int32_t String(const Protocol::Meta::String& meta, const Context& context);
    int32_t User(const Protocol::Meta::User& meta, const Context& context);
};

class EncodedObject {
    public:
    /* 0004 */ uint32_t m_refCount;

    virtual ~EncodedObject();
    virtual uint32_t GetEncodingType() const = 0;
    virtual const PrimitiveEncoder* GetEncoder() const = 0;
    virtual EncodeContext* GetContext() = 0;
    virtual const EncodeContext* GetContext() const = 0;
    virtual void Destroy() const = 0;
    virtual bool WalkAndGet(const EncodedObject* source);
    virtual bool WalkAndSet(EncodedObject* target) const;
    virtual void IncRef() const;
    virtual void DecRef() const;

    EncodedObject();
};

template <class T, class P>
class BSNEncodedObject : public EncodedObject {
    public:
    /* 0008 */ T m_data;
    /* 0008+T */ BSNPrimitiveEncoder m_encoder;
    /* 000C+T */ EncodeContext m_context;

    ~BSNEncodedObject() override;
    uint32_t GetEncodingType() const override;
    const PrimitiveEncoder* GetEncoder() const override;
    EncodeContext* GetContext() override;
    const EncodeContext* GetContext() const override;
    void Destroy() const override;

    BSNEncodedObject();
    explicit BSNEncodedObject(const EncodedObject* source);
};

template <class T, class P, int32_t ON_STACK>
class BSNEncodedObjectPtr : public EncodedObject {
    public:
    /* 0008 */ BSNPrimitiveEncoder m_encoder;
    /* 000C */ EncodeContext m_context;

    ~BSNEncodedObjectPtr() override;
    uint32_t GetEncodingType() const override;
    const PrimitiveEncoder* GetEncoder() const override;
    EncodeContext* GetContext() override;
    const EncodeContext* GetContext() const override;
    void Destroy() const override;

    explicit BSNEncodedObjectPtr(const T* data);
};
}

// VERIFY: 7567f6df manual
// OFFSET: [0x00532CC0<Battlenet::Toon::FullName, Protocol>, 0x00532D70<Battlenet::Friends::FriendsOfFriendList, Protocol>, 0x00537A60<Battlenet::Friends::FriendInviteContainer, Protocol>, 0x008C6BD0<Battlenet::Event::Update::Invoke, Protocol>, 0x008C6EB0<Battlenet::Event::Larva::Invoke, Protocol>]
template <class T, class P>
BSN::BSNEncodedObject<T, P>::BSNEncodedObject() {
    this->m_context.m_protocol = &P::s_protocol;
    this->m_context.m_typeId = P::template TypeIdOf<T>::s_typeId;
    this->m_context.m_data = &this->m_data;
    this->m_context.ResolveType();
}

// VERIFY: af0c9998 auto
// OFFSET: [0x005328E0<Battlenet::Chat::MemberList, Protocol>, 0x005329B0<Battlenet::Friends::FriendInvite, Protocol>, 0x00532A80<Battlenet::Presence::FieldVal, Protocol>, 0x00532B40<Battlenet::Presence::BlockList, Protocol>, 0x00532BF0<Battlenet::Presence::FieldUpdate, Protocol>]
template <class T, class P>
BSN::BSNEncodedObject<T, P>::BSNEncodedObject(const EncodedObject* source) {
    this->m_context.m_protocol = &P::s_protocol;
    this->m_context.m_typeId = P::template TypeIdOf<T>::s_typeId;
    this->m_context.m_data = &this->m_data;
    this->m_context.ResolveType();

    if (source) {
        this->EncodedObject::WalkAndGet(source);
    }
}

// VERIFY: d66fa723 manual
// OFFSET: [0x00532960<Battlenet::Chat::MemberList, Protocol>, 0x00532A20<Battlenet::Friends::FriendInvite, Protocol>, 0x00532B00<Battlenet::Presence::FieldVal, Protocol>, 0x00532BB0<Battlenet::Presence::BlockList, Protocol>, 0x00532C80<Battlenet::Presence::FieldUpdate, Protocol>, 0x00532D30<Battlenet::Toon::FullName, Protocol>, 0x00532DD0<Battlenet::Friends::FriendsOfFriendList, Protocol>, 0x00537AF0<Battlenet::Friends::FriendInviteContainer, Protocol>, 0x008C6C40<Battlenet::Event::Update::Invoke, Protocol>, 0x008C6F30<Battlenet::Event::Larva::Invoke, Protocol>, 0x009C25A0<Battlenet::Event::Header, Protocol>]
template <class T, class P>
BSN::BSNEncodedObject<T, P>::~BSNEncodedObject() {
}

// VERIFY: fa821c7f auto
// OFFSET: 0x008C8DE0
template <class T, class P>
uint32_t BSN::BSNEncodedObject<T, P>::GetEncodingType() const {
    return 1;
}

// VERIFY: 2a27d6bc auto
// OFFSET: [0x00532940<Battlenet::Chat::MemberList, Protocol>, 0x00532990<Battlenet::Presence::BlockList, Protocol>, 0x00532AF0<Battlenet::Presence::FieldVal, Protocol>, 0x00532C60<Battlenet::Presence::FieldUpdate, Protocol>, 0x00532C70<Battlenet::Friends::FriendInvite, Protocol>, 0x00532D10<Battlenet::Toon::FullName, Protocol>, 0x00532DB0<Battlenet::Friends::FriendsOfFriendList, Protocol>, 0x00537AD0<Battlenet::Friends::FriendInviteContainer, Protocol>, 0x00662470<Battlenet::Event::Header, Protocol>, 0x008C6C20<Battlenet::Event::Update::Invoke, Protocol>, 0x008C6F10<Battlenet::Event::Larva::Invoke, Protocol>]
template <class T, class P>
const BSN::PrimitiveEncoder* BSN::BSNEncodedObject<T, P>::GetEncoder() const {
    return &this->m_encoder;
}

// VERIFY: 56f20e47 auto
// OFFSET: [0x005329A0<Battlenet::Presence::BlockList, Protocol>, 0x00532A10<Battlenet::Friends::FriendInvite, Protocol>, 0x00532C70<Battlenet::Presence::FieldUpdate, Protocol>, 0x00532D20<Battlenet::Toon::FullName, Protocol>, 0x00532DC0<Battlenet::Friends::FriendsOfFriendList, Protocol>, 0x00537AE0<Battlenet::Friends::FriendInviteContainer, Protocol>, 0x00714230<Battlenet::Chat::MemberList, Protocol>, 0x008C6C30<Battlenet::Event::Update::Invoke, Protocol>, 0x008C6F20<Battlenet::Event::Larva::Invoke, Protocol>, 0x00978BE0<Battlenet::Presence::FieldVal, Protocol>, 0x009C2590<Battlenet::Event::Header, Protocol>]
template <class T, class P>
BSN::EncodeContext* BSN::BSNEncodedObject<T, P>::GetContext() {
    return &this->m_context;
}

// VERIFY: a189645b auto
// OFFSET: [0x005329A0<Battlenet::Presence::BlockList, Protocol>, 0x00532A10<Battlenet::Friends::FriendInvite, Protocol>, 0x00532C70<Battlenet::Presence::FieldUpdate, Protocol>, 0x00532D20<Battlenet::Toon::FullName, Protocol>, 0x00532DC0<Battlenet::Friends::FriendsOfFriendList, Protocol>, 0x00537AE0<Battlenet::Friends::FriendInviteContainer, Protocol>, 0x00714230<Battlenet::Chat::MemberList, Protocol>, 0x008C6C30<Battlenet::Event::Update::Invoke, Protocol>, 0x008C6F20<Battlenet::Event::Larva::Invoke, Protocol>, 0x00978BE0<Battlenet::Presence::FieldVal, Protocol>, 0x009C2590<Battlenet::Event::Header, Protocol>]
template <class T, class P>
const BSN::EncodeContext* BSN::BSNEncodedObject<T, P>::GetContext() const {
    return &this->m_context;
}

// VERIFY: 9953dd3a auto
// OFFSET: 0x00532950
template <class T, class P>
void BSN::BSNEncodedObject<T, P>::Destroy() const {
    delete this;
}

// INLINED
template <class T, class P, int32_t ON_STACK>
BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::BSNEncodedObjectPtr(const T* data) {
    this->m_context.m_protocol = &P::s_protocol;
    this->m_context.m_typeId = P::template TypeIdOf<T>::s_typeId;
    this->m_context.m_data = const_cast<T*>(data);
    this->m_context.ResolveType();
}

// VERIFY: d5d4c71f manual
// OFFSET: 0x009C1FE0
template <class T, class P, int32_t ON_STACK>
BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::~BSNEncodedObjectPtr() {
}

// VERIFY: f934db2f auto
// OFFSET: 0x008C8DE0
template <class T, class P, int32_t ON_STACK>
uint32_t BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::GetEncodingType() const {
    return 1;
}

// VERIFY: 8761f830 auto
// OFFSET: 0x008C6C80
template <class T, class P, int32_t ON_STACK>
const BSN::PrimitiveEncoder* BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::GetEncoder() const {
    return &this->m_encoder;
}

// VERIFY: 45425827 auto
// OFFSET: 0x009C1FD0
template <class T, class P, int32_t ON_STACK>
BSN::EncodeContext* BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::GetContext() {
    return &this->m_context;
}

// VERIFY: 1564967c auto
// OFFSET: 0x009C1FD0
template <class T, class P, int32_t ON_STACK>
const BSN::EncodeContext* BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::GetContext() const {
    return &this->m_context;
}

// VERIFY: a7da6e0e auto
// OFFSET: [0x00532950, 0x00532A60]
template <class T, class P, int32_t ON_STACK>
void BSN::BSNEncodedObjectPtr<T, P, ON_STACK>::Destroy() const {
    if (ON_STACK) {
        SErrDisplayAppFatalMessage("", "", 0);
    }

    delete this;
}

#endif
