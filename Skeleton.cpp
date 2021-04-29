//=============================================================================================
// Virus + Antibody
//=============================================================================================
#include "framework.h"

template<class T> struct Dnum {
	float f; // function value
	T d;  // derivatives
	Dnum(float f0, T d0 = T(0)) { f = f0, d = d0; }
	Dnum operator+(Dnum r) { return Dnum(f + r.f, d + r.d); }
	Dnum operator*(Dnum r) { return Dnum(f * r.f, f * r.d + d * r.f); }
	Dnum operator/(Dnum r) { return Dnum(f / r.f, (r.f * d - r.d * f) / r.f / r.f); }
};

// Elementary functions prepared for the chain rule as well
template<class T> Dnum<T> Sin(Dnum<T> g) { return  Dnum<T>(sinf(g.f), cosf(g.f)*g.d); }
template<class T> Dnum<T> Cos(Dnum<T>  g) { return  Dnum<T>(cosf(g.f), -sinf(g.f)*g.d); }
template<class T> Dnum<T> Sinh(Dnum<T> g) { return  Dnum<T>(sinh(g.f), cosh(g.f)*g.d); }
template<class T> Dnum<T> Cosh(Dnum<T> g) { return  Dnum<T>(cosh(g.f), sinh(g.f)*g.d); }
template<class T> Dnum<T> Tanh(Dnum<T> g) { return Sinh(g) / Cosh(g); }
 
typedef Dnum<vec2> Dnum2;

//---------------------------
struct Camera { // 3D camera
//---------------------------
	vec3 wEye, wLookat, wVup;   // extrinsic
	float fov = 75.0f * (float)M_PI / 180.0f, asp = (float)windowWidth / windowHeight, fp = 1, bp = 30;
public:
	mat4 V() { // view matrix: translates the center to the origin
		vec3 w = normalize(wEye - wLookat);
		vec3 u = normalize(cross(wVup, w));
		vec3 v = cross(w, u);
		return TranslateMatrix(wEye * (-1)) * mat4(u.x, v.x, w.x, 0,
			                                       u.y, v.y, w.y, 0,
			                                       u.z, v.z, w.z, 0,
			                                       0,   0,   0,   1);
	}
	mat4 P() { // projection matrix
		return mat4(1 / (tan(fov / 2)*asp), 0,                0,                      0,
			        0,                      1 / tan(fov / 2), 0,                      0,
			        0,                      0,                -(fp + bp) / (bp - fp), -1,
			        0,                      0,                -2 * fp*bp / (bp - fp),  0);
	}
};

vec4 virusColor(float xr, float yr) {
	float c = fabs(cos(xr * 100)) * fabs(cos(yr * 100));
	return vec4(c, c * 0.8f, c * 0.6f, 1);
}
vec4 rampRedBlue(float xr, float yr) { return vec4(1, 0, 0, 1) * (1 - yr) + vec4(0, 0, 1, 1) * yr; }
vec4 rampCyanMagenta(float xr, float yr) { return  vec4(0, 1, 1, 1) * (1 - yr) + vec4(0.3, 0, 1, 1) * yr; }

//---------------------------
class ProcTexture : public Texture {
//---------------------------
public:
	ProcTexture(const int width, const int height, vec4 (*color)(float xr, float yr) ) : Texture() {
		std::vector<vec4> image(width * height);
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) image[y * width + x] = color((float)x/width, (float)y/height);
		}
		create(width, height, image, GL_LINEAR);
	}
};

//---------------------------
struct RenderState {
//---------------------------
	mat4	    M, Minv, V, P;
	Texture *   texture;
	vec3	    wEye;
};

//---------------------------
class PhongShader : public GPUProgram {
//---------------------------
	const char * vertexSource = R"(
		#version 330
		precision highp float;

		const vec3 wLightPos  = vec3(3, 4, 5);	// directional light source;
		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform vec3  wEye;         // pos of eye

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;

		out vec3 wNormal;		    // normal in world space
		out vec3 wView;             // view in world space
		out vec3 wLight;		    // light dir in world space
		out vec2 texcoord;

