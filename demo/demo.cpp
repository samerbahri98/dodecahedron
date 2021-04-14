#include <math.h>
#include <stdlib.h>

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#else
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #include <windows.h>
  #endif
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif

#ifndef M_PI
  #define M_PI 3.14159265359
#endif

struct Vector {
  float x, y, z;
 
  Vector(float v = 0) : x(v), y(v), z(v) { }
  Vector(float x, float y, float z) : x(x), y(y), z(z) { }
  Vector operator+(const Vector& v) const { return Vector(x + v.x, y + v.y, z + v.z); }
  Vector operator-(const Vector& v) const { return Vector(x - v.x, y - v.y, z - v.z); }
  friend Vector operator-(float f, const Vector& v) { return Vector(f) - v; }
  Vector operator*(const Vector& v) const { return Vector(x * v.x, y * v.y, z * v.z); }
  Vector operator/(const Vector& v) const { return Vector(x / v.x, y / v.y, z / v.z); }
  Vector& operator+=(const Vector& v) { x += v.x, y += v.y, z += v.z; return *this; }
  Vector& operator-=(const Vector& v) { x -= v.x, y -= v.y, z -= v.z; return *this; }
  Vector& operator*=(const Vector& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
  Vector& operator/=(const Vector& v) { x /= v.x, y /= v.y, z /= v.z; return *this; }
  Vector operator-() const { return Vector(-x, -y, -z); }
  float dot(const Vector& v) const { return x*v.x + y*v.y + z*v.z; }
  Vector cross(const Vector& v) const { return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
  friend Vector cross(const Vector& a, const Vector& b) { return a.cross(b); }
  float length() const { return sqrt(x*x + y*y + z*z); }
  Vector normalize() const { float l = length(); if(l > 1e-3) { return (*this/l); } else { return Vector(); } }
  bool isNull() const { return length() < 1e-3; }
};

void glVertex3f(const Vector& v) {
  glVertex3f(v.x, v.y, v.z);
}

void glQuad(const Vector& a, const Vector& b, const Vector& c, const Vector& d) {
  Vector normal = cross(b-a, c-a).normalize();
  glColor3f(fabs(normal.x), fabs(normal.y), fabs(normal.z));
  glVertex3f(a); glVertex3f(b); glVertex3f(c); glVertex3f(d);
}

void drawCube(const Vector& size) {
  glBegin(GL_LINE_LOOP); {
    /*       (E)-----(A)
             /|      /|
            / |     / |
          (F)-----(B) |
           | (H)---|-(D)
           | /     | /
           |/      |/
          (G)-----(C)        */

    Vector s = size / 2;

    Vector A(+s.x, +s.y, -s.z), B(+s.x, +s.y, +s.z), C(+s.x, -s.y, +s.z), D(+s.x, -s.y, -s.z), 
           E(-s.x, +s.y, -s.z), F(-s.x, +s.y, +s.z), G(-s.x, -s.y, +s.z), H(-s.x, -s.y, -s.z);

    glQuad(A, B, C, D); glQuad(E, H, G, F); glQuad(A, E, F, B);
    glQuad(D, C, G, H); glQuad(B, F, G, C); glQuad(A, D, H, E);

  } glEnd();
}

void onDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(1, 1, 1);
  drawCube(1.0f);

  glutSwapBuffers();
}

void onIdle() {
  static bool first_call = true;
  if(first_call) {
    glutPostRedisplay();
    first_call = false;
  }
}

void onInitialization() {
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  gluPerspective(60, 1, 0.1, 10);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(-3, 2, -2, 0, 0, 0, 0, 1, 0);
}

void onKeyboard(unsigned char key, int, int) {}

void onKeyboardUp(unsigned char key, int, int) {}

void onMouse(int, int, int, int) {}

void onMouseMotion(int, int) {}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitWindowSize(600, 600);
  glutInitWindowPosition(100, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

  glutCreateWindow("Grafika pelda program");

  onInitialization();

  glutDisplayFunc(onDisplay);
  glutMouseFunc(onMouse);
  glutIdleFunc(onIdle);
  glutKeyboardFunc(onKeyboard);
  glutKeyboardUpFunc(onKeyboardUp);
  glutMotionFunc(onMouseMotion);

  glutMainLoop();

  return 0;
}