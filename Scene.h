#ifndef SCENE_H
#define SCENE_H

const float epsilon = 0.0001f;
class Scene
{
private:
    std::vector<Intersectable *> objects;
    std::vector<Light *> lights;
    Camera camera;
    vec3 La;

public:
    void build();
    void render(std::vector<vec4> &image);
    Hit firstIntersect(Ray ray);
    vec3 trace(Ray ray, int depth = 0);
    bool shadowIntersect(Ray ray);
};

#endif