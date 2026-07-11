#include <KHR\khrplatform.h>
#include <glad\glad.h>
#include <glfw3.h>
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/perpendicular.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <array>

#include "..\shaders\Shader.h"
#include "Camera3.h"
#include "..\shaders\ComputeShader.h"
#include "..\SimplexNoise.h"
#include "..\SimplexNoise.cpp" // unnecessary?

#define TAU 6.28318531
#define PI 3.141592654
#define E 2.7182818285

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

const int numCubeVertices = 108;

float yaw = 0.0;
float pitch = 0.0;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
Camera3D camera = Camera3D(glm::vec3(0.0f, 0.0f, 3.0f), 0.005f);

float dt = 0.01;

struct Body
{
    glm::vec3 pos;
    glm::vec3 vel;
    float mass;

    void update(Body other)
    {
        glm::vec3 dif = other.pos - pos;
        float dist2 = glm::dot(dif, dif);

        glm::vec3 accel = glm::normalize(dif) * (other.mass / dist2);

        this->vel += accel * dt;
        this->pos += this->vel * dt;
    }
};

struct Vertex 
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct PositionScaled
{
    glm::vec3 pos;
    float scale;
};

struct Mesh
{
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
};

struct InstancedMesh2
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint instanceVBO;

    GLsizei vertexCount;
};

struct Mesh2
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    GLsizei vertexCount;
    GLsizei indexCount;
};

class FPSHandler
{
    public:
        float fps;
        float startTime;
        float endTime;
        int frames;

        FPSHandler();

        void fpsCheck()
        {
            if (endTime - startTime > 1.0)
            {
                fps = frames / (endTime - startTime);
                startTime = glfwGetTime();
                frames = 0;

                std::cout << "FPS: " << fps << "\r" << std::flush;
            }
        }
};

FPSHandler::FPSHandler()
{
    float fps = 0.0;
    float startTime = glfwGetTime();
    float endTime = 0.0;
    int frames = 0;
}

class UniformHandler
{
    public:
        struct unfloat
        {
            float value;
            GLint loc;
        };
        struct unmat4
        {
            glm::mat4 value;
            GLint loc;
        };
        struct unvec2
        {
            glm::vec2 value;
            GLint loc;
        };
        struct unwindow
        {
            GLFWwindow* window;
            GLint sizeLoc;
        };

        std::vector<unfloat> floatList;
        std::vector<unmat4> mat4List;
        std::vector<unvec2> vec2List;
        unwindow window;

        unmat4 viewMatrix;
        unmat4 projMatrix;
        unmat4 modelMatrix;
        unmat4 translationMatrix;

        UniformHandler(const Shader& shaderProgram) : shader(shaderProgram)
        {
        }

        void addfloat(const float& x, const char* name)
        {
            GLint uniformLoc = glGetUniformLocation(shader.ID, name);
            if (uniformLoc == -1)
                throw std::invalid_argument("Invalid uniform name");
            floatList.push_back(unfloat{x, uniformLoc});
        }
        void addmat4(const glm::mat4& x, const char* name)
        {
            GLint uniformLoc = glGetUniformLocation(shader.ID, name);
            if (uniformLoc == -1)
                throw std::invalid_argument("Invalid uniform name");
            mat4List.push_back(unmat4{x, uniformLoc});
        }
        GLint getUniformLoc(const char* name)
        {
            GLint uniformLoc = glGetUniformLocation(shader.ID, name);
            if (uniformLoc == -1)
                throw std::invalid_argument("Invalid uniform name");
            return uniformLoc;
        }
        void addvec2(const glm::vec2& x, const char* name)
        {
            GLint uniformLoc = glGetUniformLocation(shader.ID, name);
            if (uniformLoc == -1)
                throw std::invalid_argument("Invalid uniform name");
            vec2List.push_back(unvec2{x, uniformLoc});
        }
        void addWindowSize(GLFWwindow* window, const char* name)
        {
            GLint uniformLoc = glGetUniformLocation(shader.ID, name);
            if (uniformLoc == -1)
                throw std::invalid_argument("Invalid uniform name");
            this->window.sizeLoc = uniformLoc;
        }
        void add3DMatrices(glm::mat4& view, glm::mat4& proj, glm::mat4& model)
        {
            viewMatrix = {view, getUniformLoc("view")};
            projMatrix = {proj, getUniformLoc("proj")};
            modelMatrix = {model, getUniformLoc("model")};
        }



