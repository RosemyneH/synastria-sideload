#ifndef WOW_SOURCE_BATTLENET_BSN_ADAPTER_H
#define WOW_SOURCE_BATTLENET_BSN_ADAPTER_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace BSN {
namespace Adapter {
namespace Array {
uint32_t GetLength(const void* data);
template <uint32_t OFFSET>
const void* GetElements(const void* data);
template <uint32_t SIZE, uint32_t OFFSET>
void* SetLength(void* data, uint32_t length);
}

namespace AsciiString {
uint32_t GetLength(const void* data);
const void* GetElements(const void* data);
void* SetLength(void* data, uint32_t length);
}

namespace Blob {
uint32_t GetLength(const void* data);
const void* GetElements(const void* data);
void* SetLength(void* data, uint32_t length);
}

namespace FixedBlob {
template <uint32_t N>
uint32_t GetLength(const void* data);
const void* GetElements(const void* data);
void* SetLength(void* data, uint32_t length);
}

namespace FixedBitArray {
template <uint32_t N>
uint32_t GetLength(const void* data);
const void* GetElements(const void* data);
void* SetLength(void* data, uint32_t length);
}

namespace String {
uint32_t GetSize(const void* data);
const void* GetElements(const void* data);
void* SetSize(void* data, uint32_t size);
void SetLength(void* data, uint32_t length);
}

namespace FourCC {
uint32_t GetValue(const void* data);
void SetValue(void* data, uint32_t value);
}

namespace Choice {
int32_t GetTag(const void* data);
const void* GetElement(const void* data);
void* SetTag(void* data, int32_t tag);
}

namespace Optional {
bool GetExists(const void* data);
template <uint32_t OFFSET>
const void* GetElement(const void* data);
template <uint32_t OFFSET>
void* SetExists(void* data, bool exists);
}
}
}

// OFFSET: [0x008BED40<4>, 0x008BF770<8>]
template <uint32_t OFFSET>
const void* BSN::Adapter::Array::GetElements(const void* data) {
    return static_cast<const uint8_t*>(data) + OFFSET;
}

// OFFSET: [0x008BE780<24, 8>, 0x008BE7C0<40, 8>, 0x008BE800<44, 4>, 0x008BE840<4, 4>, 0x008BE880<408, 4>, 0x008BE8E0<148, 4>, 0x008BE920<144, 4>, 0x008BE960<48, 4>, 0x008BE9A0<2232, 4>, 0x008BE9E0<12, 4>, 0x008BEA20<1068, 4>, 0x008BEA60<1028, 4>, 0x008BEAB0<152, 4>, 0x008BEB20<348, 4>, 0x008BEB60<2452, 4>, 0x008BEBC0<344, 4>, 0x008BEC00<260, 4>, 0x008BEC40<216, 4>, 0x008BEC80<1, 4>, 0x008BECC0<528, 8>, 0x008BED00<104, 4>, 0x008BED50<1032, 4>, 0x008BED90<16, 4>, 0x008BEDD0<48, 8>, 0x008BEE10<136, 4>, 0x008BEE50<6, 4>, 0x008BEEB0<1040, 4>, 0x008BEEF0<1004, 4>, 0x008BEF30<10048, 4>, 0x008BEF70<332, 4>, 0x008BEFB0<336, 4>, 0x008BF010<8, 8>, 0x008BF050<8, 4>, 0x008BF0A0<296, 4>, 0x008BF0E0<3956, 4>, 0x008BF120<216, 8>, 0x008BF160<2152, 8>, 0x008BF1A0<80, 4>, 0x008BF1E0<1604, 4>, 0x008BF220<96, 4>, 0x008BF260<848, 8>, 0x008BF2A0<284, 4>, 0x008BF2E0<144, 8>, 0x008BF350<40, 4>, 0x008BF390<124, 4>, 0x008BF3D0<1600, 8>, 0x008BF410<36, 4>, 0x008BF450<4136, 4>, 0x008BF490<4140, 4>, 0x008BF4D0<76, 4>, 0x008BF510<56, 8>, 0x008BF560<528, 4>, 0x008BF5A0<1572, 4>, 0x008BF5E0<39144, 4>, 0x008BF620<1280, 8>, 0x008BF660<7752, 4>, 0x008BF6A0<516, 4>, 0x008BF6E0<120, 4>, 0x008BF730<2352, 4>, 0x008BF780<132, 4>, 0x008BF7C0<1540, 4>, 0x008BF800<152, 8>, 0x008BF840<2504, 8>, 0x008BF880<2592, 8>, 0x008BF8C0<75216, 8>, 0x008BF920<524, 4>, 0x008BF960<536, 4>, 0x008BF9A0<696, 4>, 0x008BF9E0<416, 4>, 0x008BFA30<1416, 4>, 0x008BFA70<232, 8>, 0x008BFAB0<32, 8>, 0x008BFAF0<20, 4>, 0x008BFB30<2, 4>, 0x008BFB60<4108, 4>, 0x008BFBA0<56, 4>, 0x008BFBF0<72, 4>, 0x008BFC40<16, 8>]
template <uint32_t SIZE, uint32_t OFFSET>
void* BSN::Adapter::Array::SetLength(void* data, uint32_t length) {
    uint32_t* count = static_cast<uint32_t*>(data);
    uint8_t* elements = static_cast<uint8_t*>(data) + OFFSET;

    if (length > *count) {
        memset(&elements[SIZE * *count], 0, SIZE * (length - *count));
    }

    *count = length;
    return elements;
}

// OFFSET: [0x008BE470<40>, 0x008BE480<16>, 0x008BE490<4>, 0x008BE4A0<2>, 0x008BE4B0<64>, 0x0095BFB0<32>]
template <uint32_t N>
uint32_t BSN::Adapter::FixedBlob::GetLength(const void* data) {
    return N;
}

// OFFSET: [0x008BE460<336>, 0x008BE480<16>]
template <uint32_t N>
uint32_t BSN::Adapter::FixedBitArray::GetLength(const void* data) {
    return N;
}

// OFFSET: [0x008BED40<4>, 0x008BF000<1>, 0x008BF090<2>, 0x008BF770<8>]
template <uint32_t OFFSET>
const void* BSN::Adapter::Optional::GetElement(const void* data) {
    return static_cast<const uint8_t*>(data) + OFFSET;
}

// OFFSET: [0x008BE8C0<2>, 0x008BEBA0<4>, 0x008BEE90<8>, 0x008BF900<1>]
template <uint32_t OFFSET>
void* BSN::Adapter::Optional::SetExists(void* data, bool exists) {
    *static_cast<bool*>(data) = exists;

    if (!exists) {
        return nullptr;
    }

    return static_cast<uint8_t*>(data) + OFFSET;
}

#endif
