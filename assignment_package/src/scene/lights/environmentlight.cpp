#include "environmentlight.h"

Color3f EnvironmentLight::Le(const Ray &r) const
{    
    Vector3f w = glm::normalize(Vector3f(transform.invT() * Vector4f(r.direction, 0.f)));
    float phi = std::atan2(w.y, w.x);
    float u = Inv2Pi * (phi < 0 ? (phi + TwoPi) : phi);
    float v = InvPi * std::acos(glm::clamp(w.z, -1.f, 1.f));
    return Color3f(GetImageColor(Point2f(u,v), map.get()));
}

Color3f EnvironmentLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                    Vector3f *wi, Float *pdf) const
{
    Vector3f w = WarpFunctions::squareToHemisphereCosine(xi);

    // transform from TBN to world space
    Matrix3x3 tangentToWorld = Matrix3x3(ref.tangent, ref.bitangent, ref.normalGeometric);
    *wi = glm::normalize(tangentToWorld * w);

    *pdf = WarpFunctions::squareToHemisphereCosinePDF(w);

    return L(ref, *wi);
}

Color3f EnvironmentLight::L(const Intersection &isect, const Vector3f &wi) const
{
    Vector3f w = Vector3f(glm::normalize(transform.invT() * Vector4f(wi, 0.f)));
    float phi = std::atan2(w.y, w.x);
    float u = Inv2Pi * (phi < 0 ? (phi + TwoPi) : phi);
    float v = InvPi * glm::acos(glm::clamp(w.z, -1.f, 1.f));
    return Color3f(GetImageColor(Point2f(u,v), map.get()));
}

float EnvironmentLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    Matrix3x3 tangentToWorld = Matrix3x3(ref.tangent, ref.bitangent, ref.normalGeometric);
    Matrix3x3 worldToTangent = glm::transpose(tangentToWorld);

    Vector3f w = glm::normalize(worldToTangent * wi);

    return WarpFunctions::squareToHemisphereCosinePDF(w);
}

void EnvironmentLight::Preprocess(Point3f center, float radius) {
    worldCenter = center;
    worldRadius = radius;
}
