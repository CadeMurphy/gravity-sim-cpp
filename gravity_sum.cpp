#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

float screenHeight = 600.0f;
float screenWidth = 800.0f;

GLFWwindow* StartGLFW();

void DrawCircle(float centerX, float centerY, float radius, int res);

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

	std::vector<float> position = {400.0f, 300.0f};
	std::vector<float> velocity = {9.8f, 0.0f};

	//The render loop
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		DrawCircle(position[0], position[1], 50.0f, 50);

		position[0] += velocity[0];
		position[1] += velocity[1];
		velocity[1] += -9.8 / 20.0f;

		//border collision
		if(position[1] < 50.0f || position[1] > screenHeight){
			position[1] = 50.0f;
			velocity[1] *= -0.95;
		}
		if(position[0] > screenWidth - 50.0f || position[0] > screenWidth){
			position[0] = screenWidth - 50.0f;
			velocity[0] *= -0.95;
		}
		if(position[0] < 50.0f || position[0] > screenWidth){
			position[0] = 50.0f;
			velocity[0] *= -0.95;
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
};

