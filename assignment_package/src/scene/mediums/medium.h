#pragma once
#include <globals.h>
#include <samplers/sampler.h>
#include <raytracing/intersection.h>
#include "phasefunction.h"

class Medium
{
public:
    virtual ~Medium() {}

    // returns the estimate of the transmittance on the interval
    // between the ray origin and the point at distance t = ray.tMax
    virtual float Tr(const Ray &ray, std::shared_ptr<Sampler> sampler) const = 0;

    // sample the integral form of the equation of transfer, consisting
    // of a surface and medium-related term
    virtual Color3f Sample(const Ray &ray, const float x, Intersection *isect) const = 0;

    // samples an incident direction wi and a sample value in [0, 1)^2
    // does not return pdf since a call to p() will work
    virtual Color3f Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const = 0;
};

// boundary between two different types of scattering media
// represented by the surface of a geometric primitive
struct MediumInterface {
    MediumInterface(const Medium *medium, const std::shared_ptr<PhaseFunction> phase)
        : inside(medium), outside(medium), p_inside(phase), p_outside(phase) {}
    MediumInterface(const Medium *inside, const Medium *outside,
                    const std::shared_ptr<PhaseFunction> p_inside, const std::shared_ptr<PhaseFunction> p_outside)
        : inside(inside), outside(outside), p_inside(p_inside), p_outside(p_outside) {}
    bool isMediumTransition() {return inside != outside;}

    const Medium *inside, *outside; // nullptr to indicate vacuum
    const std::shared_ptr<PhaseFunction> p_inside, p_outside;
};
