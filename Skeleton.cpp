//=============================================================================================
// Computer Graphics Sample Program: 3D engine-let
// Shader: Gouraud, Phong, NPR
// Material: diffuse + Phong-Blinn
// Texture: CPU-procedural
// Geometry: sphere, tractricoid, torus, mobius, klein-bottle, boy, dini
// Camera: perspective
// Light: point or directional sources
//=============================================================================================
#include "framework.h"

//---------------------------
template <class T>
struct Dnum
{			 // Dual numbers for automatic derivation
			 //---------------------------
	float f; // function value
	T d;	 // derivatives
	Dnum(float f0 = 0, T d0 = T(0)) { f = f0, d = d0; }
	Dnum operator+(Dnum r) { return Dnum(f + r.f, d + r.d); }
	Dnum operator-(Dnum r) { return Dnum(f - r.f, d - r.d); }
	Dnum operator*(Dnum r)
	{
		return Dnum(f * r.f, f * r.d + d * r.f);
	}
	Dnum operator/(Dnum r)
	{
		return Dnum(f / r.f, (r.f * d - r.d * f) / r.f / r.f);
	}
};

// Elementary functions prepared for the chain rule as well
template <class T>
Dnum<T> Exp(Dnum<T> g) { return Dnum<T>(expf(g.f), expf(g.f) * g.d); }
template <class T>
Dnum<T> Sin(Dnum<T> g) { return Dnum<T>(sinf(g.f), cosf(g.f) * g.d); }
template <class T>
Dnum<T> Cos(Dnum<T> g) { return Dnum<T>(cosf(g.f), -sinf(g.f) * g.d); }
template <class T>
Dnum<T> Tan(Dnum<T> g) { return Sin(g) / Cos(g); }
template <class T>
Dnum<T> Sinh(Dnum<T> g) { return Dnum<T>(sinh(g.f), cosh(g.f) * g.d); }
template <class T>
Dnum<T> Cosh(Dnum<T> g) { return Dnum<T>(cosh(g.f), sinh(g.f) * g.d); }
template <class T>
Dnum<T> Tanh(Dnum<T> g) { return Sinh(g) / Cosh(g); }
template <class T>
Dnum<T> Log(Dnum<T> g) { return Dnum<T>(logf(g.f), g.d / g.f); }
template <class T>
Dnum<T> Pow(Dnum<T> g, float n)
{
	return Dnum<T>(powf(g.f, n), n * powf(g.f, n - 1) * g.d);
}

float magnitude(vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

typedef Dnum<vec2> Dnum2;

const int tessellationLevel = 20;

vec3 height(float x, float y)
{
	return vec3(x, y, cosh(x * x + y * y));
}

//---------------------------
struct Camera
{							  // 3D camera
							  //---------------------------
	vec3 wEye, wLookat, wVup; // extrinsic
	float fov, asp, fp, bp;	  // intrinsic
public:
	Camera()
	{
		asp = (float)windowWidth / windowHeight;
		fov = 75.0f * (float)M_PI / 180.0f;
		fp = 1;
		bp = 20;
	}
	mat4 V()
	{ // view matrix: translates the center to the origin
		vec3 w = normalize(wEye - wLookat);
		vec3 u = normalize(cross(wVup, w));
		vec3 v = cross(w, u);
		return TranslateMatrix(wEye * (-1)) * mat4(u.x, v.x, w.x, 0,
												   u.y, v.y, w.y, 0,
												   u.z, v.z, w.z, 0,
												   0, 0, 0, 1);
	}

	mat4 P()
	{ // projection matrix
		return mat4(1 / (tan(fov / 2) * asp), 0, 0, 0,
					0, 1 / tan(fov / 2), 0, 0,
					0, 0, -(fp + bp) / (bp - fp), -1,
					0, 0, -2 * fp * bp / (bp - fp), 0);
	}
};

//---------------------------
struct Material
{
	//---------------------------
	vec3 kd, ks, ka;
	float shininess;
};

//---------------------------
struct Light
{
	//---------------------------
	vec3 La, Le, rotationAxis;
	vec4 wLightPos; // homogeneous coordinates, can be at ideal point
	float rotationAngle;
	virtual void Animate(float tstart, float tend)
	{
		rotationAngle = 0.8f * tend;
		vec3 normal(-wLightPos.y, wLightPos.x, 0);
		normal = rotationAngle * (normal / magnitude(normal));
		wLightPos = wLightPos + vec4(normal.x, normal.y, 0, 0);
	}
};

//---------------------------
class CheckerBoardTexture : public Texture
{
	//---------------------------
public:
	CheckerBoardTexture(const int width, const int height) : Texture()
	{
		std::vector<vec4> image(width * height);
		const vec4 yellow(1, 1, 0, 1), blue(0, 0, 1, 1);
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				image[y * width + x] = (x & 1) ^ (y & 1) ? yellow : blue;
			}
		create(width, height, image, GL_NEAREST);
	}
};