        void update3DMatrices(glm::mat4& view, glm::mat4& proj, glm::mat4& model)
        {
            viewMatrix.value = view;
            projMatrix.value = proj;
            modelMatrix.value = model;
        }
        void updateWindowSize(GLFWwindow* window)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            // weird behavior if window.sizeLoc is undefined
            // the location will point to just random memory
            glUniform2f(this->window.sizeLoc, width, height);
        }
        void updateUniforms()
        {
            for (unmat4 x : mat4List)
                glUniformMatrix4fv(x.loc, 1, GL_FALSE, glm::value_ptr(x.value));
            for (unfloat x : floatList)
                glUniform1f(x.loc, x.value);
            for (unvec2 x : vec2List)
                glUniform2f(x.loc, x.value.x, x.value.y);

            glUniformMatrix4fv(viewMatrix.loc, 1, GL_FALSE, glm::value_ptr(viewMatrix.value));
            glUniformMatrix4fv(projMatrix.loc, 1, GL_FALSE, glm::value_ptr(projMatrix.value));
            glUniformMatrix4fv(modelMatrix.loc, 1, GL_FALSE, glm::value_ptr(modelMatrix.value));
        }

    private:
        Shader shader;
};

std::vector<Vertex> getCircleVert(float radius, glm::vec3 pos, glm::vec3 color, float da)
{
    std::vector<Vertex> vertices;

    vertices.push_back({
        pos,
        color
    });

    for (float i = 0.0; i < TAU; i += da)
    {
        float x = radius * cos(i);
        float y = radius * sin(i);

        vertices.push_back(
            {glm::vec3(x + pos.x, y + pos.y, 0.0f), color}
        );

        if (i + da > TAU)
        {
            vertices.push_back(
                {glm::vec3(radius + pos.x, 0.0f, 0.0f), color}
            );
        }
    }
    return vertices;
}

GLFWwindow* initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    return window;
}

Mesh2 bufferTriangle2(std::vector<Vertex> vertices, std::vector<unsigned int> indices, int buffer)
{
    Mesh2 mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();
    return mesh;
}

void updateUniforms(GLFWwindow* window, GLuint& windowSizeLoc)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glUniform2f(windowSizeLoc, width, height);
}

void updateUniformMatrices(Shader& ourShader, 
    glm::mat4& model, glm::mat4& view, glm::mat4& proj,
    GLuint& modelLoc, GLuint& viewLoc, GLuint& projLoc
)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

// https://gamedev.stackexchange.com/questions/179426/c-generate-random-float-values-between-a-range
class RandomNumberGenerator
{
    std::random_device m_randomDevice{};
    std::mt19937 m_engine{m_randomDevice()};

    public:
        // Generates a random float in the range [low, high)
        float Generate(float low, float high)
        {
            return std::uniform_real_distribution<float>{low, high}(m_engine);
        }
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.setCameraFront(glm::normalize(direction));
}

float lerp(float v0, float v1, float t) 
{
  return v0 + t * (v1 - v0);
}

Mesh bufferTriangle(std::vector<Vertex> vertices, int buffer)
{
    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mesh.vertexCount = vertices.size();
    return mesh;
}

std::vector<Vertex> vertToVectors(std::vector<float> vertices, glm::vec3 color)
{
    std::vector<Vertex> vectors;
    for (int i = 0; i < numCubeVertices; i+=3)
    {
        vectors.push_back({
            glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]),
            color
        });
    }

    return vectors;
}

