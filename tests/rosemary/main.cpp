#include <glbinding/gl43core/gl.h>
#include <glbinding/Binding.h>
#include <system/system.hpp>

using namespace granite;
using namespace granite::base;
using namespace gl43core;

// error checking for shaders
bool checkShader(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, (gl::GLenum)GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, (gl::GLenum)GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        logInfo((const char*)&log[0]);
		std::cout << string(&log[0]) << std::endl;
        return false;
    }
    return true;
}

bool checkProgram(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, (gl::GLenum)GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, (gl::GLenum)GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        logInfo((const char*)&log[0]);
		std::cout << string(&log[0]) << std::endl;
        return false;
    }
    return true;
}

void debugOutputCallback(gl::GLenum source, gl::GLenum type, GLuint id, gl::GLenum severity, GLsizei length,
						 const GLchar *message, const void *userParam) {
	string out = string("Message: ") + message + "\nSource: ";

	switch ((int)source) {
		case GL_DEBUG_SOURCE_API: out += "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: out += "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: out += "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: out += "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION: out += "Application"; break;
		case GL_DEBUG_SOURCE_OTHER: out += "Other"; break;
		default:;
	}

	out += "\nType: ";

	switch ((int)type) {
		case GL_DEBUG_TYPE_ERROR: out += "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: out += "Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: out += "Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY: out += "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: out += "Performance"; break;
		case GL_DEBUG_TYPE_MARKER: out += "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: out += "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: out += "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER: out += "Other"; break;
		default:;
	}

	out += "\nID: " + toStr(id) + "\nSeverity: ";

	switch ((int)severity) {
		case GL_DEBUG_SEVERITY_HIGH: out += "High"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: out += "Medium"; break;
		case GL_DEBUG_SEVERITY_LOW: out += "Low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: out += "Notification"; break;
		default:;
	}

	logError(out);
}

// shader handles
GLuint shaderProgram, vertexShader, fragmentShader;
GLuint currentShaderProgram, currentFragmentShader;

// uniform handles
GLint uResolution;
GLint uTime;
GLint uMouse;

void reloadShader(const string &file) {
    std::string fragment_source =
        "#version 330\n"
        "layout(location = 0) out vec4 color;\n"
		"uniform vec2 resolution;\n"
		"uniform float time;\n"
		"uniform vec4 mouse;\n"
		"in vec3 pos;\n"
		+ toStr(fs::load(file));

    // create and compiler fragment shader
    const char *source = fragment_source.c_str();
    int length = fragment_source.size();
    glShaderSource(fragmentShader, 1, &source, &length);
    glCompileShader(fragmentShader);

	// if compiled successfully -> link
    if(checkShader(fragmentShader)) {
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		// if program linked successfully -> set as current
		if(checkProgram(shaderProgram)) {
			std::swap(currentShaderProgram, shaderProgram);
			uResolution = glGetUniformLocation(currentShaderProgram, "resolution");
			uTime = glGetUniformLocation(currentShaderProgram, "time");
			uMouse = glGetUniformLocation(currentShaderProgram, "mouse");
		}
	}
}

uint32 watchId = 0;

// release resources
void shutdownGranite() {
	if (watchId != 0)
		fs::removeWatch(watchId);
	fs::close();
	log::shutdown();
}

void shutdownAll() {
    glfwTerminate();
	shutdownGranite();
}

