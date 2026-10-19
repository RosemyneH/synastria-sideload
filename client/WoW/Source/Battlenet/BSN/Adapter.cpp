#include "WoW/Source/Battlenet/BSN/Adapter.h"

// VERIFY: 46115984 auto
// OFFSET: 0x008BE770
uint32_t BSN::Adapter::Array::GetLength(const void* data) {
    return *static_cast<const uint32_t*>(data);
}

// VERIFY: 25c3874f auto
// OFFSET: 0x008BE770
uint32_t BSN::Adapter::AsciiString::GetLength(const void* data) {
    return *static_cast<const uint32_t*>(data);
}

// OFFSET: 0x008BED40
const void* BSN::Adapter::AsciiString::GetElements(const void* data) {
    return static_cast<const uint8_t*>(data) + 4;
}

// OFFSET: 0x008BF320
void* BSN::Adapter::AsciiString::SetLength(void* data, uint32_t length) {
    char* elements = static_cast<char*>(data) + 4;
    memset(elements, 0, length);
    elements[length] = '\0';
    *static_cast<uint32_t*>(data) = length;
    return elements;
}

// VERIFY: cb794046 auto
// OFFSET: 0x008BE770
uint32_t BSN::Adapter::Blob::GetLength(const void* data) {
    return *static_cast<const uint32_t*>(data);
}

// OFFSET: 0x008BED40
const void* BSN::Adapter::Blob::GetElements(const void* data) {
    return static_cast<const uint8_t*>(data) + 4;
}

// OFFSET: 0x008BFA20
void* BSN::Adapter::Blob::SetLength(void* data, uint32_t length) {
    *static_cast<uint32_t*>(data) = length;
    return static_cast<uint8_t*>(data) + 4;
}

// VERIFY: 1a075b9d auto
// OFFSET: 0x008BFC30
const void* BSN::Adapter::FixedBlob::GetElements(const void* data) {
    return data;
}

// VERIFY: 56d38bd8 auto
// OFFSET: 0x008BFC30
void* BSN::Adapter::FixedBlob::SetLength(void* data, uint32_t length) {
    return data;
}

// VERIFY: 3551350c auto
// OFFSET: 0x008BFC30
const void* BSN::Adapter::FixedBitArray::GetElements(const void* data) {
    return data;
}

// VERIFY: 48724d0e auto
// OFFSET: 0x008BFC30
void* BSN::Adapter::FixedBitArray::SetLength(void* data, uint32_t length) {
    return data;
}

// OFFSET: 0x008BEFF0
uint32_t BSN::Adapter::String::GetSize(const void* data) {
    return static_cast<const uint32_t*>(data)[1];
}

// OFFSET: 0x008BF770
const void* BSN::Adapter::String::GetElements(const void* data) {
    return static_cast<const uint8_t*>(data) + 8;
}

// OFFSET: 0x008BEAF0
void* BSN::Adapter::String::SetSize(void* data, uint32_t size) {
    char* elements = static_cast<char*>(data) + 8;
    memset(elements, 0, size);
    elements[size] = '\0';
    static_cast<uint32_t*>(data)[1] = size;
    static_cast<uint32_t*>(data)[0] = size;
    return elements;
}

// OFFSET: 0x008BEAA0
void BSN::Adapter::String::SetLength(void* data, uint32_t length) {
    *static_cast<uint32_t*>(data) = length;
}

// VERIFY: b1fe4b93 auto
// OFFSET: 0x008BE770
uint32_t BSN::Adapter::FourCC::GetValue(const void* data) {
    return *static_cast<const uint32_t*>(data);
}

// OFFSET: 0x008BEAA0
void BSN::Adapter::FourCC::SetValue(void* data, uint32_t value) {
    *static_cast<uint32_t*>(data) = value;
}

// VERIFY: 00618b97 auto
// OFFSET: 0x008BE770
int32_t BSN::Adapter::Choice::GetTag(const void* data) {
    return *static_cast<const int32_t*>(data);
}

// OFFSET: 0x008BED40
const void* BSN::Adapter::Choice::GetElement(const void* data) {
    return static_cast<const uint8_t*>(data) + 4;
}

// OFFSET: 0x008BFA20
void* BSN::Adapter::Choice::SetTag(void* data, int32_t tag) {
    *static_cast<int32_t*>(data) = tag;
    return static_cast<uint8_t*>(data) + 4;
}

// OFFSET: 0x008BECB0
bool BSN::Adapter::Optional::GetExists(const void* data) {
    return *static_cast<const uint8_t*>(data) != 0;
}