		void main() {
			gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
		    wView  = wEye - (vec4(vtxPos, 1) * M).xyz;
			wLight = wLightPos;
		    wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		    texcoord = vtxUV;
		}
	)";

	// fragment shader in GLSL
	const char * fragmentSource = R"(
		#version 330
		precision highp float;

		const vec3 ks = vec3(2, 2, 2);
		const float shininess = 50.0f;
		const vec3 La = vec3(0.1f, 0.1f, 0.1f);
		const vec3 Le = vec3(2, 2, 2);    

		uniform sampler2D diffuseTexture;

		in  vec3 wNormal;       // interpolated world sp normal
		in  vec3 wView;         // interpolated world sp view
		in  vec3 wLight;        // interpolated world sp illum dir
		in  vec2 texcoord;
		
        out vec4 fragmentColor; // output goes to frame buffer

		void main() {
			vec3 N = normalize(wNormal);
			vec3 V = normalize(wView); 
			if (dot(N, V) < 0) N = -N;
			vec3 kd = texture(diffuseTexture, texcoord).rgb;
			vec3 ka = kd * 3.14;
			vec3 L = normalize(wLight);
			vec3 H = normalize(L + V);
			float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
			fragmentColor = vec4(ka * La + (kd * cost + ks * pow(cosd, shininess)) * Le, 1);
		}
	)";
public:
	PhongShader() { create(vertexSource, fragmentSource, "fragmentColor"); }

	void Bind(RenderState state) {
		setUniform(state.M * state.V * state.P, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");
		setUniform(state.wEye, "wEye");
		setUniform(*state.texture, std::string("diffuseTexture"));
	}
};

PhongShader * shader;

struct VertexData {
	vec3 position, normal;
	vec2 texcoord;
};

//---------------------------
class Geometry {
//---------------------------
protected:
	unsigned int vao, vbo;        // vertex array object
public:
	Geometry() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
	}
	void Load(const std::vector<VertexData>& vtxData) {
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vtxData.size() * sizeof(VertexData), &vtxData[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);  // attribute array 0 = POSITION
		glEnableVertexAttribArray(1);  // attribute array 1 = NORMAL
		glEnableVertexAttribArray(2);  // attribute array 2 = TEXCOORD0
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}
	virtual void Draw() = 0;
	virtual void Animate(float t) { }
	~Geometry() {
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
};

//---------------------------
class ParamSurface : public Geometry {
//---------------------------
	unsigned int nVtxPerStrip, nStrips;
public:
	virtual VertexData GenVertexData(float u, float v) = 0;

	void create(int N = 30, int M = 30) {
		nVtxPerStrip = (M + 1) * 2;
		nStrips = N;
		std::vector<VertexData> vtxData;	// vertices on the CPU
		for (int i = 0; i < N; i++) {
			for (int j = 0; j <= M; j++) {
				vtxData.push_back(GenVertexData((float)j / M, (float)i / N));
				vtxData.push_back(GenVertexData((float)j / M, (float)(i + 1) / N));
			}
		}
		Load(vtxData);
	}

	void Draw() {
		glBindVertexArray(vao);
		for (unsigned int i = 0; i < nStrips; i++) glDrawArrays(GL_TRIANGLE_STRIP, i *  nVtxPerStrip, nVtxPerStrip);
	}
};

//---------------------------
class Sphere : public ParamSurface {
//---------------------------
public:
	Sphere() { create(); }

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		float phi = u * 2 * M_PI, theta = v * M_PI;
		vd.normal = vec3(cosf(phi) * sinf(theta), sinf(phi) * sinf(theta), cosf(theta)) * 15;
		vd.position = vd.normal;
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

//---------------------------
class VirusBody : public ParamSurface {
//---------------------------
	float phase = 0;
public:
	VirusBody() { create(); }

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		Dnum2 U(u * (float)M_PI * 2, vec2(1, 0)), V(v * M_PI, vec2(0, 1));
		Dnum2 r = Cos(U * 3 + V * 6 + phase) * 0.2f + 1.5f;
		Dnum2 X = Cos(U) * Sin(V) * r, Y = Sin(U) * Sin(V) * r, Z = Cos(V) * r;
		vd.position = vec3(X.f, Y.f, Z.f);
		vd.normal = cross(vec3(X.d.x, Y.d.x, Z.d.x), vec3(X.d.y, Y.d.y, Z.d.y));
		vd.texcoord = vec2(u, v);
		return vd;
	}
	void Animate(float t) {
		phase = t * 3;
		create();
	}
};

//---------------------------
class Tractricoid : public ParamSurface {
//---------------------------
public:
	Tractricoid() { create(); }

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		const float height = 3.0f;
		Dnum2 U(v * height, vec2(1, 0)), V(u * 2 * M_PI, vec2(0, 1));
		Dnum2 X = Cos(V) / Cosh(U), Y = Sin(V) / Cosh(U), Z = U + Tanh(U) * (-1);
		vd.position = vec3(X.f, Y.f, Z.f - 2) / 4;
		vd.normal = cross(vec3(X.d.x, Y.d.x, Z.d.x), vec3(X.d.y, Y.d.y, Z.d.y));
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

//---------------------------
class Tetra : public Geometry {
//---------------------------
	float scale;

