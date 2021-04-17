//=============================================================================================
// Computer Graphics Sample Program: Ray-tracing-let
//=============================================================================================
#include "framework.h"

enum MaterialType
{
	ROUGH,
	REFLECTIVE,
	REFRACTIVE
};

struct Material
{
	vec3 ka, kd, ks;
	float shininess;
	vec3 F0;
	float ior;
	MaterialType type;
	Material(MaterialType t) { type = t; }
};

struct RoughMaterial : Material
{
	RoughMaterial(vec3 _kd, vec3 _ks, float _shininess) : Material(ROUGH)
	{
		ka = _kd * M_PI;
		kd = _kd;
		ks = _ks;
		shininess = _shininess;
	}
};

vec3 operator/(vec3 num, vec3 denom)
{
	return vec3(num.x / denom.x, num.y / denom.y, num.z / denom.z);
}

struct ReflectiveMaterial : Material
{
	ReflectiveMaterial(vec3 n, vec3 kappa) : Material(REFLECTIVE)
	{
		vec3 one(1, 1, 1);
		F0 = ((n - one) * (n - one) + kappa * kappa) / ((n + one) * (n + one) + kappa * kappa);
	}
};

struct RefractiveMaterial : Material
{
	RefractiveMaterial(vec3 n) : Material(REFRACTIVE)
	{
		vec3 one(1, 1, 1);
		F0 = ((n - one) * (n - one)) / ((n + one) * (n + one));
		ior = n.x;
	}
};

struct Hit
{
	float t;
	vec3 position, normal;
	Material *material;
	Hit() { t = -1; }
};

struct Ray
{
	vec3 start, dir;
	Ray(vec3 _start, vec3 _dir)
	{
		start = _start;
		dir = normalize(_dir);
	}
};

class Intersectable
{
protected:
	Material *material;

public:
	virtual Hit intersect(const Ray &ray) = 0;
};

class Egg : public Intersectable
{
	vec3 center;
	float radius;

public:
	Egg(const vec3 &_center, Material *_material)
	{
		center = _center;
		radius = 1.0f;
		material = _material;
	}

	Hit intersect(const Ray &ray)
	{
		Hit hit;
		vec3 dist = ray.start - center;
		float a = dot(ray.dir, ray.dir) - (0.5 * ray.dir.x * ray.dir.x);
		float b = dot(dist, ray.dir) * 2.0f - dist.x * ray.dir.x;
		float c = dot(dist, dist) - 1 - (0.5 * dist.x * dist.x);
		float discr = b * b - 4.0f * a * c;
		if (discr < 0)
			return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0)
			return hit;
		hit.t = (t2 > 0) ? t2 : t1;
		hit.position = ray.start + ray.dir * hit.t;
		hit.normal = vec3(0.5 * (hit.position.x - center.x), hit.position.y - center.y, hit.position.z - center.z); // gradient of formula
		hit.material = material;
		return hit;
	}
};

class Face : public Intersectable
{
private:
	vec3 p1, p2, p3;

public:
	Face(vec3 _p1, vec3 _p2, vec3 _p3)
	{
		p1 = _p1;
		p2 = _p2;
		p3 = _p3;
	}
	Hit intersect(const Ray &ray)
	{
		Hit hit;
		vec3 normal = cross(p2 - p1, p3 - p1);
		if (dot(ray.dir, normal) == 0)
			return hit;
		
		hit.material = material;
		return hit;
	}
};

class Dedocahedron : public Intersectable
{
	vec3 center;
	float radius;

public:
	Dedocahedron()
	{
		std::vector<vec3> vertices = {vec3{0, 0.618, 1.618},
									  vec3{0, -0.618, 1.618},
									  vec3{0, -0.618, -1.618},
									  vec3{0, 0.618, -1.618},
									  vec3{1.618, 0, 0.618},
									  vec3{-1.618, 0, 0.618},
									  vec3{-1.618, 0, -0.618},
									  vec3{1.618, 0, -0.618},
									  vec3{0.618, 1.618, 0},
									  vec3{-0.618, 1.618, 0},
									  vec3{-0.618, -1.618, 0},
									  vec3{0.618, -1.618, 0},
									  vec3{1, 1, 1},
									  vec3{-1, 1, 1},
									  vec3{-1, -1, 1},
									  vec3{1, -1, 1},
									  vec3{1, -1, -1},
									  vec3{1, 1, -1},
									  vec3{-1, 1, -1},
									  vec3{-1, -1, -1}};
		std::vector<std::vector<float>> faces = {{0, 1, 15, 4, 12},
												 {0, 12, 8, 9, 13},
												 {0, 13, 5, 14, 1},
												 {1, 14, 10, 11, 15},
												 {2, 3, 17, 7, 16},
												 {2, 16, 11, 10, 19},
												 {2, 19, 6, 18, 3},
												 {18, 9, 8, 17, 3},
												 {15, 11, 16, 7, 4},
												 {4, 7, 17, 8, 12},
												 {13, 9, 18, 6, 5},
												 {5, 6, 19, 10, 14}};
	}

