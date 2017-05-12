#include "cylinder.h"

Bounds3f Cylinder::WorldBound() const {
   Vector3f min = Vector3f(-1.f, -1.f, -1.f);
   Vector3f max = Vector3f(1.f, 1.f, 1.f);
   Bounds3f bound = Bounds3f(min, max);
   return bound.Apply(transform);
}

float Cylinder::Area() const
{
    return TwoPi * 2.f;
}

Intersection Cylinder::Sample(const Point2f &xi, Float *pdf) const {

    Intersection inter;

    // Generate a world-space point on the surface of the shape.
    float y = glm::mix(xi[0], -1.f, 1.f);
    float phi = xi[1] * TwoPi;
    Point4f pObj = Point4f(std::cos(phi), y, std::sin(phi), 1.f);
    Point4f pW = transform.T() * pObj;

    inter.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(pObj.x, 0.f, pObj.z));
    inter.point = Point3f(pW);

    *pdf = 1 / Area();
    return inter;
}

bool Cylinder::Intersect(Ray& ray, Intersection* isect) const
{
    Ray r_loc = ray.GetTransformedCopy(transform.invT());
    Vector3f d = r_loc.direction;
    Vector3f o = r_loc.origin;

    // solve for x2 + z2 - r2 = 0 for infinite cylinder
    float A = d.x * d.x + d.z * d.z;
    float B = 2.f * (d.x * o.x + d.z * o.z);
    float C = o.x * o.x + o.z * o.z - 1.f;
    float discriminant = B*B - 4*A*C;

    if(discriminant < 0){
        return false;
    }
    float tMin = (-B - sqrt(discriminant))/(2*A);
    float tMax = (-B + sqrt(discriminant))/(2*A);
    if(tMin < 0)
    {
        tMin = tMax;
        tMax = INFINITY;
    }
    if(tMin >= 0)
    {
        if (tMin > ray.tMax) return false;
        Point3f P = glm::vec3(r_loc.origin + tMin*r_loc.direction);
        if (glm::abs(P.y) < 1.f) {
            InitializeIntersection(isect, tMin, tMax, P);
            return true;
        }
    }
    return false;
}


void Cylinder::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{

    *nor = glm::normalize(transform.invTransT() * Normal3f(P.x, 0.f, P.z));
    *tan = glm::normalize(transform.T3() * glm::cross(Vector3f(0,1,0), *nor));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}

Vector2f Cylinder::GetUVCoordinates(const glm::vec3 &point) const
{
    float phi = std::atan2(point.z, point.x);
    if (phi < 0.f) phi += TwoPi;
    return Vector2f(phi / TwoPi, (point.y + 1.f) / 2.f);
}
