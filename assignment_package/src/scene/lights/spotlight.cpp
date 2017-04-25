#include "spotlight.h"

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const {
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    return 2.f * I * Falloff(-*wi) / glm::length(pLight - ref.point);
}

float SpotLight::Falloff(const Vector3f &w) const {
    // transform w to light coordinates
    Vector3f wl = Vector3f(glm::normalize(transform.invT() * Vector4f(w, 0.f)));
    float cosTheta = wl.z;
    if (cosTheta < cosTotalWidth) return 0.f;
    if (cosTheta > cosFalloffStart) return 1.f;

    // compute falloff inside spotlight cone
    float d = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return d * d * d * d;
}

Color3f SpotLight::Power() const {
    return I * TwoPi * (1.f - 0.5f * (cosFalloffStart + cosTotalWidth));
}

// returns color at isect coming in from direction w
Color3f SpotLight::L(const Intersection &isect, const Vector3f &w) const
{
    return 2.f * I * Falloff(-w) / glm::length(pLight - isect.point);
}

float SpotLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const {
    return 0.f;
}

Point3f SpotLight::GetPosition() const
{
    return pLight;
}

