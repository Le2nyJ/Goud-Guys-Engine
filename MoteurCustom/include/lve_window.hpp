#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INLCUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lve {
    class LveWindow {
    public:
        LveWindow(int w, int h, std::string name);
        ~LveWindow();

        LveWindow(const LveWindow&) = delete;
        LveWindow& operator=(const LveWindow&) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent() { return{ static_cast<uint32_t> (width), static_cast<uint32_t> (height) }; }
        bool wasWindowResized() { return frambufferResized; }
        void resetWindowResizedFlag() { frambufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);


    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool frambufferResized = false;

        std::string windowName;
        GLFWwindow* window;
    };
} //name space lve