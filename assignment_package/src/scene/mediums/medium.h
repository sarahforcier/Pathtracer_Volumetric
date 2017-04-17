#pragma once
#include <globals.h>
#include <samplers/sampler.h>
#include <raytracing/ray.h>
#include <raytracing/intersection.h>

class Medium;  // forward declarations;

// boundary between two different types of scattering media
// represented by the surface of a geometric primitive
struct MediumInterface {
    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {}
    MediumInterface(const Medium *inside, const Medium *outside) : inside(inside), outside(outside) {}
    bool isMediumTransition() {return inside != outside;}
    const Medium *inside, *outside; // nullptr to indicate vacuum
};

class Medium
{
public:
    virtual ~Medium() {}

    // returns the estimate of the transmittance on the interval
    // between the ray origin and the point at distance t = ray.tMax
    virtual Color3f Tr(const Ray &ray, Sampler &sampler) const = 0;

    // sample the integral form of the equation of transfer, consisting
    // of a surface and medium-related term
    virtual Color3f Sample(const Ray &ray, Sampler &sampler, Intersection *isect) const = 0;

    // samples an incident direction wi and a sample value in [0, 1)^2
    // does not return pdf since a call to p() will work
    virtual float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const = 0;
};

inline Float PhaseHG(Float cosTheta, Float g) {
    Float denom = 1 + g * g + 2 * g * cosTheta;
    return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}
