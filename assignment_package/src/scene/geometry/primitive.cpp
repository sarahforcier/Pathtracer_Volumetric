#include "primitive.h"

Bounds3f Primitive::WorldBound() const {
    return shape->WorldBound();
}

bool Primitive::Intersect(const Ray &r, Intersection *isect) const
{
    if(!shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    isect->mediumInterface = mediumInterface;
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

const AreaLight* Primitive::GetAreaLight() const
{
    return areaLight.get();
}


const Material* Primitive::GetMaterial() const
{
    return material.get();
}

const MediumInterface* Primitive::GetMedium() const
{
    return mediumInterface.get();
}
