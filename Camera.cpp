#include "Camera.h"
#include "framework.h"
#include "Ray.h"

void Camera::set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov)
{
    this->eye = _eye;
    this->lookat = _lookat;
    this->fov = _fov;
    vec3 w = eye - lookat;
    float windowSize = length(w) * tanf(fov / 2);
    right = normalize(cross(vup, w) * windowSize);
    up = normalize(cross(w, right) * windowSize);
}

Ray Camera::getRay(int X, int Y)
{
    vec3 dir = lookat + right * (2 * (X + 0.5f) / windowWidth - 1) + up * (2 * (Y + 0.5f) / windowHeight - 1) - eye;
    return Ray(eye, dir);
}

void Camera::Animate(float dt)
{
    vec3 d = eye - lookat;
    eye = vec3(d.x * cos(dt) + d.z * sin(dt), d.y, -d.x * sin(dt) + d.z * cos(dt)) + lookat;
    set(eye, lookat, up, fov);
}