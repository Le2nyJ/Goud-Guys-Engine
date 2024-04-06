#include "lve_window.hpp"

#include <stdexcept>


namespace lve {
    /// <summary>
    /// Constructeur initialisant la fen�tre avec une largeur, une hauteur et un nom sp�cifi�s
    /// </summary>
    LveWindow::LveWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
        initWindow();
    }

    /// <summary>
    /// Destructeur
    /// </summary>
    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    /// <summary>
    /// Initialise la fen�tre GLFW avec les param�tres appropri�s.
    /// </summary>
    void LveWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    /// <summary>
    /// Cr�e la surface de la fen�tre pour Vulkan.
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="surface"></param>
    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("faile to create window surface");
        }
    }

    /// <summary>
    /// Callback appel� lors du redimensionnement de la fen�tre.
    /// </summary>
    /// <param name="window"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void LveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
        lveWindow->frambufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }
}// namespace lve
