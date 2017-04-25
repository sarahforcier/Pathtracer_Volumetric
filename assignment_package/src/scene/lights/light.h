#pragma once

#include <globals.h>
#include <scene/transform.h>
#include <scene/scene.h>
#include <raytracing/intersection.h>
#include <QImage>
#include <QColor>

class Intersection;
class Scene;

enum type { DELTA, AREA, INFINIT };

class Light
{
  public:
    virtual ~Light(){}
    Light(Transform t, int type)
        : transform(t), name(), type(type)
    {}

    virtual Color3f Le(const Ray &r) const;

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const = 0;

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const = 0;

    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const = 0;

    virtual Point3f GetPosition() const;

    virtual void Preprocess(const Scene &scene) {}

    bool isDelta() {return type == DELTA;}

    QString name; // For debugging
    int type;

  protected:
    const Transform transform;
};

class AreaLight : public Light
{
public:
    AreaLight(const Transform &t) : Light(t, AREA){}
    // Returns the light emitted from a point on the light's surface _isect_
    // along the direction _w_, which is leaving the surface.
    virtual Color3f L(const Intersection &isect, const Vector3f &w) const = 0;
//    virtual void Preprocess(const Scene &scene) {}
};

static Color3f GetImageColor(const Point2f &uv_coord, const QImage* const image)
{
    if(image)
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return Color3f(color.red(), color.green(), color.blue())/255.0f;
    }
    return Color3f(1.f, 1.f, 1.f);
}
