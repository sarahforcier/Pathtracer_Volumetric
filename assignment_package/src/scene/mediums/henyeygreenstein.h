#pragma once
#include "phasefunction.h"
#include <globals.h>

class HenyeyGreenstein : public PhaseFunction {
public:
    HenyeyGreenstein(Float g) : g(g) {}
    Float p(const Vector3f &woW, const Vector3f &wiW) const;
    Float Sample_p(const Vector3f &woW, Vector3f *wiW,
                   const Point2f &sample) const;
private:
    const float g; //[-1, 1] => [back, forward] scattering
};
