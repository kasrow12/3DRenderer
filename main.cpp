#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <stb_image/stb_image.h>
#include <iostream>

#include "Source/Model.h"
#include "Source/Camera.h"
#include "Source/Shader.h"
#include "Source/GameObject.h"
#include "Source/Scene.h"
#include "Source/Transform.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void updateWireFrame();
void drawImGui();
void setupScene(Scene& scene);

float deltaTime = 0.0f;
double lastFrame = 0.0;

bool wireFrame = false;
bool captureMouse = false;

Scene scene;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(scene.screenWidth, scene.screenHeight, "3DRenderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPos(window, (float)scene.screenWidth / 2, (float)scene.screenHeight / 2);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    stbi_set_flip_vertically_on_load(true);

    Shader shader("Assets/Shaders/vertex.vs", "Assets/Shaders/fragment.fs");
    Shader lightShader("Assets/Shaders/vertex.vs", "Assets/Shaders/lightFragment.fs");
	Shader tessShader("Assets/Shaders/vertex.vs", "Assets/Shaders/fragment.fs", "Assets/Shaders/tessControl.tcs", "Assets/Shaders/tessEval.tes");

	setupScene(scene);

    while (!glfwWindowShouldClose(window))
    {
		double currentFrame = glfwGetTime();
		deltaTime = (float)(currentFrame - lastFrame);
		lastFrame = currentFrame;

        processInput(window);

		scene.update(deltaTime);
		scene.draw(shader, lightShader, tessShader);
               
		drawImGui();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

	// Camera controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		scene.camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scene.camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scene.camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scene.camera.processKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        scene.camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        scene.camera.processKeyboard(DOWN, deltaTime);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastX = (float)scene.screenWidth / 2;
    static double lastY = (float)scene.screenHeight / 2;
    static bool firstMouse = true;

	if (!captureMouse)
	{
		firstMouse = true;
		return;
	}

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xOffset = xpos - lastX;
    double yOffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    scene.camera.processMouseMovement((float)xOffset, (float)yOffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!captureMouse)
		return;

    scene.camera.processMouseScroll((float)yoffset);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    scene.screenWidth = width;
    scene.screenHeight = height;
	glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
        wireFrame = !wireFrame;
		updateWireFrame();
	}
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
        scene.useBlinn = !scene.useBlinn;
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		scene.isDayLight = !scene.isDayLight;
        scene.updateNight();
	}

	// Capture mouse on left control, cannot do this in processInput because this will be called only once
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
	{
		captureMouse = !captureMouse;
		if (captureMouse)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// Change camera mode
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        scene.camera.setMode(STATIC_SCENE);
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		scene.camera.setMode(STATIC_TRACKING);
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		scene.camera.setMode(ATTACHED);
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		scene.camera.setMode(FREE);
}