class BowlTexure : public Texture
{
	//---------------------------
public:
	BowlTexure(const int width, const int height) : Texture()
	{
		std::vector<vec4> image(width * height);
		const vec4 yellow(1, 1, 0, 1), blue(0, 0, 1, 1);
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				//image[y * width + x] = (x*x + y*y) %3000 ? yellow : blue;
				image[y * width + x] = vec4(float((x * x + y * y) % 30000) / 30000, float((x * x + y * y) % 30000) / 300000, 1, 1);
				//image[y * width + x] = (x & 1) ^ (y & 1) ? yellow : blue;
			}
		create(width, height, image, GL_NEAREST);
	}
};

//---------------------------
struct RenderState
{
	//---------------------------
	mat4 MVP, M, Minv, V, P;
	Material *material;
	std::vector<Light> lights;
	Texture *texture;
	vec3 wEye;
};

//---------------------------
class Shader : public GPUProgram
{
	//---------------------------
public:
	virtual void Bind(RenderState state) = 0;

	void setUniformMaterial(const Material &material, const std::string &name)
	{
		setUniform(material.kd, name + ".kd");
		setUniform(material.ks, name + ".ks");
		setUniform(material.ka, name + ".ka");
		setUniform(material.shininess, name + ".shininess");
	}

	void setUniformLight(const Light &light, const std::string &name)
	{
		setUniform(light.La, name + ".La");
		setUniform(light.Le, name + ".Le");
		setUniform(light.wLightPos, name + ".wLightPos");
	}
};

//---------------------------
class GouraudShader : public Shader
{
	//---------------------------
	const char *vertexSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};
		
		struct Material {
			vec3 kd, ks, ka;
			float shininess;
		};

		uniform mat4  MVP, M, Minv;  // MVP, Model, Model-inverse
		uniform Light[8] lights;     // light source direction 
		uniform int   nLights;		 // number of light sources
		uniform vec3  wEye;          // pos of eye
		uniform Material  material;  // diffuse, specular, ambient ref

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space

		out vec3 radiance;		    // reflected radiance

		void main() {
			gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
			// radiance computation
			vec4 wPos = vec4(vtxPos, 1) * M;	
			vec3 V = normalize(wEye * wPos.w - wPos.xyz);
			vec3 N = normalize((Minv * vec4(vtxNorm, 0)).xyz);
			if (dot(N, V) < 0) N = -N;	// prepare for one-sided surfaces like Mobius or Klein

			radiance = vec3(0, 0, 0);
			for(int i = 0; i < nLights; i++) {
				vec3 L = normalize(lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w);
				vec3 H = normalize(L + V);
				float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
				radiance += material.ka * lights[i].La + (material.kd * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
		}
	)";

	// fragment shader in GLSL
	const char *fragmentSource = R"(
		#version 330
		precision highp float;

		in  vec3 radiance;      // interpolated radiance
		out vec4 fragmentColor; // output goes to frame buffer

		void main() {
			fragmentColor = vec4(radiance, 1);
		}
	)";

public:
	GouraudShader() { create(vertexSource, fragmentSource, "fragmentColor"); }

	void Bind(RenderState state)
	{
		Use(); // make this program run
		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");
		setUniform(state.wEye, "wEye");
		setUniformMaterial(*state.material, "material");

		setUniform((int)state.lights.size(), "nLights");
		for (unsigned int i = 0; i < state.lights.size(); i++)
		{
			setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
		}
	}
};

class BowlShader : public Shader
{
	//---------------------------
	const char *vertexSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform vec3  wEye;         // pos of eye

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;

		out vec3 wNormal;		    // normal in world space
		out vec3 wView;             // view in world space
		out vec3 wLight[8];		    // light dir in world space
		out vec2 texcoord;

