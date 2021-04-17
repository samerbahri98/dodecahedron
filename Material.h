#ifndef MATERIAL_H
#define MATERIAL_h

#include "framework.h"

enum MaterialType
{
    ROUGH,
    REFLECTIVE,
    REFRACTIVE
};

vec3 operator/(vec3 num, vec3 denom)
{
    return vec3(num.x / denom.x, num.y / denom.y, num.z / denom.z);
};

vec3 operator^(vec3 num, int n)
{
    vec3 result = num;
    for (int i = 0; i < n; i++)
    {
        result = result * num;
    }

    return result;
};

struct Material
{
    vec3 ambient, diffuse, specular, F0;
    float shininess, ior;
    MaterialType type;
    Material(MaterialType t) { type = t; }
};

struct RoughMaterial : Material
{
    RoughMaterial(vec3 d, vec3 s, float sn) : Material(ROUGH)
    {
        ambient = d * M_PI;
        diffuse = d;
        specular = s;
        shininess = sn;
    };
};

struct ReflectiveMaterial : Material
{
    ReflectiveMaterial(vec3 n, vec3 kappa) : Material(REFLECTIVE)
    {
        vec3 one(1, 1, 1);
        F0 = (((n - one) ^ 2) + (kappa ^ 2)) / (((n + one) ^ 2) + (kappa ^ 2));
    };
};

struct RefractiveMaterial : Material
{
    RefractiveMaterial(vec3 n) : Material(REFRACTIVE)
    {
        vec3 one(1, 1, 1);
        F0 = ((n - one) ^ 2) / ((n + one) ^ 2);
        ior = n.x;
    };
};

struct Hit
{
    float t;
    vec3 position, normal;
    Material *material;
    Hit() { t = -1; }
};

#endif