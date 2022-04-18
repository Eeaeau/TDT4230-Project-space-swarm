#include "gamelogic.h"


#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>

#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/transformUtils.hpp>

#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <utilities/glutils.h>
#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

#include <fstream>


enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

float nearPlane = 0.1f;
float farPlane = 350.f;

glm::mat4 projection;
glm::mat4 ViewMatrix;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* gltfNode;
SceneNode* laserNode;
SceneNode* testCubeNode;
SceneNode* markerNode;
SceneNode* textureAtlasNode;
SceneNode* textEmptyNode;
SceneNode* magmaSphereNode;
SceneNode* shipNode;

// Create Frame Buffer Object
GLuint postProcessingFBO;

GLuint postProcessingTexture;
GLuint bloomTexture;

GLuint attachments[2];

GLuint bloomFBO;
GLuint bloomBuffer;

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* phongShader;
Gloom::Shader* overlayShader;
Gloom::Shader* particleShader;
Gloom::Shader* instancingShader;
Gloom::Shader* pbrShader;
Gloom::Shader* framebufferShader;
Gloom::Shader* blurShader;

sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);
glm::vec3 boxCenter(0, -10, -80);

float cameraHeight = 10;
glm::vec3 cameraPosition = glm::vec3(0, cameraHeight, -20);
glm::mat4 orthoProject;

glm::vec3 cursorPosition;
glm::vec3 cursorProjectedPosition;

glm::mat4 VP;

GLuint rectVAO, rectVBO;

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;

double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double margin = 50;

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;
    


    //glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

   /* if ((x + deltaX  > windowWidth - margin || x + deltaX  < 0 + margin) || (y+ deltaY > windowHeight - margin || y + deltaY  < 0 + margin)) {
        glfwSetCursorPos(window, lastMouseX, lastMouseY);
    }*/
    auto x_clamped = glm::clamp(x, 0.0 + margin, static_cast<double>(windowWidth) - margin);
    auto y_clamped = glm::clamp(y, 0.0 + margin, static_cast<double>(windowHeight) - margin);

    glfwSetCursorPos(window, x_clamped, y_clamped);

    lastMouseX = x;
    lastMouseY = y;
}

void homogeneous_to_world(glm::vec3& world, const glm::vec3& homogeneous, const glm::mat4& VP)
{
    glm::mat4 transform = glm::inverse(VP);
    glm::vec4 _world = transform * glm::vec4(homogeneous, 1.0f);
    world = glm::vec3(_world) * (1.0f / _world.w);
}

bool project_screen_onto_plane(glm::vec3& point, const glm::vec2& screen, const glm::vec3& plane_point, const glm::vec3& plane_normal, const glm::mat4& VP)
{
    glm::vec3 ray_origin, ray_end;
    homogeneous_to_world(ray_origin, glm::vec3(screen, 0.0f), VP);
    homogeneous_to_world(ray_end, glm::vec3(screen, 1.0f), VP);

    glm::vec3 ray_normal = glm::normalize(ray_end - ray_origin);
    float t = glm::dot(plane_point - ray_origin, plane_normal) / glm::dot(ray_normal, plane_normal);
    point = ray_origin + t * ray_normal;

    return t >= 0.0f;
}

void mouse_on_xy_plane(glm::vec3& mouse_world, double mouse_x, double mouse_y, int window_width, int window_height, const glm::mat4& VP)
{
    glm::vec2 screen;
    screen.x = 2.0f * (float)mouse_x / (float)window_width - 1.0f;
    screen.y = 1.0f - 2.0f * (float)mouse_y / window_height;
    project_screen_onto_plane(mouse_world, screen, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), VP);
}

