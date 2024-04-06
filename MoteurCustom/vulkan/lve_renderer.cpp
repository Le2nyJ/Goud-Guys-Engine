#include "lve_renderer.hpp"

#include <stdexcept>
#include <array>
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>


namespace lve {
    /// <summary>
    /// Prend une r�f�rence � un objet LveWindow et un objet LveDevice en param�tres.
    ///Appelle les fonctions recreateSwapChain et createCommandBuffers pour initialiser le rendu
    /// </summary>
    /// <param name="window"></param>
    /// <param name="device"></param>
    LveRenderer::LveRenderer(LveWindow& window, LveDevice& device) : lveWindow{ window }, lveDevice{ device } {
        recreateSwapChain();
        createCommandBuffers();

    }
    /// <summary>
    /// Appelle la fonction freeCommandBuffers pour lib�rer les tampons de commandes
    /// </summary>
    LveRenderer::~LveRenderer() {
        freeCommandBuffers();
    }
    /// <summary>
    ///Obtient la taille de la fen�tre et attend que la taille ne soit pas nulle.
    ///Attend que le p�riph�rique Vulkan termine les op�rations en cours.
    ///Recr�e ou initialise la cha�ne d'�change Vulkan (LveSwapChain)
    /// </summary>
    void LveRenderer::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.getDevice());
        //lveSwapChain = nullptr;
        if (lveSwapChain == nullptr) {
            lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
        } else {
            std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
            lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has cganged!");
            }

        }
    }
    /// <summary>
    /// Alloue les tampons de commandes n�cessaires pour l'ex�cution des commandes GPU.
    ///Utilise la classe LveSwapChain pour d�terminer le nombre maximal de tampons de commandes en vol
    /// </summary>
    void LveRenderer::createCommandBuffers() {
        commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    /// <summary>
    /// Lib�re les tampons de commandes Vulkan
    /// </summary>
    void LveRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(lveDevice.getDevice(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }
    /// <summary>
    /// Acquiert l'image suivante de la cha�ne d'�change.
    ///V�rifie si la cha�ne d'�change a besoin d'�tre recr��e en cas de redimensionnement de la fen�tre.
    ///  Commence l'enregistrement des commandes pour le tampon de commande actuel
    /// </summary>
    /// <returns></returns>
    VkCommandBuffer LveRenderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");
        auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image !");
        }
        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }
    /// <summary>
    /// Termine l'enregistrement des commandes pour le tampon de commande actuel.
    /// Soumet les commandes au GPU via la cha�ne d'�change.
    /// V�rifie si la cha�ne d'�change doit �tre recr��e en cas de redimensionnement de la fen�tre
    /// </summary>
    void LveRenderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer !");
        }
        auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image !");
        }
        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    }
    /// <summary>
    /// Commence une passe de rendu pour la cha�ne d'�change actuelle.
    ///Configure les param�tres de la passe de rendu, tels que la couleur de fond
    /// </summary>
    /// <param name="commandBuffer"></param>
    void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can begin render pass on command buffer from a different frame");
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, lveSwapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    /// <summary>
    /// Termine la passe de rendu pour la cha�ne d'�change actuelle
    /// </summary>
    /// <param name="commandBuffer"></param>
    void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }
}