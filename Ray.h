#ifndef RAY_H
#define RAY_H

#include "framework.h"

struct Ray
{
    vec3 start, dir;
    Ray(vec3 _start, vec3 _dir)
    {
        start = _start;
        dir = normalize(dir);
    }
};

#endif