void mouse_on_xz_plane(glm::vec3& mouse_world, double mouse_x, double mouse_y, int window_width, int window_height, const glm::mat4& VP)
{
    glm::vec2 screen;
    screen.x = 2.0f * (float)mouse_x / (float)window_width - 1.0f;
    screen.y = 1.0f - 2.0f * (float)mouse_y / window_height;
    project_screen_onto_plane(mouse_world, screen, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), VP);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);

        cursorPosition.x = 2 * (xpos / windowWidth) - 1;
        cursorPosition.y = 2 * (ypos / windowHeight) - 1;;
        cursorPosition.z = 0;
        std::cout << "Cursor screen Position at (" << cursorPosition.x << " : " << cursorPosition.y << std::endl;

        //cursorProjectedPosition = glm::vec3(glm::vec4(cursorPosition, 1) * glm::inverse(VP));

        //glm::vec4 nearPlane = glm::vec4((xpos - windowWidth / 2) / (windowWidth / 2), -1 * (ypos - windowHeight / 2) / (windowHeight / 2), -1, 1.0);
        //glm::vec4 farPlane = glm::vec4((xpos - windowWidth / 2) / (windowWidth / 2), -1 * (ypos - windowHeight / 2) / (windowHeight / 2), 1, 1.0);
        //glm::vec4 nearResult = glm::inverse(VP) * nearPlane;
        //glm::vec4 farResult = glm::inverse(VP) * farPlane;
        //nearResult /= nearResult.w;
        //farResult /= farResult.w;
        //glm::vec3 dir = farResult - nearResult;

        mouse_on_xz_plane(cursorProjectedPosition, xpos, ypos, windowWidth, windowHeight, VP);

        std::cout << "Cursor projected Position at (" << cursorProjectedPosition.x << " : " << cursorProjectedPosition.y << " : " << cursorProjectedPosition.z << ")" << std::endl;
        //std::(dir)
    }
}

void keyCallback(GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods) {



}

float scrollFactor = 5;
float maxCameraHeight = 80;
float minCameraHeight = -10;

void scrollCallback(GLFWwindow* window, double x, double y) {
    //scrollFactor = y;
    cameraHeight -= y * scrollFactor;
    //cameraHeight = glm::min(glm::max(cameraHeight, minCameraHeight), maxCameraHeight);
    cameraHeight = glm::clamp(cameraHeight, minCameraHeight, maxCameraHeight);
}

static std::string GetFilePathExtension(const std::string& FileName) {
    if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
    return "";
}

//// A few lines to help you if you've never used c++ structs
//struct LightSource {
//    LightSource() {
//        lightColor = glm::vec3(4);
//        float constant = 1.0;
//        float linear = 0.0009;
//        float quadratic = 0.0032;
//    }
//    glm::vec3 lightColor;
//    float constant;
//    float linear;
//    float quadratic;
// };
//LightSource lightSources[3];

std::vector<SceneNode*> lightSources;
int NumLightProcessed = 0;

//tinygltf::Model modelFromglTF;

std::vector <glm::vec3> distributeOnGrid(unsigned int amount, int width, float offset) {
    std::vector <glm::vec3> instancePos(amount);
    int x, y = 0;
    for (unsigned int i = 0; i < amount; i++)
    {
        if (i % width == 0) {
            y++;
        }
        x = i % width;
        glm::vec3 pos = glm::vec3(x, 0, y);

        instancePos[i] = offset * pos;
    }
    return instancePos;
}


std::vector <glm::mat4> distributeOnDisc(unsigned int amount, float radius, float offset) {
    std::vector <glm::mat4> instanceMatrices(amount);
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model(1);
        //model = glm::mat4(1);
        //model = glm::translate(model, glm::vec3(1, 0, 0));
        //model = model * glm::rotate(glm::vec3(i).x, glm::vec3(1, 0, 0));



        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        /*float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));*/

        // 4. now add to list of matrices
        instanceMatrices[i] = model;
    }
    return instanceMatrices;
}


