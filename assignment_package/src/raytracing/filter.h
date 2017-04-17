#pragma once
#include <globals.h>

class Filter
{
public:
    virtual ~Filter();
    Filter(const Vector2f &radius)
        : radius(radius), invRadius(Vector2f(1/radius.x, 1/radius.y)) {}

    // returns filter value at point p relative to center of filter
    // never call with points outside of filter's extent (2 * radius)
    virtual float Evaluate(const Point2f &p) const = 0;

    const Vector2f radius, invRadius;
};
