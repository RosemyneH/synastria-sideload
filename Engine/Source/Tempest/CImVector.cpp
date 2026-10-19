#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CMath.h"
#include "Engine/Source/Tempest/CImVector.h"

// VERIFY: 29546a0f manual
// OFFSET: 0x0048BD20
void CImVector::Set(float a, float r, float g, float b) {
    this->a = static_cast<uint8_t>(CMath::ftol_round_0_255_(a));
    this->r = static_cast<uint8_t>(CMath::ftol_round_0_255_(r));
    this->g = static_cast<uint8_t>(CMath::ftol_round_0_255_(g));
    this->b = static_cast<uint8_t>(CMath::ftol_round_0_255_(b));
}

// VERIFY: abf2e53e auto
// OFFSET: 0x006ACC50
void CImVector::InterpColor(int32_t t, const CImVector* src) {
    uint32_t& value = *reinterpret_cast<uint32_t*>(this);

    if (t == 255) {
        value = (value & 0xFF000000) | (*reinterpret_cast<const uint32_t*>(src) & 0x00FFFFFF);
        return;
    }

    uint8_t r = this->r + ((static_cast<uint32_t>(src->r - this->r) * t) >> 8);
    uint8_t g = this->g + ((static_cast<uint32_t>(src->g - this->g) * t) >> 8);
    uint8_t b = this->b + ((static_cast<uint32_t>(src->b - this->b) * t) >> 8);

    value = (value & 0xFF000000) | (r << 16) | (g << 8) | b;
}

// VERIFY: 5249a0a5 manual
// OFFSET: 0x009851A0
CImVector& CImVector::operator=(const C3Vector& color) {
    this->a = 0xFF;
    this->r = static_cast<uint8_t>(CMath::fint_(color.x * 255.0f));
    this->g = static_cast<uint8_t>(CMath::fint_(color.y * 255.0f));
    this->b = static_cast<uint8_t>(CMath::fint_(color.z * 255.0f));
    return *this;
}

// VERIFY: 6899672e manual
// OFFSET: 0x0057E480
void CImVector::InterpColorRGB(uint32_t t, const CImVector* src) {
    uint8_t r = this->r + ((t * (src->r - this->r)) >> 8);
    uint8_t g = this->g + ((t * (src->g - this->g)) >> 8);
    uint8_t b = this->b + ((t * (src->b - this->b)) >> 8);
    this->r = r;
    this->g = g;
    this->b = b;
}
