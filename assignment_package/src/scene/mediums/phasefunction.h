#pragma once
#include <globals.h>

class PhaseFunction
{
public:
    virtual ~PhaseFunction() {}
    virtual float p(const Vector3f &woW, const Vector3f &wiW) const = 0;
    virtual float Sample_p(const Vector3f &woW, Vector3f *wiW, const Point2f &u) const = 0;
};

inline float PhaseHG(float cosTheta, float g) {
    float denom = 1.f + g * g + 2.f * g * cosTheta;
    return Inv4Pi * (1.f - g * g) / (denom * glm::sqrt(denom));
}
