/* I declare that this code is my own work */
/* Author <Lewin Elston> <lelston1@sheffield.ac.uk> */


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <string>
#include <functional>
#include <map>

#include "stb_image.h"
#include "shader.h"
#include "camera.h"

struct Node {
	std::string object;
	glm::mat4 model;
	std::vector< std::reference_wrapper<Node>> children;

	void updateTranslate(glm::vec3 translation) {
		model = glm::translate(model, translation);
		for (Node& x : children) {
			x.updateTranslate(translation);
		}
	}

	void updateRotate(float angle, glm::vec3 direction) {
		model = glm::rotate(model, angle, direction);
		for (Node& x : children) {
			x.updateRotate(angle, direction);
		}
		
	}

	void updateScale(glm::vec3 scale) {
		model = glm::scale(model, scale);
		for (Node& x : children) {
			x.updateScale(scale);
		}

	}
};

enum LampState
{
	Default,
	Crouched1,
	Crouched2,
	Other1, 
	Other2
};

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_resize(GLFWwindow* window, int width, int height);
unsigned int loadTexture(char const* path);
unsigned int loadSkybox(std::vector<std::string> faces);
void renderCube();
void renderTable(Shader& tableShader);
void renderSphere();
void renderLamp(Shader lampShader, glm::vec3 pos, glm::vec3 scale, float angle, glm::vec3 axis, LampState state, int lampNum);
void renderNode(Shader& shader, Node node);



const int WIDTH = 1280;
const int HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
bool firstMouse = true; // Keeps track of if mouse has been used yet
float lastX = WIDTH / 2; // Keeps track of mouse since last frame
float lastY = HEIGHT / 2;  // Keeps track of mouse since last frame

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

unsigned int floorTexture, floorTextureSpec, wallTexture, wallTextureSpec, windowTextureLeft, windowTextureRight, eggTexture, eggSpec, skyboxTexture, cloudTexture, tableTexture, tableSpec, lampTexture;
LampState currentLamp1State = Default;
LampState currentLamp2State = Default;
bool lamp1Key = false; 
bool lamp2Key = false;
bool lamp1OnKey = false;
bool lamp2OnKey = false;
bool lamp1On = true;
bool lamp2On = true; 
bool dirLightKey = false;
bool dirLightOn = true;  


bool eggAnimating = false; 
float animationCounter = 0.0f;
float nextJump = 10.0f; 

