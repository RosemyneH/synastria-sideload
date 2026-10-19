#include <cstddef>
#include <cstdint>
#include "Engine/Source/Services/ParticleSystem.h"
#include "Engine/Source/Storm/H/storm.h"

// INLINED
CParticle::~CParticle() {
    this->m_age = 0.0f;
}

// VERIFY: 07404e3e manual
// OFFSET: 0x00982280
CParticleEmitter::~CParticleEmitter() {
}

// VERIFY: 520cd03a auto
// OFFSET: 0x009822F0
void CParticleEmitter::DecRef() {
    if (this->m_refCount-- == 1) {
        delete this;
    }
}
