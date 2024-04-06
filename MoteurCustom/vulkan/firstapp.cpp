#include "firstapp.hpp"
#include "lve_simple_render_system.hpp"
#include "point_light_system.hpp"
#include "lve_camera.hpp"
#include "Keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "Colision.hpp"

//std
#include <stdexcept>
#include <array>
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>
#include <numeric>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "Colision.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define MS_PER_UPDATE 0.0166666666// 1/60
#define SECOND 1.0

namespace lve {

    FirstApp::FirstApp() {
        globalPool = LveDescriptorPool::Builder(lveDevice).setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    /// <summary>
    /// Fonction principale pour exécuter l'application, gère l'initialisation, la boucle principale, et la libération des ressources.
    /// </summary>
    void FirstApp::run() {
        std::vector < std::unique_ptr<LveBuffer>>uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(lveDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }
        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        //SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

        SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout() };
        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -5.5f;
        viewerObject.transform.translation.y = -3.5f;
        viewerObject.transform.rotation.x = -0.5f;
        KeyboardMovementController cameraController{};


        double lag = 0.0, previous = getCurrentTime(), current = 0.0, secondeCount = 0.0f;
        float gameObjectsIncrement = 1.0f;
        int etatClavier = 0;
        auto cubeMovement = gameObjects.find(0);
        cubeMovement->second.transform.vitesse = { 0.016f, 0.016f, 0.f };
        cubeMovement->second.transform.friction = 0.94f;
        while (!lveWindow.shouldClose()) {
            current = getCurrentTime();
            lag += current - previous;
            previous = current;

            while (lag >= MS_PER_UPDATE) {
                cameraController.moveInPanelXZ(lveWindow.getGLFWwindow(), (float)lag, viewerObject);
                camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);


                gameObjects.find(5)->second.transform.translation = {lveImgui.getPositionSliderValue(0), lveImgui.getPositionSliderValue(1), lveImgui.getPositionSliderValue(2)};
                gameObjects.find(5)->second.transform.rotation = {lveImgui.getRotationSliderValue(0), lveImgui.getRotationSliderValue(1), lveImgui.getRotationSliderValue(2)};
                gameObjects.find(5)->second.transform.scale = {lveImgui.getScaleSliderValue(0), lveImgui.getScaleSliderValue(1), lveImgui.getScaleSliderValue(2)};
               

                //petit test des colisions sur des cubes
                for (auto items = gameObjects.find(1); items != gameObjects.cend(); items++) {
                    if (cubeMovement->second.transform.colisionBox.isIntersectAABB(items->second.transform.colisionBox)) {
                        cubeMovement->second.transform.bouncingAABB(items->second.transform.colisionBox);
                        cubeMovement->second.transform.updateAcceleration();
                    }
                }

                //Appelle de la fonction de décélaration sur le cube en mouvement toute les secondes
                if (secondeCount >= 1) {
                    cubeMovement->second.transform.updateAcceleration();
                    secondeCount = 0.0f;
                }

                //Fonction qui update les déplacement du cube
                cubeMovement->second.transform.update();
                
                glfwPollEvents();

                //Relance du cube lorsque l'on apuis sur la touche espace
                //Détection de l'instant où l'on releve la touche espace
                if ((glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_SPACE)) == GLFW_RELEASE && etatClavier == GLFW_PRESS) {
                    gameObjectsIncrement = -gameObjectsIncrement;
                    etatClavier = GLFW_RELEASE;
                }

                if ((etatClavier = glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_SPACE)) == GLFW_PRESS) {
                    cubeMovement->second.transform.setTranslation({ 0.01f * gameObjectsIncrement,  0.499f * gameObjectsIncrement, 2.5f });
                    cubeMovement->second.transform.vitesse = { 0.016f,  0.016f , 0.0f };
                }

