#include "framework.hpp"
#include "../base/base.hpp"

namespace granite { namespace system {

float fabs(float a) {
	return a < 0.f ? -a : a;
}

int test() {
	using namespace granite::base;
    GLFWwindow* window;
    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_SAMPLES, 8);
    window = glfwCreateWindow(1200, 1200, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window) &&
		   !glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float ar = 1200.f / 1200.f;
		glFrustum(-1.f, 1.f, -ar, ar, 0.9f, 100.f);

		static float an = 0.f;
		an += 0.3f;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.f, 1.0f, -2.5f);
		glRotatef(30.f, 1.f, 0.f, 0.f);
		glRotatef(an, 0.f, 1.f, 0.f);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		glPolygonMode(GL_BACK, GL_FILL);
		glPolygonMode(GL_FRONT, GL_LINE);

		float mm = 5.f;
		float ms = 0.05f;
		glLineWidth(2.f);
		auto fy = [](float x, float z) {
			return -pow(sqrtf(x * x + z * z), -1.f);
			//float dst = sqrtf(x * x + z * z);
			//return cosf(10.f * dst) * (1.f - dst / 10.f) * 0.3f - 1.f;
		};

		glBegin(GL_QUADS);
		for (float x = -mm; x <= mm; x += ms) {
			for (float z = -mm; z <= mm; z += ms) {
				float c = 1.f - vec(x, 0.f, z).distance(vec(0.f, 0.f, 0.f)) / (mm / 2.5f);
				//c *= 1.f / fy(x, z);
				glColor3f(c, c, c);
				glVertex3f(x, fy(x, z), z);
				glVertex3f(x + ms, fy(x + ms, z), z);
				glVertex3f(x + ms, fy(x + ms, z + ms), z + ms);
				glVertex3f(x, fy(x, z + ms), z + ms);
			}
		}
		glEnd();

		glPolygonMode(GL_FRONT, GL_FILL);
		glBegin(GL_QUADS);
		for (float x = -mm; x <= mm; x += ms) {
			for (float z = -mm; z <= mm; z += ms) {
				float c = 1.f - vec(x, 0.f, z).distance(vec(0.f, 0.f, 0.f)) / (mm / 2.5f);
				//c *= 1.f / fy(x, z);
				float off = 0.01f;
				glColor3f(0.f, 0.f, 0.f);
				glVertex3f(x, fy(x, z) - off, z);
				glVertex3f(x + ms, fy(x + ms, z) - off, z);
				glVertex3f(x + ms, fy(x + ms, z + ms) - off, z + ms);
				glVertex3f(x, fy(x, z + ms) - off, z + ms);
			}
		}
		glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
	return 0;
}

}}