std::vector<float> getCubeVert()
{
    std::vector<float> vec = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    return vec;
}

std::vector<float> getPrismVert(float height)
{
    // bottom of the prism is at 0.0
    std::vector<float> vec = {
        -1.0f,  height, -1.0f,
        -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f,  height, -1.0f,
        -1.0f,  height, -1.0f,

        -1.0f, 0.0f,  1.0f,
        -1.0f, 0.0f, -1.0f,
        -1.0f,  height, -1.0f,
        -1.0f,  height, -1.0f,
        -1.0f,  height,  1.0f,
        -1.0f, 0.0f,  1.0f,

        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f,  1.0f,
        1.0f,  height,  1.0f,
        1.0f,  height,  1.0f,
        1.0f,  height, -1.0f,
        1.0f, 0.0f, -1.0f,

        -1.0f, 0.0f,  1.0f,
        -1.0f,  height,  1.0f,
        1.0f,  height,  1.0f,
        1.0f,  height,  1.0f,
        1.0f, 0.0f,  1.0f,
        -1.0f, 0.0f,  1.0f,

        -1.0f,  height, -1.0f,
        1.0f,  height, -1.0f,
        1.0f,  height,  1.0f,
        1.0f,  height,  1.0f,
        -1.0f,  height,  1.0f,
        -1.0f,  height, -1.0f,

        -1.0f, 0.0f, -1.0f,
        -1.0f, 0.0f,  1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        -1.0f, 0.0f,  1.0f,
        1.0f, 0.0f,  1.0f
    };
    return vec;
}

float sumOctavesNoise(int maxIterations, float x, float y, float persistence, float scale, float max, float min)
{
    float maxAmp = 0.0;
    float amp = 1.0;
    float frequency = scale;
    float noise = 0.0;

    for (int i = 0; i < maxIterations; i++)
    {
        noise += SimplexNoise::noise(x * frequency, y * frequency) * amp;
        maxAmp += amp;
        amp *= persistence;
        frequency *= 2.0;
    }

    return (noise / maxAmp) * (max - min) / 2.0 + (max + min) / 2.0;
;
}

std::vector<Vertex> getMapVert(int columns, int rows, glm::vec3 color, int noiseLayers)
{
    SimplexNoise noiser;
    std::vector<Vertex> vertices;
    float invCols = 2.0 / columns;
    float invRows = 2.0 / rows;

    const float scale = 2.0;
    const float heightFrac = 0.25;
    const int maxIterations = 32;

    for (float x = -1.0; x < 1.0; x += invCols)
    {
        for (float y = -1.0; y < 1.0; y += invRows)
        {
            float newX = x + invCols;
            float newY = y + invRows;

            vertices.push_back({glm::vec3(x, 0.0, y), color});
            vertices.push_back({glm::vec3(x, 0.0, newY), color});
            vertices.push_back({glm::vec3(newX, 0.0, newY), color});

            vertices.push_back({glm::vec3(newX, 0.0, newY), color});
            vertices.push_back({glm::vec3(x, 0.0, y), color});
            vertices.push_back({glm::vec3(newX, 0.0, y), color});
        }
    }

    for (int i = 0; i < vertices.size(); i++)
    {
        // float noiseResult = sumOctavesNoise(maxIterations, vertices[i].pos.x, vertices[i].pos.z, 0.5, scale, 1, 0);
        float noiseResult = ((noiser.fractal(maxIterations, vertices[i].pos.x * scale, vertices[i].pos.z * scale) + 1.0) / 2.0) * heightFrac;

        vertices[i].pos.y += noiseResult;
        vertices[i].color += glm::vec3(noiseResult);
    }

    return vertices;
}

std::vector<Vertex> multiplyVertices(std::vector<Vertex>& vertices0, float x)
{
    for (Vertex& v : vertices0)
        v.pos *= x;
    return vertices0;
}

