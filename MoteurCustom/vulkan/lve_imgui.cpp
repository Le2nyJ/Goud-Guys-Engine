#include "lve_imgui.hpp"

// glm
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// imgui
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "imgui_internal.h"

// std
#include <stdexcept>
#include <iostream>

namespace lve {
    static float f = 0.0f;
    glm::vec3 position(0.0f, 1.5f, 0.0f);
    glm::vec3 rotation(0.0f, 0.0f, 0.0f);
    glm::vec3 scale(0.5f, 0.5f, 0.5f);
    /// <summary>
    /// Il prend une référence à un objet LveWindow, LveDevice, et LveRenderer en paramètre.
    ///Initialise un pool de descripteurs pour ImGui.
    /// Appelle la fonction initImGui() pour initialiser ImGui
    /// </summary>
    /// <param name="window"></param>
    /// <param name="device"></param>
    /// <param name="renderer"></param>
    LveImgui::LveImgui(LveWindow& window, LveDevice& device, LveRenderer& renderer) : lveWindow{ window }, lveDevice{ device }, lveRenderer{ renderer } {
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        VkResult result = vkCreateDescriptorPool(lveDevice.getDevice(), &pool_info, nullptr, &imguiPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool for ImGui!");
        }

        initImGui();
    }
    /// <summary>
    /// Appelle les fonctions de fermeture de ImGui et libère les ressources associées, y compris le pool de descripteurs
    /// </summary>
    LveImgui::~LveImgui() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(lveDevice.getDevice(), imguiPool, nullptr);
    }
    /// <summary>
    /// Cette fonction est appelée pour rendre l'interface ImGui.
    ///Initialise un nouveau frame ImGui et rend le contenu.
    /// Appelle la fonction initInspector() pour initialiser une fenêtre d'inspection
    /// </summary>
    /// <param name="commandBuffer"></param>
    void LveImgui::renderImGui(VkCommandBuffer commandBuffer) {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::ShowDemoWindow();
        initInspector();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0);
    }
    /// <summary>
    /// Return the scale walue from the UI
    /// </summary>
    /// <param name="xyz"></param>
    /// <returns></returns>
    float LveImgui::getScaleSliderValue(int xyz) {
        return scale[xyz];
    }
    /// <summary>
    /// Return the scale walue from the UI
    /// </summary>
    /// <param name="xyz"></param>
    /// <returns></returns>
    float LveImgui::getRotationSliderValue(int xyz) {
        return rotation[xyz];
    }
    /// <summary>
    /// Return the scale walue from the UI
    /// </summary>
    /// <param name="xyz"></param>
    /// <returns></returns>
    float LveImgui::getPositionSliderValue(int xyz) {
        return position[xyz];
    }
    /// <summary>
    /// Initialise le contexte ImGui, configure le style, et initialise les backends pour GLFW et Vulkan.
    ///Crée la texture de polices ImGui
    /// </summary>
    void LveImgui::initImGui() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        io.ConfigDockingWithShift = false;
        io.ConfigDockingAlwaysTabBar = true;
        io.ConfigDockingTransparentPayload = true;

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(lveWindow.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = lveDevice.getInstance();
        init_info.PhysicalDevice = lveDevice.getPhysicalDevice();
        init_info.Device = lveDevice.getDevice();
        init_info.QueueFamily = lveDevice.getGraphicsQueueFamily();
        init_info.Queue = lveDevice.getGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imguiPool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.CheckVkResultFn = [](VkResult result) {
            if (result != VK_SUCCESS) {
                throw std::runtime_error("ImGui_ImplVulkan_Init failed !");
            }
        };
        ImGui_ImplVulkan_Init(&init_info, lveRenderer.getSwapChainRenderPass());

        //Upload Fonts
        ImGui_ImplVulkan_CreateFontsTexture();
    }
    /// <summary>
    /// Initialise une fenêtre d'inspection ImGui.
    /// Affiche la démo ImGui si l'indicateur show_demo_window est activé.
    /// Affiche un bouton "Create Cube" qui active la création d'un cube (cubecre est défini à true).
    /// Si cubecre est true, affiche les contrôles ImGui pour définir les propriétés du cube(myCube).
    /// Affiche un texte avec le temps moyen par frame et les FPS de l'application

    /// </summary>
    void LveImgui::initInspector() {
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        struct GameObject {
            glm::vec3 position;
            glm::vec3 rotation;
            glm::vec3 scale;
            glm::vec3 color;
        };

        GameObject myGameObject;                                // Créez une instance de l'objet de jeuGameObject myCube;
        GameObject myCube;


        ImGui::Begin("Inspecteur");
        ImGui::Text("Inspecteur ");                             // ecrire texte 

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        ImGui::Button("Create Cube");
        // Initialiser ou réinitialiser les paramètres du cube
        myCube = GameObject();
        float maxScaleValue = 10.0f;

        ImGui::InputScalarN("Translation", ImGuiDataType_Float, glm::value_ptr(position), 3, NULL, NULL, "%.3f");


        ImGui::Text("Rotation");
        ImGui::InputScalarN("Rotation", ImGuiDataType_Float, glm::value_ptr(rotation), 3, NULL, NULL, "%.3f");


        ImGui::Text("Scale");
        ImGui::InputScalarN("Scale", ImGuiDataType_Float, glm::value_ptr(scale), 3, NULL, NULL, "%.3f");


        ImGui::ColorEdit3("Color", glm::value_ptr(myGameObject.color));
        scale = glm::clamp(scale, glm::vec3(0.0f), glm::vec3(maxScaleValue));

        //compteur fps
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


        ImGui::End();
    }
}