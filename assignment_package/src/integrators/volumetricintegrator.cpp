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

    while (depth > 0) {
        Intersection isect;
        bool hit = scene.Intersect(ray, &isect);

        // sample medium
        Intersection mi; // mi
        if (ray.medium) energy *= ray.medium->Sample(ray, sampler->Get1D(), &mi);
        if (IsBlack(energy)) break;

        // handle intersection with medium
        Vector3f woW = -ray.direction;
        if (mi.medInterface) {
            Color3f Ld = Color3f(0.f);
            Float lightPdf = 0, scatteringPdf = 0;
            // Sample light source with multiple importance sampling
            int index = std::min((int)(sampler->Get1D() * num), num - 1);
            const std::shared_ptr<Light> &light = scene.lights[index];
            Color3f Li = light->Sample_Li(mi, sampler->Get2D(), &wiW, &lightPdf);
            float f = 0.f;
            // Evaluate phase function for light sampling strategy
            if (lightPdf > 0.f && !IsBlack(Li)) scatteringPdf = mi.medInterface->inside->p(woW, wiW);

            if (scatteringPdf > 0.f) {
                // Compute effect of visibility for light source sample
                Ray light_ray(mi.SpawnRay(wiW));
                Color3f Tr(1.f);
                while (true) {
                    Intersection inter;
                    bool hitSurface = scene.Intersect(ray, &inter);
                    // Handle opaque surface along ray's path
                    if (hitSurface && isect.objectHit->GetMaterial() != nullptr)
                        return Color3f(0.f);

                    // Update transmittance for current ray segment
                    if (ray.medium) Tr *= ray.medium->Tr(light_ray, sampler);

                    // Generate next ray segment or return final transmittance
                    if (!hitSurface) break;
                    light_ray = inter.SpawnRay(wiW);
                }
                Li *= Tr;

                if (!IsBlack(Li)) {
                    Float weight = PowerHeuristic(1, lightPdf, 1, f);
                    Ld += scatteringPdf * Li * weight / lightPdf;
                }
            }

            // Sample scattered direction for medium interactions
            Color3f p = mi.medInterface->inside->Sample_p(woW, &wiW, sampler->Get2D());

            if (!IsBlack(p)) {
                // Account for light contributions along sampled direction _wi_
                Float weight = 1;
                lightPdf = light->Pdf_Li(mi, wiW);
                if (lightPdf != 0) {
                    weight = PowerHeuristic(1, p.x, 1, lightPdf);

                    // Find intersection and compute transmittance
                    Intersection lightIsect;
                    Ray ray = mi.SpawnRay(wiW);
                    Color3f Tr(1.f);
                    bool foundSurfaceInteraction = scene.IntersectTr(ray, sampler, &lightIsect, &Tr);

                    // Add light contribution from material sampling
                    Color3f Li(0.f);
                    if (foundSurfaceInteraction) {
                        if (lightIsect.objectHit->areaLight == scene.lights[index]) Li = lightIsect.Le(-wiW);
                    } else Li = light->Le(ray);
                    if (!IsBlack(Li)) Ld += f * Li * Tr * weight / p;
                }
            }

            color += energy * Ld;
            mi.medInterface->inside->Sample_p(woW, &wiW, sampler->Get2D());

            // Step 1: increment along ray by ds (ds is RayEpsilon in SpawnRay)
            ray = mi.SpawnRay(wiW);

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
            float wg = PowerHeuristic(1, pdf, 1, isect.bsdf->Pdf(woW, wiW));
            Color3f gColor(0.f);
            Intersection shad_Feel;
            Ray shadowRay = isect.SpawnRay(wiW);
            if (scene.Intersect(shadowRay, &shad_Feel)) {
                if (pdf > 0.f && shad_Feel.objectHit->areaLight == scene.lights[index])
                    gColor = f2 * li2 * AbsDot(wiW, isect.normalGeometric)/pdf;
            }
            // bsdf
            Color3f f1 = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf);
            Color3f fColor(0.f);
            Intersection isect_Test;
            Ray indirectRay = isect.SpawnRay(glm::normalize(wiW));
            if (pdf > 0.f && scene.Intersect(indirectRay, &isect_Test)) {
                if (isect_Test.objectHit->areaLight == scene.lights[index])
                    fColor = light->L(isect_Test, -wiW) * f1 * AbsDot(wiW, isect.normalGeometric)/pdf;
            }
            float wf = PowerHeuristic(1, pdf, 1, light->Pdf_Li(isect, wiW));
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


