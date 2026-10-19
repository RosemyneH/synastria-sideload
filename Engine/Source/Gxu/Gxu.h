#ifndef ENGINE_SOURCE_GXU_GXU_H
#define ENGINE_SOURCE_GXU_GXU_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Gx/CGxTex.h"
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/C44Matrix.h"

void GxuUpdateSingleColorTexture(EGxTexCommand command, uint32_t width, uint32_t height, uint32_t face, uint32_t level, void* userArg, uint32_t& texelStrideInBytes, const void*& texels);
int32_t GxuXformCalc3dScreenCoords(uint32_t count, const C3Vector* points, C3Vector* screen);
void GxuXformCalcFrustumCorners(const C44Matrix& view, const C44Matrix& proj, C3Vector* corners);
void GxuXformCreateLookAtSgCompat(const C3Vector& eye, const C3Vector& center, const C3Vector& up, C44Matrix& dst);
void GxuXformCreateLookAtXXX(const C3Vector& eye, const C3Vector& center, const C3Vector& up, C44Matrix& dst);
void GxuXformCreateOrtho(float minX, float maxX, float minY, float maxY, float minZ, float maxZ, C44Matrix& dst);
void GxuXformCreateOrthoDepth(float minX, float maxX, float minY, float maxY, float minZ, float maxZ, C44Matrix& dst);
void GxuXformCreateProjection_Exact(float fovyInRadians, float aspect, float minZ, float maxZ, C44Matrix& dst);
void GxuXformCreateProjection_SG(float fovyInRadians, float aspect, float minZ, float maxZ, C44Matrix& dst);

#endif
