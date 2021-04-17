#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
private:
    vec3 eye, lookat, right, up;
    float fov;

public:
    void set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov);
    Ray getRay(int X, int Y);
    void Animate(float dt);
};

#endif