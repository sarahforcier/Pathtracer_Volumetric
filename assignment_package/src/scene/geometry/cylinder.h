#pragma once

#include <scene/geometry/shape.h>

//A Cylinder is assumed to have a radius of 1, centered at <0,0,0> with and a height of 2 along the y axis
class Cylinder : public Shape
{
public:
    virtual bool Intersect(Ray &ray, Intersection *isect) const;
    virtual Point2f GetUVCoordinates(const Point3f &point) const;
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;

    virtual float Area() const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, float *pdf) const;

    Bounds3f WorldBound() const;

    void create();
};
