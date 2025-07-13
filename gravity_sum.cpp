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

float yaw    = 90.0f;             // rotation around vertical
float pitch  = 0.0f;             // tilt up/down
float dist   = 800.0f;           // how far the camera sits back
const float cx = screenWidth/2;  // look-at centre X
const float cy = screenHeight/2; // look-at centre Y
// Compute cameraZ
float fovRad = 60.0f * (M_PI/180.0f);
float cameraZ = (screenWidth*0.5f) / std::tan(fovRad*0.5f);		 //
glm::vec3 cameraPos   = glm::vec3(cx, cy, cameraZ);  // start up above the grid
glm::vec3 cameraFront = glm::vec3(0,0,-1);
glm::vec3 cameraUp    = glm::vec3(0,1, 0);



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

float computeWell(const std::vector<Object>& objs,
                  float x, float y) 
{
    float z = 0.0f;
    const float Gscale = 1e-20f;    // tune this first
    const float MAX_DIP = 150.0f;   // clamp after summing
    for (auto &o : objs) {
        float dx = x - o.position[0];
        float dy = y - o.position[1];
        float r  = std::sqrt(dx*dx + dy*dy);

        // 1) clamp to avoid singularity at r→0
        r = std::max(r, o.radius);

        // 2) Newtonian well
        z += -Gscale * o.mass / r;
    }
    // 3) clamp the final depth
    return std::max(z, -MAX_DIP);
}		

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

const float camSpeed = 300.0f; // px per second

// 1) Update yaw/pitch from arrow keys (or mouse)
if(glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS)  yaw   -= 90.0f * dtCam;
if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)  yaw   += 90.0f * dtCam;
if(glfwGetKey(window, GLFW_KEY_UP   ) == GLFW_PRESS)  pitch  = std::min(pitch + 90.0f * dtCam,  89.0f);
if(glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS)  pitch  = std::max(pitch - 90.0f * dtCam, -89.0f);

// 2) Recompute front vector
float yawRad   = glm::radians(yaw);
float pitchRad = glm::radians(pitch);
cameraFront.x = cos(yawRad) * cos(pitchRad);
cameraFront.y = sin(pitchRad);
cameraFront.z = sin(yawRad) * cos(pitchRad);
cameraFront    = glm::normalize(cameraFront);

// 3) Compute right (and you already have worldUp = (0,1,0))
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

// 4) Handle WASD for movement
if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += camSpeed * dtCam * cameraFront;
if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= camSpeed * dtCam * cameraFront;
if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= camSpeed * dtCam * cameraRight;
if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += camSpeed * dtCam * cameraRight;

// 5) Set the view
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glm::vec3 lookAt = cameraPos + cameraFront;
gluLookAt(
    cameraPos.x, cameraPos.y, cameraPos.z,
    lookAt.x,    lookAt.y,    lookAt.z,
    cameraUp.x,  cameraUp.y,  cameraUp.z
);

		//Flamm Paraboloid
		// Grid resolution
const int GRID_W = 40, GRID_H = 30;
float stepX = screenWidth  / float(GRID_W);
float stepY = screenHeight / float(GRID_H);

// Draw horizontal lines
glDisable(GL_LIGHTING);          // turn lighting off for lines
glColor3f(0.4f,0.4f,0.4f);
glLineWidth(1.0f);

for(int iy = 0; iy <= GRID_H; ++iy) {
    float y = iy * stepY;
    glBegin(GL_LINE_STRIP);
    for(int ix = 0; ix <= GRID_W; ++ix) {
        float x = ix * stepX;
        float z = computeWell(objs, x, y);
        glVertex3f(x, y, z);
    }
    glEnd();
}

// Draw vertical lines
for(int ix = 0; ix <= GRID_W; ++ix) {
    float x = ix * stepX;
    glBegin(GL_LINE_STRIP);
    for(int iy = 0; iy <= GRID_H; ++iy) {
        float y = iy * stepY;
        float z = computeWell(objs, x, y);
        glVertex3f(x, y, z);
    }
    glEnd();
}

glEnable(GL_LIGHTING);    
	
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


