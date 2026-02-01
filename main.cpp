#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "core/mesh.h"
#include "core/assimpLoader.h"
#include "core/texture.h"

//#define MAC_CLION
#define VSTUDIO

#ifdef MAC_CLION
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

#ifdef VSTUDIO
// Note: install imgui with:
//     ./vcpkg.exe install imgui[glfw-binding,opengl3-binding]
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

float deltaTime = 0.0f;


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

int g_width = 800;
int g_height = 600;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraDirection;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraDirection;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraDirection, cameraUp)) * cameraSpeed;
}
//the camera stuff is min

void framebufferSizeCallback(GLFWwindow *window,
                             int width, int height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

std::string readFileToString(const std::string &filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        printf("Could not open file: %s\n", filePath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

GLuint generateShader(const std::string &shaderPath, GLuint shaderType) {
    printf("Loading shader: %s\n", shaderPath.c_str());
    const std::string shaderText = readFileToString(shaderPath);
    const GLuint shader = glCreateShader(shaderType);
    const char *s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Error! Shader issue [%s]: %s\n", shaderPath.c_str(), infoLog);
    }
    return shader;
}



int main() {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(g_width, g_height, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    //Setup platforms
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set up frame buffer

    unsigned int framebuffer;    // create buffer2
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColorbuffer;    //create texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_width,g_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //texture details

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,textureColorbuffer, 0);

    GLuint RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, g_width, g_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //end frame buffer

    //SET UP PLANE FOR THE FINAL RENDER

    float screenQuadVertices[] = {
        //positions     //uv
        -1.0f,  1.0f, 0.0f,  0.0f,1.0f, //tl
        -1.0f, -1.0f, 0.0f,  0.0f,0.0f, //bl
        1.0f, -1.0f, 0.0f,  1.0f,0.0f,   //br

        -1.0f,  1.0f, 0.0f,  0.0f,1.0f,  //tl
        1.0f,  -1.0f, 0.0f,  1.0f,0.0f,   //br
        1.0f,  1.0f, 0.0f,  1.0f,1.0f,   //tr
    };
    GLuint screenVAO = 0;
    GLuint screenVBO;
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);



    const GLuint modelVertexShader = generateShader("shaders/modelVertex.vs", GL_VERTEX_SHADER);
    const GLuint fragmentShader = generateShader("shaders/fragment.fs", GL_FRAGMENT_SHADER);
    const GLuint textureShader = generateShader("shaders/texture.fs", GL_FRAGMENT_SHADER);
///ADS model shaders generatiion
    const GLuint adsVertexShader = generateShader("shaders/adsVertex.vs", GL_VERTEX_SHADER);
    const GLuint adsFragmentShader = generateShader("shaders/adsFragment.fs", GL_FRAGMENT_SHADER);

    const GLuint planeVertexShader = generateShader("shaders/screen.vs", GL_VERTEX_SHADER);
    const GLuint planeFragmentShader = generateShader("shaders/screen.fs", GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];
    const unsigned int modelShaderProgram = glCreateProgram();
    glAttachShader(modelShaderProgram, modelVertexShader);
    glAttachShader(modelShaderProgram, fragmentShader);
    glLinkProgram(modelShaderProgram);
    glGetProgramiv(modelShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(modelShaderProgram, 512, NULL, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }


    /// Ads model program creation
    const unsigned int adsShaderProgram = glCreateProgram();
    glAttachShader(adsShaderProgram, adsVertexShader);
    glAttachShader(adsShaderProgram, adsFragmentShader);
    glLinkProgram(adsShaderProgram);
    glGetProgramiv(adsShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(adsShaderProgram, 512, NULL, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    const unsigned int textureShaderProgram = glCreateProgram();
    glAttachShader(textureShaderProgram, modelVertexShader);
    glAttachShader(textureShaderProgram, textureShader);
    glLinkProgram(textureShaderProgram);
    glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(textureShaderProgram, 512, NULL, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    const unsigned int planeShaderProgram = glCreateProgram();
    glAttachShader(planeShaderProgram, planeVertexShader);
    glAttachShader(planeShaderProgram, planeFragmentShader);
    glLinkProgram(planeShaderProgram);
    glGetProgramiv(planeShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(planeShaderProgram, 512, NULL, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }


    glDeleteShader(modelVertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);
    ///ads shader deletition
    glDeleteShader(adsVertexShader);
    glDeleteShader(adsFragmentShader);

    glDeleteShader(planeVertexShader);
    glDeleteShader(planeFragmentShader);

    core::Mesh quad = core::Mesh::generateQuad();
    core::Model quadModel({quad});
    quadModel.translate(glm::vec3(0,0,-2.5));
    quadModel.scale(glm::vec3(5, 5, 1));

    core::Model BdayKitty = core::AssimpLoader::loadModel("models/BdayKitty.fbx");
    core::Model sphere = core::AssimpLoader::loadModel("models/new_sphere.fbx");
    core::Texture cmgtGatoTexture("textures/CMGaTo_crop.png"); //adding a texture like writing a variable
    core::Texture cmgtGingerTexture("textures/GingerTexture.png");
    core::Texture CoubleFoundationTexture("textures/CoubleFoundation_1024x1024.png");
    BdayKitty.id = cmgtGingerTexture.getId();
    sphere.id = CoubleFoundationTexture.getId();
    BdayKitty.is_in_scene1 = true;

    sphere.is_in_scene1 = false;



    //scene switching
    std::vector<core::Model*> models; //made pointer
    models.push_back(&BdayKitty);   //&adress
    std::vector<core::Model*> scene1;
    std::vector<core::Model*> scene2;
    scene1.push_back(&sphere);
    scene2.push_back(&BdayKitty);
    scene2.push_back(&sphere);
    std::vector<core::Model*> currentScene;
    currentScene = scene1;



    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.r,
                 clearColor.g, clearColor.b, clearColor.a);



    GLint mvpMatrixUniform = glGetUniformLocation(modelShaderProgram, "mvpMatrix");
    GLint textureModelUniform = glGetUniformLocation(textureShaderProgram, "mvpMatrix");
    GLint textureUniform = glGetUniformLocation(textureShaderProgram, "text");
    ///ads model uniforms
    GLint adsMvpMatrixUniform = glGetUniformLocation(adsShaderProgram, "mvpMatrix");
    GLint adsMMatrixUniform = glGetUniformLocation(adsShaderProgram, "mMatrix");

    GLint adsAmbientLightIntensityUniform = glGetUniformLocation(textureShaderProgram, "ambientLightIntensity");
    GLint adsAmbientLightColorUniform = glGetUniformLocation(adsShaderProgram, "ambientLightColor");

    GLint adsLightDirectionUniform = glGetUniformLocation(adsShaderProgram, "lightDirection");
    GLint adsLightColorUniform = glGetUniformLocation(adsShaderProgram, "lightColor");

    GLint lightPositionUniform = glGetUniformLocation(adsShaderProgram, "lightPosition");
    GLint cameraPositionUniform = glGetUniformLocation(adsShaderProgram, "cameraPosition");

    GLint adsUvGridTexUniform = glGetUniformLocation(adsShaderProgram, "uvGridTexture");

    GLint screenTextureUniform = glGetUniformLocation(planeShaderProgram, "textureUniform");

    GLint frameBufferEffects = glGetUniformLocation(planeShaderProgram,"frameBufferEffect");

    double currentTime = glfwGetTime();
    double finishFrameTime = 0.0;

    float rotationStrength = 100.0f;
    float x_distance = 100.0f;
    bool framebuffer_is_active = true;
    float frameBufferEffect = 0;

    bool menu = true;

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Raw Engine v2");
        ImGui::Text("Hello :)");
        ImGui::End();
        ImGui::SliderFloat("X LightPosition", &x_distance, -5.0f, 5.0f);

        if (ImGui::Button("Change Framebuffer")) {
            framebuffer_is_active = !framebuffer_is_active;
            if (framebuffer_is_active) {
                frameBufferEffect = 0;

            }
            else if (!framebuffer_is_active){ frameBufferEffect = 1; }
            // else  (!!framebuffer_is_active) { frameBufferEffect = 2; }
        }

        processInput(window);
        BdayKitty.rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rotationStrength) * static_cast<float>(deltaTime));
        sphere.translate(glm::vec3(0.0f,(sin(glfwGetTime()) * 0.03f), 0.0f));



        //VP
        const float radius = 10.0f;
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;
        // glm::mat4 view;
        // view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 view = glm::lookAt(cameraPos,cameraTarget, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(textureShaderProgram);
        glUniformMatrix4fv(textureModelUniform, 1, GL_FALSE, glm::value_ptr(projection * view * quadModel.getModelMatrix()));
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(textureUniform, 0);
        glBindTexture(GL_TEXTURE_2D, cmgtGatoTexture.getId());
        quadModel.render();
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        menu = true;

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            //menu = true;
            currentScene=scene1;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            //menu = false;
            currentScene=scene2;
        }


        // if (menu == true) {
        for (int i = 0; i < currentScene.size(); i++) {
            core::Model *model = currentScene[i];
            // if (model->is_in_scene1 == menu) {
                // ///ADS model light parameters
                float ambientLightIntensity = 1.0f;
                glm::vec3 ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

                glm::vec3 LightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
                glm::vec3 LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
                glm::vec3 lightPosition = glm::vec3(5.0f, 5.0f, 5.0f);
                // x.distance = 50.0f;

                glUseProgram(adsShaderProgram);
                glUniformMatrix4fv(adsMvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection * view * model->getModelMatrix()));
                glUniformMatrix4fv(adsMMatrixUniform, 1, GL_FALSE, glm::value_ptr(model->getModelMatrix()));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, model->id);

                glUniform1i(adsUvGridTexUniform,0);


                glUniform1f(adsAmbientLightIntensityUniform, ambientLightIntensity);
                glUniform3f(adsAmbientLightColorUniform, ambientLightColor.x, ambientLightColor.y, ambientLightColor.z);
                glUniform3f(adsLightDirectionUniform, LightDirection.x, LightDirection.y, LightDirection.z);
                glUniform3f(adsLightColorUniform, LightColor.x, LightColor.y, LightColor.z);
                glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
                glUniform3f(cameraPositionUniform, cameraPos.x, cameraPos.y, cameraPos.z);


                model->render();
            //}

            //render frame to show final image



        }


        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(planeShaderProgram);
        glBindVertexArray(screenVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(textureUniform, 0);
        glUniform1f(frameBufferEffects, frameBufferEffect);
        ///to load the gato
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES,0 ,6);
        glBindVertexArray(0);


        //ADS model light


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        finishFrameTime = glfwGetTime();
        deltaTime = static_cast<float>(finishFrameTime - currentTime);
        currentTime = finishFrameTime;
    }

    glDeleteProgram(modelShaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}