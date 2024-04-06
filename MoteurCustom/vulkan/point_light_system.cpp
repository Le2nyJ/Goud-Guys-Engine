#include "point_light_system.hpp"

#include <stdexcept>
#include <array>
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define MS_PER_UPDATE 0.016 // 1/60

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <map>

namespace lve {
    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    /// <summary>
    /// Constructeur initialisant le système avec un périphérique logique, un passe de rendu Vulkan, et une mise en page de descripteurs globaux.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="renderPass"></param>
    /// <param name="globalSetLayout"></param>
    PointLightSystem::PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    /// <summary>
    /// Destructeur
    /// </summary>
    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(lveDevice.getDevice(), pipelineLayout, nullptr);
    }

    /// <summary>
    /// Crée la mise en page du pipeline.
    /// </summary>
    /// <param name="globalSetLayout"></param>
    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

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
    /// Retourne le temps actuel de la machine
    /// </summary>
    /// <returns></returns>
    double PointLightSystem::getCurrentTime() {
        auto current_time = std::chrono::system_clock::now();
        auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
        return duration_in_seconds.count();
    }

    /// <summary>
    /// Met à jour les informations sur les lumières à chaque frame.
    /// </summary>
    /// <param name="frameInfo"></param>
    /// <param name="ubo"></param>
    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
        auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, { 0.f, -1.f, 0.f });

        int lightIndex = 0;
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");
            // update light positions
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));


            // copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }

    /// <summary>
    /// Crée le pipeline graphique pour le système de lumières ponctuelles.
    /// </summary>
    /// <param name="renderPass"></param>
    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline pipeline before pipeline layout");

        PipeLineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipeLineConfigInfo(pipelineConfig);
        LvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "./shaders/SPIR-V/point_light.vert.spv", "./shaders/SPIR-V/point_light.frag.spv", pipelineConfig);
    }

    /// <summary>
    /// Effectue le rendu des lumières ponctuelles.
    /// </summary>
    /// <param name="frameInfo"></param>
    void PointLightSystem::render(FrameInfo& frameInfo) {
        // sort lights
        std::map<float, LveGameObject::id_t> sorted;
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            // calculate distance
            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getId();
        }
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        // iterate through sorted lights in reverse order
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
            // use game obj id to find light object
            auto& obj = frameInfo.gameObjects.at(it->second);

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}