#include "camera.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ----- constants ------ //
const float CAMERA_SPEED = 0.05f;
const float CUBE_VERTICES[] = {
    -0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
    -0.05f,  0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,

    -0.05f, -0.05f,  0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,
    -0.05f, -0.05f,  0.05f,

    -0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,
    -0.05f, -0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,

     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,

    -0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f, -0.05f,  0.05f,
    -0.05f, -0.05f,  0.05f,
    -0.05f, -0.05f, -0.05f,

    -0.05f,  0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f, -0.05f
};

// ----- function declarations ----- //
static void draw(GLFWwindow* window, unsigned int shaderProgram, unsigned int VAO, unsigned int VBO, unsigned int EBO);
static void frameBufferResized(GLFWwindow* window, int width, int height);
static void initialize(GLFWwindow** window, unsigned int windowWidth, unsigned int windowHeight, unsigned int& shaderProgram, unsigned int& VAO, unsigned int& VBO, unsigned int& EBO);
static std::vector<glm::vec3> getPointsFromFile(const std::string& path);
static unsigned int initializeShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
static std::string readFromFile(const std::string& path);
static void update(GLFWwindow* window, Camera& cam, unsigned int shaderProgram);

// ----- function implementations ----- //
void draw(GLFWwindow* window, unsigned int shaderProgram, unsigned int VAO, unsigned int VBO, unsigned int EBO)
{
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // not really needed every time if the vertices aren't changing
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // required every time we draw
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    auto cubePositions = getPointsFromFile("C:/Users/hbruce/Desktop/dev-plan/CrossPlots/data.txt");

    for (const glm::vec3& pos : cubePositions)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
}

void frameBufferResized(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void initialize(GLFWwindow** window, unsigned int windowWidth, unsigned int windowHeight, unsigned int& shaderProgram, unsigned int& VAO, unsigned int& VBO, unsigned int& EBO)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(windowWidth, windowHeight, "CrossPlot", NULL, NULL);
    if (window == nullptr)
    {
        throw std::runtime_error(std::string("Failed to create window in MyWindow::MyWindow"));
    }

    glfwMakeContextCurrent(*window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        throw std::runtime_error(std::string("Glad loader failed in MyWindow::MyWindow"));
    }

    glViewport(0, 0, windowWidth, windowHeight);

    glfwSetFramebufferSizeCallback(*window, frameBufferResized);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    shaderProgram = initializeShaderProgram("C:/Users/hbruce/Desktop/dev-plan/CrossPlots/VertexShader.glsl", "C:/Users/hbruce/Desktop/dev-plan/CrossPlots/FragmentShader.glsl");
}

static std::vector<glm::vec3> getPointsFromFile(const std::string& path)
{
    std::vector<glm::vec3> positions;
    
    std::stringstream text;
    text << readFromFile(path);

    std::string token;
    while (text >> token)
    {
        float x = std::stof(token);
        text >> token;
        float y = std::stof(token);
        text >> token;
        float z = std::stof(token);

        positions.push_back(glm::vec3(x, y, z));
    }

    return positions;
}

unsigned int initializeShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    std::string vertexShaderSource = readFromFile(vertexShaderPath);
    const char* vSource = vertexShaderSource.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[2058];
        glGetShaderInfoLog(vertexShader, 2058, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        throw;
    }

    std::string fragementShaderSource = readFromFile(fragmentShaderPath);
    const char* fSource = fragementShaderSource.c_str();

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[2058];
        glGetShaderInfoLog(fragmentShader, 2058, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(fragmentShader);
        throw;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[2058];
        glGetProgramInfoLog(shaderProgram, 2058, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

std::string readFromFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

void update(GLFWwindow* window, Camera& cam, unsigned int shaderProgram)
{
    glUseProgram(shaderProgram);

    // should maybe be a handle input events function
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam.pos += CAMERA_SPEED * cam.front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.pos -= CAMERA_SPEED * cam.front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.pos -= glm::normalize(glm::cross(cam.front, cam.up)) * CAMERA_SPEED;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.pos += glm::normalize(glm::cross(cam.front, cam.up)) * CAMERA_SPEED;

        glm::mat4 view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    int vertexColorLocation = glGetUniformLocation(shaderProgram, "inputColor");
    glUniform4f(vertexColorLocation, 0.0f, 0.75f, 0.0f, 1.0f);
}

int main(int argc, char* argv)
{
    GLFWwindow* window = nullptr;
    int winWidth = 1600;
    int winHeight = 1200;

    unsigned int shaderProgram;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    Camera cam;
    cam.pos = glm::vec3(0.0f, 0.0f, 3.0f);
    cam.front = glm::vec3(0.0f, 0.0f, -1.0f);
    cam.up = glm::vec3(0.0f, 1.0f, 0.0f);

    initialize(&window, winWidth, winHeight, shaderProgram, VAO, VBO, EBO);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.5f, 0.0f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update(window, cam, shaderProgram);
        draw(window, shaderProgram, VAO, VBO, EBO);
    }

    return 0;
}
  