		void main() {
			gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
			// vectors for radiance computation
			vec4 wPos = vec4(vtxPos, 1) * M;
			for(int i = 0; i < nLights; i++) {
				wLight[i] = lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w;
			}
		    wView  = wEye * wPos.w - wPos.xyz;
		    wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		    texcoord = vtxUV;
		}
	)";

	// fragment shader in GLSL
	const char *fragmentSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		struct Material {
			vec3 kd, ks, ka;
			float shininess;
		};

		uniform Material material;
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform sampler2D diffuseTexture;

		in  vec3 wNormal;       // interpolated world sp normal
		in  vec3 wView;         // interpolated world sp view
		in  vec3 wLight[8];     // interpolated world sp illum dir
		in  vec2 texcoord;
		
        out vec4 fragmentColor; // output goes to frame buffer

		void main() {
			vec3 N = normalize(wNormal);
			vec3 V = normalize(wView); 
			if (dot(N, V) < 0) N = -N;	// prepare for one-sided surfaces like Mobius or Klein
			vec3 texColor = texture(diffuseTexture, texcoord).rgb;
			vec3 ka = material.ka * texColor;
			vec3 kd = material.kd * texColor;

			vec3 radiance = vec3(0, 0, 0);
			for(int i = 0; i < nLights; i++) {
				vec3 L = normalize(wLight[i]);
				vec3 H = normalize(L + V);
				float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
				// kd and ka are modulated by the texture
				radiance += ka * lights[i].La + 
                           (kd * texColor * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
			fragmentColor = vec4(radiance, 1);
		}
	)";

public:
	BowlShader() { create(vertexSource, fragmentSource, "fragmentColor"); }

	void Bind(RenderState state)
	{
		Use(); // make this program run
		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");
		setUniform(state.wEye, "wEye");
		setUniform(*state.texture, std::string("diffuseTexture"));
		setUniformMaterial(*state.material, "material");

		setUniform((int)state.lights.size(), "nLights");
		for (unsigned int i = 0; i < state.lights.size(); i++)
		{
			setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
		}
	}
};

//---------------------------
class PhongShader : public Shader
{
	//---------------------------
	const char *vertexSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform vec3  wEye;         // pos of eye

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;

		out vec3 wNormal;		    // normal in world space
		out vec3 wView;             // view in world space
		out vec3 wLight[8];		    // light dir in world space
		out vec2 texcoord;

		void main() {
			gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
			// vectors for radiance computation
			vec4 wPos = vec4(vtxPos, 1) * M;
			for(int i = 0; i < nLights; i++) {
				wLight[i] = lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w;
			}
		    wView  = wEye * wPos.w - wPos.xyz;
		    wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		    texcoord = vtxUV;
		}
	)";

	// fragment shader in GLSL
	const char *fragmentSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		struct Material {
			vec3 kd, ks, ka;
			float shininess;
		};

		uniform Material material;
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform sampler2D diffuseTexture;

		in  vec3 wNormal;       // interpolated world sp normal
		in  vec3 wView;         // interpolated world sp view
		in  vec3 wLight[8];     // interpolated world sp illum dir
		in  vec2 texcoord;
		
        out vec4 fragmentColor; // output goes to frame buffer

		void main() {
			vec3 N = normalize(wNormal);
			vec3 V = normalize(wView); 
			if (dot(N, V) < 0) N = -N;	// prepare for one-sided surfaces like Mobius or Klein
			vec3 texColor = texture(diffuseTexture, texcoord).rgb;
			vec3 ka = material.ka * texColor;
			vec3 kd = material.kd * texColor;

			vec3 radiance = vec3(0, 0, 0);
			for(int i = 0; i < nLights; i++) {
				vec3 L = normalize(wLight[i]);
				vec3 H = normalize(L + V);
				float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
				// kd and ka are modulated by the texture
				radiance += ka * lights[i].La + 
                           (kd * texColor * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
			fragmentColor = vec4(radiance, 1);
		}
	)";

public:
	PhongShader() { create(vertexSource, fragmentSource, "fragmentColor"); }

	void Bind(RenderState state)
	{
		Use(); // make this program run
		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");
		setUniform(state.wEye, "wEye");
		setUniform(*state.texture, std::string("diffuseTexture"));
		setUniformMaterial(*state.material, "material");

		setUniform((int)state.lights.size(), "nLights");
		for (unsigned int i = 0; i < state.lights.size(); i++)
		{
			setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
		}
	}
};

//---------------------------
class NPRShader : public Shader
{
	//---------------------------
	const char *vertexSource = R"(
		#version 330
		precision highp float;

		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform	vec4  wLightPos;
		uniform vec3  wEye;         // pos of eye

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;

