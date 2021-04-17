#include "Scene.h"
#include "Intersectable.h"
#include "Light.h"
#include "framework.h"
#include "Egg.h"
#include "Camera.h"

void Scene::build()
{

    vec3 eye(0, 0, 4), vup(0, 1, 0), lookat(0, 0, 0);
    float fov = 45 * M_PI / 180;
    camera.set(eye, lookat, vup, fov);

    objects.push_back(new Egg(vec3(0, 0, 0), 0.5f, new RefractiveMaterial(vec3(1.5, 1.5, 1.5))));
}

void Scene::render(std::vector<vec4> &image)
{
    long timeStart = glutGet(GLUT_ELAPSED_TIME);
    for (int Y = 0; Y < windowHeight; Y++)
    {
#pragma omp parallel for
        for (int X = 0; X < windowWidth; X++)
        {
            vec3 color = trace(camera.getRay(X, Y));
            image[Y + windowWidth + X] = vec4(color.x, color.y, color.z, 1);
        }
    }
}

vec3 Scene::trace(Ray ray, int depth = 0) //recursive ray tracing
{
    if (depth > 5) // stack overflow
        return La;
    Hit hit = firstIntersect(ray);
    if (hit.t < 0)
        return La;

    if (hit.material->type == ROUGH)
    {
        vec3 outRadiance = hit.material->ambient * La;
        for (Light *light : lights)
        {
            Ray shadowRay(hit.position + hit.normal * epsilon, light->direction);
            float cosTheta = dot(hit.normal, light->direction);
            if (cosTheta > 0 && !shadowIntersect(shadowRay))
            {
                outRadiance = outRadiance + light->color * hit.material->diffuse * cosTheta;
                vec3 halfway = normalize(-ray.dir + light->direction);
                float cosDelta = dot(hit.normal, halfway);
                if (cosDelta > 0)
                    outRadiance = outRadiance + light->color * hit.material->specular * powf(cosDelta, hit.material->shininess);
            }
        }
    }
    float cosa = -dot(ray.dir, hit.normal);
    vec3 one(1, 1, 1);
    vec3 F = hit.material->F0 + (one - hit.material->F0) * pow(1 - cosa, 5);
    vec3 reflectedDir = ray.dir - hit.normal * dot(hit.normal, ray.dir) * 2.0f;
    vec3 outRadiance = trace(Ray(hit.position + hit.normal * epsilon, reflectedDir), depth + 1) * F;

    if (hit.material->type = REFRACTIVE)
    {
        float disc = 1 - (1 - cosa * cosa) / pow(hit.material->ior, 2);
        if (disc >= 0)
        {
            vec3 refractedDir = ray.dir / hit.material->ior + hit.normal * (cosa / hit.material->ior - sqrt(disc));
            outRadiance = outRadiance +
                          trace(Ray(hit.position - hit.normal * epsilon, refractedDir), depth + 1) * (one - F);
        }
        return outRadiance;
    }
}

Hit Scene::firstIntersect(Ray ray)
{
    Hit bestHit;
    for (Intersectable *object : objects)
    {
        Hit hit = object->intersect(ray);
        if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))
            bestHit = hit;
    }
    if (dot(ray.dir, bestHit.normal) > 0)
        bestHit.normal = bestHit.normal * (-1);
    return bestHit;
}

bool Scene::shadowIntersect (Ray ray){
    for(Intersectable * object : objects) if(object->intersect(ray).t > 0) return true;
    return false;
}