	Hit intersect(const Ray &ray)
	{
		Hit hit;
		vec3 dist = ray.start - center;
		float a = dot(ray.dir, ray.dir);
		float b = dot(dist, ray.dir) * 2.0f;
		float c = dot(dist, dist) - 1;
		float discr = b * b - 4.0f * a * c;
		if (discr < 0)
			return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a; // t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0)
			return hit;
		hit.t = (t2 > 0) ? t2 : t1;
		hit.position = ray.start + ray.dir * hit.t;
		hit.normal = (hit.position - center);
		hit.material = material;
		return hit;
	}
};

class Camera
{
	vec3 eye, lookat, right, up;
	float fov;

public:
	void set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov)
	{
		eye = _eye;
		lookat = _lookat;
		fov = _fov;
		vec3 w = eye - lookat;
		float windowSize = length(w) * tanf(fov / 2);
		right = normalize(cross(vup, w)) * windowSize;
		up = normalize(cross(w, right)) * windowSize;
	}

	Ray getRay(int X, int Y)
	{
		vec3 dir = lookat + right * (2 * (X + 0.5f) / windowWidth - 1) + up * (2 * (Y + 0.5f) / windowHeight - 1) - eye;
		return Ray(eye, dir);
	}

	void Animate(float dt)
	{
		vec3 d = eye - lookat;
		eye = vec3(d.x * cos(dt) + d.z * sin(dt), d.y, -d.x * sin(dt) + d.z * cos(dt)) + lookat;
		set(eye, lookat, up, fov);
	}
};

struct Light
{
	vec3 direction;
	vec3 Le;
	Light(vec3 _direction, vec3 _Le)
	{
		direction = normalize(_direction);
		Le = _Le;
	}
};

const float epsilon = 0.0001f;

class Scene
{
	std::vector<Intersectable *> objects;
	std::vector<Light *> lights;
	Camera camera;
	vec3 La;

public:
	void build()
	{
		vec3 eye = vec3(0, 0, 4), vup = vec3(0, 1, 0), lookat = vec3(0, 0, 0);
		float fov = 45 * M_PI / 180;
		camera.set(eye, lookat, vup, fov);

		La = vec3(0.4f, 0.4f, 0.4f);
		vec3 lightDirection(1, 1, 1), Le(2, 2, 2);
		lights.push_back(new Light(lightDirection, Le));

		vec3 ks(3.1, 2.7, 1.9);

		objects.push_back(new Egg(vec3(0, 0, 0),
								  new ReflectiveMaterial(vec3(0.17, 0.35, 1.5), ks)));
		objects.push_back(new Egg(vec3(2, 2, 0),
								  new RoughMaterial(vec3(0.1, 0.2, 0.3), ks, 100)));
		// objects.push_back(new Egg(vec3(0, 0.5, -0.8), 0.5,
		// 	              new RoughMaterial(vec3(0.3, 0, 0.2), ks, 20)));
		// objects.push_back(new Egg(vec3(0, 0,  0), 0.5,
		// 	              new RefractiveMaterial(vec3(1.2, 1.2, 1.2))));
		// objects.push_back(new Egg(vec3(0, 0, 0), 6,
		// 	              new ReflectiveMaterial(vec3(0.14, 0.16, 0.13), vec3(4.1, 2.3, 3.1))));
	}

	void render(std::vector<vec4> &image)
	{
		long timeStart = glutGet(GLUT_ELAPSED_TIME);

		for (int Y = 0; Y < windowHeight; Y++)
		{
#pragma omp parallel for
			for (int X = 0; X < windowWidth; X++)
			{
				vec3 color = trace(camera.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}

		printf("Rendering time: %d milliseconds\n", glutGet(GLUT_ELAPSED_TIME) - timeStart);
	}

	Hit firstIntersect(Ray ray)
	{
		Hit bestHit;
		for (Intersectable *object : objects)
		{
			Hit hit = object->intersect(ray); //  hit.t < 0 if no intersection
			if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))
				bestHit = hit;
		}
		if (dot(ray.dir, bestHit.normal) > 0)
			bestHit.normal = bestHit.normal * (-1);
		return bestHit;
	}

