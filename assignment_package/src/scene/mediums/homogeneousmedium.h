#pragma once
#include "medium.h"

// represents region of space with constant absorption and out-scattering
// uses Henyey-Greenstein phase function with constant g

class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const Color3f &sigma_a, const Color3f sigma_s, float g)
        : sigma_a(sigma_a), sigma_s(sigma_s), sigma_t(sigma_s + sigma_t), g(g) {}
private:
    const Color3f3 sigma_a, sigma_s, sigma_t;
    const float g;
};
