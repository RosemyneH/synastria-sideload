#ifndef ENGINE_SOURCE_TEMPEST_C3SPLINE_BEZIER3_H
#define ENGINE_SOURCE_TEMPEST_C3SPLINE_BEZIER3_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Spline.h"

class C3Spline_Bezier3 : public C3Spline {
    public:
    void EvaluateDer1(uint32_t seg, float t, C3Vector& vel);
    void ParametricSegT(float t, uint32_t& seg, float& segT);

    float ILength() override;
    void IValidateCache() override;
    void IPosArclength(float t, C3Vector& pos) override;
    void IPosParametric(float t, C3Vector& pos) override;
    void IVelArclength(float t, C3Vector& vel) override;
    void IVelParametric(float t, C3Vector& vel) override;
    void IFrameArclength(float t, C44Matrix& frame) override;
    void ISetPoints(const C3Vector* points, uint32_t count) override;
};

#endif