	bool shadowIntersect(Ray ray)
	{ // for directional lights
		for (Intersectable *object : objects)
			if (object->intersect(ray).t > 0)
				return true;
		return false;
	}

	vec3 trace(Ray ray, int depth = 0)
	{
		if (depth > 5)
			return La;
		Hit hit = firstIntersect(ray);
		if (hit.t < 0)
			return La;

		if (hit.material->type == ROUGH)
		{
			vec3 outRadiance = hit.material->ka * La;
			for (Light *light : lights)
			{
				Ray shadowRay(hit.position + hit.normal * epsilon, light->direction);
				float cosTheta = dot(hit.normal, light->direction);
				if (cosTheta > 0 && !shadowIntersect(shadowRay))
				{ // shadow computation
					outRadiance = outRadiance + light->Le * hit.material->kd * cosTheta;
					vec3 halfway = normalize(-ray.dir + light->direction);
					float cosDelta = dot(hit.normal, halfway);
					if (cosDelta > 0)
						outRadiance = outRadiance + light->Le * hit.material->ks * powf(cosDelta, hit.material->shininess);
				}
			}
			return outRadiance;
		}

		float cosa = -dot(ray.dir, hit.normal);
		vec3 one(1, 1, 1);
		vec3 F = hit.material->F0 + (one - hit.material->F0) * pow(1 - cosa, 5);
		vec3 reflectedDir = ray.dir - hit.normal * dot(hit.normal, ray.dir) * 2.0f;
		vec3 outRadiance = trace(Ray(hit.position + hit.normal * epsilon, reflectedDir), depth + 1) * F;

		if (hit.material->type == REFRACTIVE)
		{
			float disc = 1 - (1 - cosa * cosa) / hit.material->ior / hit.material->ior; // scalar n
			if (disc >= 0)
			{
				vec3 refractedDir = ray.dir / hit.material->ior + hit.normal * (cosa / hit.material->ior - sqrt(disc));
				outRadiance = outRadiance +
							  trace(Ray(hit.position - hit.normal * epsilon, refractedDir), depth + 1) * (one - F);
			}
		}
		return outRadiance;
	}

	void Animate(float dt) { camera.Animate(dt); }
};

Scene scene;
GPUProgram gpuProgram; // vertex and fragment shaders

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() { fragmentColor = texture(textureUnit, texcoord); }
)";

class FullScreenTexturedQuad
{
	unsigned int vao = 0, textureId = 0; // vertex array object id and texture id
public:
	FullScreenTexturedQuad(int windowWidth, int windowHeight)
	{
		glGenVertexArrays(1, &vao); // create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo;	   // vertex buffer objects
		glGenBuffers(1, &vbo); // Generate 1 vertex buffer objects

		// vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo);					 // make it active, it is an array
		float vertexCoords[] = {-1, -1, 1, -1, 1, 1, -1, 1}; // two triangles forming a quad
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glGenTextures(1, &textureId);									  // id generation
		glBindTexture(GL_TEXTURE_2D, textureId);						  // binding
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // sampling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void LoadTexture(std::vector<vec4> &image)
	{
		glBindTexture(GL_TEXTURE_2D, textureId);															 // binding
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, &image[0]); // To GPU
	}

	void Draw()
	{
		glBindVertexArray(vao); // make the vao and its vbos active playing the role of the data source
		int location = glGetUniformLocation(gpuProgram.getId(), "textureUnit");
		const unsigned int textureUnit = 0;
		if (location >= 0)
		{
			glUniform1i(location, textureUnit);
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, textureId);
		}
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // draw two triangles forming a quad
	}
};

FullScreenTexturedQuad *fullScreenTexturedQuad;

// Initialization, create an OpenGL context
void onInitialization()
{
	glViewport(0, 0, windowWidth, windowHeight);
	scene.build();
	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight);
	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor"); // create program for the GPU
}

// Window has become invalid: Redraw
void onDisplay()
{
	std::vector<vec4> image(windowWidth * windowHeight);
	scene.render(image);						// Execute ray casting
	fullScreenTexturedQuad->LoadTexture(image); // copy image to GPU as a texture
	fullScreenTexturedQuad->Draw();				// Display rendered image on screen
	glutSwapBuffers();							// exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {}

// Idle event indicating that some time elapsed: do animation here
void onIdle()
{
	scene.Animate(0.1f);
	glutPostRedisplay();
}