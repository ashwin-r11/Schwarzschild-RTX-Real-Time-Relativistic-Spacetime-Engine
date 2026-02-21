#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

class Display {
    
    private:
        int window_width, window_height;
        std::string window_title;
        GLFWwindow* window;

        // OpenGL related variables
        uint32_t textureID;
        uint32_t pboID;
        uint32_t fboID;

    public:
        Display(int width, int height, const std::string& title) : window_width(width), window_height(height) {

            if(!glfwInit()) {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                return;
            }

            window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
            if (!window) {
                std::cerr << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return;
            }
            
            glfwMakeContextCurrent(window);

            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                return; 
            }

            // 1. Projector screen (texture)
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            // 2. Scaling filters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            // 3. Initialize Texture Storage (MUST happen before attaching to FBO)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // 4. Create FBO and attach the initialized texture
            glGenFramebuffers(1, &fboID);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // Unbind when done

            // 5. PBO
            glGenBuffers(1, &pboID);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);

            // Allocate memory in GPU for pixels
            glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(uint32_t), nullptr, GL_STREAM_DRAW);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }

        ~Display() {
            // Clean up the FBO as well!
            glDeleteFramebuffers(1, &fboID);
            glDeleteBuffers(1, &pboID);
            glDeleteTextures(1, &textureID);
            
            if (window) {
                glfwDestroyWindow(window);
            }
            glfwTerminate();
        }

        bool shouldClose() {
            return glfwWindowShouldClose(window);
        }

        void update(const uint32_t* pixels) {
            // Bind our PBO staging area
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);

            // Copy C++ array to the GPU buffer
            glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, window_width * window_height * sizeof(uint32_t), pixels);

            // Copy from Buffer to Texture
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT);

            // Blit from FBO to standard window screen
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 0 is the default window screen
            glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
};