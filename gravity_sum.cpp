#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

float screenHeight = 600.0f;
float screenWidth = 800.0f;

const double G = 6.6743e-11;
const double METRES_PER_PIXEL = 1e6;

//Camera Setup

float yaw    = 0.0f;             // rotation around vertical
float pitch  = 0.0f;             // tilt up/down
float dist   = 800.0f;           // how far the camera sits back
const float cx = screenWidth/2;  // look-at centre X
const float cy = screenHeight/2; // look-at centre Y



GLFWwindow* StartGLFW();

class Object {
	public:
	
		std::vector<float> position;
		std::vector<float> velocity;
		float radius;
		float mass;

		Object(std::vector<float> position, std::vector<float> velocity, float mass, float radius = 15.0f){
			this->position = position;
			this->velocity = velocity;
			this->radius = radius;
			this->mass = mass;
		}
		
		void accelerate(float x, float y, float dt){
			this->velocity[0] += x * dt;
			this->velocity[1] += y * dt;
		};
		void updatePos(float dt){
			this->position[0] += this->velocity[0] * dt;
        		this->position[1] += this->velocity[1] * dt;
		}
		void draw() const {
			glPushMatrix();
			glTranslatef(position[0], position[1], position[2]);
			glColor3f(0.8f, 0.2f, 0.2f);

			const int slices = 24;   // longitude subdivisions
			const int stacks = 16;   // latitude subdivisions

			// build the sphere one stack at a time
			for(int i = 0; i < stacks; ++i) {
			    // polar angles for this band
			    float lat0 = M_PI * ( -0.5f + float(i)   / stacks );
			    float lat1 = M_PI * ( -0.5f + float(i+1) / stacks );
			    float z0   = sin(lat0);
			    float zr0  = cos(lat0);
			    float z1   = sin(lat1);
			    float zr1  = cos(lat1);

			    glBegin(GL_QUAD_STRIP);
			    for(int j = 0; j <= slices; ++j) {
				float lng = 2 * M_PI * float(j) / slices;
				float x = cos(lng);
				float y = sin(lng);

				// normal + vertex at stack i
				glNormal3f(x*zr0, y*zr0, z0);
				glVertex3f(x*zr0 * radius, y*zr0 * radius, z0 * radius);

				// normal + vertex at stack i+1
				glNormal3f(x*zr1, y*zr1, z1);
				glVertex3f(x*zr1 * radius, y*zr1 * radius, z1 * radius);
			    }
			    glEnd();
			}

			glPopMatrix();
		}
};


		

int main(){
	//Setup drawing ability
	GLFWwindow* window = StartGLFW();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, 800, 600);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//Lighting
	// enable lighting and a single light source
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// let glColor-calls tint the material
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// auto-normalize normals after scaling
	glEnable(GL_NORMALIZE);

	// place the light somewhere “up and to the side”
	GLfloat lightPos[] = { 200.0f, 200.0f, 200.0f, 1.0f };
	GLfloat lightCol[] = { 1.0f, 1.0f, 0.9f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightCol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);

	// dark grey background so your white spheres pop
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0,
               screenWidth/screenHeight,
               0.1,
               2000.0);

	// Compute cameraZ
	float fovRad = 60.0f * (M_PI/180.0f);
	float cameraZ = (screenWidth*0.5f) / std::tan(fovRad*0.5f);

	// Initial view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(  screenWidth*0.5f,
		    screenHeight*0.5f,
		    cameraZ,
		    screenWidth*0.5f,
		    screenHeight*0.5f,
		    0.0f,
		    0,1,0
	);

	float v = 0.00011074f;
	std::vector<Object> objs = {
		Object({300,300,0}, {  0.0f, +v, 0.0f}, 7.35e22f),
    		Object({500,300,0}, {  0.0f, -v, 0.0f}, 7.35e22f)
	};


	glfwSwapInterval(1);
	double lastTime = glfwGetTime();

	//The render loop
	while(!glfwWindowShouldClose(window)) {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double now   = glfwGetTime();
		double realDt= now - lastTime;
		lastTime     = now;

		// dt for camera (seconds of real time)
		float dtCam = float(realDt);

		// dt for physics (seconds of simulation time)
		const float timeScale = 86400.0f;
		float dtPhys = dtCam * timeScale;

		//Camera Computing
		// handle simple key input for yaw/pitch
		const float camSpeed = 90.0f;
		if(glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS)  yaw  -= camSpeed * dtCam;
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)  yaw  += camSpeed * dtCam;
		if(glfwGetKey(window, GLFW_KEY_UP   ) == GLFW_PRESS)  pitch = std::min(pitch+camSpeed*dtCam,  89.0f);
		if(glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS)  pitch = std::max(pitch-camSpeed*dtCam, -89.0f);

		// convert to radians
		float yrad = glm::radians(yaw);
		float prad = glm::radians(pitch);

		// spherical → cartesian
		float ex = cx + dist * std::cos(prad) * std::sin(yrad);
		float ey = cy + dist * std::sin(prad);
		float ez =            dist * std::cos(prad) * std::cos(yrad);

		// reset view
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(ex, ey, ez,
			  cx, cy, 0.0f,
			  0, 1, 0);

		// 1) compute accelerations from gravity
		//
		for (size_t i = 0; i < objs.size(); ++i) {
		    float ax = 0.0f, ay = 0.0f;

		    for (size_t j = 0; j < objs.size(); ++j) {
			if (i == j) continue;
			// vector from i→j in metres
			float dxm = (objs[j].position[0] - objs[i].position[0]) * METRES_PER_PIXEL;
			float dym = (objs[j].position[1] - objs[i].position[1]) * METRES_PER_PIXEL;
			float r2  = dxm*dxm + dym*dym + 1e-6f;
			float invR = 1.0f / std::sqrt(r2);
			// acceleration magnitude = G * m_j / r^2
			// but we want ax,ay: G*m_j * (dx/r³)
			float invR3 = invR*invR*invR;
			float a_m   = G * objs[j].mass * invR3;

			ax += a_m * dxm;  
			ay += a_m * dym;
		    }

		    // convert back to pixels/sec²
		    ax /= METRES_PER_PIXEL;
		    ay /= METRES_PER_PIXEL;

		    objs[i].accelerate(ax, ay, dtPhys);
		}


		for(auto& obj : objs){
			obj.updatePos(dtPhys);
			obj.draw();
		}
	
	glfwSwapBuffers(window);
	glfwPollEvents();
	}
}

GLFWwindow* StartGLFW(){
	if(!glfwInit()){
		std::cerr << "failed to initialize glfw" << std::endl;
		return nullptr;
	}
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "gravity_sim", NULL, NULL);

	return window;
};