	void subdivide(const std::vector<VertexData>& ping, std::vector<VertexData>& pong) {
		pong.clear();
		for (int i = 0; i < ping.size() / 3; i++) {
			vec3 a = ping[3 * i].position, b = ping[3 * i + 1].position, c = ping[3 * i + 2].position;
			vec3 d = (a + b + c) / 3 + normalize(ping[3 * i].normal) * scale;
			addFace(pong, a, (a + b) / 2, (a + c) / 2);
			addFace(pong, (a + b) / 2, b, (b + c) / 2);
			addFace(pong, (a + c) / 2, (b + c) / 2, c);
			addFace(pong, (a + b) / 2, (b + c) / 2, d);
			addFace(pong, (c + a) / 2, (a + b) / 2, d);
			addFace(pong, (b + c) / 2, (c + a) / 2, d);
		}
	}
	void addFace(std::vector<VertexData>& vtxData, vec3 a, vec3 b, vec3 c) {
		VertexData v;
		v.normal = cross(c - a, b - a);
		v.texcoord = vec2(0, 0);
		v.position = a; vtxData.push_back(v);
		v.position = b; vtxData.push_back(v);
		v.position = c; vtxData.push_back(v);
	}
public:
	void create() {
		const vec3 b(1, 1, 1), a(1, -1, -1), c(-1, 1, -1), d(-1, -1, 1);
		std::vector<VertexData> vtxDataPing, vtxDataPong;
		addFace(vtxDataPing, a, b, c);
		addFace(vtxDataPing, a, d, b);
		addFace(vtxDataPing, a, c, d);
		addFace(vtxDataPing, b, d, c);
		subdivide(vtxDataPing, vtxDataPong);
		subdivide(vtxDataPong, vtxDataPing);
		Load(vtxDataPing);	
	}
	void Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 4 * 6 * 6 * 3);
	}
	void Animate(float t) {
		scale = 2 + sin(t * 5);
		create();
	}
};

//---------------------------
struct Object {
//---------------------------
	Texture *  texture;
	Geometry * geometry;
	vec3 translation, rotationAxis;
	float rotationAngle;
public:
	Object(Texture * _texture, Geometry * _geometry) :
		translation(vec3(0, 0, 0)), rotationAxis(0, 0, 1), rotationAngle(0) {
		texture = _texture;
		geometry = _geometry;
	}
	virtual void Draw(RenderState state) {
		state.M = RotationMatrix(rotationAngle, rotationAxis) * TranslateMatrix(translation);
		state.Minv = TranslateMatrix(-translation) * RotationMatrix(-rotationAngle, rotationAxis);
		state.texture = texture;
		shader->Bind(state);
		geometry->Draw();
	}
	virtual void Animate(float tstart, float tend) { geometry->Animate(tend); }
};

vec4 qmul(vec4 q1, vec4 q2) {	// quaternion multiplication
	vec3 d1(q1.x, q1.y, q1.z), d2(q2.x, q2.y, q2.z);
	vec3 imag = d2 * q1.w + d1 * q2.w + cross(d1, d2);
	return vec4(imag.x, imag.y, imag.z, q1.w * q2.w - dot(d1, d2));
}

mat4 quatToMatrix(vec4 q, vec4 qinv) {
	return mat4(qmul(qmul(q, vec4(1, 0, 0, 0)), qinv),
		        qmul(qmul(q, vec4(0, 1, 0, 0)), qinv),
		        qmul(qmul(q, vec4(0, 0, 1, 0)), qinv),
		        vec4(0, 0, 0, 1));
}

//---------------------------
struct Virus : Object {
//---------------------------
	bool alive = true;
	std::vector<vec2> uvhorns;
	vec3 pivot = vec3(2, 2, 2), position;
	vec4 q, qinv;
	Tractricoid horn;
	ProcTexture textureHorn;

	Virus( ) : Object(new ProcTexture(256, 256, virusColor), new VirusBody()), textureHorn(256, 256, rampRedBlue) {
		for (float v = 0.05; v < 1; v += 0.1) {
			int n = 20 * sin(v * M_PI);
			for (int i = 0; i < n; i++) uvhorns.push_back(vec2((float)i / n, v));
		}
	}

