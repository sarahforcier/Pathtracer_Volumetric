#include "csg.h"

struct CSGNode {
    void InitLeaf(int index) {
        shape_index = index;
        oper = 0;
    }
    void InitInterior(int operation, CSGNode *c0, CSGNode *c1) {
        children.push_back(c0);
        children.push_back(c1);
        oper = operation;
        shape_index = -1;
    }
    std::vector<CSGNode*> children;
    int shape_index;
    int oper;
};

void deleteNode(CSGNode* node) {
    if (node->children.size() != 0) {
        for (auto i : node->children) deleteNode(i);
    }
    delete node;
}

CSG::~CSG() {
    deleteNode(root);
}

// shape_num: index into shape array
// oper_num: index into operator array
CSGNode* CSG::recursiveBuild(int* shape_num, int* oper_num) {

    CSGNode* node = new CSGNode();
    operation oper = operators[(*oper_num)++];
    switch (oper) {
    case OBJECT :
        node->InitLeaf((*shape_num)++);
        break;
    default :
        CSGNode *left = recursiveBuild(shape_num, oper_num);
        CSGNode *right = recursiveBuild(shape_num, oper_num);
        node->InitInterior(oper, left, right);
    }
    return node;
}

CSG::CSG(std::vector<std::shared_ptr<Primitive>> ps, std::vector<operation> ops)
    : primitives(ps), operators(ops) {
    int shape_num = 0;
    int oper_num = 0;
    root = recursiveBuild(&shape_num, &oper_num);
}

bool CSG::recursiveIntersect(CSGNode* node, Ray &ray, Intersection *isect) const {
    bool hit = false;
    Intersection inter0, inter1;
    bool hit0, hit1;
    switch (node->oper) {
    case OBJECT:
        if (primitives[node->shape_index]->Intersect(ray, &inter0)) {
            hit = true;
            *isect = inter0;
        }
        break;
    case UNION:
        hit0 = recursiveIntersect(node->children[0], ray, &inter0);
        hit1 = recursiveIntersect(node->children[1], ray, &inter1);
        if (hit0 || hit1) {
            hit = true;
            if (!hit0) *isect = inter1;
            else if (!hit1) *isect = inter0;
            else if (inter0.t < inter1.t) *isect = inter0;
            else *isect = inter1;
        }
        break;
    case DIFFER:
        hit0 = recursiveIntersect(node->children[0], ray, &inter0);
        hit1 = recursiveIntersect(node->children[1], ray, &inter1);
        if (hit0 && hit1) {
            if (inter0.t < inter1.t || inter1.tMax < inter0.t ) {
                hit = true;
                *isect = inter0;
            }
            else if (inter1.tMax < inter0.tMax) {
                hit = true;
                *isect = inter1;
                isect->objectHit = inter0.objectHit; // color
            }
        }
        if (hit0 && !hit1) {
            hit = true;
            *isect = inter0;
        }
        break;
    case INTER:
        hit0 = recursiveIntersect(node->children[0], ray, &inter0);
        hit1 = recursiveIntersect(node->children[1], ray, &inter1);
        if (hit0 && hit1) {
            if (inter0.t > inter1.t && inter0.tMax < inter1.tMax) {
                hit = true; *isect = inter0; // 0 inside 1
            } else if (inter1.t > inter0.t && inter1.tMax < inter0.tMax) {
                hit = true; *isect = inter1; // 1 inside 0
                isect->objectHit = inter0.objectHit;
            } else if (inter0.t < inter1.t && inter0.tMax > inter1.t) {
                hit = true; *isect = inter1;
                isect->objectHit = inter0.objectHit; // color
            } else if (inter1.t < inter0.t && inter1.tMax > inter0.t) {
                hit = true; *isect = inter0;
            }
        }
    }
    return hit;
}

bool CSG::Intersect(Ray &ray, Intersection *isect) const {
    return recursiveIntersect(root, ray, isect);
}
