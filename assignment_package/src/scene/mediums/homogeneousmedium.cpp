#include "homogeneousmedium.h"

// Beer's Law
float HomogeneousMedium::Tr(const Ray &ray, std::shared_ptr<Sampler> sampler) const
{
    // TODO need to handle case when no intersection so tMax = infinity
    // TODO need to insure ray.direction is always normalized for calc of tMax
    return glm::exp(-sigma_t * ray.tMax * glm::length(ray.direction));
}

Color3f HomogeneousMedium::Sample(const Ray &ray, const float x, Intersection *inter) const
{
    // sample a distance along the ray
    float t = - std::log(x) / sigma_t;
    float pdf = sigma_t * std::exp(-sigma_t * t);
    bool sampledMedium = t < ray.tMax;
    if (sampledMedium) {
        std::shared_ptr<HenyeyGreenstein> phase = std::make_shared<HenyeyGreenstein>(g);
        inter->medInterface = std::make_shared<MediumInterface>(this, phase);
    }

    // compute the transmittance and sampling density
    float Tr = glm::exp(-sigma_t * ray.tMax * glm::length(ray.direction));

    // return weighting factor for scattering from homogeneous medium
    float c = sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
    return Color3f(c);
}

Color3f HomogeneousMedium::Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const {
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
    *wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
    float c = PhaseHG(-cosTheta, g);
    return Color3f(c);
}
