#pragma once

#include "lve_device.hpp"
#include <string>
#include <vector>
namespace lve {
    struct PipeLineConfigInfo {
        PipeLineConfigInfo(const PipeLineConfigInfo&) = delete;
        PipeLineConfigInfo& operator=(const PipeLineConfigInfo&) = delete;
        PipeLineConfigInfo() = default;

        //VkViewport viewport;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;

    };

    class LvePipeline {
    public:
        LvePipeline(LveDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipeLineConfigInfo& configInfo);
        ~LvePipeline();

        LvePipeline(const LvePipeline&) = delete;
        LvePipeline& operator=(const LvePipeline&) = delete;
        LvePipeline() = default;

        void bind(VkCommandBuffer(commandBuffer));
        static void defaultPipeLineConfigInfo(PipeLineConfigInfo& configInfo);
        static void enableAlphaBlending(PipeLineConfigInfo& configInfo);

    private:
        static std::vector<char> readFile(const std::string& filepath);
        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipeLineConfigInfo& configInfo);
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        LveDevice& lveDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}