		out vec3 wNormal, wView, wLight;				// in world space
		out vec2 texcoord;

		void main() {
		   gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
		   vec4 wPos = vec4(vtxPos, 1) * M;
		   wLight = wLightPos.xyz * wPos.w - wPos.xyz * wLightPos.w;
		   wView  = wEye * wPos.w - wPos.xyz;
		   wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		   texcoord = vtxUV;
		}
	)";

	// fragment shader in GLSL
	const char *fragmentSource = R"(
		#version 330
		precision highp float;

		uniform sampler2D diffuseTexture;

		in  vec3 wNormal, wView, wLight;	// interpolated
		in  vec2 texcoord;
		out vec4 fragmentColor;    			// output goes to frame buffer

		void main() {
		   vec3 N = normalize(wNormal), V = normalize(wView), L = normalize(wLight);
		   if (dot(N, V) < 0) N = -N;	// prepare for one-sided surfaces like Mobius or Klein
		   float y = (dot(N, L) > 0.5) ? 1 : 0.5;
		   if (abs(dot(N, V)) < 0.2) fragmentColor = vec4(0, 0, 0, 1);
		   else						 fragmentColor = vec4(y * texture(diffuseTexture, texcoord).rgb, 1);
		}
	)";

public:
	NPRShader() { create(vertexSource, fragmentSource, "fragmentColor"); }

	void Bind(RenderState state)
	{
		Use(); // make this program run
		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");
		setUniform(state.wEye, "wEye");
		setUniform(*state.texture, std::string("diffuseTexture"));
		setUniform(state.lights[0].wLightPos, "wLightPos");
	}
};

//---------------------------
class Geometry
{
	//---------------------------
protected:
	unsigned int vao, vbo; // vertex array object
public:
	Geometry()
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}
	virtual void Draw() = 0;
	~Geometry()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
};

//---------------------------
class ParamSurface : public Geometry
{
	//---------------------------
public:
	struct VertexData
	{
		vec3 position, normal;
		vec2 texcoord;
	};

	unsigned int nVtxPerStrip, nStrips;

	ParamSurface() { nVtxPerStrip = nStrips = 0; }

	virtual void eval(Dnum2 &U, Dnum2 &V, Dnum2 &X, Dnum2 &Y, Dnum2 &Z) = 0;

	VertexData GenVertexData(float u, float v)
	{
		VertexData vtxData;
		vtxData.texcoord = vec2(u, v);
		Dnum2 X, Y, Z;
		Dnum2 U(u, vec2(1, 0)), V(v, vec2(0, 1));
		eval(U, V, X, Y, Z);
		vtxData.position = vec3(X.f, Y.f, Z.f);
		vec3 drdU(X.d.x, Y.d.x, Z.d.x), drdV(X.d.y, Y.d.y, Z.d.y);
		vtxData.normal = cross(drdU, drdV);
		return vtxData;
	}

	void create(int N = tessellationLevel, int M = tessellationLevel)
	{
		nVtxPerStrip = (M + 1) * 2;
		nStrips = N;
		std::vector<VertexData> vtxData; // vertices on the CPU
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j <= M; j++)
			{
				vtxData.push_back(GenVertexData((float)j / M, (float)i / N));
				vtxData.push_back(GenVertexData((float)j / M, (float)(i + 1) / N));
			}
		}
		glBufferData(GL_ARRAY_BUFFER, nVtxPerStrip * nStrips * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0); // attribute array 0 = POSITION
		glEnableVertexAttribArray(1); // attribute array 1 = NORMAL
		glEnableVertexAttribArray(2); // attribute array 2 = TEXCOORD0
		// attribute array, components/attribute, component type, normalize?, stride, offset
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, texcoord));
	}

	void Draw()
	{
		glBindVertexArray(vao);
		for (unsigned int i = 0; i < nStrips; i++)
			glDrawArrays(GL_TRIANGLE_STRIP, i * nVtxPerStrip, nVtxPerStrip);
	}
};

//--------------------------- Samer
class Bowl : public ParamSurface
{
	float x, y;

public:
	Bowl(float _x, float _y)
	{

		this->x = _x;
		this->y = _y;
		create();
	}
	void eval(Dnum2 &U, Dnum2 &V, Dnum2 &X, Dnum2 &Y, Dnum2 &Z)
	{
		//U = U * 2.0f * (float)M_PI, V = V * (float)M_PI;
		X = U * this->x;
		Y = V * this->y;
		Z = Cosh(Pow(X, 2) + Pow(Y, 2));
	}
};