std::pair<std::vector<Vertex>, std::vector<unsigned int>> getSphereVert(float radius, int sides, int strips, glm::vec3 color)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= strips; i++)
    {
        float V = ((float) i) / ((float) strips);
        float phi = V * PI;

        for (int u = 0; u <= sides; u++)
        {
            float U = ((float) u) / ((float) sides);
            float theta = U * (PI * 2.0);

            float x = (radius * cos(theta) * sin(phi));
            float y = (radius * cos(phi));
            float z = (radius * sin(theta) * sin(phi));

            vertices.push_back({glm::vec3(x, y, z), color});
        }
    }

    for (int i = 0; i < strips; i++)
    {
        for (int u = 0; u < sides; u++)
        {
            int v0 = i * (sides + 1) + u;
            int v1 = v0 + 1;
            int v2 = (i + 1) * (sides + 1) + u;
            int v3 = v2 + 1;

            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);

            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);
        }
    }

    return std::make_pair(vertices, indices);
}

Mesh2 bufferTrianglesIndexed(std::vector<Vertex> vertices, std::vector<unsigned int> indices, int buffer)
{
    Mesh2 mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();
    return mesh;
}

void updateTranslationMatrix(glm::vec3& pos, GLuint& matrixLoc)
{
    glm::mat4 m = glm::mat4(1.0);
    m = glm::translate(m, pos);
    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(m));
}

void updateTranslationMatrix(GLuint& matrixLoc)
{
    glm::mat4 m = glm::mat4(1.0);
    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(m));
}

