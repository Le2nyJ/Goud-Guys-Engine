#pragma once

#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_descriptors.hpp"
#include "lve_imgui.hpp"

//std
#include <memory>
#include <vector>

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp& operator=(const FirstApp&) = delete;

        void run();
    private:

        double getCurrentTime();
        void loadGameObjects();
        void loadCubesCollision();


        LveWindow lveWindow{ WIDTH, HEIGHT, "GG ENGINE" };
        LveDevice lveDevice{ lveWindow };
        LveRenderer lveRenderer{ lveWindow,lveDevice };
        LveImgui lveImgui{ lveWindow, lveDevice, lveRenderer };

        // note: order of declarations matters
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;
    };
}