	void Draw(RenderState state) {
		mat4 M = RotationMatrix(rotationAngle, rotationAxis) * TranslateMatrix(-pivot) * quatToMatrix(q, qinv) * TranslateMatrix(pivot);
		vec4 hPosition = vec4(0, 0, 0, 1) * M;
		position = vec3(hPosition.x, hPosition.y, hPosition.z);
		mat4 Minv = TranslateMatrix(-pivot) * quatToMatrix(qinv, q) * TranslateMatrix(vec3(pivot)) * RotationMatrix(-rotationAngle, rotationAxis);

		state.M = M;
		state.Minv = Minv;
		state.texture = texture;
		shader->Bind(state);
		geometry->Draw();

		state.texture = &textureHorn;
		for (auto uvhorn : uvhorns) {
			VertexData vd = ((ParamSurface *)geometry)->GenVertexData(uvhorn.x, uvhorn.y);
			vec3 hornAxis = cross(vec3(0, 0, 1), vd.normal);
			float hornAngle = acosf(dot(vec3(0, 0, 1), normalize(vd.normal)));
			state.M = RotationMatrix(hornAngle, hornAxis) * TranslateMatrix(vd.position) * M;
			state.Minv = Minv * TranslateMatrix(-vd.position) * RotationMatrix(-hornAngle, hornAxis);
			shader->Bind(state);
			horn.Draw();
		}
	}
	void Animate(float tStart, float tEnd) {
		if (alive) {
			q = vec4(sin(tEnd/2), sin(tEnd/3), sin(tEnd/5), cos(tEnd));
			q = q / sqrtf(dot(q, q));
			qinv = vec4(-q.x, -q.y, -q.z, q.w);
			rotationAngle = tEnd;
			geometry->Animate(tEnd);
		}
	}
};

float rnd() { return (float)rand() / RAND_MAX; }

//---------------------------
struct AntiBody : Object {
//---------------------------
	vec3 velocity = vec3(0, 0, 0);

	AntiBody() : Object(new ProcTexture(1, 1, rampRedBlue), new Tetra()) { translation = vec3(-3, 0, 0); }

	void Animate(float tStart, float tEnd) {
		translation = translation + velocity * (tEnd - tStart);
		rotationAngle = 2 * tEnd;
		geometry->Animate(tEnd);
	}
	void SampleVelocity(vec3 dir) {
		velocity = vec3(rnd() - 0.5f, rnd() - 0.5f, rnd() - 0.5f) * 5 + dir;
	}
};

//---------------------------
class Scene {
//---------------------------
	Camera camera; // 3D camera
	Virus * virusObject;
	AntiBody * antibodyObject;
	Object * sphereObject;
public:
	void Build() {
		shader = new PhongShader();

		antibodyObject = new AntiBody();
		virusObject = new Virus();
		sphereObject = new Object(new ProcTexture(128, 128, rampCyanMagenta), new Sphere());

		camera.wEye = vec3(0, 0, 12); camera.wLookat = vec3(0, 0, 0); camera.wVup = vec3(0, 1, 0);
	}

	void Render() {
		RenderState state;
		state.wEye = camera.wEye;
		state.V = camera.V(); state.P = camera.P();
		sphereObject->Draw(state); virusObject->Draw(state); antibodyObject->Draw(state);
	}

	void Animate(float tstart, float tend) {
		virusObject->Animate(tstart, tend); antibodyObject->Animate(tstart, tend);
		if (length(virusObject->position - antibodyObject->translation) < 1.5) virusObject->alive = false;
	}
	void SetAvatarVelocity(vec3 d) { antibodyObject->SampleVelocity(d); }
};
Scene scene;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	scene.Build();
}

void onDisplay() {
	glClearColor(0.5f, 0.5f, 0.8f, 1.0f);							// background color 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen
	scene.Render();
	glutSwapBuffers();									// exchange the two buffers
}

int keyPressed[256] = { 0 };
void onKeyboard(unsigned char key, int pX, int pY) { keyPressed[key] = 1; }
void onKeyboardUp(unsigned char key, int pX, int pY) { keyPressed[key] = 0; }
void onMouse(int button, int state, int pX, int pY) { }
void onMouseMotion(int pX, int pY) {}

void onIdle() {
	static float tend = 0, tEvent = 0;
	const float dt = 0.05f, dtEvent = 0.1f; // dt is ”infinitesimal”
	float t = tend;
	tend = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	while (t < tend) {
		float te = fmin(fmin(t + dt, tend), tEvent);
		scene.Animate(t, te);
		if (te == tEvent) {
			scene.SetAvatarVelocity(vec3(keyPressed['x'] - keyPressed['X'], keyPressed['y'] - keyPressed['Y'], keyPressed['z'] - keyPressed['Z']));
			tEvent += dtEvent;
		}
		t = te;
	}	
	glutPostRedisplay();
}