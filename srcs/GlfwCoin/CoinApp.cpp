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

#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

namespace zen
{
CoinApp::CoinApp(const char *title) : impl(new CoinAppImpl)
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

    impl->camera = nullptr;
    impl->gizmo_transform = nullptr;

    auto root = impl->root;

    if (auto camera = SearchForCamera(root)) {
        impl->camera = camera;
    } else {
        SoPerspectiveCamera *pcam = new SoPerspectiveCamera;
        pcam->heightAngle = glm::radians(45.f);
        pcam->nearDistance = 0.01f;
        pcam->farDistance = 1000.0f;
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

    impl->camera->viewAll(root, impl->render_manager->getViewportRegion());
}

void CoinApp::SetImGuiCallback(std::function<void()> callback)
{
    impl->imGuiCallback = std::move(callback);
}

void CoinApp::SetGizmoTransform(SoTransform *transform)
{
    impl->gizmo_transform = transform;
}

} // namespace zen