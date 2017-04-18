#pragma once
#include "medium.h"
#include "henyeygreenstein.h"


// represents region of space with constant absorption and out-scattering
// uses Henyey-Greenstein phase function with constant g

class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const Color3f &sigma_a, const Color3f sigma_s, float g)
        : sigma_a(sigma_a), sigma_s(sigma_s), sigma_t(sigma_s + sigma_t), g(g) {}

    float Tr(const Ray &ray, std::shared_ptr<Sampler> sampler) const;

    // samples a medium scattering interaction somewhere along ray before tMax
    // returns sampling weight
    // Li = Tr * Lo + Integral[Tr' * Ls]
        // Tr : Transmittance between ray origin and surface intersection
        // Lo : reflected radiance back along ray from point at surface intersection
        // Tr': Transmittance between ray origin and sampled point
        // Ls : added radiance along ray at sampled point due to volume scattering and emission
    float Sample(const Ray &ray, const float x, MediumInteraction *isect) const;
    float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const;

private:
    const float sigma_a, sigma_s, sigma_t;
    const float g;
    PhaseFunction phase;
};
