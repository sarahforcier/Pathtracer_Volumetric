#pragma once
#include "medium.h"
#include "warpfunctions.h"

// represents region of space with constant absorption and out-scattering
// uses Henyey-Greenstein phase function with constant g

class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const float &sigma_a, const float sigma_s, float g, float density)
        : sigma_a(sigma_a), sigma_s(sigma_s), g(g), density(density) { sigma_t = sigma_s + sigma_a;}

    float Tr(const Ray &ray) const;

    // samples a medium scattering interaction somewhere along ray before tMax
    // returns sampling weight
    // Li = Tr * Lo + Integral[Tr' * Ls]
        // Tr : Transmittance between ray origin and surface intersection
        // Lo : reflected radiance back along ray from point at surface intersection
        // Tr': Transmittance between ray origin and sampled point
        // Ls : added radiance along ray at sampled point due to volume scattering and emission
    float Sample(const Ray &ray, const float x, Intersection *isect) const;

    // returns Phase pdf
    float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const;

    // Henyey Greenstein phase function
    float p(const Vector3f &wo, const Vector3f &wi) const;

private:
    float sigma_a, sigma_s, sigma_t, g, density;
};
