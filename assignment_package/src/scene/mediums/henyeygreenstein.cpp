#include "henyeygreenstein.h"

float HenyeyGreenstein::Sample_p(const Vector3f &wo, Vector3f *wi,
                                 const Point2f &u) const {
    float cosTheta;
    if (std::abs(g) < 0.001)
        cosTheta = 1.f - 2.f * u[0];
    else {
        float sqrTerm = (1.f - g * g) / (1.f - g + 2.f * g * u[0]);
        cosTheta = (1.f + g * g - sqrTerm * sqrTerm) / (2.f * g);
    }

    // Compute direction
    float sinTheta = glm::sqrt(glm::max(0.f, 1.f - cosTheta * cosTheta));
    float phi = 2.f * Pi * u[1];
    Vector3f v1, v2;
    CoordinateSystem(wo, &v1, &v2);
    *wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
    return PhaseHG(-cosTheta, g);
}

float HenyeyGreenstein::p(const Vector3f &wo, const Vector3f &wi) const {
    ProfilePhase _(Prof::PhaseFuncEvaluation);
    return PhaseHG(Dot(wo, wi), g);
}
