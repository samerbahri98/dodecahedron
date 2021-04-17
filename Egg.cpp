#include "Egg.h"
#include "Intersectable.h"
#include "framework.h"

Egg::Egg(vec3 _center, float _radius, Material *_material)
{
    this->center = _center;
    this->radius = radius;
    this->material = material;
}

Hit Egg::intersect(const Ray &ray)
{
    Hit hit;
    vec3 dist = ray.start - center;
    float a = dot(ray.dir, ray.dir);
    float b = dot(dist, ray.dir) * 2.0f;
    float c = dot(dist, dist) - radius * radius;
    float delta = b * b - 4.0f * a * c;
    if (delta < 0)
        return hit;
    float sqrt_delta = sqrtf(delta);
    float t1 = (-b + sqrt_delta) / (2.0f * delta);
    float t2 = (-b - sqrt_delta) / (2.0f * delta);
    if (t1 <= 0)
        return hit;
    hit.t = (t2 > 0) ? t2 : t1;
    hit.position = ray.start + ray.dir * hit.t;
    hit.normal = (hit.position - center) / radius;
    hit.material = material;
    return hit;
}