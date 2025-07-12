#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>

float screenHeight = 600.0f;
float screenWidth = 800.0f;

GLFWwindow* StartGLFW();

int main(){
	GLFWwindow* window = StartGLFW();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// map x from [0…800], y from [0…600] into clip-space automatically:
	glOrtho(0.0, screenWidth, 0.0, screenHeight, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float centerX = screenWidth / 2.0f;
	float centerY = screenHeight / 2.0f;
	float radius = 50.0f;
	int res = 100;

	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2d(centerX, centerY);

		for(int i = 0; i <= res; ++i){
			float angle = 2.0f * 3.14159265358979323 * (static_cast<float> (i) / res);
			float x_pix = centerX + cos(angle) * radius;
			float y_pix = centerY + sin(angle) * radius;


			//float x_ndc = (x_pix / screenWidth ) * 2.0f - 1.0f;
			//float y_ndc = (y_pix / screenHeight) * 2.0f - 1.0f;
			glVertex2f(x_pix, y_pix);
		}

		glEnd();

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