int main()
{

	// initialization and setup 

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Scene View", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "GLFW Window could not be created" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Makes context on current thread. 
	glfwSetFramebufferSizeCallback(window, framebuffer_resize); // Sets resizing function
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwWindowHint(GLFW_SAMPLES, 8); // multisample buffer (4 Samples)

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD failed to load" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(false);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Scene code 

	// floor setup

	float floorVertices[] = {
		// vertex pos         // normal pos     // texture coords
		10.0f, -0.0f, 10.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
	   -10.0f, -0.0f, 10.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
	   -10.0f, -0.0f, -10.0f, 0.0f, 1.0f, 0.0f,  0.0f, 2.0f, 

	    10.0f, -0.0f, 10.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
	   -10.0f, -0.0f, -10.0f, 0.0f, 1.0f, 0.0f,  0.0f, 2.0f,
	    10.0f, -0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f
	};
    
	unsigned int floorVAO, floorVBO;
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);


	// wall setup

	float wallVertices[] = {
		// vertex pos         // normal pos     // texture coords
		-5.0f,  5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 2.0f, 0.0f, 
		-5.0f,  5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 2.0f, 2.0f, 
		-5.0f, -5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 2.0f, 
		-5.0f, -5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 2.0f, 
		-5.0f, -5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 0.0f, 
		-5.0f,  5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 2.0f, 0.0f, 
	};

	unsigned int wallVAO, wallVBO;
	glGenVertexArrays(1, &wallVAO);
	glGenBuffers(1, &wallVBO);
	glBindVertexArray(wallVAO);
	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// window setup

	float windowVertices[] = {
		// vertex pos         // normal pos     // texture coords
		-5.0f,  5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 1.0f, 0.0f,
		-5.0f,  5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		-5.0f, -5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-5.0f, -5.0f, -5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-5.0f, -5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		-5.0f,  5.0f,  5.0f, -5.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	};

	unsigned int winVAO, winVBO;
	glGenVertexArrays(1, &winVAO);
	glGenBuffers(1, &winVBO);
	glBindVertexArray(winVAO);
	glBindBuffer(GL_ARRAY_BUFFER, winVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertices), windowVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// skybox setup 

	float skyboxVertices[] = {
		// positions          
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

	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// shaders and textures
	// 
	// textures

	floorTexture = loadTexture("Resources/Textures/Wood Floor_007_SD/Wood_Floor_007_COLOR.jpg");
	floorTextureSpec = loadTexture("Resources/Textures/Wood Floor_007_SD/Wood_Floor_007_DISP.png");
	wallTexture = loadTexture("Resources/Textures/Concrete_017/Concrete_017_basecolor.jpg");
	wallTextureSpec = loadTexture("Resources/Textures/Concrete_017/Concrete_017_roughness.jpg");
	windowTextureLeft = loadTexture("Resources/Textures/Window_001/Window_001_basecolor_left.png");
	windowTextureRight = loadTexture("Resources/Textures/Window_001/Window_001_basecolor_right.png");
	eggTexture = loadTexture("Resources/Textures/Egg-texture/diffuse.png");
	eggSpec = loadTexture("Resources/Textures/Egg-texture/specular.png");
	cloudTexture = loadTexture("Resources/Textures/blue-cloud-PNG-transparent.png");

	tableTexture = loadTexture("Resources/Textures/Wood013/Wood013_1K_Color.jpg");
	tableSpec = loadTexture("Resources/Textures/Wood013/Wood013_1K_Displacement.jpg");

	lampTexture = loadTexture("Resources/Textures/Animal-Fur/stylized-animal-fur_albedo.png");

	std::vector<std::string> skyboxFaces
	{
		"Resources/Textures/skybox_test/right.jpg",
		"Resources/Textures/skybox_test/left.jpg",
		"Resources/Textures/skybox_test/top.jpg",
		"Resources/Textures/skybox_test/bottom.jpg",
		"Resources/Textures/skybox_test/front.jpg",
		"Resources/Textures/skybox_test/back.jpg"
	};
	skyboxTexture = loadSkybox(skyboxFaces);

	// shaders

	Shader roomShader("Shaders/room.vert", "Shaders/room.frag");
	Shader skyboxShader("Shaders/skybox.vert", "Shaders/skybox.frag");
	skyboxShader.setInt("skybox", 0);
	roomShader.setInt("material.diffuse", 0);
	roomShader.setInt("material.specular", 1);

	std::vector<glm::vec3> cloudPositions = {
		glm::vec3(35.0f, 10.0f, 0.0f),
		glm::vec3(35.0f, 5.0f, -10.0f),
		glm::vec3(35.0f, 7.5f,  20.0f),
	};

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// egg animation
		if (nextJump <= 0) {
			eggAnimating = true;
		} else {
			nextJump -= deltaTime;
		}

		// rendering commands
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 50.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		roomShader.use();

		// lamps

		renderLamp(roomShader, glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), currentLamp1State, 1);
		renderLamp(roomShader, glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.75f, 0.75f, 0.75f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), currentLamp2State, 2);

		// floor

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorTextureSpec);


		model = glm::mat4(1.0f);

		roomShader.use();
		roomShader.setMat4("projection", projection);
		roomShader.setMat4("view", view);
		roomShader.setMat4("model", model);

		roomShader.setVec3("viewPos", camera.Position);
		roomShader.setFloat("material.shininess", 32.0f);
		roomShader.setBool("dirLightOn", dirLightOn); // handle directional lighting on/off
		roomShader.setBool("lightingOn", true);
		roomShader.setBool("lamp1On", lamp1On);
		roomShader.setBool("lamp2On", lamp2On);

		// directionalLight
		roomShader.setVec3("dirLights[0].direction", -10.0f, -10.0f, 0.0f);
		roomShader.setVec3("dirLights[0].ambient", 0.05f, 0.05f, 0.05f);
		roomShader.setVec3("dirLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		roomShader.setVec3("dirLights[0].specular", 0.5f, 0.5f, 0.5f);

		roomShader.setVec3("dirLights[1].direction", 10.0f, 10.0f, -5.0f);
		roomShader.setVec3("dirLights[1].ambient", 0.05f, 0.05f, 0.05f);
		roomShader.setVec3("dirLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		roomShader.setVec3("dirLights[1].specular", 0.5f, 0.5f, 0.5f);


		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// walls

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wallTextureSpec);

		// bottom left
		model = glm::mat4(1.0f);
	    model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 5.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0)); 
		model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(wallVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// top left
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, 5.0f, -5.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(wallVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// bottom right
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(15.0f, 5.0f, 5.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(wallVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// top right
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(15.0f, 5.0f, -5.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(wallVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Room Items

		renderTable(roomShader);

		// window

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, windowTextureRight);

		// back right
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::translate(model, glm::vec3(15.0f, 5.0f, -5.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(winVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, windowTextureLeft);

		// back left
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::translate(model, glm::vec3(15.0f, -5.0f, -5.0f));
		roomShader.use();
		roomShader.setMat4("model", model);
		glBindVertexArray(winVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// skybox
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_CLAMP);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		glBindVertexArray(skyVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDisable(GL_DEPTH_CLAMP);
		glDepthFunc(GL_LESS);

		// clouds
		roomShader.use();
		roomShader.setBool("lightingOn", false);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cloudTexture);

		for (int i = 0; i < cloudPositions.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			if (cloudPositions[i].z < -40.0f) {
				cloudPositions[i].z += 80.0f;
			}
			model = glm::translate(model, glm::vec3(cloudPositions[i].x, cloudPositions[i].y, cloudPositions[i].z -= (deltaTime * 3.0f)));
			roomShader.setMat4("model", model);
			glBindVertexArray(winVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		roomShader.setBool("lightingOn", true);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


void renderTable(Shader& tableShader) {

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tableTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tableSpec);

	// base
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(2.0f, 0.125f, 2.0f));
	model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	// front right leg
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.8f, 1.25f, 1.8f));
	model = glm::scale(model, glm::vec3(0.125f, 1.25f, 0.125f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	//front left leg
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.8f, 1.25f, 1.8f));
	model = glm::scale(model, glm::vec3(0.125f, 1.25f, 0.125f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	//top right leg
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.8f, 1.25f, -1.8f));
	model = glm::scale(model, glm::vec3(0.125f, 1.25f, 0.125f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	//top left leg
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.8f, 1.25f, -1.8f));
	model = glm::scale(model, glm::vec3(0.125f, 1.25f, 0.125f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	// egg base
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5f, 0.125f, 0.5f));
	model = glm::translate(model, glm::vec3(0.0f, 21.0f, 0.0f));
	tableShader.use();
	tableShader.setMat4("model", model);
	renderCube();

	// egg

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eggTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, eggSpec);

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.0f, 1.5f, 1.0f));
	model = glm::translate(model, glm::vec3(0.0f, 2.25f, 0.0f));

	
	// time of animation 1.257156
	if (eggAnimating == true) { // handles animation
		animationCounter += deltaTime;
		model = glm::translate(model, glm::vec3(0.0f, (sin(animationCounter * 2.5) * 0.25f) + 0.01f, 0.0f));
		float rotateDegree = (360.0 / 1.2571) * animationCounter;
		model = glm::rotate(model, glm::radians(rotateDegree), glm::vec3(0.0f, 1.0f, 0.0f));
		if (((sin(animationCounter * 2.5) * 0.25f)) + 0.01f <= 0.01f) {
			eggAnimating = false;
			nextJump = 20.0f;
			animationCounter = 0.0f;

		}
	}



	tableShader.use();
	tableShader.setMat4("model", model);
	tableShader.setFloat("material.shininess", 16.0f);
	renderSphere();
	tableShader.setFloat("material.shininess", 32.0f);

	
}

void renderLamp(Shader lampShader, glm::vec3 pos, glm::vec3 scale, float angle, glm::vec3 axis, LampState state, int lampNum)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lampTexture);

	// scene graph implemented using nodes
	Node bulb = {
		"cube",
		glm::mat4(1.0f),
		{},
	};
	Node horn = {
		"sphere",
		glm::mat4(1.0f),
		{},
	};
	Node horn2 = {
		"sphere",
		glm::mat4(1.0f),
		{},
	};
	Node head = {
		"cube",
		glm::mat4(1.0f),
		{bulb, horn, horn2},
	};
	Node upperarm = {
		"cube",
		glm::mat4(1.0f),
		{head},
	};
	Node tail = {
		"sphere",
		glm::mat4(1.0f),
		{},
	};
	Node hinge = {
		"sphere",
		glm::mat4(1.0f),
		{tail, upperarm},
	};
	Node lowerarm = {
		"cube",
		glm::mat4(1.0f),
		{hinge},
	};
	Node base = {
		"cube",
		glm::mat4(1.0f),
		{lowerarm},
	};

	glm::vec3 baseScale = glm::vec3(0.5f, 0.125f, 0.5f) * scale;
	glm::vec3 lowerarmScale = glm::vec3(0.125f, 2.0f, 0.125f) * scale;
	glm::vec3 hingeScale = glm::vec3(0.5f, 0.5f, 0.5f) * scale;
	glm::vec3 tailScale = glm::vec3(1, 0.175f, 0.175f) * scale;
	glm::vec3 upperarmScale = glm::vec3(0.125f, 2.0f, 0.125f) * scale;
	glm::vec3 headScale = glm::vec3(0.40f, 0.25f, 0.25f) * scale;
	glm::vec3 bulbScale = glm::vec3(0.125f, 0.125f, 0.125f) * scale;
	glm::vec3 hornScale = glm::vec3(1.0f, 0.125f, 0.125f) * scale;

	if (state == Default) {

		// base setup
		base.updateRotate(glm::radians(angle), axis);
		base.updateTranslate(pos);
		base.updateScale(baseScale);

		// lower arm
		lowerarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / baseScale); //Always divide by parent scale
		lowerarm.updateScale(lowerarmScale / baseScale);

		// hinge
		hinge.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / lowerarmScale);
		hinge.updateScale(hingeScale / lowerarmScale);

		// tail 
		tail.updateTranslate((glm::vec3(-0.25f, -0.125f, 0.0f) * scale) / hingeScale);
		tail.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		tail.updateScale(tailScale / hingeScale);

		// upper arm
		upperarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / hingeScale);
		upperarm.updateScale(upperarmScale / hingeScale);

		// head
		head.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / upperarmScale);
		head.updateRotate(glm::radians(-4.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		head.updateScale(headScale / upperarmScale);

		// bulb 
		bulb.updateTranslate((glm::vec3(0.5f, 0.0f, 0.0f) * scale) / headScale);
		bulb.updateScale(bulbScale / headScale);

		// horn
		horn.updateTranslate((glm::vec3(0.5f, 0.2f, 0.0f) * scale) / headScale);
		horn.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn.updateScale(hornScale / headScale);

		// horn2
		horn2.updateTranslate((glm::vec3(0.0f, 0.25f, 0.0f) * scale) / headScale);
		horn2.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn2.updateScale(hornScale / headScale);

	}
	else if (state == Crouched1) {
		// base setup
		base.updateRotate(glm::radians(angle), axis);
		base.updateTranslate(pos);
		base.updateScale(baseScale);

		// lower arm
		lowerarm.updateRotate(glm::radians(4.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		lowerarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / baseScale); //Always divide by parent scale
		lowerarm.updateScale(lowerarmScale / baseScale);

		// hinge
		hinge.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / lowerarmScale);
		hinge.updateScale(hingeScale / lowerarmScale);

		// tail 
		tail.updateTranslate((glm::vec3(-0.25f, -0.125f, 0.0f) * scale) / hingeScale);
		tail.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		tail.updateScale(tailScale / hingeScale);

		// upper arm
		upperarm.updateRotate(glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		upperarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / hingeScale);
		upperarm.updateScale(upperarmScale / hingeScale);

		// head
		head.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / upperarmScale);
		head.updateScale(headScale / upperarmScale);

		// bulb 
		bulb.updateTranslate((glm::vec3(0.5f, 0.0f, 0.0f) * scale) / headScale);
		bulb.updateScale(bulbScale / headScale);

		// horn
		horn.updateTranslate((glm::vec3(0.5f, 0.2f, 0.0f) * scale) / headScale);
		horn.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn.updateScale(hornScale / headScale);

		// horn2
		horn2.updateTranslate((glm::vec3(0.0f, 0.25f, 0.0f) * scale) / headScale);
		horn2.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn2.updateScale(hornScale / headScale);
	}

	else if (state == Crouched2) {
		// base setup
		base.updateRotate(glm::radians(angle), axis);
		base.updateTranslate(pos);
		base.updateScale(baseScale);

		// lower arm
		lowerarm.updateRotate(glm::radians(8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		lowerarm.updateTranslate((glm::vec3(0.0f, 1.5f, 0.0f) * scale) / baseScale); //Always divide by parent scale
		lowerarm.updateScale(lowerarmScale / baseScale);

		// hinge
		hinge.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / lowerarmScale);
		hinge.updateScale(hingeScale / lowerarmScale);

		// tail 
		tail.updateTranslate((glm::vec3(-0.25f, -0.125f, 0.0f) * scale) / hingeScale);
		tail.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		tail.updateScale(tailScale / hingeScale);

		// upper arm
		upperarm.updateRotate(glm::radians(-60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		upperarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / hingeScale);
		upperarm.updateScale(upperarmScale / hingeScale);

		// head
		head.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / upperarmScale);
		head.updateRotate(glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		head.updateScale(headScale / upperarmScale);

		// bulb 
		bulb.updateTranslate((glm::vec3(0.5f, 0.0f, 0.0f) * scale) / headScale);
		bulb.updateScale(bulbScale / headScale);

		// horn
		horn.updateTranslate((glm::vec3(0.5f, 0.2f, 0.0f) * scale) / headScale);
		horn.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn.updateScale(hornScale / headScale);

		// horn2
		horn2.updateTranslate((glm::vec3(0.0f, 0.25f, 0.0f) * scale) / headScale);
		horn2.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn2.updateScale(hornScale / headScale);
	}

	else if (state == Other1) {
		// base setup
		base.updateRotate(glm::radians(angle), axis);
		base.updateTranslate(pos);
		base.updateScale(baseScale);

		// lower arm
		lowerarm.updateRotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lowerarm.updateRotate(glm::radians(-8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		lowerarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / baseScale); //Always divide by parent scale
		lowerarm.updateScale(lowerarmScale / baseScale);

		// hinge
		hinge.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / lowerarmScale);
		hinge.updateScale(hingeScale / lowerarmScale);

		// tail 
		tail.updateTranslate((glm::vec3(-0.25f, -0.125f, 0.0f) * scale) / hingeScale);
		tail.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		tail.updateScale(tailScale / hingeScale);

		// upper arm
		upperarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / hingeScale);
		upperarm.updateScale(upperarmScale / hingeScale);

		// head
		head.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / upperarmScale);
		head.updateScale(headScale / upperarmScale);

		// bulb 
		bulb.updateTranslate((glm::vec3(0.5f, 0.0f, 0.0f) * scale) / headScale);
		bulb.updateScale(bulbScale / headScale);

		// horn
		horn.updateTranslate((glm::vec3(0.5f, 0.2f, 0.0f) * scale) / headScale);
		horn.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn.updateScale(hornScale / headScale);

		// horn2
		horn2.updateTranslate((glm::vec3(0.0f, 0.25f, 0.0f) * scale) / headScale);
		horn2.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn2.updateScale(hornScale / headScale);
	} 

	else if (state == Other2) {

		// base setup
		base.updateRotate(glm::radians(angle), axis);
		base.updateTranslate(pos);
		base.updateScale(baseScale);

		// lower arm
		lowerarm.updateRotate(glm::radians(-8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		lowerarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / baseScale); //Always divide by parent scale
		lowerarm.updateScale(lowerarmScale / baseScale);

		// hinge
		hinge.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / lowerarmScale);
		hinge.updateScale(hingeScale / lowerarmScale);

		// tail 
		tail.updateTranslate((glm::vec3(-0.25f, -0.125f, 0.0f) * scale) / hingeScale);
		tail.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		tail.updateScale(tailScale / hingeScale);

		// upper arm
		upperarm.updateRotate(glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		upperarm.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / hingeScale);
		upperarm.updateScale(upperarmScale / hingeScale);

		// head
		head.updateTranslate((glm::vec3(0.0f, 2.0f, 0.0f) * scale) / upperarmScale);
		head.updateRotate(glm::radians(-2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		head.updateScale(headScale / upperarmScale);

		// bulb 
		bulb.updateTranslate((glm::vec3(0.5f, 0.0f, 0.0f) * scale) / headScale);
		bulb.updateScale(bulbScale / headScale);

		// horn
		horn.updateTranslate((glm::vec3(0.5f, 0.2f, 0.0f) * scale) / headScale);
		horn.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn.updateScale(hornScale / headScale);

		// horn2
		horn2.updateTranslate((glm::vec3(0.0f, 0.25f, 0.0f) * scale) / headScale);
		horn2.updateRotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		horn2.updateScale(hornScale / headScale);

	}



	// handle lighting for bulb

	// 0.5f, 2.75f, 0.5f, 1.0f (base) 
	glm::vec4 eggBasePos = glm::vec4(0.5f, 3.0f, 0.0f, 1.0f);
	glm::vec4 bulbPos = bulb.model * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 bulbDir = (eggBasePos - bulbPos);

	if (state == Other1) {
		bulbDir = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	}

	lampShader.setVec3("spotLights[" + std::to_string(lampNum - 1) + "].position", glm::vec3(bulbPos.x, bulbPos.y, bulbPos.z));
	lampShader.setVec3("spotLights[" + std::to_string(lampNum - 1) + "].direction", bulbDir);
	lampShader.setVec3("spotLights[" + std::to_string(lampNum - 1) + "].ambient", 0.1f, 0.1f, 0.1f);
	lampShader.setVec3("spotLights[" + std::to_string(lampNum - 1) + "].diffuse", 1.0f, 1.0f, 1.0f);
	lampShader.setVec3("spotLights[" + std::to_string(lampNum - 1) + "].specular", 1.0f, 1.0f, 1.0f);
	lampShader.setFloat("spotLights[" + std::to_string(lampNum - 1) + "].constant", 1.0f);
	lampShader.setFloat("spotLights[" + std::to_string(lampNum - 1) + "].linear", 0.09f);
	lampShader.setFloat("spotLights[" + std::to_string(lampNum - 1) + "].quadratic", 0.032f);
	lampShader.setFloat("spotLights[" + std::to_string(lampNum - 1) + "].cutOff", glm::cos(glm::radians(12.5f)));
	lampShader.setFloat("spotLights[" + std::to_string(lampNum - 1) + "].outerCutOff", glm::cos(glm::radians(15.0f)));



	// render tree from root node which is base
	renderNode(lampShader, base);

}

void renderNode(Shader& shader, Node node)
{
	shader.setMat4("model", node.model);
	if (node.object == "cube") {
		renderCube();
	} else if (node.object == "sphere"){
		renderSphere();
	}

	if (node.children.size() != 0) {
		for (int i = 0; i < node.children.size(); i++) {
			renderNode(shader, node.children[i]);
		}
	}
	
}


void renderCube()
{
	float cubeVertices[] = {
		// vertex pos         // normal pos     // texture coords
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void renderSphere() {

	const  int XLONG = 30;
	const int YLAT = 30;

	double r = 0.5;
	const int step = 8;

	float vertices[XLONG * YLAT * step];

	for (int j = 0; j < YLAT; ++j) {
		double b = glm::radians(-90 + 180 * (double)(j) / (YLAT - 1));
		for (int i = 0; i < XLONG; ++i) {
			double a = glm::radians(360 * (double)(i) / (XLONG - 1));
			double z = glm::cos(b) * glm::cos(a);
			double x = glm::cos(b) * glm::sin(a);
			double y = glm::sin(b);
			int base = j * XLONG * step;
			vertices[base + i * step + 0] = (float)(r * x);
			vertices[base + i * step + 1] = (float)(r * y);
			vertices[base + i * step + 2] = (float)(r * z);

			vertices[base + i * step + 3] = (float)x;
			vertices[base + i * step + 4] = (float)y;
			vertices[base + i * step + 5] = (float)z;

			vertices[base + i * step + 6] = (float)(i) / (float)(XLONG - 1);
			vertices[base + i * step + 7] = (float)(j) / (float)(YLAT - 1);
		}
	}

	int indices[(XLONG - 1) * (YLAT - 1) * 6];
	for (int j = 0; j < YLAT - 1; ++j) {
		for (int i = 0; i < XLONG - 1; ++i) {
			int base = j * (XLONG - 1) * 6;
			indices[base + i * 6 + 0] = j * XLONG + i;
			indices[base + i * 6 + 1] = j * XLONG + i + 1;
			indices[base + i * 6 + 2] = (j + 1) * XLONG + i + 1;
			indices[base + i * 6 + 3] = j * XLONG + i;
			indices[base + i * 6 + 4] = (j + 1) * XLONG + i + 1;
			indices[base + i * 6 + 5] = (j + 1) * XLONG + i;
		}
	}
	
	unsigned int sphereVAO, sphereVBO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int sphereEBO;
	glGenBuffers(1, &sphereEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);   
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);

}


void processInput(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // change lamp 1 positions
		lamp2Key = true;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
		if (lamp2Key == true) {
			if (currentLamp2State == Default) {
				currentLamp2State = Crouched1;
			}
			else if (currentLamp2State == Crouched1) {
				currentLamp2State = Crouched2;
			}
			else {
				currentLamp2State = Default;
			}
			lamp2Key = false;
		}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) // change lamp 2 positions
		lamp1Key = true;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		if (lamp1Key == true) {
			if (currentLamp1State == Default) {
				currentLamp1State = Other1;
			}
			else if (currentLamp1State == Other1) {
				currentLamp1State = Other2;
			}
			else {
				currentLamp1State = Default;
			}
			lamp1Key = false;
		}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		dirLightKey = true;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
		if (dirLightKey == true) {
			if (dirLightOn == true) {
				dirLightOn = false;
			}
			else {
				dirLightOn = true;
			}
			dirLightKey = false;
		}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) //lamp 1 on/off
		lamp1OnKey = true;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
		if (lamp1OnKey == true) {
			if (lamp1On == true) {
				lamp1On = false;
			}
			else {
				lamp1On = true;
			}
			lamp1OnKey = false;
		}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) //lamp 2 on/off
		lamp2OnKey = true;
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
		if (lamp2OnKey == true) {
			if (lamp2On == true) {
				lamp2On = false;
			}
			else {
				lamp2On = true;
			}
			lamp2OnKey = false;
		}



}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

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

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_resize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

unsigned int loadTexture(char const* path)
{
	unsigned int ID;
	glGenTextures(1, &ID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum type;
		if (nrComponents == 1)
			type = GL_RED;
		else if (nrComponents == 3)
			type = GL_RGB;
		else if (nrComponents == 4)
			type = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);



		if (type == GL_RGBA) { // if alpha value found then change border options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture not found at path" << path << std::endl;
		stbi_image_free(data);
	}

	return ID;

}

unsigned int loadSkybox(std::vector<std::string> faces)
{
	unsigned int ID;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Skybox could not be loaded" << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return ID;
}