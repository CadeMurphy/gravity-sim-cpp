#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <GL/glu.h>

float screenHeight = 600.0f;
float screenWidth = 800.0f;
const double G = 6.6743e-11; // m^3 kg^-1 s^-2
const double METRES_PER_PIXEL = 1e6;
double camX = 0, camY = 0, camZ = 600;
const double panSpeed = 200.0;   // units per second


GLFWwindow* StartGLFW();

void DrawParaboloidGrid(int gridLines = 80,
                        float radius    = 400.0f,
                        float k         = 0.000005f) 
{
    glColor3f(0.6f,0.6f,0.6f);
    float step = (2*radius)/gridLines;

    // lines in X direction
    for(int i = 0; i <= gridLines; ++i){
        float x = -radius + i*step;
        glBegin(GL_LINE_STRIP);
        for(int j = 0; j <= gridLines; ++j){
            float y = -radius + j*step;
            float z = -k*(x*x + y*y);
            glVertex3f(x, z, y);
        }
        glEnd();
    }

    // lines in Y direction
    for(int j = 0; j <= gridLines; ++j){
        float y = -radius + j*step;
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i <= gridLines; ++i){
            float x = -radius + i*step;
            float z = -k*(x*x + y*y);
            glVertex3f(x, z, y);
        }
        glEnd();
    }
}


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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0,               // 45° vertical FOV
               screenWidth/screenHeight,
               0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	std::vector<Object> objs = {
		Object(
			{300.0f, 300.0f},    // x=400−100, y=300
			{0.0f,  0.00011074f},// vx=0, vy=+0.00011074
			7.35e22f             // mass in kg
		    ),
		    // B: 100 px right of centre, moving down at the same speed
		    Object(
			{500.0f, 300.0f},    // x=400+100, y=300
			{0.0f, -0.00011074f},// vx=0, vy=−0.00011074
			7.35e22f
		    )
			};


	glfwSwapInterval(1);
	double lastTime = glfwGetTime();

	//The render loop
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		double now   = glfwGetTime();
		double realDt= now - lastTime;
		lastTime     = now;

		glfwPollEvents();
		  if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) camY += panSpeed * realDt;
		  if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS) camY -= panSpeed * realDt;
		  if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS) camX -= panSpeed * realDt;
		  if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS) camX += panSpeed * realDt;

		  // 2) camera
		  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		  glMatrixMode(GL_MODELVIEW);
		  glLoadIdentity();
		  gluLookAt(camX, camY, camZ,
			    camX, camY,   0,
			      0,   1,   0);

		  // 3) draw the wire-frame paraboloid grid
		    DrawParaboloidGrid(
			/*gridLines*/ 80,
			/*radius*/    400.0f,
			/*k*/         0.000005f
		    );

		    // 4) draw each mass as a red disc indented in the mesh
		    const float markerRadius = 10.0f;
		    const int   markerRes    = 24;
		    for(auto &obj : objs){
			// shift sim-coords so (0,0) is bowl‐centre
			float x = obj.position[0] - screenWidth*0.5f;
			float y = obj.position[1] - screenHeight*0.5f;
			// bowl height
			float z = -0.000005f * (x*x + y*y);

			// draw a filled red disc (slightly lifted so it never z-fights)
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_TRIANGLE_FAN);
			  glVertex3f(x,   z + 0.5f,  y);
			  for(int t = 0; t <= markerRes; ++t){
			      float θ = 2.0f * M_PI * t / markerRes;
			      glVertex3f(
				  x + std::cos(θ)*markerRadius,
				  z + 0.5f,
				  y + std::sin(θ)*markerRadius
			      );
			  }
			glEnd();
		  			   	

		const float timeScale = 7 * 86400.0f;    // one real second = one simulated day
		float dt = float(realDt * timeScale);


		for(auto& obj : objs){
			for(auto& obj2 : objs){  
				if (&obj2 == &obj) { continue; }

				// 1) compute separation in metres
			    double dx_m = (obj2.position[0] - obj.position[0]) * METRES_PER_PIXEL;
			    double dy_m = (obj2.position[1] - obj.position[1]) * METRES_PER_PIXEL;
			    double dist_m = std::hypot(dx_m, dy_m);
			    if(dist_m < 1.0) continue;            // guard against zero

			    // 2) unit direction
			    double ux = dx_m/dist_m;
			    double uy = dy_m/dist_m;

			    // 3) gravitational accel magnitude (m/s²)
			    double a = G * obj2.mass / (dist_m*dist_m);

			    // 4) convert accel back to pixels/sec²
			    double ax =  (a * ux) / METRES_PER_PIXEL;
			    double ay =  (a * uy) / METRES_PER_PIXEL;

			    // 5) apply it over real-time dt
			    obj.accelerate((float)ax, (float)ay, dt);
								
			}


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