//---------------------------
class Sphere : public ParamSurface
{
	//---------------------------
public:
	Sphere() { create(); }
	void eval(Dnum2 &U, Dnum2 &V, Dnum2 &X, Dnum2 &Y, Dnum2 &Z)
	{
		U = U * 2.0f * (float)M_PI, V = V * (float)M_PI;
		X = Cos(U) * Sin(V);
		Y = Sin(U) * Sin(V);
		Z = Cos(V);
	}
};

//---------------------------
struct Object
{
	//---------------------------
	Shader *shader;
	Material *material;
	Texture *texture;
	Geometry *geometry;
	vec3 scale, translation, rotationAxis;
	float rotationAngle;

public:
	Object(Shader *_shader, Material *_material, Texture *_texture, Geometry *_geometry) : scale(vec3(1, 1, 1)), translation(vec3(0, 0, 0)), rotationAxis(0, 0, 1), rotationAngle(0)
	{
		shader = _shader;
		texture = _texture;
		material = _material;
		geometry = _geometry;
	}

	virtual void SetModelingTransform(mat4 &M, mat4 &Minv)
	{
		M = ScaleMatrix(scale) * RotationMatrix(rotationAngle, rotationAxis) * TranslateMatrix(translation);
		Minv = TranslateMatrix(-translation) * RotationMatrix(-rotationAngle, rotationAxis) * ScaleMatrix(vec3(1 / scale.x, 1 / scale.y, 1 / scale.z));
	}

	void Draw(RenderState state)
	{
		mat4 M, Minv;
		SetModelingTransform(M, Minv);
		state.M = M;
		state.Minv = Minv;
		state.MVP = state.M * state.V * state.P;
		state.material = material;
		state.texture = texture;
		shader->Bind(state);
		geometry->Draw();
	}

	virtual void Animate(float tstart, float tend)
	{
		//rotationAngle = 0.8f * tend;
	}
};

class Ball : public Object
{
public:
	vec3 direction, normal, velocity, acceleration, gravity;
	Ball(vec3 _velocity,
		 vec3 _normal,
		 vec3 _direction,
		 Shader *_shader,
		 Material *_material,
		 Texture *_texture,
		 Geometry *_geometry)
		: Object(_shader, _material, _texture, _geometry)
	{
		this->velocity = _velocity;
		this->normal = _normal;
		this->gravity = vec3(0, 0, -3);
		this->direction = _direction;
	}
	void Animate(float tstart, float tend) override
	{
		this->acceleration = gravity - dot(gravity, normal) * normal;
		this->velocity = this->velocity + this->acceleration * 0.001f * tend;
		this->direction = this->direction + this->velocity * 0.001f * tend;
		// snap to the bowl
		float signx = this->direction.x / abs(this->direction.x), signy = this->direction.y / abs(this->direction.y);
		Bowl *referenceBowl = new Bowl(signx, signy);
		normal = referenceBowl->GenVertexData(abs(this->direction.x), abs(this->direction.y)).normal;
		normal = normal / magnitude(normal);
		vec3 position = (2 * height(this->direction.x, this->direction.y) + 0.1 * normal);
		this->translation = vec3(position.x, position.y, position.z);

		// printf("%f , %f , %f\n",
		// 	   this->translation.x,
		// 	   this->translation.y,
		// 	   this->translation.z);
	}
};

//---------------------------
class Scene
{
	//---------------------------
	std::vector<Object *> objects;
	Camera camera; // 3D camera
	std::vector<Light> lights;
	vec3 masterNormal, masterPosition;

public:
	void addSphere(float px, float py)
	{
		Shader *gouraudShader = new GouraudShader();
		Material *material0 = new Material;
		material0->kd = vec3(0.6f, 0.4f, 0.2f);
		material0->ks = vec3(4, 4, 4);
		material0->ka = vec3(0.1f, 0.1f, 0.1f);
		material0->shininess = 100;
		Texture *sphereTexture = new CheckerBoardTexture(15, 20);
		Geometry *sphere = new Sphere();
		Ball *sphereObject1 = new Ball(vec3(px, 1 - py, 0), masterNormal, masterPosition, gouraudShader, material0, sphereTexture, sphere);
		printf("%f , %f \n",
			   px,
			   py);
		sphereObject1->translation = masterPosition;
		sphereObject1->scale = vec3(0.1f, 0.1f, 0.1f);
		objects.push_back(sphereObject1);
	}

