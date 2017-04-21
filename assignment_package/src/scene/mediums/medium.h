#pragma once
#include <globals.h>
#include <samplers/sampler.h>
#include <raytracing/intersection.h>

class Medium
{
public:
    virtual ~Medium() {}

    // returns the estimate of the transmittance on the interval
    // between the ray origin and the point at distance t = ray.tMax
    virtual float Tr(const Ray &ray) const = 0;

    // sample the integral form of the equation of transfer, consisting
    // of a surface and medium-related term
    virtual float Sample(const Ray &ray, bool* sampledMedium, Intersection *isect) const = 0;

    // samples an incident direction wi and a sample value in [0, 1)^2
    // does not return pdf since a call to p() will work
    virtual float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const = 0;

    virtual float p(const Vector3f &wo, const Vector3f &wi) const = 0;
};

// boundary between two different types of scattering media
// represented by the surface of a geometric primitive
struct MediumInterface {
    MediumInterface(std::shared_ptr<Medium> medium)
        : inside(medium), outside(medium) {}
    MediumInterface(std::shared_ptr<Medium> inside, std::shared_ptr<Medium> outside)
        : inside(inside), outside(outside) {}
    bool IsInterface() { return outside != inside;}
    void Swap() {auto temp = outside; outside = inside; inside = temp;}

    std::shared_ptr<Medium> inside, outside; // nullptr to indicate vacuum
};

inline float PhaseHG(float cosTheta, float g) {
    float denom = 1.f + g * g - 2.f * g * cosTheta;
    return Inv4Pi * (1.f - g * g) / (denom * glm::sqrt(denom));
}
