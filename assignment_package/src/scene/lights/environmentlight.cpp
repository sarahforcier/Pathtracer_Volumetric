#include "environmentlight.h"

Color3f EnvironmentLight::Le(const Ray &r) const
{
    Vector3f w = Vector3f(glm::normalize(transform.invT() * Vector4f(r.direction, 0.f)));
    float phi = glm::atan2(w.y, w.x);
    float u = Inv2Pi * (phi < 0 ? (phi + TwoPi) : phi);
    float v = InvPi * glm::acos(glm::clamp(w.z, -1.f, 1.f));
    return Color3f(GetMaterialColor(Point2f(u,v), map));
}

Color3f EnvironmentLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                    Vector3f *wi, Float *pdf) const
{
    Point2f uv(); // TODO
    if (mapPdf == 0.f) return Color3f(0.f);

    float phi = uv[0] * TwoPi, theta = uv[1] * Pi;
    float cosTheta = glm::cos(theta), sinTheta = glm::sin(theta);
    float cosPhi = glm::cos(phi), sinPhi = glm::sin(phi);
    Vector4f w(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta, 0.f);
    *wi = Vector3f(glm::normalize(transform.T() * w));

    *pdf = (sinTheta == 0.f) ? 0.f : mapPdf / (TwoPi * Pi * sinTheta);

    return Color3f(GetMaterialColor(uv, map));
}

Color3f EnvironmentLight::L(const Intersection &isect, const Vector3f &w) const
{
    return Color3f(0.f);
}

float EnvironmentLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    Vector3f w = Vector3f(glm::normalize(transform.invT() * Vector4f(wi, 0.f)));
    float theta = glm::acos(glm::clamp(w.z, -1.f, 1.f));
    float p = glm::atan2(w.y, w.x); phi = Inv2Pi * (p < 0 ? (p + TwoPi) : p);
    float sinTheta = glm::sin(theta);
    if (sinTheta == 0) return 0.f;
    return distribution->Pdf(Point2f(phi * Inv2Pi, theta * InvPi)) / // TODO
            (TwoPi * Pi * sinTheta);
}

Color3f EnvironmentLight::Power() const {
    return Pi * worldRadius * worldRadius * GetImageColor(Point2f(), map); //TODO
}

void EnvironmentLight::Preprocess(const Scene &scene)
{
    scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
}
