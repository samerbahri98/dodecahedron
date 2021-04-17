#ifndef INTERSECTABLE_H
#define INTERSECTABLE_H
#include "Material.h"
#include "Ray.h"

class Intersectable
{
protected:
    Material *material;

public:
    virtual Hit intersect(const Ray &ray) = 0;
};

#endif
