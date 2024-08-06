#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <cstdio>
#include <cstdlib>

static const char* shaderCodeVertex = R"(

	#version 460 core

	layout(std140, binding = 0) uniform PerFrameData {
		uniform mat4 MVP;
		uniform int isWireframe;
	};

	layout(location = 0) out vec3 color;

	const vec3 pos[8] = vec3[8](
		vec3(-1.0,-1.0, 1.0),
		vec3( 1.0,-1.0, 1.0),
		vec3( 1.0, 1.0, 1.0),
		vec3(-1.0, 1.0, 1.0),

		vec3(-1.0,-1.0,-1.0),
		vec3( 1.0,-1.0,-1.0),
		vec3( 1.0, 1.0,-1.0),
		vec3(-1.0, 1.0,-1.0)
	);

	const vec3 col[8] = vec3[8](
		vec3( 1.0, 0.0, 0.0),
		vec3( 0.0, 1.0, 0.0),
		vec3( 0.0, 0.0, 1.0),
		vec3( 1.0, 1.0, 0.0),

		vec3( 1.0, 1.0, 0.0),
		vec3( 0.0, 0.0, 1.0),
		vec3( 0.0, 1.0, 0.0),
		vec3( 1.0, 0.0, 0.0)
	);

	const int indices[36] = int[36](
		// front
		0, 1, 2, 2, 3, 0,
		// right
		1, 5, 6, 6, 2, 1,
		// back
		7, 6, 5, 5, 4, 7,
		// left
		4, 0, 3, 3, 7, 4,
		// bottom
		4, 5, 1, 1, 0, 4,
		// top
		3, 2, 6, 6, 7, 3
	);

	void main() {
		int idx = indices[gl_VertexID];
		gl_Position = MVP * vec4(pos[idx], 1.0);
		color = isWireframe > 0 ? vec3(0.0) : col[idx];
	}

)";

static const char* shaderCodeFragment = R"(

	#version 460 core

	layout(location = 0) in vec3 color;
	layout(location = 0) out vec4 out_FragColor;

	void main() {
		out_FragColor = vec4(color, 1.0);
	}

)";

struct PerFrameData {
	glm::mat4 mvp;
	int isWireframe;
};

int main()
{
	// GLFW SETUP

	glfwSetErrorCallback(
		[](int error, const char* description) {
			fprintf(stderr, "Error: %s\n", description);
		});

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(
		GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Simple example", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE); 
	}

	glfwSetKeyCallback(window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GLFW_TRUE);
		});

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow* window, int width, int height) {
			glViewport(0, 0, width, height);
		});

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

	// VAO

	GLuint VAO;
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// UNIFORMS

	const GLsizeiptr kBufferSize = sizeof(PerFrameData);
	GLuint perFrameDataBuf;
	glCreateBuffers(1, &perFrameDataBuf);
	glNamedBufferStorage(perFrameDataBuf, kBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, 0, kBufferSize);

	// SHADERS

	const GLuint shaderVertex =
		glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(
		shaderVertex, 1, &shaderCodeVertex, nullptr);
	glCompileShader(shaderVertex);

	const GLuint shaderFragment =
		glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(
		shaderFragment, 1, &shaderCodeFragment, nullptr);
	glCompileShader(shaderFragment);

	const GLuint program = glCreateProgram();
	glAttachShader(program, shaderVertex);
	glAttachShader(program, shaderFragment);
	glLinkProgram(program);
	glUseProgram(program);

	glDeleteShader(shaderVertex);
	glDeleteShader(shaderFragment);
	
	// FINAL SETUP

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	// LOOP

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int width;
		int height;
		glfwGetWindowSize(window, &width, &height);

		const float ratio = width / (float)height;
		
		const glm::mat4 m = glm::rotate(
			glm::translate(
				glm::mat4{ 1.0f },
				glm::vec3{ 0.0f, 0.0f, -3.5f }),
			(float)glfwGetTime(), 
			glm::vec3{ 1.0f, 1.0f, 1.0f });

		const glm::mat4 p = glm::perspective(
			45.0f, ratio, 0.1f, 1000.0f); 

		PerFrameData perFrameData = {
			.mvp = p * m,
			.isWireframe = false
		};

		glNamedBufferSubData(
			perFrameDataBuf, 0, kBufferSize, &perFrameData);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		perFrameData.isWireframe = true;
		glNamedBufferSubData(
			perFrameDataBuf, 0, kBufferSize, &perFrameData);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(program);
	glDeleteVertexArrays(1, &VAO);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}