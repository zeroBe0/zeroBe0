#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// 时间设定
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// 初始化、配置
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 取消注释此语句以修复OS X上的编译
#endif

	// 创建窗口
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 捕捉鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad:加载所有OpenGL函数指针
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 配置全局opengl状态
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// 构建着色器
	Shader Bodyshader("body.vs", "body.fs");
	Shader Headshader("head.vs", "head.fs");
	Shader Handshader("hand.vs", "hand.fs");
	Shader Legshader("leg.vs", "leg.fs");

	Model Body("body.obj");
	Model Head("head.obj");
	Model Hand("hand.obj");
	Model Leg("leg.obj");

	Shader skyboxShader("skybox.vs", "skybox.fs");


	float skyboxVertices[] = {          
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


	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	vector<std::string> faces
	{
		("right.jpg"),
		("left.jpg"),
		("top.jpg"),
		("bottom.jpg"),
		("front.jpg"),
		("back.jpg")
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// 着色器配置
	// --------------------
	Bodyshader.use();
	Bodyshader.setInt("skybox", 0);

	Headshader.use();
	Headshader.setInt("skybox", 0);

	Handshader.use();
	Handshader.setInt("skybox", 0);

	Legshader.use();
	Legshader.setInt("skybox", 0);
	

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// 渲染循环
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		
		processInput(window);

		
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 身体
		Bodyshader.use();
		glm::mat4 view = camera.GetViewMatrix(); 
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		Bodyshader.setMat4("view", view);
		Bodyshader.setMat4("projection", projection);
		Bodyshader.setVec3("cameraPos", camera.Position);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); //移至中心
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// 缩小
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, -1.0f, 0.0f));//随时间变化旋转

		Bodyshader.setMat4("model", model);
		Body.Draw(Bodyshader);//

		//头
		Headshader.use();

		glm::mat4 Hview = camera.GetViewMatrix();
		glm::mat4 Hprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		Headshader.setMat4("Hview", Hview);
		Headshader.setMat4("Hprojection", Hprojection);


		glm::mat4 Hmodel = glm::mat4(1.0f);
		Hmodel = glm::translate(Hmodel, glm::vec3(0.0f, -0.75f+ sin((float)glfwGetTime()), 0.0f));
		Hmodel = glm::scale(Hmodel, glm::vec3(0.2f, 0.2f, 0.2f));	

		Hmodel = glm::rotate(Hmodel, 0.3f*3.14f*sin((float)glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		Headshader.setMat4("Hmodel", Hmodel);
		Head.Draw(Headshader);

		//手
		Handshader.use();
		glm::mat4 hview = camera.GetViewMatrix();
		glm::mat4 hprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		Handshader.setMat4("hview", hview);
		Handshader.setMat4("hprojection", hprojection);

		glm::mat4 hmodel = glm::mat4(1.0f);
		hmodel = glm::translate(hmodel, glm::vec3(sin((float)glfwGetTime()), -1.75f, 0.0f));
		hmodel = glm::scale(hmodel, glm::vec3(0.2f, 0.2f, 0.2f));
		//旋转
		hmodel = glm::rotate(hmodel, (float)glfwGetTime(), glm::vec3(0.0f, -1.0f, 0.0f));
		Handshader.setMat4("hmodel", hmodel);
		Hand.Draw(Handshader);

		//腿
		Legshader.use();
		glm::mat4 lview = camera.GetViewMatrix();
		glm::mat4 lprojection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		Legshader.setMat4("lview", lview);
		Legshader.setMat4("lprojection", lprojection);

		glm::mat4 lmodel = glm::mat4(1.0f);
		lmodel = glm::translate(lmodel, glm::vec3(0.0f, -1.75f, sin((float)glfwGetTime())));
		lmodel = glm::scale(lmodel, glm::vec3(0.2f, 0.2f, 0.2f));
		//旋转
		lmodel = glm::rotate(lmodel, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, -0.0f));
		Legshader.setMat4("lmodel", lmodel);
		Leg.Draw(Legshader);

		// 绘画天空盒
		glDepthFunc(GL_LEQUAL);  // 更改深度函数，以便深度测试在值等于深度缓冲区的内容时通过
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 从视图矩阵中使得东西在立方体贴图前面
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // 将深度功能设置回默认值

		// glfw:交换缓冲区和轮询IO事件（按下/释放按键，移动鼠标等）
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 删除释放空间
	//glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	//glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &skyboxVAO);

	glfwTerminate();
	return 0;
}

// 输入函数，键盘控制
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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
}

// glfw：每当窗口大小改变时，此回调函数就会执行
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw:鼠标控制
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: 鼠标滑轮控制
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// 用于从文件加载2D纹理
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// 用6个不同的纹理加载天空盒纹理
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}