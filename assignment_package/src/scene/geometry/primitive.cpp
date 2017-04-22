#include "primitive.h"

Bounds3f Primitive::WorldBound() const {
    if (!shape) return Bounds3f();
    return shape->WorldBound();
}

bool Primitive::Intersect(Ray &r, Intersection *isect) const
{
    if(!shape || !shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    isect->mediumInterface = mediumInterface;
    if (glm::dot(isect->normalGeometric, -r.direction) < 0.f) {
        isect->mediumInterface->Swap();
    }
    return true;
}

bool Primitive::ProduceBSDF(Intersection *isect) const
{
    if(material)
    {
        material->ProduceBSDF(isect);
        return true;
    }
    return false;
}

const Light* Primitive::GetLight() const
{
    return light.get();
}

const Material* Primitive::GetMaterial() const
{
    return material.get();
}

const MediumInterface* Primitive::GetMedium() const
{
    return mediumInterface.get();
}