	void Build()
	{
		// Shaders
		Shader *phongShader = new PhongShader();
		Shader *gouraudShader = new GouraudShader();
		Shader *bowlShader = new BowlShader();
		Shader *nprShader = new NPRShader();

		// Materials
		Material *material0 = new Material;
		material0->kd = vec3(0.6f, 0.4f, 0.2f);
		material0->ks = vec3(4, 4, 4);
		material0->ka = vec3(0.1f, 0.1f, 0.1f);
		material0->shininess = 100;

		Material *material1 = new Material;
		material1->kd = vec3(0.8f, 0.6f, 0.4f);
		material1->ks = vec3(0.3f, 0.3f, 0.3f);
		material1->ka = vec3(0.2f, 0.2f, 0.2f);
		material1->shininess = 30;

		// Textures
		Texture *sphereTexture = new CheckerBoardTexture(15, 20);
		Texture *bowlTexure = new BowlTexure(512, 512);

		// Geometries
		Geometry *sphere = new Sphere();
		std::vector<Geometry *> bowls;

		//bowl = new Bowl(1.0f,1.0f);
		bowls.push_back(new Bowl(1.0f, 1.0f));
		bowls.push_back(new Bowl(1.0f, -1.0f));
		bowls.push_back(new Bowl(-1.0f, 1.0f));
		bowls.push_back(new Bowl(-1.0f, -1.0f));

		// Create objects by setting up their vertex data on the GPU

		for (int i = 0; i < 4; i++)
		{
			Object *BowlObject = new Object(bowlShader, material1, bowlTexure, bowls.at(i));
			BowlObject->translation = vec3(0, 0, 0);
			BowlObject->scale = vec3(2, 2, 2);
			objects.push_back(BowlObject);
		}
		Bowl *buttomLeftCorner = new Bowl(1.0f, 1.0f);
		vec3 normal = buttomLeftCorner->GenVertexData(0.5, 0.5).normal;
		normal = normal / magnitude(normal);
		vec3 direction = (2 * height(0.5, 0.5) + 0.1 * normal);
		Object *sphereObject1 = new Object(gouraudShader, material0, sphereTexture, sphere);
		masterPosition = vec3(-direction.x, -direction.y, direction.z);
		sphereObject1->translation = masterPosition;
		masterNormal = normal;
		
		sphereObject1->scale = vec3(0.1f, 0.1f, 0.1f);
		objects.push_back(sphereObject1);

		int nObjects = objects.size();

		// Camera
		camera.wEye = vec3(0, 4, 8);
		camera.wLookat = vec3(0, 0, 1);
		camera.wVup = vec3(0, 1, 0);

		// Lights
		lights.resize(2);
		lights[0].wLightPos = vec4(5, 5, 4, 0); // ideal point -> directional light source
		lights[0].La = vec3(0.1f, 0.1f, 1);
		lights[0].Le = vec3(3, 0, 0);

		lights[1].wLightPos = vec4(-5, 5, 5, 0); // ideal point -> directional light source
		lights[1].La = vec3(0.1f, 0.1f, 0.1f);
		lights[1].Le = vec3(0, 0, 3);
	}

	void Render()
	{
		RenderState state;
		state.wEye = camera.wEye;
		state.V = camera.V();
		state.P = camera.P();
		state.lights = lights;
		for (Object *obj : objects)
			obj->Draw(state);
	}

	void Animate(float tstart, float tend)
	{
		for (Object *obj : objects)
			obj->Animate(tstart, tend);
		for (int i = 0; i < 2; i++)
			lights.at(i).Animate(tstart, tend);
	}
};

Scene scene;

// Initialization, create an OpenGL context
void onInitialization()
{
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	scene.Build();
}

// Window has become invalid: Redraw
void onDisplay()
{
	glClearColor(0.5f, 0.5f, 0.8f, 1.0f);				// background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen
	scene.Render();
	glutSwapBuffers(); // exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY)
{
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY)
{
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY)
{
	if (state)
		scene.addSphere((float)pX / windowWidth, (float)pY / windowHeight);
	glutPostRedisplay();
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY)
{
}

// Idle event indicating that some time elapsed: do animation here
void onIdle()
{
	static float tend = 0;
	const float dt = 0.1f; // dt is ???infinitesimal???
	float tstart = tend;
	tend = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	for (float t = tstart; t < tend; t += dt)
	{
		float Dt = fmin(dt, tend - t);
		scene.Animate(t, t + Dt);
	}
	glutPostRedisplay();
}