void deleteGame() {
    glDeleteFramebuffers(1, &postProcessingFBO);
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    

    options = gameOptions;

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    
    orthoProject = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), nearPlane, farPlane);
    //orthoProject = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, nearPlane, farPlane);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetScrollCallback(window, scrollCallback);
    //glfwSetKeyCallback(window, keyCallback);

    phongShader = new Gloom::Shader();
    phongShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    phongShader->activate();

    overlayShader = new Gloom::Shader();
    overlayShader->makeBasicShader("../res/shaders/overlay.vert", "../res/shaders/overlay.frag");

    particleShader = new Gloom::Shader();
    particleShader->makeBasicShader("../res/shaders/particle.vert", "../res/shaders/particle.frag");

    instancingShader = new Gloom::Shader();
    instancingShader->makeBasicShader("../res/shaders/instancing.vert", "../res/shaders/simple.frag");
    
    pbrShader = new Gloom::Shader();
    pbrShader->makeBasicShader("../res/shaders/pbr.vert", "../res/shaders/pbr.frag");
    
    framebufferShader = new Gloom::Shader();
    framebufferShader->makeBasicShader("../res/shaders/framebuffer.vert", "../res/shaders/framebuffer.frag");
    
    blurShader = new Gloom::Shader();
    blurShader->makeBasicShader("../res/shaders/framebuffer.vert", "../res/shaders/blur.frag");

    projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane);

    //----------------------------------------------------------------------------------------------------------//
    float rectangleVertices[] =
    {
        //  Coords   // texCoords
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,

         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    auto gamma = 2.2f;
    auto exposure = 0.5;

    framebufferShader->activate();
    glUniform1i(framebufferShader->getUniformFromName("screenTexture"), 0);
    glUniform1i(framebufferShader->getUniformFromName("bloomTexture"), 1);
    glUniform1f(framebufferShader->getUniformFromName("gamma"), gamma);
    glUniform1f(framebufferShader->getUniformFromName("exposure"), exposure);
    blurShader->activate();
    glUniform1i(blurShader->getUniformFromName("screenTexture"), 1);

    // Prepare framebuffer rectangle VBO and VAO
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    // Create Frame Buffer Object
    glGenFramebuffers(1, &postProcessingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);

    glGenTextures(1, &postProcessingTexture);
    glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

    // Create Second Framebuffer Texture

    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);



    // Tell OpenGL we need to draw to both attachments
    //attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    attachments[0] = GL_COLOR_ATTACHMENT0;
    attachments[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(2, attachments);
    
    // Error checking framebuffer
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "1 Post-Processing Framebuffer error: " << fboStatus << std::endl;

    glGenFramebuffers(1, &bloomFBO);
    glGenTextures(1, &bloomBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
    glBindTexture(GL_TEXTURE_2D, bloomBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomBuffer, 0);

    // Error checking framebuffer
    fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "2 Post-Processing Framebuffer error: " << fboStatus << std::endl;


    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //----------------------------------------------------------------------------------------------------------//

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh testCube = cube(glm::vec3(2), glm::vec2(90));
    Mesh sphere = generateSphere(1.0, 40, 40);
    Mesh sphere2 = generateSphere(5.0, 40, 40);
    Mesh textMesh = generateTextGeometryBuffer("Click to begin!", 39 / 29, 0.5);
    Mesh textEmptyMesh = generateTextGeometryBuffer("               ", 39 / 29, 0.5);

    // ------ Fill buffers ------ //
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int ball2VAO = generateBuffer(sphere2);
    unsigned int boxVAO  = generateBuffer(box);
    
    unsigned int padVAO  = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(textMesh);
    unsigned int textEmptyVAO = generateBuffer(textEmptyMesh);

    // ------ Construct scene ------ //
    //create scene nodes
    rootNode = createSceneNode();
    gltfNode = createSceneNode();
    magmaSphereNode = createSceneNode();
    shipNode = createSceneNode();

    testCubeNode = createSceneNode();
    markerNode  = createSceneNode();
    textureAtlasNode = createSceneNode();
    textureAtlasNode->position = boxCenter;

    textEmptyNode = createSceneNode();
    textEmptyNode->position = boxCenter;

    laserNode = createSceneNode();

    // assign VAO

    testCubeNode->VAOIndexCount = testCube.indices.size();

    textureAtlasNode->vertexArrayObjectID = textVAO;
    textureAtlasNode->VAOIndexCount = textMesh.indices.size();
    
    textEmptyNode->vertexArrayObjectID = textEmptyVAO;
    textEmptyNode->VAOIndexCount = textEmptyMesh.indices.size();

    // ------------------- textures ------------------- // 

    textureAtlasNode->nodeType = OVERLAY;
    textureAtlasNode->diffuseTexture = loadPNGFile("../res/textures/charmap.png");
    unsigned int atlasDiffuseID = generateTexture(textureAtlasNode->diffuseTexture, false);
    textureAtlasNode->diffuseTextureID = atlasDiffuseID;
    
    textEmptyNode->nodeType = OVERLAY;
    //textEmptyNode->diffuseTexture = loadPNGFile("../res/textures/charmap.png");
    unsigned int emptyDiffuseID = generateTexture(textureAtlasNode->diffuseTexture, false);
    textEmptyNode->diffuseTextureID = emptyDiffuseID;
    
    unsigned int amount = 2000;

    std::srand(glfwGetTime()); // initialize random seed	
    float radius = 40.0;
    float offset = 5.0;

    auto instanceMatrices = distributeOnDisc(amount, radius, offset);
    auto instancePos = distributeOnGrid(amount, amount/2, 10);

    //testCubeNode->nodeType = INCTANCED_GEOMETRY;
    //testCubeNode->instanceMatrices = instanceMatrices;
    //testCubeNode->vertexArrayObjectID = generateInctancedBuffer2(testCube, testCubeNode->instanceMatrices);

    //std::string input_filename = "../res/mesh/magma_sphere/magma_sphere.gltf";
    std::string teapotPath = "../res/mesh/teapot/teapot.gltf";

    std::string magmaSpherePath = "../res/mesh/magma_sphere/magma_sphere.gltf";
    std::string markerPath = "../res/mesh/magma_sphere/marker.gltf";
    std::string laserPlanePath = "../res/mesh/magma_sphere/laser_plane.gltf";

    //std::string markerPath = "..res/mesh/marker/marker.gltf";

    //std::string laserPlanePath = "..res/mesh/laser_plane/laser_plane.gltf";
    std::string suzanePath = "../res/mesh/suzane/suzane.gltf";
    std::string shipPath = "..res/mesh/MC90/MC90.gltf";

    magmaSphereNode->instanceMatrices = instanceMatrices;
    magmaSphereNode->model = GLModel(markerPath.c_str(), amount, instanceMatrices);
    magmaSphereNode->nodeType = GLTF_GEOMETRY;
    //magmaSphere->position = boxCenter+glm::vec3(0,2,0);
    magmaSphereNode->scale= glm::vec3(3);
    //rootNode->children.push_back(magmaSphereNode);
    

    //shipNode->model = GLModel(markerPath.c_str());
    //shipNode->nodeType = GLTF_GEOMETRY;
    //shipNode->position = boxCenter+glm::vec3(0,2,0);
    //shipNode->scale= glm::vec3(3);
    //rootNode->children.push_back(shipNode);   
    // 

    markerNode->instanceMatrices = instanceMatrices;
    markerNode->model = GLModel(markerPath.c_str());
    markerNode->scale = glm::vec3(10);
    markerNode->nodeType = GLTF_GEOMETRY;
    rootNode->children.push_back(markerNode);
    
    laserNode->instanceMatrices = instanceMatrices;
    laserNode->model = GLModel(laserPlanePath.c_str());
    laserNode->scale = glm::vec3(3);
    laserNode->nodeType = GLTF_GEOMETRY;
    rootNode->children.push_back(laserNode);

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "GL_MAX_VERTEX_ATTRIBS: " << GL_MAX_VERTEX_ATTRIBS << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double timeDelta = getTimeDeltaSeconds();
    double fractionFrameComplete;

    //const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    //const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    //const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    //const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    //const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    //const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    //const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    //const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;

            textureAtlasNode->VAOIndexCount = textEmptyNode->VAOIndexCount;
            textureAtlasNode->vertexArrayObjectID = textEmptyNode->vertexArrayObjectID;
        }

       /* ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);*/
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            testCubeNode->rotation.y += timeDelta*0.1;

            //magmaSphere->rotation.y += timeDelta*0.01;

            //magmaSphere->setPoint = glm::vec3(100.0f*glm::sin(gameElapsedTime), 0, 100.0f * glm::sin(gameElapsedTime+3.14f/2));

            //auto cursorProjectedPosition = cursorPosition;

            magmaSphereNode->setPoint = cursorProjectedPosition;

            markerNode->position = cursorProjectedPosition;

            for (auto& transformation : magmaSphereNode->instanceMatrices) {
                //transformation *= glm::rotate(static_cast<float>(timeDelta), glm::vec3(0, 1, 0));
                //glm::mat4 transformation; // your transformation matrix.
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(transformation, scale, rotation, translation, skew, perspective);
                
                auto dist = magmaSphereNode->setPoint - translation;
                auto dir = glm::normalize(dist);

                transformation = glm::translate(transformation, 1.0f * static_cast<float>(timeDelta) * dir * (0.1f + glm::length(dist)));
                //transformation = glm::translate(transformation, glm::vec3(1.0f,0,0) );
                //transformation = glm::composeTransform(scale, rotation, translation + dir * (1.0f+glm::length(dist)), glm::vec3(0), glm::vec4(0), glm::vec3(0));
            }

            magmaSphereNode->model.updateInstanceMatrix(magmaSphereNode->instanceMatrices);

            //double ballYCoord;

            //KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            //KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Make ball move
            /*const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;*/

            if(options.enableAutoplay) {
                //padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                //padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            //if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
            //    //double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
            //    //double padRightX = padLeftX + padDimensions.x;
            //    //double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
            //    //double padBackZ  = padFrontZ + padDimensions.z;

            //    if (   ballPosition.x < padLeftX
            //        || ballPosition.x > padRightX
            //        || ballPosition.z < padFrontZ
            //        || ballPosition.z > padBackZ
            //    ) {
            //        hasLost = true;
            //        if (options.enableMusic) {
            //            sound->stop();
            //            delete sound;
            //        }
            //    }
            //}
        }
    }

    //orthoProject = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), nearPlane, farPlane);

    // ------------------------------------ Camera position ------------------------------------ //

    float cameraFaceDirectionFactor = glm::abs((cameraHeight - minCameraHeight)/(maxCameraHeight - minCameraHeight))+0.5;

    float speed = cameraFaceDirectionFactor*50.0;
    float speedModifier = 3.0;


    auto cameraFaceDirection = glm::vec3(0.0, -2.0* cameraFaceDirectionFactor, -1.0);
    auto cameraPlaneDirection = glm::vec3(0.0, 0.0, -1.0);
    auto rightDirection = glm::vec3(1.0, 0.0, 0.0);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speed *= speedModifier;
    }

    if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS) {
        cameraPosition += cameraPlaneDirection * static_cast<float>(timeDelta) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosition -= cameraPlaneDirection * static_cast<float>(timeDelta) * speed;
    }

    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPosition -= rightDirection * static_cast<float>(timeDelta) * speed;
    }

    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += rightDirection * static_cast<float>(timeDelta) * speed;
    }

    cameraPosition.y = cameraHeight;
    

    // ----------------------------------------------------------------------------------------- //

    float yawFactor = 0;
    
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        yawFactor = 0.5f;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        yawFactor = -0.5f;
    }

    cameraFaceDirection = glm::vec3(glm::rotate(yawFactor, glm::vec3(0, 1, 0))*glm::vec4(cameraFaceDirection, 0));

    // Some math to make the camera move in a nice way
    //float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    //glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
    //glm::rotate(0.3f + 0.2f * float(-padPositionZ * padPositionZ), glm::vec3(1, 0, 0))*
    /*
    glm::mat4 cameraTransform =
                    glm::translate(cameraPosition) *
                    glm::rotate(1.0f, glm::vec3(0, 1, 0)) *
                    glm::rotate(yawFactor, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);*/
    //glm::vec3 up = glm::cross(rightDirection, cameraFaceDirection);
    glm::vec3 up = glm::vec3(0,1,0);

    ViewMatrix = glm::lookAt(
        cameraPosition,           // Camera is here
        cameraPosition + cameraFaceDirection, // and looks here : at the same position, plus "direction"
        up                  // Head is up
    );

    VP = projection * ViewMatrix;


    // Move and rotate various SceneNodes
    //boxNode->position = { 0, -10, -80 };

    //ballNode->position = ballPosition;
    //ballNode->scale = glm::vec3(ballRadius);
    //ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    /*padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };*/

    updateNodeTransformations(rootNode, VP);
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix = glm::composeTransform(node->scale, node->rotation, node->position, glm::vec3(0), glm::vec4(0), node->referencePoint);
            /*  glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);*/
    node->modelMatrix = transformationMatrix; // M
    node->modelViewMatrix = ViewMatrix * transformationMatrix; // MV
    node->viewProjectionMatrix = projection * ViewMatrix; // MV
    node->modelViewProjectionMatrix = transformationThusFar * transformationMatrix; // model view projection matrix

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->modelViewProjectionMatrix);
    }
}