void updateWireFrame()
{
	if (wireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void setupScene(Scene& scene)
{
    Model* trainModel = new Model("Assets/Objects/GEVO/Gevo.obj", false);
    Model* sphereModel = new Model("Assets/Objects/basics/sphere.obj");
    Model* floorModel = new Model("Assets/Objects/basics/floor.obj");
    Model* trexModel = new Model("Assets/Objects/trex/trex.obj");

	scene.sphereModel = sphereModel;

    Transform trainTransform;
    trainTransform.scale = glm::vec3(1.0f);
    auto train = std::make_unique<GameObject>(trainModel, trainTransform, "Train");
    train->isMoving = true;
    scene.gameObjects.push_back(std::move(train));
    scene.camera.TargetTransform = &scene.gameObjects[0]->transform;

    Transform floorTransform;
    floorTransform.scale = glm::vec3(100, 0.0001f, 100);
    scene.gameObjects.push_back(std::make_unique<GameObject>(floorModel, floorTransform, "Floor"));

    Transform sphereTransform;
	sphereTransform.position = glm::vec3(0.0f, 1.0f, 0.0f);
    scene.gameObjects.push_back(std::make_unique<GameObject>(sphereModel, sphereTransform, "Sphere"));

    Transform trexTransform;
    trexTransform.position = glm::vec3(0.0f, -0.05f, 7.0f);
    scene.gameObjects.push_back(std::make_unique<GameObject>(trexModel, trexTransform, "T-rex"));

    Transform trex2Transform;
    trex2Transform.position = glm::vec3(20.0f, -0.05f, -7.0f);
    scene.gameObjects.push_back(std::make_unique<GameObject>(trexModel, trex2Transform, "T-rex2"));

    Transform trex3Transform;
    trex3Transform.position = glm::vec3(-40.0f, -0.05f, 7.0f);
    scene.gameObjects.push_back(std::make_unique<GameObject>(trexModel, trex3Transform, "T-rex3"));
}

void drawImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");
    if (ImGui::Checkbox("Wireframe (F)", &wireFrame))
    {
        updateWireFrame();
    }
    ImGui::Checkbox("Blinn (B)", &scene.useBlinn);
    if (ImGui::Checkbox("Day/Night (N)", &scene.isDayLight))
    {
        scene.updateNight();
    }

    ImGui::SliderFloat("Fog Distance", &scene.fogDistance, 0.0f, 100.0f);
    ImGui::ColorEdit3("Sky Color", &scene.skyColor.x);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    // Point Lights
    if (ImGui::CollapsingHeader("Point Lights"))
    {
        for (size_t i = 0; i < scene.pointLights.size(); i++)
        {
            if (ImGui::TreeNode(("Point Light " + std::to_string(i + 1)).c_str()))
            {
                ImGui::SliderFloat3(("Position##" + std::to_string(i)).c_str(), &scene.pointLights[i].position.x, -10.0f, 10.0f);
                ImGui::SliderFloat(("Constant##" + std::to_string(i)).c_str(), &scene.pointLights[i].constant, 0.0f, 1.0f);
                ImGui::SliderFloat(("Linear##" + std::to_string(i)).c_str(), &scene.pointLights[i].linear, 0.0f, 0.1f);
                ImGui::SliderFloat(("Quadratic##" + std::to_string(i)).c_str(), &scene.pointLights[i].quadratic, 0.0f, 0.1f);
                ImGui::ColorEdit3(("Ambient##" + std::to_string(i)).c_str(), &scene.pointLights[i].ambient.x);
                ImGui::ColorEdit3(("Diffuse##" + std::to_string(i)).c_str(), &scene.pointLights[i].diffuse.x);
                ImGui::ColorEdit3(("Specular##" + std::to_string(i)).c_str(), &scene.pointLights[i].specular.x);
                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Directional Light"))
    {
        ImGui::SliderFloat3("Direction##", &scene.dirLight.direction.x, -10.0f, 10.0f);
        ImGui::ColorEdit3("Ambient##", &scene.dirLight.ambient.x);
        ImGui::ColorEdit3("Diffuse##", &scene.dirLight.diffuse.x);
        ImGui::ColorEdit3("Specular##", &scene.dirLight.specular.x);
    }

    if (ImGui::CollapsingHeader("Train Spot Light"))
    {
        ImGui::SliderFloat3("Train light direction", &scene.trainLightDirection.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Cut Off", &scene.spotLight.cutOff, 0.0f, 90.0f);
        ImGui::SliderFloat("Outer Cut Off", &scene.spotLight.outerCutOff, 0.0f, 90.0f);
        ImGui::SliderFloat("Constant", &scene.spotLight.constant, 0.0f, 1.0f);
        ImGui::SliderFloat("Linear", &scene.spotLight.linear, 0.0f, 0.1f);
        ImGui::SliderFloat("Quadratic", &scene.spotLight.quadratic, 0.0f, 0.1f);
        ImGui::ColorEdit3("Ambient", &scene.spotLight.ambient.x);
        ImGui::ColorEdit3("Diffuse", &scene.spotLight.diffuse.x);
        ImGui::ColorEdit3("Specular", &scene.spotLight.specular.x);
    }

    if (ImGui::CollapsingHeader("Objects"))
    {
        for (auto& obj : scene.gameObjects)
        {
            if (ImGui::TreeNode(obj->name.c_str()))
            {
                ImGui::SliderFloat3("Position", &obj->transform.position.x, -20.0f, 20.0f);
                ImGui::SliderFloat3("Rotation", &obj->transform.rotation.x, 0.0f, 360.0f);
                ImGui::SliderFloat3("Scale", &obj->transform.scale.x, 0.1f, 20.0f);
                ImGui::TreePop();
            }
        }
    }

    // Train movement
    if (!scene.gameObjects.empty())
    {
        ImGui::Text("Train Movement");
        ImGui::Checkbox("Enable Movement", &scene.gameObjects[0]->isMoving);
        ImGui::SliderFloat("Movement Radius", &scene.gameObjects[0]->radius, 1.0f, 50.0f);
        ImGui::SliderFloat("Movement Speed", &scene.gameObjects[0]->speed, 0.1f, 2.0f);
    }

    if (ImGui::CollapsingHeader("Bezier Patch"))
    {
        ImGui::SliderFloat3("Position##Bezier", &scene.bezierTransform.position.x, -20.0f, 20.0f);
        ImGui::SliderFloat3("Rotation##Bezier", &scene.bezierTransform.rotation.x, 0.0f, 360.0f);
        ImGui::SliderFloat3("Scale##Bezier", &scene.bezierTransform.scale.x, 0.1f, 20.0f);
    }

    ImGui::SliderFloat("Tessellation Level", &scene.tessLevel, 1.0f, 64.0f);

    ImGui::End();

    ImGui::Begin("Controls");
    ImGui::Text("[1] - Static camera");
    ImGui::Text("[2] - Static tracking camera");
    ImGui::Text("[3] - Train camera");
    ImGui::Text("[4] - Free camera");
    ImGui::Text("Camera Controls:");
    ImGui::Text("WASD - Move Camera");
    ImGui::Text("Space - Move Up");
    ImGui::Text("Left Shift - Move Down");
    ImGui::Text("Left Control - Capture Mouse");
    ImGui::Text("Scroll - Zoom");
    ImGui::Text("F - Toggle Wireframe");
    ImGui::Text("B - Toggle Blinn-Phong");
    ImGui::Text("N - Toggle Day/Night");
    ImGui::Text("ESC - Exit");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}