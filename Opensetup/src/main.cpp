#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <vector>
#include <iostream>

class CatmullRomSpline {
private:
	const std::vector<float> controlPoints = {
		0.0f, 0.0f,  // First point
		1.0f, 1.0f,  // Second point
		2.0f, 3.0f,  // Third point
		5.0f, 1.0f,  // Fourth point
		7.0f, 8.0f   // Fifth point
	};

	const float EXTRUSION_WIDTH = 10.0f;
	const float TEXTURE_REPEAT = 5.0f;

	const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 projection;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = projection * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

	const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D texture1;
        uniform vec3 color;
        uniform bool useTexture;
        
        void main() {
            if(useTexture)
                FragColor = texture(texture1, TexCoord);
            else
                FragColor = vec4(color, 1.0);
        }
    )";

	unsigned int shaderProgram, VBO, VAO, EBO;
	unsigned int pointVAO, pointVBO;
	unsigned int texture;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec2> splinePoints;
	std::vector<float> scaledControlPoints;

	float catmullRom(float p0, float p1, float p2, float p3, float t) {
		float t2 = t * t;
		float t3 = t2 * t;
		return 0.5f * (
			(2.0f * p1) +
			(-p0 + p2) * t +
			(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
			(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
			);
	}

	std::vector<float> getExtendedControlPoints() {
		std::vector<float> extended;

		float vx0 = controlPoints[0] - (controlPoints[2] - controlPoints[0]);
		float vy0 = controlPoints[1] - (controlPoints[3] - controlPoints[1]);
		extended.push_back(vx0);
		extended.push_back(vy0);

		extended.insert(extended.end(), controlPoints.begin(), controlPoints.end());

		float vxn = controlPoints[controlPoints.size() - 2] +
			(controlPoints[controlPoints.size() - 2] - controlPoints[controlPoints.size() - 4]);
		float vyn = controlPoints[controlPoints.size() - 1] +
			(controlPoints[controlPoints.size() - 1] - controlPoints[controlPoints.size() - 3]);
		extended.push_back(vxn);
		extended.push_back(vyn);

		return extended;
	}
	void loadTexture() {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load("resources/textures/wood.jpeg",
			&width, &height, &nrChannels, 0);

		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}

	void generateExtrudedGeometry() {
		vertices.clear();
		indices.clear();

		float totalLength = 0.0f;
		std::vector<float> segmentLengths;

		// Calculate total spline length for texture mapping
		for (size_t i = 1; i < splinePoints.size(); ++i) {
			float length = glm::length(splinePoints[i] - splinePoints[i - 1]);
			totalLength += length;
			segmentLengths.push_back(length);
		}

		// Generate vertices and texture coordinates
		float currentLength = 0.0f;
		for (size_t i = 0; i < splinePoints.size(); ++i) {
			glm::vec2 direction;
			if (i < splinePoints.size() - 1) {
				direction = glm::normalize(splinePoints[i + 1] - splinePoints[i]);
			}
			else {
				direction = glm::normalize(splinePoints[i] - splinePoints[i - 1]);
			}

			glm::vec2 normal(-direction.y, direction.x);
			normal *= EXTRUSION_WIDTH;

			// Left vertex
			vertices.push_back(splinePoints[i].x - normal.x);
			vertices.push_back(splinePoints[i].y - normal.y);
			vertices.push_back(0.0f);
			vertices.push_back(currentLength / totalLength * TEXTURE_REPEAT);
			vertices.push_back(0.0f);

			// Right vertex
			vertices.push_back(splinePoints[i].x + normal.x);
			vertices.push_back(splinePoints[i].y + normal.y);
			vertices.push_back(0.0f);
			vertices.push_back(currentLength / totalLength * TEXTURE_REPEAT);
			vertices.push_back(1.0f);

			if (i < splinePoints.size() - 1) {
				currentLength += segmentLengths[i];
			}
		}

		// Generate indices for triangle strip
		for (unsigned int i = 0; i < splinePoints.size() - 1; ++i) {
			indices.push_back(i * 2);
			indices.push_back(i * 2 + 1);
			indices.push_back(i * 2 + 2);
			indices.push_back(i * 2 + 1);
			indices.push_back(i * 2 + 2);
			indices.push_back(i * 2 + 3);
		}
	}
	public:
		CatmullRomSpline() {
			// Shader compilation
			unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glCompileShader(vertexShader);

			int success;
			char infoLog[512];
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				std::cout << "Vertex shader compilation failed:\n" << infoLog << std::endl;
			}

			unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			glCompileShader(fragmentShader);

			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				std::cout << "Fragment shader compilation failed:\n" << infoLog << std::endl;
			}

			shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);
			glLinkProgram(shaderProgram);

			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
				std::cout << "Shader program linking failed:\n" << infoLog << std::endl;
			}

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			loadTexture();
		}

		void generateSpline() {
			splinePoints.clear();
			std::vector<float> extendedPoints = getExtendedControlPoints();

			for (size_t i = 0; i < extendedPoints.size() / 2 - 3; ++i) {
				for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
					float x = catmullRom(
						extendedPoints[i * 2],
						extendedPoints[(i + 1) * 2],
						extendedPoints[(i + 2) * 2],
						extendedPoints[(i + 3) * 2],
						t
					);
					float y = catmullRom(
						extendedPoints[i * 2 + 1],
						extendedPoints[(i + 1) * 2 + 1],
						extendedPoints[(i + 2) * 2 + 1],
						extendedPoints[(i + 3) * 2 + 1],
						t
					);
					splinePoints.push_back(glm::vec2(x * 100.0f, y * 100.0f));
				}
			}

			generateExtrudedGeometry();

			// Setup VAO, VBO, and EBO
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
				vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
				indices.data(), GL_STATIC_DRAW);

			// Position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// Texture attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
				(void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		void draw(int width, int height) {
			glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width),
				static_cast<float>(height), 0.0f, -1.0f, 1.0f);

			glUseProgram(shaderProgram);

			unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			// Draw textured geometry
			glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO); 
			
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

			// Draw control points
		    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
		    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
			glPointSize(8.0f);
			glBindVertexArray(pointVAO);
			glDrawArrays(GL_POINTS, 0, scaledControlPoints.size() / 2);
		}

		~CatmullRomSpline() {
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteVertexArrays(1, &pointVAO);
			glDeleteBuffers(1, &pointVBO);
			glDeleteTextures(1, &texture);
			glDeleteProgram(shaderProgram);
		}
};

// Main function and window management code remains the same as before
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Textured Spline", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	CatmullRomSpline spline;
	spline.generateSpline();

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		spline.draw(width, height);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

