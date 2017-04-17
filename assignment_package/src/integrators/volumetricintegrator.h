#pragma once
#include "integrator.h"

class VolumetricIntegrator : public Integrator
{
public:
    VolumetricIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // energy transmitted along the ray back to its origin
    virtual Color3f Li(Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;
};
