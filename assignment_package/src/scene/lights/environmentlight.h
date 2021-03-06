#pragma once

#include "light.h"
#include "warpfunctions.h"

class EnvironmentLight : public Light
{
public:
    EnvironmentLight(const Transform &t, const Color3f& Le,
                     const std::shared_ptr<QImage> map)
        : Light(t, INFINIT), map(map), lightColor(Le)
    {}

    Color3f Le(const Ray &r) const; // vtable issue -> not virtual

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const;

    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    virtual void Preprocess(Point3f center, float radius);

    // Member variables
    Point3f worldCenter;
    float worldRadius;
    const Color3f lightColor;
    const std::shared_ptr<QImage> map;
};
