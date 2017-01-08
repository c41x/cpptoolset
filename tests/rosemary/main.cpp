#define GLFW_INCLUDE_VULKAN
#include <system/system.hpp>

using namespace granite;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                             VkDebugReportObjectTypeEXT objType,
                                             uint64_t obj,
                                             size_t location,
                                             int32 code,
                                             const char *layerPrefix,
                                             const char *msg,
                                             void *userData) {
    std::cout << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

int main(int argc, char**argv)  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Rosemary Vulkan", nullptr, nullptr);

    //- checking vulkan extension support
    uint32 vkExtensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionsCount, nullptr);
    std::vector<VkExtensionProperties> vkExtensions(vkExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionsCount, vkExtensions.data());
    for (const auto &e : vkExtensions) {
        std::cout << "supported extension: " << e.extensionName << std::endl;
    }

    uint32 layersCount = 0;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());
    for (const auto &e : availableLayers) {
        std::cout << "supported validation layer: " << e.layerName << std::endl;
    }

    //- initialize vulkan instance
    uint32 extensionsCount = 0;
    const char **extensions;
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount); // checks extensions required by glfw
    std::vector<const char*> vkRequiredExtensions;
    vkRequiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    for (uint32 i = 0; i < extensionsCount; ++i) {
        vkRequiredExtensions.push_back(extensions[i]);
        std::cout << "required extension: " << extensions[i] << std::endl;
    }

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    // verify that all required extensions are supported
    for (const auto &ext : vkRequiredExtensions) {
        if (std::find_if(vkExtensions.begin(), vkExtensions.end(),
                         [&ext](const VkExtensionProperties &prop) {
                             return strcmp(ext, prop.extensionName) == 0;
                         }) == std::end(vkExtensions)) {
            std::cout << "extension: " << ext << " not supported" << std::endl;
        }
    }

    // verify validation layers support
    for (uint32 i = 0; i < validationLayers.size(); ++i) {
        if (std::find_if(availableLayers.begin(), availableLayers.end(),
                         [&validationLayers, &i](const VkLayerProperties &lay) {
                             return strcmp(validationLayers[i], lay.layerName) == 0;
                         }) == std::end(availableLayers)) {
            std::cout << "validation layer: " << validationLayers[i] << " not supported" << std::endl;
        }
    }


    VkInstance instance;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Rosemary";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
    appInfo.pEngineName = "Granite";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = vkRequiredExtensions.size();
    createInfo.ppEnabledExtensionNames = vkRequiredExtensions.data();
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    auto result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        std::cout << "failed to create vulkan instance" << std::endl;
    }
    else {
        std::cout << "created vulkan instance" << std::endl;
    }

    //- setup needed function pointers
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

    if (vkCreateDebugReportCallbackEXT == nullptr) { std::cout << "extension not present: vkCreateDebugReportCallbackEXT" << std::endl; }
    if (vkDestroyDebugReportCallbackEXT == nullptr) { std::cout << "extension not present: vkDestroyDebugReportCallbackEXT" << std::endl; }

    //- setup debug callback
    VkDebugReportCallbackCreateInfoEXT createDebugInfo = {};
    createDebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createDebugInfo.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT// |
        //VK_DEBUG_REPORT_DEBUG_BIT_EXT
        ;
    createDebugInfo.pfnCallback = debugCallback;

    VkDebugReportCallbackEXT callback;
    result = vkCreateDebugReportCallbackEXT(instance, &createDebugInfo, nullptr, &callback);

    if (result != VK_SUCCESS) {
        std::cout << "failed to setup vulkan debug report" << std::endl;
    }
    else {
        std::cout << "created vulkan debug report" << std::endl;
    }

    //- enumerate devices
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cout << "no devices found" << std::endl;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto &device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = device;
            std::cout << "choosing device: " << deviceProperties.deviceName << std::endl;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        std::cout << "could not found physical device" << std::endl;
    }

    uint32 queueFamilyCount = 0;
    int queueFamilyIndex = -1; // graphics queue index
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    for (const auto &queue : queueFamilies) {
        queueFamilyIndex++;
        if (queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            std::cout << "graphics queue found for selected physical device" << std::endl;
            break;
        }
    }

    //- initialize logical device
    VkDevice device;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    float queuePriorities = 1.0f;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.enabledLayerCount = validationLayers.size(); // same as in instance
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();

    result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

    if (result != VK_SUCCESS) {
        std::cout << "failed to create vulkan device" << std::endl;
    }
    else {
        std::cout << "created vulkan device" << std::endl;
    }

    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    //- run applcation loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroyDevice(device, nullptr);
    vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

/*
#include <glbinding/gl43core/gl.h>
#include <glbinding/Binding.h>
#include <system/system.hpp>

// defines naming collisions (glbinding defines GLenum as enum's)
#undef GL_DEBUG_SOURCE_API
#undef GL_DEBUG_SOURCE_WINDOW_SYSTEM
#undef GL_DEBUG_SOURCE_SHADER_COMPILER
#undef GL_DEBUG_SOURCE_THIRD_PARTY
#undef GL_DEBUG_SOURCE_APPLICATION
#undef GL_DEBUG_SOURCE_OTHER
#undef GL_DEBUG_TYPE_ERROR
#undef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#undef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#undef GL_DEBUG_TYPE_PORTABILITY
#undef GL_DEBUG_TYPE_PERFORMANCE
#undef GL_DEBUG_TYPE_MARKER
#undef GL_DEBUG_TYPE_PUSH_GROUP
#undef GL_DEBUG_TYPE_POP_GROUP
#undef GL_DEBUG_TYPE_OTHER
#undef GL_DEBUG_SEVERITY_HIGH
#undef GL_DEBUG_SEVERITY_MEDIUM
#undef GL_DEBUG_SEVERITY_LOW
#undef GL_DEBUG_SEVERITY_NOTIFICATION

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

    switch (source) {
        case GL_DEBUG_SOURCE_API: out += "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: out += "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: out += "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: out += "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: out += "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: out += "Other"; break;
        default:;
    }

    out += "\nType: ";

    switch (type) {
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

    switch (severity) {
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
        logError("incoplete program arguments: <shader-file> <width> <height> | <monitor-id, -1 to default> <debug-output, true/false>");

        // display available monitors
        if (glfwInit()) {
            int monitorsCount;
            GLFWmonitor **monitors = glfwGetMonitors(&monitorsCount);
            for (int i = 0; i < monitorsCount; ++i) {
                int monitorWidth;
                int monitorHeight;
                auto mode = glfwGetVideoMode(monitors[i]);
                monitorWidth = mode->width;
                monitorHeight = mode->height;
                logInfo(strf("monitor (%): % @%x%", i, glfwGetMonitorName(monitors[i]),
                             monitorWidth, monitorHeight));
            }
            glfwTerminate();
        }

        // shutdown
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

    // monitor (only fullscreen, thanks GLFW)
    int customMonitor = -1;
    if (argc >= 5) {
        if (strIs<int>(argv[4]))
            customMonitor = fromStr<int>(argv[4]);
    }

    // debug output flag
    bool debugOutput = false;
    if (argc >= 6) {
        if (strIs<bool>(argv[5]))
            debugOutput = fromStr<bool>(argv[5]);
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
    int monitorsCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorsCount);
    if (customMonitor == -1 || customMonitor >= monitorsCount) {
        window = glfwCreateWindow(width, height, "Rosemary", NULL, NULL);
    }
    else {
        auto mode = glfwGetVideoMode(monitors[customMonitor]);
        window = glfwCreateWindow(mode->width, mode->height, "Rosemary", monitors[customMonitor], NULL);
    }
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
*/
