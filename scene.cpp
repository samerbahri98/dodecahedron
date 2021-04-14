// #include "scene.h"
// #include "framework.h"
// #include "obj.h"

// void Scene::vertex3f(const vec3 &v)
// {
//     glVertex3f(v.x, v.y, v.z);
// }

// void Scene::glFace(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, const vec3 &e)
// {
//     vertex3f(a);
//     vertex3f(b);
//     vertex3f(c);
//     vertex3f(d);
//     vertex3f(e);
// }

// void Scene::drawDedo()
// {
//     glBegin(GL_TRIANGLE_FAN);
//     {
//         for (int i = 0; i < 12; i++)
//         {

//             std::vector<vec3> dots;
//             for (int j = 0; j < 5; j++)
//             {
//                 float x = (float)verticesOG[facesOG[j][i]][0];
//                 float y = (float)verticesOG[facesOG[j][i]][1];
//                 float z = (float)verticesOG[facesOG[j][i]][2];
//                 dots.push_back(vec3(x, y, z));
//             }
//             glFace(dots.at(0), dots.at(1), dots.at(2), dots.at(3), dots.at(4));
//         }
//     }
//     glEnd();
// }