void updateMatrix(glm::vec3& matrix, GLuint& matrixLoc)
{
    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

glm::vec3 getGravitationalAccel(const Body& b0, const Body& b1)
{
    glm::vec3 dif = b1.pos - b0.pos;
    float dist2 = glm::dot(dif, dif);
    return glm::normalize(dif) * (b1.mass / dist2);
} 

struct Derivative
{
    glm::vec3 dPos;
    glm::vec3 dVel;
};

Derivative evaluateDerivative(const Body& b0, const Body& b1)
{
    Derivative d;
    d.dPos = b0.vel;
    d.dVel = getGravitationalAccel(b0, b1);
    return d;
}

void rk4(Body& b0, const Body& b1)
{
    Derivative k1 = evaluateDerivative(b0, b1);

    Body b = b0;
    b.pos += k1.dPos * (dt * 0.5f);
    b.vel += k1.dVel * (dt * 0.5f);

    Derivative k2 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k2.dPos * (dt * 0.5f);
    b.vel += k2.dVel * (dt * 0.5f);

    Derivative k3 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k3.dPos * dt;
    b.vel += k3.dVel * dt;

    Derivative k4 = evaluateDerivative(b, b1);


    b0.pos += ((dt / 6.0f) * (k1.dPos + 2.0f * k2.dPos + 2.0f * k3.dPos + k4.dPos));
    b0.vel += ((dt / 6.0f) * (k1.dVel + 2.0f * k2.dVel + 2.0f * k3.dVel + k4.dVel));
}

struct EnergyRecorder
{
    double terminationTime;
    float energy;
    std::ofstream energyRecords;
    bool record;

    EnergyRecorder(double endTime, const char* filename, char* argv[])
    {
        terminationTime = endTime;
        record = std::atof(argv[1]) == 1.0;
        if (record)
        {
            energyRecords.open(filename, std::ofstream::out | std::ofstream::trunc);
            energyRecords << "dt value: " << dt << "\n";
        }
    }

    EnergyRecorder()
    {
        record = false;
    }

    void run(GLFWwindow* window, float& curTime, Body& earth, Body& moon)
    {
        if (record)
        {
            if (curTime > terminationTime)
                glfwSetWindowShouldClose(window, 1);
            else
            {
                energy = (0.5 * moon.mass * glm::dot(moon.vel, moon.vel)) + 
                    (moon.mass * -glm::length(getGravitationalAccel(moon, earth)) * glm::length(moon.pos));
                energyRecords << -energy << "\n";
            }
        }
    }
};

class OrbitTracker
{
    public:
    Body primaryBody;
    Body secondaryBody;
    glm::vec3 apoapsis;
    glm::vec3 periapsis;
    float eccentricity;
    float sma;
    float trueAnomaly;
    float decimalMultiplier;

    OrbitTracker(Body& primaryBody, Body& secondaryBody, int decimals)
    {
        this->primaryBody = primaryBody;
        this->secondaryBody = secondaryBody;

        apoapsis = primaryBody.pos;
        periapsis = primaryBody.pos;
        eccentricity = (glm::length(apoapsis) - glm::length(periapsis)) /
            (glm::length(apoapsis) + glm::length(periapsis));
        sma = (glm::length(periapsis) + glm::length(apoapsis)) / 2.0;
        trueAnomaly = 0.0;

        decimalMultiplier = pow(10, decimals);
    }

    void track(Body& primaryBody, Body& secondaryBody)
    {
        this->primaryBody = primaryBody;
        this->secondaryBody = secondaryBody;
        
        float mag = glm::length(primaryBody.pos);
        float roundedMagnitude = ((float)((int)(mag * decimalMultiplier))) / decimalMultiplier;
        if (roundedMagnitude > glm::length(apoapsis))
        {
            apoapsis = roundedMagnitude * (primaryBody.pos / mag);
            // std::cout << "new apoapsis at " << glm::length(apoapsis) << " u\n";
        }
        if (roundedMagnitude < glm::length(periapsis))
        {
            periapsis = roundedMagnitude * (primaryBody.pos / mag);
            // std::cout << "new periapsis at " << glm::length(periapsis) << " u\n"; 
        }

        eccentricity = (glm::length(apoapsis) - glm::length(periapsis)) /
            (glm::length(apoapsis) + glm::length(periapsis));
        sma = (glm::length(periapsis) + glm::length(apoapsis)) / 2.0;
    }
};

template <size_t N>
InstancedMesh2 bufferInstancedBodies(
    std::vector<Vertex>& sphereVert, 
    std::vector<unsigned int>& sphereIndices, 
    std::array<Body, N>& bodies
)
{
    std::array<PositionScaled, bodies.size()> translations;
    
    for (int i = 0; i < translations.size(); i++)
        translations[i] = PositionScaled{bodies[i].pos, bodies[i].mass};

    InstancedMesh2 mesh;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glGenBuffers(1, &mesh.instanceVBO);

    glBindVertexArray(mesh.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVert.size() * sizeof(Vertex), sphereVert.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVBO);    
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(PositionScaled), translations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PositionScaled), (void*)offsetof(PositionScaled, pos));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(PositionScaled), (void*)offsetof(PositionScaled, scale));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    mesh.vertexCount = sphereVert.size();

    return mesh;
}

struct RotVec
{
    glm::vec3 axis;
    float angle;

    RotVec(glm::vec3 axis, float angle)
    {
        this->axis = axis;
        this->angle = angle;
    }

    void apply(glm::vec3& vec)
    {
        glm::vec3 p = glm::cross(axis, vec);
        vec = vec + (sin(angle) * (p)) + ((1.0f - cos(angle)) * glm::cross(axis, p));
    }
};

