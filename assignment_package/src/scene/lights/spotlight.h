#pragma once

#include "light.h"
//#include "warpfunctions.h"

class SpotLight : public Light
{
public:
    SpotLight(const Transform &t, const Color3f& Le,
                     const float totalWidth, const float falloffStart)
        : Light(t, true), I(Le), pLight(t.position()),
          cosTotalWidth(std::cos(Radians(totalWidth))),
          cosFalloffStart(std::cos(Radians(falloffStart)))
    {}

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const;

    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    float Falloff(const Vector3f &w) const;
    Color3f Power() const;
    virtual Point3f GetPosition() const;
    virtual void Preprocess(const Scene &scene) {}

    // Member variables
    const Color3f I;
    const Point3f pLight;
    const float cosTotalWidth, cosFalloffStart;
};
