#include "primitive.h"

Bounds3f Primitive::WorldBound() const {
    return shape->WorldBound();
}

bool Primitive::Intersect(const Ray &r, Intersection *isect) const
{
    if(!shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    // set the medium interface at the intersection point
    //if (mediumInterface.isMediumTransition()) isect->mediumInterface = mediumInterface;
    //else isect->mediumInterface = MediumInterface(r.medium);
    // We create a BSDF for this intersection in our Integrator classes
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