//- program entry
int main(int argc, char**argv) {
	// initialize logger
	string programDir = fs::getUserDirectory() + "/.rosemary";
	fs::createFolderTree(programDir);
	log::init(programDir + "/log.txt");

	// check program arguments
	if (argc < 4) {
		logError("incoplete program arguments: please specify fragment shader file");
		shutdownGranite();
		return -1;
	}

	// setup working directory and set project file (fragment shader)
	string shaderFile = argv[1];
	if (!fs::exists(shaderFile)) {
		logError(strs("specified shader file does not exist: ", shaderFile));
		shutdownGranite();
		return -1;
	}
	string shaderDir = extractFilePath(shaderFile);
	if (!fs::open(shaderDir)) {
		shutdownGranite();
		return -1;
	}

	// extract resulution from arguments
	int width = 640;
	int height = 480;
	if (strIs<int>(argv[2]))
		width = fromStr<int>(argv[2]);
	if (strIs<int>(argv[3]))
		height = fromStr<int>(argv[3]);

	// debug output flag
	bool debugOutput = false;
	if (argc >= 5) {
		if (strIs<bool>(argv[4]))
			debugOutput = fromStr<bool>(argv[4]);
	}

	// setup file watcher
	string shaderFileName = extractFileName(shaderFile);
	watchId = fs::addWatch(shaderFileName);
	if (watchId == 0) {
		logInfo(strs("could not setup file watch for firectory: ", shaderDir));
	}

	// create OpenGL window
	GLFWwindow* window;
    if (!glfwInit()) {
		logError("glfwInit error");
		shutdownAll();
        return -1;
	}

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	if (debugOutput) {
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}
    window = glfwCreateWindow(width, height, "Rosemary", NULL, NULL);
    if (!window) {
		logError("glfwCreateWindow error");
		shutdownAll();
        return -1;
    }
    glfwMakeContextCurrent(window);

	// initialize glbinding
	glbinding::Binding::initialize();

	// print OpenGL version
	logInfo((const char*)glGetString(GL_VERSION));
	logInfo((const char*)glGetString(GL_VENDOR));
	logInfo((const char*)glGetString(GL_RENDERER));
	logInfo((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	// initialize error callback
	if (debugOutput) {
		if (glfwExtensionSupported("GL_ARB_debug_output")) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debugOutputCallback, nullptr);
			logInfo("GL_ARB_debug_output initialized");
		}
		else {
			logError("GL_ARB_debug_output not supported");
		}
	}

	//- setup VAO
	// create fullscreen quad VBO
	GLfloat vertexData[] = {
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f,-1.0f, 0.0f,
		-1.0f,-1.0f, 0.0f
    };
    GLuint indexData[] = { 0, 1, 2, 2, 1, 3, };
	GLuint vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer((gl::GLenum)GL_ARRAY_BUFFER, vbo);
    glBufferData((gl::GLenum)GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 3, vertexData, (gl::GLenum)GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, (gl::GLenum)GL_FLOAT, (gl::GLboolean)GL_FALSE, 0, 0);
    glGenBuffers(1, &ibo);
	glBindBuffer((gl::GLenum)GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData((gl::GLenum)GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 2 * 3, indexData, (gl::GLenum)GL_STATIC_DRAW);
	glBindVertexArray(0);

	//- setup shaders
    vertexShader = glCreateShader((gl::GLenum)GL_VERTEX_SHADER);
    fragmentShader = glCreateShader((gl::GLenum)GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
	currentFragmentShader = glCreateShader((gl::GLenum)GL_FRAGMENT_SHADER);
	currentShaderProgram = glCreateProgram();

	// vertex shader is not changing
    std::string vertex_source =
        "#version 330\n"
        "layout(location = 0) in vec3 p;\n"
		"out vec3 pos;\n"
        "void main() {\n"
		"   pos = p.xyz;\n"
        "   gl_Position = vec4(p, 1.0);\n"
        "}\n";

    const char *source = vertex_source.c_str();
    int length = vertex_source.size();
    glShaderSource(vertexShader, 1, &source, &length);
    glCompileShader(vertexShader);
    if(!checkShader(vertexShader)) {
		shutdownAll();
		return -1;
    }

	// hot reload fragment shader and link result
	reloadShader(shaderFileName);

	//- OpenGL setup
	glDisable(GL_DEPTH_TEST);
	timer t;
	t.init();
	t.reset();
	double mouseX = 0.0, mouseY = 0.0;

    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS) {
        glfwPollEvents();

		// reload shader when file is changed
		auto cs = fs::pollWatch(watchId);
		for (auto c : cs) {
			if (std::get<0>(c) == fs::fileMonitorModify && std::get<1>(c) == shaderFileName) {
				reloadShader(shaderFileName);
				break;
			}
		}

		// poll input data
		glfwGetCursorPos(window, &mouseX, &mouseY);
		float btnL = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? 1.0f : 0.0f;
		float btnR = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 1.0f : 0.0f;

		// draw fullscreen quad with shader
        glUseProgram(currentShaderProgram);
		glUniform2f(uResolution, (GLfloat)width, (GLfloat)height);
		glUniform1f(uTime, (GLfloat)t.getTimeS());
		glUniform4f(uMouse, (GLfloat)mouseX, (GLfloat)mouseY, btnL, btnR);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

		// process log
		log::process();
    }

	shutdownAll();
	return 0;
}

// TODO: delete all resources
// TODO: OSC emacs integration
// TODO: investigate double reloads when editing shader in emacs
// TODO: texture / sound support