                float aspect = lveRenderer.getAspectRatio();
                //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
                camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
                if (auto commandBuffer = lveRenderer.beginFrame()) {
                    int frameIndex = lveRenderer.getFrameIndex();
                    FrameInfo frameInfo{ frameIndex, static_cast<float>(lag), commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

                    //update
                    GlobalUbo ubo{};
                    ubo.projection = camera.getProjection();
                    ubo.view = camera.getView();
                    ubo.inverseView = camera.getInverseView();
                    pointLightSystem.update(frameInfo, ubo);
                    uboBuffers[frameIndex]->writeToBuffer(&ubo);
                    uboBuffers[frameIndex]->flush();

                    //render
                    lveRenderer.beginSwapChainRenderPass(commandBuffer);

                    // order matters
                    simpleRenderSystem.renderGameObjects(frameInfo);
                    pointLightSystem.render(frameInfo);
                    lveImgui.renderImGui(commandBuffer);

                    lveRenderer.endSwapChainRenderPass(commandBuffer);
                    lveRenderer.endFrame();
                }
               /* secondeCount += lag;*/
                lag -= MS_PER_UPDATE;
            }

        }
        vkDeviceWaitIdle(lveDevice.getDevice());
    }

    /// <summary>
    /// Retourne le temps actuel de la machine
    /// </summary>
    /// <returns></returns>
    double FirstApp::getCurrentTime() {
        auto current_time = std::chrono::system_clock::now();
        auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
        return duration_in_seconds.count();
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
    std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
        LveModel::Builder modelBuilder{};
        modelBuilder.vertices = {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };

        for (auto& v : modelBuilder.vertices) {
            v.position += offset;
        }

        modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

        return std::make_unique<LveModel>(device, modelBuilder);
    }

    /// <summary>
    /// chargement de tous les objects de la scène
    /// </summary>
    void FirstApp::loadGameObjects() {
        loadCubesCollision();

        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/NOEL1.obj");
        auto gameObject = LveGameObject::createGameObject();
        gameObject.model = lveModel;
        gameObject.transform.translation = { .0f,1.5f,.0f };
        gameObject.transform.scale = { 0.5f,.5f,0.5f };

        gameObjects.emplace(gameObject.getId(), std::move(gameObject));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad_model.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 3.f, 3.f };
        gameObjects.emplace(floor.getId(), std::move(floor));

        // cercle de lumière
        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}
        };

       for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -.5f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    /// <summary>
    /// Chargement des cubes pour la demo des colisions 
    /// </summary>
    void FirstApp::loadCubesCollision() {
        std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, { .0f, -4.f, -5.f });

        //cube au centre de l'écrant
        auto cube = LveGameObject::createGameObject();
        cube.model = lveModel;
        cube.transform.setTransform({ 0.0f,0.5f,2.5f }, { .5f,.5f,.5f });
        gameObjects.emplace(cube.getId(), std::move(cube));

        //cube de gauche
        auto cube2 = LveGameObject::createGameObject();
        cube2.model = lveModel;
        cube2.transform.setTransform({ -1.0f,.0f,2.5f }, { .5f,.5f,.5f });
        gameObjects.emplace(cube2.getId(), std::move(cube2));

        //cube du haut
        auto cube3 = LveGameObject::createGameObject();
        cube3.model = lveModel;
        cube3.transform.setTransform({ 0.0f,-1.0f,2.5f }, { .5f,.5f,.5f });
        gameObjects.emplace(cube3.getId(), std::move(cube3));

        //cube de droite
        auto cube4 = LveGameObject::createGameObject();
        cube4.model = lveModel;
        cube4.transform.setTransform({ 1.0f,.0f,2.5f }, { .5f,.5f,.5f });
        gameObjects.emplace(cube4.getId(), std::move(cube4));

        //Cube du bas
        auto cube5 = LveGameObject::createGameObject();
        cube5.model = lveModel;
        cube5.transform.setTransform({ 0.0f,1.0f,2.5f }, { .5f,.5f,.5f });
        gameObjects.emplace(cube5.getId(), std::move(cube5));
    }
}