#ifndef SCENE_H
#define SCENE_H
#include "framework.h"

class Scene
{
public:
    void vertex3f(const vec3 &v);
    void glFace(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, const vec3 &e);
    void drawDedo();
};

#endif