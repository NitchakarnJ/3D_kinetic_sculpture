// kinetic_wave_pro.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <string>

// ---------- Callbacks / helpers ----------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// ---------- Settings ----------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 3.0f, 12.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// UX toggles
bool g_autoCamRotate = true;   // 'R'
bool g_autoOrbitLight = true;  // 'O'
int  g_waveMode = 2;           // 1=radial, 2=multi-layer, 3=two-source

// Grid (ตั้ง spacing = 1.0 เพื่อให้ด้านข้าง “ชนเกือบพอดี”)
const int   GRID_X = 22;
const int   GRID_Z = 22;
const float SPACING = 1.0f;

int main() {
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Kinetic Sculpture — Pro", NULL, NULL);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to initialize GLAD\n"; return -1; }

    // GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Shaders
    Shader lightingShader("6.multiple_lights.vs", "6.multiple_lights.fs");
    Shader lightCubeShader("6.light_cube.vs", "6.light_cube.fs");

    // Cube vertex data (unit cube)
    float vertices[] = {
        // pos                // normal         // uv
        -0.5f,-0.5f,-0.5f,   0,0,-1,          0,0,
         0.5f,-0.5f,-0.5f,   0,0,-1,          1,0,
         0.5f, 0.5f,-0.5f,   0,0,-1,          1,1,
         0.5f, 0.5f,-0.5f,   0,0,-1,          1,1,
        -0.5f, 0.5f,-0.5f,   0,0,-1,          0,1,
        -0.5f,-0.5f,-0.5f,   0,0,-1,          0,0,

        -0.5f,-0.5f, 0.5f,   0,0, 1,          0,0,
         0.5f,-0.5f, 0.5f,   0,0, 1,          1,0,
         0.5f, 0.5f, 0.5f,   0,0, 1,          1,1,
         0.5f, 0.5f, 0.5f,   0,0, 1,          1,1,
        -0.5f, 0.5f, 0.5f,   0,0, 1,          0,1,
        -0.5f,-0.5f, 0.5f,   0,0, 1,          0,0,

        -0.5f, 0.5f, 0.5f,  -1,0,0,           1,0,
        -0.5f, 0.5f,-0.5f,  -1,0,0,           1,1,
        -0.5f,-0.5f,-0.5f,  -1,0,0,           0,1,
        -0.5f,-0.5f,-0.5f,  -1,0,0,           0,1,
        -0.5f,-0.5f, 0.5f,  -1,0,0,           0,0,
        -0.5f, 0.5f, 0.5f,  -1,0,0,           1,0,

         0.5f, 0.5f, 0.5f,   1,0,0,           1,0,
         0.5f, 0.5f,-0.5f,   1,0,0,           1,1,
         0.5f,-0.5f,-0.5f,   1,0,0,           0,1,
         0.5f,-0.5f,-0.5f,   1,0,0,           0,1,
         0.5f,-0.5f, 0.5f,   1,0,0,           0,0,
         0.5f, 0.5f, 0.5f,   1,0,0,           1,0,

        -0.5f,-0.5f,-0.5f,   0,-1,0,          0,1,
         0.5f,-0.5f,-0.5f,   0,-1,0,          1,1,
         0.5f,-0.5f, 0.5f,   0,-1,0,          1,0,
         0.5f,-0.5f, 0.5f,   0,-1,0,          1,0,
        -0.5f,-0.5f, 0.5f,   0,-1,0,          0,0,
        -0.5f,-0.5f,-0.5f,   0,-1,0,          0,1,

        -0.5f, 0.5f,-0.5f,   0, 1,0,          0,1,
         0.5f, 0.5f,-0.5f,   0, 1,0,          1,1,
         0.5f, 0.5f, 0.5f,   0, 1,0,          1,0,
         0.5f, 0.5f, 0.5f,   0, 1,0,          1,0,
        -0.5f, 0.5f, 0.5f,   0, 1,0,          0,0,
        -0.5f, 0.5f,-0.5f,   0, 1,0,          0,1
    };

    // Cube VAO/VBO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light cube VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Textures
    stbi_set_flip_vertically_on_load(true);
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/Metal049A_1K-PNG_Color.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/Metal049A_1K-PNG_Metalness.png").c_str());

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // Grid positions
    std::vector<glm::vec3> basePositions;
    basePositions.reserve(GRID_X * GRID_Z);
    for (int gx = 0; gx < GRID_X; ++gx) {
        for (int gz = 0; gz < GRID_Z; ++gz) {
            float x = (gx - (GRID_X - 1) * 0.5f) * SPACING;
            float z = (gz - (GRID_Z - 1) * 0.5f) * SPACING - 8.0f;
            basePositions.emplace_back(x, 0.0f, z);
        }
    }

    // Point lights (4 ดวง)
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),  // orbit
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f,-12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float t = (float)glfwGetTime();
        deltaTime = t - lastFrame;
        lastFrame = t;
        processInput(window);

        // Clear
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Animate moving light & color
        if (g_autoOrbitLight) {
            float R = 6.5f;
            pointLightPositions[0] = glm::vec3(sinf(t) * R, 1.6f + 0.6f * sinf(t * 0.7f), cosf(t) * R);
        }
        glm::vec3 colorCycle(
            sinf(t * 0.7f) * 0.5f + 0.5f,
            sinf(t * 1.3f + 2.0f) * 0.5f + 0.5f,
            sinf(t * 0.9f + 4.0f) * 0.5f + 0.5f
        );

        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // Dynamic shininess
        float shininess = 16.0f + 32.0f * (sinf(t * 2.0f) * 0.5f + 0.5f);
        lightingShader.setFloat("material.shininess", shininess);

        // Directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.03f, 0.03f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.35f, 0.35f, 0.45f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.6f);

        // Point lights
        for (int i = 0; i < 4; ++i) {
            std::string idx = "pointLights[" + std::to_string(i) + "]";
            lightingShader.setVec3((idx + ".position").c_str(), pointLightPositions[i]);
            if (i == 0) {
                // color cycling สำหรับดวงที่โคจร
                lightingShader.setVec3((idx + ".ambient").c_str(), colorCycle * 0.05f);
                lightingShader.setVec3((idx + ".diffuse").c_str(), colorCycle * 0.9f);
                lightingShader.setVec3((idx + ".specular").c_str(), colorCycle);
            }
            else {
                lightingShader.setVec3((idx + ".ambient").c_str(), 0.05f, 0.05f, 0.05f);
                lightingShader.setVec3((idx + ".diffuse").c_str(), 0.8f, 0.8f, 0.8f);
                lightingShader.setVec3((idx + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
            }
            lightingShader.setFloat((idx + ".constant").c_str(), 1.0f);
            lightingShader.setFloat((idx + ".linear").c_str(), 0.09f);
            lightingShader.setFloat((idx + ".quadratic").c_str(), 0.032f);
        }

        // Spotlight ติดกล้อง
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // View / Projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 120.0f);
        glm::mat4 view = camera.GetViewMatrix();
        if (g_autoCamRotate) {
            view = glm::rotate(view, 0.07f * sinf(t * 0.4f), glm::vec3(0, 1, 0));
        }
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // Bind textures
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, specularMap);

        // Draw kinetic “solid” grid
        glBindVertexArray(cubeVAO);

        // Wave params
        const float AMP = 0.6f;
        const float SPEED = 1.8f;
        const float K = 0.55f;
        const float epsilon = 0.995f; // กัน z-fighting (ชนเกือบพอดี)

        for (int i = 0; i < (int)basePositions.size(); ++i) {
            const glm::vec3& b = basePositions[i];

            // เลือกสูตรคลื่นตามโหมด
            float y = 0.0f;
            if (g_waveMode == 1) {
                // Radial ripple
                float r = glm::length(glm::vec2(b.x, b.z));
                y = AMP * sinf(K * r - SPEED * t);
            }
            else if (g_waveMode == 2) {
                // Multi-layer waves
                float r = glm::length(glm::vec2(b.x, b.z));
                float phase1 = 0.7f * sinf(0.6f * r - 1.8f * t);
                float phase2 = 0.5f * sinf(0.9f * b.x + 1.4f * t);
                float phase3 = 0.4f * sinf(1.3f * b.z - 2.2f * t);
                y = phase1 + phase2 + phase3;
            }
            else { // g_waveMode == 3
                // Two-source interference
                glm::vec2 s1(-6.0f, 0.0f), s2(6.0f, 0.0f);
                float d1 = glm::length(glm::vec2(b.x, b.z) - s1);
                float d2 = glm::length(glm::vec2(b.x, b.z) - s2);
                y = 0.7f * sinf(0.8f * d1 - 1.7f * t) + 0.7f * sinf(0.8f * d2 - 1.7f * t);
            }

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(b.x, y, b.z));
            model = glm::rotate(model, 0.35f * sinf(0.8f * y + t * 0.5f), glm::vec3(1.0f, 0.3f, 0.2f));
            model = glm::scale(model, glm::vec3(SPACING * epsilon, 1.0f, SPACING * epsilon)); // ขยายหน้าตัดให้เกือบเต็ม

            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ----- Base block (ทำให้ดูเป็นก้อนตันจริง ๆ) -----
        {
            float baseW = GRID_X * SPACING;
            float baseD = GRID_Z * SPACING;
            float baseH = 4.0f;  // ความหนาฐาน

            float centerX = 0.0f;
            float centerZ = -8.0f;

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(centerX, -baseH * 0.5f - 1.5f, centerZ));
            model = glm::scale(model, glm::vec3(baseW, baseH, baseD));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw light bulbs (optional visualizers)
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        for (int i = 0; i < 4; ++i) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// ---------- Input ----------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);

    // Toggles
    static bool rPrev = false, oPrev = false, k1Prev = false, k2Prev = false, k3Prev = false;
    bool rNow = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    bool oNow = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;
    bool k1Now = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    bool k2Now = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    bool k3Now = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;

    if (rNow && !rPrev) g_autoCamRotate = !g_autoCamRotate;
    if (oNow && !oPrev) g_autoOrbitLight = !g_autoOrbitLight;
    if (k1Now && !k1Prev) g_waveMode = 1;
    if (k2Now && !k2Prev) g_waveMode = 2;
    if (k3Now && !k3Prev) g_waveMode = 3;

    rPrev = rNow; oPrev = oNow; k1Prev = k1Now; k2Prev = k2Now; k3Prev = k3Now;
}

// ---------- Callbacks ----------
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xposIn, double yposIn) {
    float xpos = (float)xposIn, ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

// ---------- Texture loader ----------
unsigned int loadTexture(const char* path) {
    unsigned int texID; glGenTextures(1, &texID);
    int w, h, n;
    unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (data) {
        GLenum format = (n == 1) ? GL_RED : (n == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return texID;
}
