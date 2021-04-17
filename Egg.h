#ifndef EGG_H
#define EGG_H


class Egg : public Intersectable
{
private:
    vec3 center;
    float radius;

public:
    Egg(vec3 _center, float _radius, Material *_material);
    Hit intersect(const Ray &ray);
};

#endif