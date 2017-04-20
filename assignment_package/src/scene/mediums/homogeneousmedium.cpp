#include "homogeneousmedium.h"

// Beer's Law
float HomogeneousMedium::Tr(const Ray &ray) const
{
    // TODO need to handle case when no intersection so tMax = infinity
    // TODO need to insure ray.direction is always normalized for calc of tMax
    return glm::exp(-sigma_t * ray.tMax * glm::length(ray.direction));
}

float HomogeneousMedium::Sample(const Ray &ray, const float x, Intersection *inter) const
{
    // sample a distance along the ray
    float t = glm::min(- std::log(1 - x) / sigma_t, ray.tMax); // sample free path
    float pdf = sigma_t * std::exp(-sigma_t * t);
    bool sampledMedium = t < ray.tMax;

    if (sampledMedium) { // rewrite inter
        *inter = Intersection();
        inter->point = ray(t);
        inter->mediumInterface = std::make_shared<MediumInterface>(ray.medium);
        inter->t = t;
    }

    // compute the transmittance
    float Tr = glm::exp(-sigma_t * t * glm::length(ray.direction));

    // return weighting factor for in-scattering (3rd term)
    return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}

float HomogeneousMedium::Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const {
    // sample cosTheta using henyey greenstein inversion method
    float cosTheta;
    if (std::abs(g) < 0.001) cosTheta = 1 - 2 * u[0];
    else {
        float sqrTerm = (1 - g*g) / (1 - g + 2*g*u[0]);
        cosTheta = (1 + g*g - sqrTerm*sqrTerm) / (2*g);
    }

    // compute wi
    float sinTheta = std::sqrt(std::max(0.f, 1 - cosTheta*cosTheta));
    float phi = TwoPi * u[1];
    Vector3f v1, v2;
    CoordinateSystem(wo, &v1, &v2);
    *wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
    return PhaseHG(-cosTheta, g);
}

float HomogeneousMedium::p(const Vector3f &wo, const Vector3f &wi) const {
    // cosTheta = -w . w
    return PhaseHG(glm::dot(-wo, wi), g);
}
