#ifndef LIGHT_H
#define LIGHT_H

#include "framework.h"

struct Light
{
    vec3 direction;
    vec3 color;
    Light(vec3 _direction, vec3 _color)
    {
        direction = normalize(_direction);
        color = _color;
        }
};

#endif