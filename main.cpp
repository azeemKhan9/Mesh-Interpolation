#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/GLEW.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "SOIL2/SOIL2.h"
#include "meshLoader.h"
#include "meshLoader2D.h"

#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

const GLuint WIDTH = 1000, HEIGHT = 600;

int main()
{
	glfwInit();

	//Uncomment these if on Mac OSX
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Linear Interpolation", nullptr, nullptr);

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight); //so size of window is gauged depending on size of screen

	if (nullptr == window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialise GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader ourShader("verShader.txt", "fragShader.txt"); //Build and compile shader program

	std::vector<glm::vec3> vertices, goal_vertices, normals, goal_normals, next_vertices, next_normals, final_vertices, final_normals;
	std::vector<unsigned int> indices, goal_indices, next_indices, final_indices;
	std::vector<glm::vec2> uvs, goal_uvs, next_uvs, final_uvs;
	//Read the mesh file and output the data as array of vertices
	bool res = meshLoader("tinysphere.obj", vertices, indices, uvs, normals);
	bool res2 = meshLoader("sphere.obj", goal_vertices, goal_indices, goal_uvs, goal_normals);
	bool res3 = meshLoader("spike.obj", next_vertices, next_indices, next_uvs, next_normals);
	bool res4 = meshLoader("ellipsoid.obj", final_vertices, final_indices, final_uvs, final_normals);
	//bool res = meshLoader2D("man.obj", vertices);
	//bool res2 = meshLoader2D("stretch.obj", goal_vertices);

	if (vertices.size() != goal_vertices.size()) {
		std::cout << "Meshes are not same size!" << std::endl;
		return false;
	}


	//Load and create texture
	GLuint texture;
	int texWidth, texHeight;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char *image = SOIL_load_image("texture_42.jpg", &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Attribute pointers
	GLuint VBO, VAO;
	/*glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);*/

	float t = 0.0f;
	int count = 0;
	std::vector<glm::vec3> init_vertices, sphere_vertices;
	bool bla = meshLoader("tinysphere.obj", init_vertices, indices, uvs, normals);
	bool bla2 = meshLoader("sphere.obj", sphere_vertices, indices, uvs, normals);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);

		std::vector<glm::vec3> interp;
		for (int i = 0; i < vertices.size(); i++) {
			interp.push_back((1 - t)*vertices[i] + t*goal_vertices[i]);
		}
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, interp.size() * sizeof(glm::vec3), &interp[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		if (t >= 1.0f) {
			t = 0.0f;
			count += 1;
			switch (count)
			{
			case 1:
				std::swap(vertices, goal_vertices);
				goal_vertices = next_vertices;
				break;
			case 2:
				vertices = next_vertices;
				goal_vertices = final_vertices;
				break;
			case 3:
				vertices = final_vertices;
				goal_vertices = init_vertices;
				break;
			case 4:
				count = 0;
				vertices = init_vertices;
				goal_vertices = sphere_vertices;
				break;
			}
		}
		else {
			t = t + 0.01f;
		}

		//draw opengl stuff
		ourShader.Use();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Uncomment if just want solid shape
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 7, 0));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		/*transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, (GLfloat)glfwGetTime() * 3.0f, glm::vec3(0.0f, 0.0f, 1.0f)); //Axis of rotation
		GLint transformLocation = glGetUniformLocation(ourShader.Program, "transform");
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));*/

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	//Terminate GLFW to clear any resources allocated by it
	glfwTerminate();
	return EXIT_SUCCESS;
}