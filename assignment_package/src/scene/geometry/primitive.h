#pragma once
#include "shape.h"
#include <scene/lights/light.h>
#include <scene/materials/material.h>
//#include <scene/mediums/medium.h>
#include <scene/bounds.h>

// forward declaration only says the class will exist, but not what member functions it will have
// thus any uses of shape must be defined in the cpp file
class AreaLight;
class Shape;

// A class that holds all the information for a given object in a scene,
// such as its Shape, its Material, and (if applicable) ita AreaLight
// Parallels the GeometricPrimitive class from PBRT
class Primitive
{
public:
    Primitive() :
        name("Some Primitive"), shape(nullptr), material(nullptr), areaLight(nullptr)
    {}
    Primitive(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material = nullptr, std::shared_ptr<AreaLight> areaLight = nullptr)
        : shape(shape), material(material), areaLight(areaLight)
    {}
    // Returns whether or not the given Ray intersects this Primitive.
    // Passes additional intersection data through the Intersection pointer
    virtual bool Intersect(const Ray& r, Intersection* isect) const;

    const AreaLight* GetAreaLight() const;
    const Material* GetMaterial() const;
    // Ask our _material_ to generate a BSDF containing
    // BxDFs and store it in isect.
    bool ProduceBSDF(Intersection *isect) const;

    virtual Bounds3f WorldBound() const;

    QString name;//Mainly used for debugging purposes
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material; // can be nullptr
    std::shared_ptr<AreaLight> areaLight;
    //std::shared_ptr<MediumInterface> mediumInterface;
};
