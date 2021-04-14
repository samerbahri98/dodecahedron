#ifndef SCENE_H
#define SCENE_H
#include "framework.h"

// class Scene
// {
// public:
//     void vertex3f(const vec3 &v);
//     void glFace(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, const vec3 &e);
//     void drawDedo();
// };
// void glVertex3f(const vec3& v) {
//   glVertex3f(v.x, v.y, v.z);
// }
void glVertex3f(const vec3 &v)
{
    glVertex3f(v.x, v.y, v.z);
}

void glQuad(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d)
{
    vec3 normal = normalize(cross(b - a, c - a));
    glColor3f(fabs(normal.x), fabs(normal.y), fabs(normal.z));
    glVertex3f(a);
    glVertex3f(b);
    glVertex3f(c);
    glVertex3f(d);
}
void drawCube(const vec3 &size)
{
    glBegin(GL_LINE_LOOP);
    {
        /*       (E)-----(A)
             /|      /|
            / |     / |
          (F)-----(B) |
           | (H)---|-(D)
           | /     | /
           |/      |/
          (G)-----(C)        */

        vec3 s = size / 2;

        vec3 A(+s.x, +s.y, -s.z), B(+s.x, +s.y, +s.z), C(+s.x, -s.y, +s.z), D(+s.x, -s.y, -s.z),
            E(-s.x, +s.y, -s.z), F(-s.x, +s.y, +s.z), G(-s.x, -s.y, +s.z), H(-s.x, -s.y, -s.z);

        glQuad(A, B, C, D);
        glQuad(E, H, G, F);
        glQuad(A, E, F, B);
        glQuad(D, C, G, H);
        glQuad(B, F, G, C);
        glQuad(A, D, H, E);
    }
    glEnd();
}

#endif