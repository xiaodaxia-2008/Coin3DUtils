/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file CoinApp.cpp
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 16:53:09, April 10, 2025
 */
#define GLM_ENABLE_EXPERIMENTAL

#include <CoinApp.h>

#include "CoinAppImpl.h"
#include "EventCallback.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// #define EIGEN_DEFAULT_IO_FORMAT                                                \
//     Eigen::IOFormat(4, 0, ", ", "\n", "", "", "", "")
// #include <Eigen/Dense>

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

namespace zen
{

SoNode *CreateDemoScene(CoinAppImpl *impl);

CoinApp::CoinApp(const char *title, bool create_demo_scene)
    : impl(new CoinAppImpl)
{
    glfwSetErrorCallback([](int error, const char *description) {
        SPDLOG_ERROR("{}:{}", error, description);
    });

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    impl->window = glfwCreateWindow(1'920, 1'120, title, NULL, NULL);
    if (!impl->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(impl->window, impl);
    glfwSetFramebufferSizeCallback(impl->window, framebufferSizeCallback);
    glfwMakeContextCurrent(impl->window);
    glfwSetKeyCallback(impl->window, keyCallback);
    glfwSetMouseButtonCallback(impl->window, mouseClickCallback);
    glfwSetCursorPosCallback(impl->window, mouseMoveCallback);
    glfwSetScrollCallback(impl->window, mouseWheelCallback);

    if (create_demo_scene) {
        SetSceneGraph(CreateDemoScene(impl));
    }
}

CoinApp::~CoinApp()
{
    if (impl) {
        delete impl;
    }
    glfwTerminate();
}

void CoinApp::Run()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    impl->ImGuiInit();

    while (!glfwWindowShouldClose(impl->window)) {
        glfwPollEvents();
        impl->UpdateViewport();

        impl->ImGuiNewFrame();

        impl->IdleCallback();
        impl->Render();

        impl->ImGuiDraw();
        impl->ImGuiRender();
        glfwSwapBuffers(impl->window);
    }

    impl->ImGuiDestroy();
}

void CoinApp::SetSceneGraph(SoNode *scene)
{
    if (impl->root) {
        impl->root->unref();
    }

    impl->root = new SoSeparator;
    impl->root->ref();

    auto root = impl->root;

    if (auto camera = SearchForCamera(root)) {
        impl->camera = camera;
    } else {
        SoPerspectiveCamera *pcam = new SoPerspectiveCamera;
        pcam->heightAngle = glm::radians(45.f);
        pcam->nearDistance = 0.01f;
        pcam->farDistance = 100.0f;
        pcam->position = SbVec3f(0, 0, 10);
        pcam->focalDistance = 10.0f;
        pcam->pointAt(SbVec3f(0, 0, 0), SbVec3f(0, 1, 0));
        impl->camera = pcam;
        root->addChild(pcam);
    }

    SoDirectionalLight *light = new SoDirectionalLight;
    light->direction = SbVec3f(0, 0, -1);

    root->addChild(light);

    root->addChild(scene);

    // disable depth test for imgui
    // auto sodepthBuffer = new SoDepthBuffer;
    // sodepthBuffer->test = false;
    // root->addChild(sodepthBuffer);

    auto socb = new SoCallback;
    socb->setCallback(SoCallbackImGuiEventHandle, impl);
    root->addChild(socb);

    // super imposition doesn't get handle event action
    // impl->renderManager->addSuperimposition(socb);

    impl->render_manager->setSceneGraph(root);
    impl->render_manager->setCamera(impl->camera);

    impl->event_manager->setSceneGraph(root);
    impl->event_manager->setCamera(impl->camera);
}

void CoinApp::SetImGuiCallback(std::function<void()> callback)
{
    impl->imGuiCallback = std::move(callback);
}

SoNode *CreateDemoScene(CoinAppImpl *impl)
{
    SoSeparator *scene = new SoSeparator;
    SoMaterial *mat = new SoMaterial;
    mat->ambientColor.setValue(1.0, 0.0, 0.0);
    mat->diffuseColor.setValue(1.0, 0.0, 0.0);
    mat->specularColor.setValue(1.0, 1.0, 1.0);
    SoTransform *trans = new SoTransform;
    impl->transform = trans;
    scene->addChild(trans);
    scene->addChild(mat);
    auto cone = new SoCone;
    scene->addChild(cone);

    auto sodepthBuffer = new SoDepthBuffer;
    sodepthBuffer->test = false;
    scene->addChild(sodepthBuffer);

    auto socb = new SoCallback;
    socb->setCallback(
        [](void *data, SoAction *action) {
            auto cone = static_cast<SoCone *>(data);
            if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
                // must invalidate the cache to trigger the callback every frame
                SoCacheElement::invalidate(action->getState());
                ImGui::Begin("Cone");
                static float bottomRadius = cone->bottomRadius.getValue();
                if (ImGui::SliderFloat("Bottom Radius", &bottomRadius, 1.f,
                                       10.0f)) {
                    cone->bottomRadius.setValue(bottomRadius);
                }
                static float height = cone->height.getValue();
                if (ImGui::SliderFloat("Height", &height, 1.f, 10.0f)) {
                    cone->height.setValue(height);
                }
                ImGui::End();
            }
        },
        cone);
    scene->addChild(socb);

    return scene;
}

} // namespace zen