int main(int argc, char* argv[])
{
    GLFWwindow* window = initWindow();
    Shader mainShader("shaders/vshader.glsl", "shaders/fshader.glsl");
    UniformHandler uniformer(mainShader);
    uniformer.addWindowSize(window, "windowSize");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    proj = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));

    uniformer.add3DMatrices(view, proj, model);

    glm::vec2 windowSize = glm::vec2(SCR_WIDTH, SCR_HEIGHT);
    // RandomNumberGenerator rng = RandomNumberGenerator();

    FPSHandler fpsCounter = FPSHandler();

    GLuint translationLoc = glGetUniformLocation(mainShader.ID, "translation");

    std::pair<std::vector<Vertex>, std::vector<unsigned int>> t1 = getSphereVert(0.2, 10, 10, glm::vec3(0.6, 0.6, 0.6));
    std::vector<Vertex> moonVert = t1.first;

    std::array<Body, 11> bodies = {
        Body{glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), 1.0}, // main body, no touchy

        Body{glm::vec3(2.0, 0.0, 0.0), glm::vec3(0.0, 0.5, -0.5), 0.5},
        Body{glm::vec3(1.61803, 0.0, 1.17557), glm::vec3(0.29389, 0.5, -0.40451), 0.5},
        Body{glm::vec3(0.61803, 0.0, 1.90211), glm::vec3(0.47553, 0.5, -0.15451), 0.5},
        Body{glm::vec3(-0.61803, 0.0, 1.90211), glm::vec3(0.47553, 0.5, 0.15451), 0.5},
        Body{glm::vec3(-1.61803, 0.0, 1.17557), glm::vec3(0.29389, 0.5, 0.40451), 0.5},
        Body{glm::vec3(-2.0, 0.0, -0.0), glm::vec3(0.0, 0.5, 0.5), 0.5},
        Body{glm::vec3(-1.61803, 0.0, -1.17557), glm::vec3(-0.29389, 0.5, 0.40451), 0.5},
        Body{glm::vec3(-0.61803, 0.0, -1.90211), glm::vec3(-0.47553, 0.5, 0.15451), 0.5},
        Body{glm::vec3(0.61803, 0.0, -1.90211), glm::vec3(-0.47553, 0.5, -0.15451), 0.5},
        Body{glm::vec3(1.61803, 0.0, -1.17557), glm::vec3(-0.29389, 0.5, -0.40451), 0.5}
    };
    InstancedMesh2 bodyMesh = bufferInstancedBodies(t1.first, t1.second, bodies);

    glm::mat4 translationMatrix = glm::mat4(1.0);

    EnergyRecorder records;
    if (argc == 3 && argv[2] != nullptr)
        records = EnergyRecorder(std::atof(argv[2]), "EnergyRecords.txt", argv);

    // OrbitTracker moonOrbit(moon, earth, 2);

    float t = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        // records.run(window, t, earth, moon);
        // moonOrbit.track(moon, earth);

        fpsCounter.frames += 1;
        fpsCounter.fpsCheck();
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader.use();

        t = glfwGetTime();
        // rk4(moon, earth);
        // rk4(randomThing, earth);
        for (int i = 0; i < bodies.size(); i++)
        {
            if (i > 0) rk4(bodies[i], bodies[0]);
        }

        // float incl = acos(moon.vel.y / glm::length(glm::vec2(moon.vel.x, moon.vel.y))) * 180.0;
        // std::cout << "Inclination: " << ((360.0 - incl < 180.0) ? 360.0 - incl : incl) << std::endl;

        std::array<PositionScaled, bodies.size()> positions;
        for (int i = 0; i < positions.size(); i++)
        {
            if (i == 0) 
                positions[i] = PositionScaled{bodies[i].pos, bodies[i].mass * 3.0f};
            else 
                positions[i] = PositionScaled{bodies[i].pos, bodies[i].mass};
        }

        glBindBuffer(GL_ARRAY_BUFFER, bodyMesh.instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(PositionScaled), positions.data());
        glBindVertexArray(bodyMesh.VAO);
        glDrawElementsInstanced(
            GL_TRIANGLES, static_cast<GLsizei>(t1.second.size()), 
            GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(positions.size())
        );

        camera.doCameraMovement(window);
        camera.updateView(view);

        uniformer.updateUniforms();
        uniformer.updateWindowSize(window);
        uniformer.update3DMatrices(view, proj, model);

        glfwSwapBuffers(window);
        glfwPollEvents();

        fpsCounter.endTime = glfwGetTime();
    }

    return 0;
}