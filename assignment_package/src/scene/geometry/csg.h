#pragma once

#include "shape.h"
#include "primitive.h"

struct CSGNode;
enum operation { OBJECT, UNION , INTER, DIFFER };

class CSG : public Primitive
{
public:
    CSG(std::vector<std::shared_ptr<Primitive>> ps, std::vector<operation> ops);
    ~CSG();
    Bounds3f WorldBound() const {
        Bounds3f bound;
        for (std::shared_ptr<Primitive> s : primitives) bound = Union(bound, s->WorldBound());
        return bound;
    }
    virtual bool Intersect(const Ray &ray, Intersection *isect) const;

    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<operation> operators;

private:
    CSGNode *recursiveBuild(int* shape_num, int* oper_num);
    bool recursiveIntersect(CSGNode* node, const Ray &ray, Intersection *isect) const;
    CSGNode *root = nullptr;
};
