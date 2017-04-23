#include "volumetricintegrator.h"

// Color = Sum[dt * c(x) * p(x) * T(r, t)]
// density per unit length
// T(Ray r, float t) = exp(integral)

// Notes:
// 1. participating medium has no color, but can consider
// different densities for RGB
// 2. can precompute light transmittance at voxel centers
// 3. book: Ls = Le + sigma_s Integral(phase function * Li dw)

Color3f VolumetricIntegrator::Li(Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{

    Color3f energy = Color3f(1.f); // beta
    Color3f color = Color3f(0.f); // L
    // declarations
    float pdf; Vector3f wiW;
    BxDFType type = BSDF_DIFFUSE;
    BxDFType all = BSDF_ALL;
    int recursionLimit = depth;

    // number of lights
    int num= scene.lights.length();
    if (num == 0) return Color3f(0.f);
    bool sampledMedium;

    while (depth > 0) {
        Intersection isect;
        bool hit = scene.Intersect(ray, &isect);

        sampledMedium = false;
        // sample medium
        if (ray.medium) {
            energy *= ray.medium->Sample(ray, &sampledMedium, &isect);
        } else {
            if (!hit) break;
        }
        if (IsBlack(energy)) break;

        // handle intersection with medium
        Vector3f woW = -ray.direction;
        if (sampledMedium) {
            Color3f Ld = Color3f(0.f);
            float lightPdf = 0, phaseLight = 0;

            // Sample light source with multiple importance sampling
            int index = std::min((int)(sampler->Get1D() * num), num - 1);
            const std::shared_ptr<Light> &light = scene.lights[index];
            Color3f Li = light->Sample_Li(isect, sampler->Get2D(), &wiW, &lightPdf);

            // Evaluate phase function for light sampling
            // phase function = pdf
            if (lightPdf > 0.f && !IsBlack(Li)) phaseLight = isect.mediumInterface->outside->p(woW, wiW);

            if (phaseLight > 0.f) {
                // Compute effect of visibility for light source sample
                Ray light_ray = Ray(isect.point, wiW, ray.medium);
                Color3f Tr(1.f);
                Intersection shadFeel;
                bool hitSurface = scene.Intersect(light_ray, &shadFeel);
                // return final transmittance if not hit anything before light
                if (hitSurface && shadFeel.objectHit->light == scene.lights[index]) {
                    // Update transmittance for current ray segment
                    if (light_ray.medium) Tr *= light_ray.medium->Tr(light_ray);
                } else Li = Color3f(0.f);

                Li *= Tr;

                if (!IsBlack(Li)) {
                    Float weightLight = PowerHeuristic(1, lightPdf, 1, phaseLight);
                    Ld += phaseLight * Li * weightLight / lightPdf;
                }
            }

            // Sample scattered direction for medium interactions
            float phaseMedium = isect.mediumInterface->outside->Sample_p(woW, &wiW, sampler->Get2D());

            if (phaseMedium > 0.f) {
                // Account for light contributions along sampled direction _wi_
                float weightMedium = 1;
                lightPdf = light->Pdf_Li(isect, wiW);
                if (lightPdf > 0.f) {
                    weightMedium = PowerHeuristic(1, phaseMedium, 1, lightPdf);

                    // Find intersection and compute transmittance
                    Intersection lightIsect;
                    Ray ray = isect.SpawnRay(wiW);
                    Color3f Tr(1.f);
                    bool foundSurfaceInteraction = scene.IntersectTr(ray, sampler, &lightIsect, &Tr);

                    // Add light contribution from material sampling
                    Color3f Li(0.f);
                    if (foundSurfaceInteraction) {
                        if (lightIsect.objectHit->light == scene.lights[index])
                            Li = lightIsect.Le(-wiW);
                    } else Li = light->Le(ray);

                    if (!IsBlack(Li))
                        Ld += Li * Tr * weightMedium;
                }
            }

            color += energy * Ld;
            isect.mediumInterface->outside->Sample_p(woW, &wiW, sampler->Get2D());

            // spawn ray from sampled point in medium
            ray = Ray(isect.point, wiW, ray.medium);

            // handle intersection with surface (full lighting)
        } else {
            if (!hit) break;
            if (!isect.objectHit->GetMaterial()) {
                if ((BSDF_SPECULAR & type) != 0 || depth == recursionLimit) color += energy * isect.Le(woW);
                break;
            }
            isect.ProduceBSDF();

            // light
            int index = std::min((int)(sampler->Get1D() * num), num - 1);
            const std::shared_ptr<Light> &light = scene.lights[index];
            Color3f li2 = light->Sample_Li(isect, sampler->Get2D(), &wiW, &pdf);
            Color3f f2 = isect.bsdf->f(woW, wiW);
            float wg = 1.f, wf = 1.f;
            Color3f gColor(0.f);
            Intersection shad_Feel;
            Ray shadowRay = (light->isDelta) ? isect.SpawnRayTo(light->GetPosition()) : isect.SpawnRay(wiW);
            if (scene.Intersect(shadowRay, &shad_Feel)) {
                if (pdf > 0.f && shad_Feel.objectHit->light == scene.lights[index]) {
                    wg = PowerHeuristic(1, pdf, 1, isect.bsdf->Pdf(woW, wiW));
                    gColor = f2 * li2 * AbsDot(wiW, isect.normalGeometric)/pdf;
                }
            } else if (light->isDelta) gColor = f2 * li2 * AbsDot(wiW, isect.normalGeometric)/pdf;

            // bsdf
            Color3f fColor(0.f);
            if (!light->isDelta) {
                Color3f f1 = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf);
                Intersection isect_Test;
                Ray indirectRay = isect.SpawnRay(glm::normalize(wiW));
                if (pdf > 0.f && scene.Intersect(indirectRay, &isect_Test)) {
                    if (isect_Test.objectHit->light == scene.lights[index])
                        fColor = light->L(isect_Test, -wiW) * f1 * AbsDot(wiW, isect.normalGeometric)/pdf;
                }
                wf = PowerHeuristic(1, pdf, 1, light->Pdf_Li(isect, wiW));
            }
            Color3f d_color = float(num) * (wf * fColor + wg * gColor);

            color += energy * d_color;

            // global illumination
            Color3f f0 = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf, all, &type);
            if (pdf > 0.f) {
                Color3f f = f0 * AbsDot(wiW, isect.normalGeometric)/pdf;
                energy *= f;
            }

            // update recursive ray bounce
            ray = isect.SpawnRay(wiW);
        }
        
        // russian roulette termination check
        float q = sampler->Get1D();
        float E = glm::max(energy.x, glm::max(energy.y, energy.z));
        if (depth < 3) {
            if (E < q) break;
            energy /= (1 - q);
        }

        // loop updates
        depth--;
    }
    return color;
}


