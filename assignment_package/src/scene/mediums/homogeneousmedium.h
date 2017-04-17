#pragma once
#include "medium.h"

// represents region of space with constant absorption and out-scattering
// uses Henyey-Greenstein phase function with constant g

class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const Color3f &sigma_a, const Color3f sigma_s, float g)
        : sigma_a(sigma_a), sigma_s(sigma_s), sigma_t(sigma_s + sigma_t), g(g) {}

    Color3f Tr(const Ray &ray, Sampler &sampler) const;
    Color3f Sample(const Ray &ray, Sampler &sampler, Intersection *isect) const;
    float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const;

private:
    const Color3f sigma_a, sigma_s, sigma_t;
    const float g;
};
