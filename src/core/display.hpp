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

    // --- Shaders ---
    GLuint sceneProgram;     // blackhole.vert + blackhole.frag
    GLuint blurProgram;      // blackhole.vert + bloom_blur.frag
    GLuint compositeProgram; // blackhole.vert + bloom_final.frag

    // --- Full-screen quad ---
    GLuint quadVAO, quadVBO;

    // --- Framebuffers for bloom pipeline ---
    GLuint sceneFBO, sceneTexture;       // Scene renders here (HDR)
    GLuint pingFBO, pingTexture;         // Blur ping
    GLuint pongFBO, pongTexture;         // Blur pong

    // --- Bloom parameters ---
    int bloomIterations;
    float bloomStrength;
    float exposure;

    // --- Shader utilities ---
    GLuint compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

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

    GLuint linkProgram(GLuint vert, GLuint frag) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        int success;
        glGetProgramiv(prog, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(prog, 1024, nullptr, infoLog);
            std::cerr << "SHADER LINK ERROR:\n" << infoLog << std::endl;
        }
        return prog;
    }

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h) {
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);
        // Use RGBA16F for HDR storage
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resizeFBOs(int w, int h) {
        auto resizeTex = [](GLuint tex, int w, int h) {
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        };
        resizeTex(sceneTexture, w, h);
        resizeTex(pingTexture, w, h);
        resizeTex(pongTexture, w, h);
    }

public:
    Display(int width, int height, const std::string& title,
            const std::string& shaderDir)
        : window_width(width), window_height(height),
          bloomIterations(8), bloomStrength(0.15f), exposure(1.2f)
    {
        // --- GLFW Init ---
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }

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
        glfwSwapInterval(1);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int newW, int newH) {
            Display* self = static_cast<Display*>(glfwGetWindowUserPointer(w));
            if (self) {
                self->window_width = newW;
                self->window_height = newH;
                self->resizeFBOs(newW, newH);
            }
            glViewport(0, 0, newW, newH);
        });

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return;
        }

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;

        // --- Fullscreen quad ---
        float quadVerts[] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        // --- Create bloom FBOs (RGBA16F for HDR) ---
        createFBO(sceneFBO, sceneTexture, width, height);
        createFBO(pingFBO, pingTexture, width, height);
        createFBO(pongFBO, pongTexture, width, height);

        // --- Compile all shader programs ---
        std::string vertSrc = loadShaderFile(shaderDir + "/blackhole.vert");
        std::string fragScene = loadShaderFile(shaderDir + "/blackhole.frag");
        std::string fragBlur = loadShaderFile(shaderDir + "/bloom_blur.frag");
        std::string fragComp = loadShaderFile(shaderDir + "/bloom_final.frag");

        GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fScene = compileShader(GL_FRAGMENT_SHADER, fragScene);
        GLuint fBlur = compileShader(GL_FRAGMENT_SHADER, fragBlur);
        GLuint fComp = compileShader(GL_FRAGMENT_SHADER, fragComp);

        sceneProgram = linkProgram(vert, fScene);
        blurProgram = linkProgram(vert, fBlur);
        compositeProgram = linkProgram(vert, fComp);

        glDeleteShader(vert);
        glDeleteShader(fScene);
        glDeleteShader(fBlur);
        glDeleteShader(fComp);
    }

    ~Display() {
        glDeleteFramebuffers(1, &sceneFBO);
        glDeleteFramebuffers(1, &pingFBO);
        glDeleteFramebuffers(1, &pongFBO);
        glDeleteTextures(1, &sceneTexture);
        glDeleteTextures(1, &pingTexture);
        glDeleteTextures(1, &pongTexture);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteProgram(sceneProgram);
        glDeleteProgram(blurProgram);
        glDeleteProgram(compositeProgram);
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    }

    // --- Use the scene shader for setting uniforms ---
    void useSceneShader() {
        glUseProgram(sceneProgram);
    }

    // --- Set uniforms on the currently active program ---
    void setUniform1f(const char* name, float v) {
        glUniform1f(glGetUniformLocation(sceneProgram, name), v);
    }
    void setUniform2f(const char* name, float x, float y) {
        glUniform2f(glGetUniformLocation(sceneProgram, name), x, y);
    }
    void setUniform3f(const char* name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(sceneProgram, name), x, y, z);
    }

    // --- Full bloom render pipeline ---
    void draw() {
        // ===== PASS 1: Render black hole scene to HDR FBO =====
        glUseProgram(sceneProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ===== PASS 2: Gaussian blur (ping-pong) =====
        glUseProgram(blurProgram);
        bool horizontal = true;
        bool firstPass = true;

        for (int i = 0; i < bloomIterations * 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, horizontal ? pingFBO : pongFBO);
            glUniform1i(glGetUniformLocation(blurProgram, "uHorizontal"), horizontal);

            // First pass reads from scene; subsequent passes ping-pong
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, firstPass ? sceneTexture : (horizontal ? pongTexture : pingTexture));
            glUniform1i(glGetUniformLocation(blurProgram, "uImage"), 0);

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            horizontal = !horizontal;
            firstPass = false;
        }

        // ===== PASS 3: Composite scene + bloom to screen =====
        glUseProgram(compositeProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer (screen)
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind scene texture to unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
        glUniform1i(glGetUniformLocation(compositeProgram, "uScene"), 0);

        // Bind bloom (last blurred result) to unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, horizontal ? pongTexture : pingTexture);
        glUniform1i(glGetUniformLocation(compositeProgram, "uBloom"), 1);

        glUniform1f(glGetUniformLocation(compositeProgram, "uBloomStrength"), bloomStrength);
        glUniform1f(glGetUniformLocation(compositeProgram, "uExposure"), exposure);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    bool shouldClose() { return glfwWindowShouldClose(window); }
    bool isKeyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
    GLFWwindow* getWindow() { return window; }
    int getWidth() const { return window_width; }
    int getHeight() const { return window_height; }

    // --- Bloom tuning ---
    void setBloomStrength(float s) { bloomStrength = s; }
    void setBloomIterations(int n) { bloomIterations = n; }
    void setExposure(float e) { exposure = e; }
};