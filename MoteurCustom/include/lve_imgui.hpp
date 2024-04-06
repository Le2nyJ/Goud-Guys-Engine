#pragma once

#include "lve_window.hpp"
#include "lve_swap_chain.hpp"
#include "lve_renderer.hpp"

namespace lve {
    class LveImgui {
    public:
        LveImgui(LveWindow& window, LveDevice& device, LveRenderer& renderer);
        ~LveImgui();

        LveImgui(const LveImgui&) = delete;
        LveImgui& operator=(const LveImgui&) = delete;

        void renderImGui(VkCommandBuffer commandBuffer);
        float getScaleSliderValue(int xyz);
        float getRotationSliderValue(int xyz);
        float getPositionSliderValue(int xyz);


    private:
        void initImGui();
        void initInspector();

        LveWindow& lveWindow;
        LveDevice& lveDevice;
        LveRenderer& lveRenderer;
        VkDescriptorPool imguiPool;
    };
}