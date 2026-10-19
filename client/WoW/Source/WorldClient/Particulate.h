#ifndef WOW_SOURCE_WORLDCLIENT_PARTICULATE_H
#define WOW_SOURCE_WORLDCLIENT_PARTICULATE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"

class CTexture;

struct LiquidParticle {
    /* 0000 */ C3Vector m_position;
    /* 000C */ float m_size;
};

class Particulate {
    public:
    /* 0000 */ LiquidParticle m_particles [4000];
    /* FA00 */ uint32_t m_maxParticles;
    /* FA04 */ C3Vector m_prevCameraPos;
    /* FA10 */ CTexture* m_texture;
    /* FA14 */ uint8_t m_enabled;
    /* FA15 */ uint8_t m_padding [3];
    /* FA18 */ float m_particleSize;
    /* FA1C */ float m_spawnAreaSize;
    /* FA20 */ float m_unkFloat1;
    /* FA24 */ uint32_t m_liquidType;
    /* FA28 */ C3Vector m_direction;
    /* FA34 */ float m_speed;
    /* FA38 */ float m_timer;
    /* FA3C */ float m_rotationSpeed;

    Particulate(float particleSize, float spawnAreaSize, const char* textureName);
    ~Particulate();
    C3Vector ComputeMovement(float elapsed);
    void InitMovement();
    void InitParticles(uint32_t count);
    void Render();
    void Update();
    void SetScale(float scale);
    void SetTexture(const char* textureName);
};

#endif
