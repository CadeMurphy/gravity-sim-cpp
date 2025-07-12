#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>

float screenHeight = 600.0f;
float screenWidth = 800.0f;

GLFWwindow* StartGLFW();

class Object {
	public:
	
		std::vector<float> position;
		std::vector<float> velocity;
		float radius;
		Object(std::vector<float> position, std::vector<float> velocity, float radius = 15.0f){
			this->position = position;
			this->velocity = velocity;
			this->radius = radius;
		}
		
		void accelerate(float x, float y, float dt){
			this->velocity[0] += x * dt;
			this->velocity[1] += y * dt;
		};
		void updatePos(float dt){
			this->position[0] += this->velocity[0] * dt;
			this->position[1] += this->velocity[1] * dt;
		}

		void DrawCircle(float centerX, float centerY, float radius, int res) {
		glBegin(GL_TRIANGLE_FAN);
		glVertex2d(centerX, centerY);

		//Calculate th vertices around the center point to draw a circle
		for(int i = 0; i <= res; ++i){
			float angle = 2.0f * 3.14159265358979323 * (static_cast<float> (i) / res);
			float x_pix = centerX + cos(angle) * radius;
			float y_pix = centerY + sin(angle) * radius;

			glVertex2f(x_pix, y_pix);
		}

		glEnd();
		}

		void handleCollision(float screenW, float screenH){
			float r = this->radius;        // e.g. 50.0f
			// left wall
			if (this->position[0] < r) {
			    this->position[0] = r;
			    this->velocity[0] *= -0.95f;
			}
			// right wall
			if (this->position[0] > screenWidth - r) {
			    this->position[0] = screenWidth - r;
			    this->velocity[0] *= -0.95f;
			}
			// bottom wall
			if (this->position[1] < r) {
			    this->position[1] = r;
			    this->velocity[1] *= -0.95f;
			}
			// top wall
			if (this->position[1] > screenHeight - r) {
			    this->position[1] = screenHeight - r;
			    this->velocity[1] *= -0.95f;
			}
		  
		}
		};



int main(){
	//Setup drawing ability
	GLFWwindow* window = StartGLFW();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// map x from [0…800], y from [0…600] into clip-space automatically:
	glOrtho(0.0, screenWidth, 0.0, screenHeight, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	std::vector<Object> objs = {
		Object(std::vector<float>{400.0f, 800.0f}, std::vector<float>{0.0f, 0.0f}),
		Object(std::vector<float>{400.0f, 500.0f}, std::vector<float>{0.0f, 0.0f})
	};


	glfwSwapInterval(1);
	double lastTime = glfwGetTime();
	const float slowDown = 0.3f;   

	//The render loop
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		double now   = glfwGetTime();
		double realDt= now - lastTime;
		lastTime     = now;

		// convert seconds → “frames at 60 fps” and apply slowDown
		float dt = float(realDt * 60.0 * slowDown);

		for(auto& obj : objs){
			obj.accelerate(0.0f, -9.8f, dt);
    			obj.updatePos(dt);
		}

		// ——————————————————————————————————
		// 1) Ball–ball collisions
		// ——————————————————————————————————
		for(size_t i = 0; i < objs.size(); ++i){
		  for(size_t j = i+1; j < objs.size(); ++j){
		    auto &A = objs[i];
		    auto &B = objs[j];

		    // vector from B to A
		    float dx = A.position[0] - B.position[0];
		    float dy = A.position[1] - B.position[1];
		    float dist2 = dx*dx + dy*dy;
		    float rSum  = A.radius + B.radius;

		    // if they overlap…
		    if(dist2 < rSum*rSum){
		      float dist = std::sqrt(dist2);
		      if(dist == 0.0f) dist = rSum * 0.5f;  // avoid div0

		      // 2) collision normal (unit)
		      float nx = dx / dist;
		      float ny = dy / dist;

		      // 3) push them apart by half the overlap each
		      float overlap = (rSum - dist);
		      A.position[0] +=  nx * (overlap * 0.5f);
		      A.position[1] +=  ny * (overlap * 0.5f);
		      B.position[0] += -nx * (overlap * 0.5f);
		      B.position[1] += -ny * (overlap * 0.5f);

		      // 4) swap velocity component along the normal
		      float dvx = A.velocity[0] - B.velocity[0];
		      float dvy = A.velocity[1] - B.velocity[1];
		      float vAlong = dvx*nx + dvy*ny;
		      if(vAlong < 0.0f){              // only if they’re moving into each other
			float e = 0.95f;              // restitution
			float j = -(1.0f + e) * vAlong / 2.0f;
			float ix = j * nx;
			float iy = j * ny;
			A.velocity[0] +=  ix;
			A.velocity[1] +=  iy;
			B.velocity[0] -=  ix;
			B.velocity[1] -=  iy;
		      }
		    }
		  }
		}


		for(auto& obj : objs){
		  obj.handleCollision(screenWidth, screenHeight); // walls
		  obj.DrawCircle( obj.position[0],
				  obj.position[1],
				  obj.radius,
				  50 );
		}
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

//Window Creation
GLFWwindow* StartGLFW(){
	if(!glfwInit()){
		std::cerr << "failed to initialize glfw" << std::endl;
		return nullptr;
	}
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "gravity_sim", NULL, NULL);

	return window;
};


