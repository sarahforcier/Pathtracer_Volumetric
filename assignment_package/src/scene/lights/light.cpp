#include "light.h"

Color3f Light::Le(const Ray &r) const
{
    return Color3f(0.f);
}

Point3f Light::GetPosition() const
{
    return transform.position();
}
