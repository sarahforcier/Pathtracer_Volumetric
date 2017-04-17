#include "volumetricintegrator.h"

Color3f VolumetricIntegrator::Li(Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const {
    // intersect ray with scene and store intersection in isect
    // sample participating medium if any
    // handle interaction
        // handle scattering at a point in medium
        // handle surface
    // terminate with russian roulette
}
