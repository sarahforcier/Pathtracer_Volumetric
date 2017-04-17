#include "homogeneousmedium.h"

// Beer's Law
Color3f HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const
{
    // TODO need to handle case when no intersection so tMax = infinity
    // TODO need to insure ray.direction is always normalized for calc of tMax
    return glm::exp(-sigma_t * ray.tMax * ray.direction.length());
}

Color3f HomogeneousMedium::Sample(const Ray &ray, Sampler &sampler, Intersection *isect) const {
    // sample a channel (because sigma_t varies by wavelength)
    int channel = glm::min((int)(sampler.Get1D() * Spectrum::nSaamples), Spectrum::nSamples - 1);

    // sample a distance along the ray
    float dist = -std::log(1 - sampler.Get1D()) / sigma_t[channel];
    float t = std::min(dist * ray.direction.length(), ray.tMax);
    bool sampledMedium = t < ray.tMax;
    if (sampledMedium) {
        isect->medium = this;
    }

    // compute the transmittance and sampling density
    Color3f Tr = Tr(ray, sampler);

    // return weighting factor for scattering from homogeneous medium
    Color3f density = sampleMedium ? (sigma_t * Tr) : Tr;
    float pdf = 0.f;
    for (int i = 0; i < Spectrum::nSamples; ++i) pdf += density[i];
    pdf *= 1/ (float) Spectrum::nSamples;
    return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}

float HomogeneousMedium::Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const {
    // sample cosTheta
    float cosTheta;
    if (std::abs(g) < 0.001) cosTheta = 1 - 2 * u[0];
    else {
        float sqrTerm = (1 - g*g) / (1 - g + 2*g*u[0]);
        cosTheta = (1 - g*g - sqrTerm*sqrTerm) / (2*g);
    }

    // compute wi
    float sinTheta = std::sqrt(std::max(0.f, 1 - cosTheta*cosTheta));
    float phi = 2 * Pi * u[1];
    Vector3f v1, v2;
    CoordinateSystem(wo, &v1, &v2);
    *wi = SphericalDirection(sinTheta, cosTheta, phi, vi, v2, -wo);
    return PhaseHG(-cosTheta, g);
}
