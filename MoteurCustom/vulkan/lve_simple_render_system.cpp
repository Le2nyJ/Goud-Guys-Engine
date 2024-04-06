#include "lve_simple_render_system.hpp"

#include <stdexcept>
#include <array>
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define MS_PER_UPDATE 0.016 // 1/60

namespace lve {
    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };
    /// <summary>
    /// Prend une référence à un objet LveDevice, un VkRenderPass et un VkDescriptorSetLayout en paramètres.
    ///Appelle la fonction createPipelineLayout pour créer la mise en page du pipeline.
    ///   Appelle la fonction createPipeline pour créer le pipeline de rendu
    /// </summary>
    /// <param name="device"></param>
    /// <param name="renderPass"></param>
    /// <param name="globalSetLayout"></param>
    SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
    /// <summary>
    /// Détruit le pipeline layout Vulkan
    /// </summary>
    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.getDevice(), pipelineLayout, nullptr);
    }
    /// <summary>
    /// Crée la mise en page du pipeline Vulkan (pipelineLayout).
    ///Utilise un VkPushConstantRange pour spécifier des données pouvant être modifiées entre les trames.
    /// Utilise un ensemble de descripteurs global(globalSetLayout).
    /// </summary>
    /// <param name="globalSetLayout"></param>
    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutinfo{};
        pipelineLayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutinfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());;
        pipelineLayoutinfo.pSetLayouts = descriptorSetLayouts.data();;
        pipelineLayoutinfo.pushConstantRangeCount = 1;
        pipelineLayoutinfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.getDevice(), &pipelineLayoutinfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    /// <summary>
    /// Retourne le temps actuel de la machine en secondes
    /// </summary>
    /// <returns></returns>
    double SimpleRenderSystem::getCurrentTime() {
        auto current_time = std::chrono::system_clock::now();
        auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
        return duration_in_seconds.count();
    }
    /// <summary>
    /// Crée le pipeline de rendu (lvePipeline).
    ///Utilise la configuration du pipeline Vulkan(PipeLineConfigInfo).
    /// Utilise les shaders vertex et fragment spécifiés
    /// </summary>
    /// <param name="renderPass"></param>
    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline pipeline before pipeline layout");

        PipeLineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipeLineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "./shaders/SPIR-V/simple_shader.vert.spv", "./shaders/SPIR-V/simple_shader.frag.spv", pipelineConfig);
    }
    /// <summary>
    /// Lie le pipeline de rendu et les ensembles de descripteurs.
    ///Itère sur les objets de jeu dans frameInfo.gameObjects.
    ///    Pour chaque objet :
    ///Met à jour les constantes de poussée(push constants) avec la transformation actuelle de l'objet.
    ///   Lie le modèle de l'objet et déclenche le dessin
    /// </summary>
    /// <param name="frameInfo"></param>
    void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.model == nullptr) continue;
            //obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
            //obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

}