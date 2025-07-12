#include <GLFW/glfw3.h>
#include <iostream>

int main(){
    // 1) initialize GLFW
    if(!glfwInit()){
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    // 2) create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Minimal Test", nullptr, nullptr);
    if(!window){
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    // 3) make the window's context current
    glfwMakeContextCurrent(window);

    // (viewport is automatically set to full-window on most drivers)

    // 4) main loop
    while(!glfwWindowShouldClose(window)){
        // clear to a visible color
        glClearColor(0.2f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw a simple triangle in clip‐space (coordinates in [-1,1])
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.2f, 0.2f);
            glVertex2f( 0.0f,  0.5f);  // top
            glVertex2f( 0.5f, -0.5f);  // bottom right
            glVertex2f(-0.5f, -0.5f);  // bottom left
        glEnd();

        // swap and poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 5) clean up
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

