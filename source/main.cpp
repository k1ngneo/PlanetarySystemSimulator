#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/utilities/timer.h"
#include "StarSystemSim/utilities/load_text_file.h"

#include "StarSystemSim/graphics/primitives/plane.h"
#include "StarSystemSim/graphics/primitives/cube.h"
#include "StarSystemSim/graphics/primitives/point_light.h"
#include "StarSystemSim/graphics/primitives/icosahedron.h"
#include "StarSystemSim/graphics/model.h"
#include "StarSystemSim/graphics/planet.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 900;

struct VertexData {
    glm::vec3 pos;
    glm::vec3 col;
    glm::vec3 normal;
};

utils::Timer timer;
graphics::Camera mainCamera(glm::vec3(0.0f, 0.5f, 3.0f));

glm::vec2 mousePos;
bool firstMouse = true;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 2);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "StarSystemSim", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowPos(window, 20, 60);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader basicShader;
    basicShader.compileShaders(utils::loadTextFile("shaders/basic.shader").c_str());
    basicShader.linkShaders();

    Shader lightingShader;
    lightingShader.compileShaders("shaders/lighting.shader", true);
    lightingShader.linkShaders();

    Shader lightSourceShader;
    lightSourceShader.compileShaders(utils::loadTextFile("shaders/light_source.shader").c_str());
    lightSourceShader.linkShaders();

    graphics::primitives::Cube::initVBO();
    graphics::primitives::PointLight::initVBO();

    graphics::primitives::Plane floor;
    floor.scale(glm::vec3(3.0f));

    glm::mat4 projMat4(1.0f), viewMat4(1.0f);
    //projMat4 = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, 0.0f, 100.0f);

    graphics::primitives::PointLight light1;
    light1.setPos(glm::vec3(1.2f, 4.0f, 2.0f));
    light1.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    light1.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    light1.attenuation = glm::vec3(1.0f, 0.001f, 0.00001f);
    graphics::primitives::PointLight light2;
    light2.setPos(glm::vec3(1.2f, 4.0f, 2.0f));
    light2.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    light2.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    light2.attenuation = glm::vec3(1.0f, 0.001f, 0.00001f);

    graphics::Model robot("robot");
    robot.translate(glm::vec3(1.0f, 0.0f, 0.0f));
    robot.scale(glm::vec3(0.1f));

    graphics::Planet earth("earth");
    earth.translate(glm::vec3(-1.0f, 1.0f, 0.0f));
    earth.scale(glm::vec3(0.5f));

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        timer.measureTime();
        processInput(window);

        projMat4 = glm::perspective(glm::radians(mainCamera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        viewMat4 = glm::lookAt(mainCamera.pos, mainCamera.pos + mainCamera.front, mainCamera.up);

        light1.setPos(glm::vec3(2.0f * sin(0.2f * glfwGetTime()), 1.0f, 2.0f * cos(0.2f * glfwGetTime())));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightSourceShader.use();
        lightSourceShader.setUniformMat4("_projMat", projMat4);
        lightSourceShader.setUniformMat4("_viewMat", viewMat4);
        lightSourceShader.unuse();

        basicShader.use();
        basicShader.setUniformMat4("_projMat", projMat4);
        basicShader.setUniformMat4("_viewMat", viewMat4);
        basicShader.unuse();

        graphics::primitives::PointLight::bindVAO();
        light1.draw(lightSourceShader);
        light2.draw(lightSourceShader);
        graphics::primitives::PointLight::unbindVAO();

        lightingShader.use();
        
        lightingShader.setUniform3f("_viewPos", mainCamera.pos);

        lightingShader.setUniform3f("_light1.pos", light1.getPos());
        lightingShader.setUniform3f("_light1.amb", light1.ambient);
        lightingShader.setUniform3f("_light1.diff", light1.diffuse);
        lightingShader.setUniform3f("_light1.spec", light1.specular);
        lightingShader.setUniform3f("_light1.attenuation", light1.attenuation);

        lightingShader.setUniform3f("_light2.pos", light2.getPos());
        lightingShader.setUniform3f("_light2.amb", light2.ambient);
        lightingShader.setUniform3f("_light2.diff", light2.diffuse);
        lightingShader.setUniform3f("_light2.spec", light2.specular);
        lightingShader.setUniform3f("_light2.attenuation", light2.attenuation);
        
        lightingShader.setUniformMat4("_projMat", projMat4);
        lightingShader.setUniformMat4("_viewMat", viewMat4);

        lightingShader.setUniform3f("_surfM.amb", glm::vec3(0.1f));
        lightingShader.setUniform3f("_surfM.diff", glm::vec3(1.0f));
        lightingShader.setUniform3f("_surfM.spec", glm::vec3(1.0f));
        lightingShader.setUniform1f("_surfM.shine", 32.0f);

        lightingShader.unuse();

        //floor.draw(basicShader);
        robot.draw(lightingShader);
        earth.draw(lightingShader);

        glfwSwapBuffers(window);
        glfwPollEvents();

        lightingShader.reload();
    }

    graphics::primitives::PointLight::destroyVBO();
    graphics::primitives::Cube::destroyVBO();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camSpeed = 2.0f;
    else
        camSpeed = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 camRight = glm::normalize(glm::cross(mainCamera.front, mainCamera.up));
        mainCamera.pos += glm::normalize(glm::cross(mainCamera.up, camRight)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 camRight = glm::normalize(glm::cross(mainCamera.front, mainCamera.up));
        mainCamera.pos -= glm::normalize(glm::cross(mainCamera.up, camRight)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        mainCamera.pos -= glm::normalize(glm::cross(mainCamera.front, mainCamera.up)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        mainCamera.pos += glm::normalize(glm::cross(mainCamera.front, mainCamera.up)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        mainCamera.pos += camSpeed * mainCamera.up * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        mainCamera.pos -= camSpeed * mainCamera.up * timer.deltaTime;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        mousePos = glm::vec2(xpos, ypos);
        firstMouse = false;
    }

    glm::vec2 offset = glm::vec2(xpos - mousePos.x, mousePos.y - ypos);
    mousePos = glm::vec2(xpos, ypos);

    float sensitivity = 0.1f;
    offset *= sensitivity;

    mainCamera.yaw += offset.x;
    mainCamera.pitch += offset.y;

    if (mainCamera.pitch > 89.0f)
        mainCamera.pitch = 89.0f;
    if (mainCamera.pitch < -89.0f)
        mainCamera.pitch = -89.0f;

    mainCamera.dir.x = cos(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.dir.y = sin(glm::radians(mainCamera.pitch));
    mainCamera.dir.z = sin(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.front = glm::normalize(mainCamera.dir);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    mainCamera.fov -= (float)yoffset;
    if (mainCamera.fov < 1.0f)
        mainCamera.fov = 1.0f;
    if (mainCamera.fov > 45.0f)
        mainCamera.fov = 45.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}