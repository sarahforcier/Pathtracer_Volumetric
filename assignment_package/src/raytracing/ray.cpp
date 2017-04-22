#include <raytracing/ray.h>

Ray::Ray(const Point3f &o, const Vector3f &d, std::shared_ptr<Medium> medium):
    origin(o),
    direction(d),
    medium(medium),
    tMax(INFINITY)
{}

Ray::Ray(const Point3f &o, const Vector3f &d, float tMax):
    origin(o),
    direction(d),
    medium(nullptr),
    tMax(tMax)
{}

Ray::Ray(const Point3f &o, const Vector3f &d):
    origin(o),
    direction(d),
    medium(nullptr),
    tMax(INFINITY)
{}

Ray::Ray(const glm::vec4 &o, const glm::vec4 &d):
    Ray(Point3f(o), Vector3f(d))
{}

Ray::Ray(const Ray &r):
    Ray(r.origin, r.direction)
{}

Ray Ray::GetTransformedCopy(const Matrix4x4 &T) const
{

    glm::vec4 o = glm::vec4(origin, 1);
    glm::vec4 d = glm::vec4(direction, 0);

    o = T * o;
    d = T * d;

    return Ray(o, d);
}