void renderNode(SceneNode* node) {

    //auto screenPos = glm::vec3(node->modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

    // Common uniforms for phong shader 
    phongShader->activate();
    glUniformMatrix4fv(phongShader->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(node->modelViewProjectionMatrix)); // MVP
    
    glUniformMatrix4fv(phongShader->getUniformFromName("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelViewMatrix));

    glUniformMatrix4fv(phongShader->getUniformFromName("viewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(node->viewProjectionMatrix));

    //glUniform3fv(phongShader->getUniformFromName("textPos"), 1, glm::value_ptr(screenPos));

    glUniformMatrix4fv(phongShader->getUniformFromName("modelMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelMatrix));

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(node->modelMatrix)));
    glUniformMatrix3fv(phongShader->getUniformFromName("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));


    glUniform1i(phongShader->getUniformFromName("useTexture"), 0);
    glUniform1i(instancingShader->getUniformFromName("useTexture"), 0);
    //glUniform1i(phongShader->getUniformFromName("useInstance"), 0);

    std::string number = std::to_string(NumLightProcessed);
    //std::string numSprite = std::to_string(0);
    //std::string numPBR = std::to_string(0);
    switch(node->nodeType) {
        //case GEOMETRY:
        //    phongShader->activate();
        //    if(node->vertexArrayObjectID != -1) {
        //        glBindVertexArray(node->vertexArrayObjectID);
        //        glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
        //    }
        //    break;

        case GLTF_GEOMETRY:
            pbrShader->activate();
            //phongShader->activate();
            glUniformMatrix4fv(pbrShader->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(node->modelViewProjectionMatrix)); // MVP
            
            //glUniform1f(pbrShader->getUniformFromName("useInstancing"), 1);
            //glUniform1i(phongShader->getUniformFromName("useTexture"), 1);
            //if (node->vertexArrayObjectID != -1) {
            //}
                //drawModel(node->vertexArrayObjectID, node->model);
            glUniformMatrix3fv(pbrShader->getUniformFromName("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
            node->model.drawModel(pbrShader);
            break;

        case INCTANCED_GEOMETRY:
            instancingShader->activate();
            //glUniform1i(phongShader->getUniformFromName("useInstance"), 1);
            glUniform1i(instancingShader->getUniformFromName("useTexture"), 0);
            glUniformMatrix4fv(instancingShader->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(node->modelViewProjectionMatrix)); // MVP
            glUniformMatrix4fv(instancingShader->getUniformFromName("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelViewMatrix));
            glUniformMatrix4fv(instancingShader->getUniformFromName("viewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(node->viewProjectionMatrix));
            glUniformMatrix3fv(instancingShader->getUniformFromName("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
            glUniformMatrix4fv(instancingShader->getUniformFromName("modelMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelMatrix));

            //node->model.drawModel(instancingShader);

            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElementsInstanced(
                    GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, 0, node->instanceMatrices.size()
                );
                glBindVertexArray(0);
            }
            /*for (unsigned int i = 0; i < node->modelMatrices.size(); i++)
            {
            }*/

            //    //glBindVertexArray(node->vertexArrayObjectID);
            //    //glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            //}
            //break;
        
        case OVERLAY: 
            overlayShader->activate();
            // Common uniforms for overlay phongShader 
            //glBindTextureUnit(overlayShader->getUniformFromName(("textSprite[" + numSprite + "].position").c_str()), textureID);
            glBindTextureUnit(3, node->diffuseTextureID);

            glUniformMatrix4fv(overlayShader->getUniformFromName("modelMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelMatrix));
            glUniformMatrix4fv(overlayShader->getUniformFromName("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelViewMatrix));
            glUniformMatrix4fv(overlayShader->getUniformFromName("orthoMatrix"), 1, GL_FALSE, glm::value_ptr(orthoProject));
    
            glm::mat4 customMatrix(1.0f);
            //customMatrix = customMatrix * glm::translate(glm::vec3(0,0,0));
            glUniformMatrix4fv(overlayShader->getUniformFromName("customMatrix"), 1, GL_FALSE, glm::value_ptr(customMatrix));


            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }

            break;
        case SPRITE: 
            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            //auto pos = (node->currentTransformationMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
            //glUniform3fv(phongShader->getUniformFromName(("textSprite[" + numSprite + "].position").c_str()), 1, glm::value_ptr(glm::vec3(pos.x, pos.y, pos.z)));
            break;
        case POINT_LIGHT: 
            phongShader->activate();
            //glUniform1ui(7, lightSources.size());
            auto pos = (node->modelViewProjectionMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
            glUniform3fv(phongShader->getUniformFromName(("pointLights["+ number +"].position").c_str()), 1, glm::value_ptr(glm::vec3(pos.x, pos.y, pos.z)));
            //glUniform3fv(phongShader->getUniformFromName(("pointLights["+ number +"].lightColor").c_str()), 1, glm::value_ptr(lightSources[NumLightProcessed].lightColor));
            glUniform3fv(phongShader->getUniformFromName(("pointLights["+ number +"].lightColor").c_str()), 1, glm::value_ptr(node->lightColor));
            glUniform1f(phongShader->getUniformFromName(("pointLight[" + number + "].constant").c_str()), node->constant);
            glUniform1f(phongShader->getUniformFromName(("pointLight[" + number + "].linear").c_str()), node->linear);
            glUniform1f(phongShader->getUniformFromName(("pointLight[" + number + "].quadratic").c_str()), 0.0f);

            instancingShader->activate();
            glUniform3fv(instancingShader->getUniformFromName(("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(glm::vec3(pos.x, pos.y, pos.z)));
            //glUniform3fv(phongShader->getUniformFromName(("pointLights["+ number +"].lightColor").c_str()), 1, glm::value_ptr(lightSources[NumLightProcessed].lightColor));
            glUniform3fv(instancingShader->getUniformFromName(("pointLights[" + number + "].lightColor").c_str()), 1, glm::value_ptr(node->lightColor));
            glUniform1f(instancingShader->getUniformFromName(("pointLight[" + number + "].constant").c_str()), node->constant);
            glUniform1f(instancingShader->getUniformFromName(("pointLight[" + number + "].linear").c_str()), node->linear);
            glUniform1f(instancingShader->getUniformFromName(("pointLight[" + number + "].quadratic").c_str()), 0.0f);

            NumLightProcessed++;
            break;
        case SPOT_LIGHT: break;
        default: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {

    // -------------------- Initiate post process -------------------- //
    // 
    // Bind the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);
    // Specify the color of the background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Clean the back buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enable depth testing since it's disabled when drawing the framebuffer rectangle
    glEnable(GL_DEPTH_TEST);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);
    // -------------------- Draw geo-------------------- //

    overlayShader->activate();
    glUniform3fv(overlayShader->getUniformFromName("viewPos"), 1, glm::value_ptr(cameraPosition));

    phongShader->activate();
    glUniform3fv(phongShader->getUniformFromName("viewPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(phongShader->getUniformFromName("lightTest"), 1, glm::value_ptr(glm::vec3(1, 0, 0)));
    //auto ballPos = glm::vec3(ballNode->modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
    //glUniform3fv(phongShader->getUniformFromName("ballPos"), 1, glm::value_ptr(ballPos));

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    renderNode(rootNode);
    NumLightProcessed = 0; // reset 


    // -------------------- Post process -------------------- //
    glCullFace(GL_FRONT);

    blurShader->activate();
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);

    // In the first bounc we want to get the data from the bloomTexture

    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Render the image
    glBindVertexArray(rectVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw the framebuffer rectangle
    framebufferShader->activate();
    glBindVertexArray(rectVAO);
    glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomBuffer);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
}
