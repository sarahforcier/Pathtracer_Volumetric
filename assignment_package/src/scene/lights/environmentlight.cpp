#include "environmentlight.h"

Color3f EnvironmentLight::Le(const Ray &r) const
{    
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());

    float A = pow(r_loc.direction.x, 2.f) + pow(r_loc.direction.y, 2.f) + pow(r_loc.direction.z, 2.f);
    float B = 2*(r_loc.direction.x*r_loc.origin.x + r_loc.direction.y * r_loc.origin.y + r_loc.direction.z * r_loc.origin.z);
    float C = pow(r_loc.origin.x, 2.f) + pow(r_loc.origin.y, 2.f) + pow(r_loc.origin.z, 2.f) - 1.f;//Radius is 1.f
    float discriminant = B*B - 4*A*C;

    float t = (-B + sqrt(discriminant))/(2*A);

    Point3f P = glm::vec3(r_loc.origin + t*r_loc.direction);

    Vector3f w = P - worldCenter;
    float phi = std::atan2(w.y, w.x);
    float u = Inv2Pi * (phi < 0 ? (phi + TwoPi) : phi);
    float v = InvPi * glm::acos(glm::clamp(w.z, -1.f, 1.f));
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

void EnvironmentLight::Preprocess(const Scene &scene) {
    scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
}
