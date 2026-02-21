#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Display {

private:
    int window_width, window_height;
    GLFWwindow* window;

    // Shader program
    GLuint shaderProgram;

    // Full-screen quad
    GLuint quadVAO, quadVBO;

    // --- Shader compilation utility ---
    GLuint compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        // Check for errors
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER COMPILE ERROR ("
                      << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                      << "):\n" << infoLog << std::endl;
            return 0;
        }
        return shader;
    }

    std::string loadShaderFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot open shader file: " << filepath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

public:
    Display(int width, int height, const std::string& title,
            const std::string& vertPath, const std::string& fragPath)
        : window_width(width), window_height(height) {

        // --- GLFW Init ---
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }

        // Request OpenGL 3.3 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // VSync on — prevents GPU from overworking

        // Store 'this' pointer so the resize callback can update our dimensions
        glfwSetWindowUserPointer(window, this);

        // Framebuffer resize callback — keeps viewport matched to window size
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int newW, int newH) {
            Display* self = static_cast<Display*>(glfwGetWindowUserPointer(w));
            if (self) {
                self->window_width = newW;
                self->window_height = newH;
            }
            glViewport(0, 0, newW, newH);
        });

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return;
        }

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;

        // --- Build full-screen quad ---
        // Two triangles covering [-1,1] in NDC with UVs [0,1]
        float quadVertices[] = {
            // pos.x  pos.y   uv.x  uv.y
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f,

            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f,
            -1.0f,  1.0f,   0.0f, 1.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Position attribute (location = 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // UV attribute (location = 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        // --- Compile & link shaders ---
        std::string vertSrc = loadShaderFile(vertPath);
        std::string fragSrc = loadShaderFile(fragPath);

        if (vertSrc.empty() || fragSrc.empty()) {
            std::cerr << "Failed to load shader files!" << std::endl;
            return;
        }

        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);

        if (!vertShader || !fragShader) {
            std::cerr << "Shader compilation failed! Aborting." << std::endl;
            return;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertShader);
        glAttachShader(shaderProgram, fragShader);
        glLinkProgram(shaderProgram);

        // Check linking
        int success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
            std::cerr << "SHADER LINK ERROR:\n" << infoLog << std::endl;
        }

        // Clean up individual shaders (they're now in the program)
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        // Activate the shader program
        glUseProgram(shaderProgram);
    }

    ~Display() {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteProgram(shaderProgram);

        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    // --- Uniform setters ---
    void setUniform1f(const char* name, float v) {
        glUniform1f(glGetUniformLocation(shaderProgram, name), v);
    }

    void setUniform2f(const char* name, float x, float y) {
        glUniform2f(glGetUniformLocation(shaderProgram, name), x, y);
    }

    void setUniform3f(const char* name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(shaderProgram, name), x, y, z);
    }

    // --- Draw the full-screen quad (triggers fragment shader on every pixel) ---
    void draw() {
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    bool shouldClose() {
        return glfwWindowShouldClose(window);
    }

    bool isKeyPressed(int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    }

    GLFWwindow* getWindow() { return window; }
    int getWidth() const { return window_width; }
    int getHeight() const { return window_height; }
};