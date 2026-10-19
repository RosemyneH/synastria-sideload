#ifndef WOW_SOURCE_MAGIC_SPELL_H
#define WOW_SOURCE_MAGIC_SPELL_H

#include <cstddef>
#include <cstdint>

class C3Vector;

bool Spell_IsAuraEffect(int32_t effect);
bool Spell_IsUnitAuraEffect(int32_t effect);
bool Spell_IsAreaAuraEffect(int32_t effect);
float SpellUtils_ComputeMissileTrajectoryTravelTime(float pitch, float speed, const C3Vector* delta, float gravity);
int32_t SpellUtils_ComputeMissileTrajectory(float pitch, const C3Vector* delta, float gravity, float* speed, float* time, C3Vector* velocity);

#endif
