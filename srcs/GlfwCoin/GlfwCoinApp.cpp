/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file GlfwCoinApp.cpp
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 16:53:09, April 10, 2025
 */

#include "GlfwCoinApp.h"

#include <Inventor/SoDB.h>
#include <Inventor/SoEventManager.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/SoRenderManager.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/scxml/ScXML.h>
#include <Inventor/scxml/SoScXMLStateMachine.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace zen
{

const char DEFAULT_NAVIGATIONFILE[] = "coin:scxml/navigation/examiner.xml";

struct GlfwCoinApp::Pimpl {
    GLFWwindow *window{nullptr};
    SoRenderManager *renderManager{nullptr};
    SoEventManager *eventManager{nullptr};
    SoCamera *camera{nullptr};
    SoSeparator *root{nullptr};
    SoMouseButtonEvent mouseButtonEvt;
    SoLocation2Event location2Evt;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::mat4 modelMatrix{1.0f};

    void updateImGuizmo()
    {
        if (!camera) {
            return;
        }

        if (camera->isOfType(SoPerspectiveCamera::getClassTypeId())) {
            auto cam = static_cast<SoPerspectiveCamera *>(camera);

            float fov = cam->heightAngle.getValue();
            float aspect = cam->aspectRatio.getValue();
            float near = cam->nearDistance.getValue();
            float far = cam->farDistance.getValue();
            projectionMatrix = glm::perspective(fov, aspect, near, far);

            float qx, qy, qz, qw, x, y, z;
            cam->orientation.getValue().getValue(qx, qy, qz, qw);
            cam->position.getValue().getValue(x, y, z);
            glm::mat4 cameraMatrix = glm::mat4_cast(glm::quat(qx, qy, qz, qw));
            cameraMatrix[3] = glm::vec4(x, y, z, 1.0f);
            // view matrix is the inverse of the camera matrix
            viewMatrix = glm::inverse(cameraMatrix);
        }
    }

    void syncImGuizmo()
    {
        if (camera) {
            ///@note glm matrix is column major, the 3rd column is the camera
            /// position

            // camera matrix is the inverse of the view matrix
            auto cameraMatrix = glm::inverse(viewMatrix);

            camera->position.setValue(cameraMatrix[3][0], cameraMatrix[3][1],
                                      cameraMatrix[3][2]);
            glm::quat q(cameraMatrix);
            camera->orientation.setValue(q.x, q.y, q.z, q.w);
        }
    }

    SoCamera *searchForCamera(SoNode *root)
    {
        SoSearchAction sa;
        sa.setInterest(SoSearchAction::FIRST);
        sa.setType(SoCamera::getClassTypeId());
        sa.apply(root);

        if (sa.getPath()) {
            SoNode *node = sa.getPath()->getTail();
            if (node && node->isOfType(SoCamera::getClassTypeId())) {
                return (SoCamera *)node;
            }
        }
        return NULL;
    }

    std::pair<int, int> windowSize()
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return {width, height};
    }

    void updateViewport()
    {
        auto [width, height] = windowSize();
        framebufferSizeCallback(window, width, height);
    }

    void exposeCallback() { renderManager->render(); }

    void idleCallback()
    {
        SoDB::getSensorManager()->processTimerQueue();
        SoDB::getSensorManager()->processDelayQueue(true);
    }

    static void framebufferSizeCallback(GLFWwindow *window, int w, int h)
    {
        auto impl = static_cast<Pimpl *>(glfwGetWindowUserPointer(window));
        SbViewportRegion vp(w, h);
        if (impl->renderManager) {
            impl->renderManager->setViewportRegion(vp);
        }
        if (impl->eventManager) {
            impl->eventManager->setViewportRegion(vp);
        }
    }

    static void keyCallback(GLFWwindow *window, int key, int scancode,
                            int action, int mods)
    {
        auto impl = static_cast<Pimpl *>(glfwGetWindowUserPointer(window));
    }

    static void mouseClickCallback(GLFWwindow *window, int button, int action,
                                   int mods)
    {
        auto impl = static_cast<Pimpl *>(glfwGetWindowUserPointer(window));

        auto &event = impl->mouseButtonEvt;

        if (action == GLFW_PRESS) {
            event.setState(SoButtonEvent::State::DOWN);
        } else if (action == GLFW_RELEASE) {
            event.setState(SoButtonEvent::State::UP);
        } else {
            event.setState(SoButtonEvent::State::UNKNOWN);
        }

        event.setShiftDown(mods & GLFW_MOD_SHIFT);
        event.setCtrlDown(mods & GLFW_MOD_CONTROL);
        event.setAltDown(mods & GLFW_MOD_ALT);

        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: {
            event.setButton(SoMouseButtonEvent::BUTTON1);
        } break;
        case GLFW_MOUSE_BUTTON_RIGHT: {
            event.setButton(SoMouseButtonEvent::BUTTON3);
        } break;
        case GLFW_MOUSE_BUTTON_MIDDLE: {
            event.setButton(SoMouseButtonEvent::BUTTON2);
        } break;
        default: {
            event.setButton(SoMouseButtonEvent::ANY);
        } break;
        }

        spdlog::debug("mouse click: {} {} {}", button, action, mods);
        impl->eventManager->processEvent(&event);
    }

    static void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
    {
        auto impl = static_cast<Pimpl *>(glfwGetWindowUserPointer(window));

        auto [width, height] = impl->windowSize();

        auto &event = impl->location2Evt;
        SbVec2s pos(static_cast<short>(xpos),
                    static_cast<short>(height - ypos));
        event.setPosition(pos);
        impl->mouseButtonEvt.setPosition(pos);

        spdlog::debug("mouse move: {:.2f} {:.2f}", xpos, ypos);
        impl->eventManager->processEvent(&event);
    }

    static void mouseWheelCallback(GLFWwindow *window, double xoffset,
                                   double yoffset)
    {
        auto impl = static_cast<Pimpl *>(glfwGetWindowUserPointer(window));
        auto &event = impl->mouseButtonEvt;
        event.setState(SoButtonEvent::DOWN);
        if (yoffset > 0) {
            event.setButton(SoMouseButtonEvent::BUTTON4);
        } else {
            event.setButton(SoMouseButtonEvent::BUTTON5);
        }

        spdlog::debug("mouse wheel: {:.2f} {:.2f}", xoffset, yoffset);
        impl->eventManager->processEvent(&event);
    }

    static void redrawCallback(void *user, SoRenderManager *manager)
    {
        manager->render();
    }

    void ImGuiDraw()
    {
        updateImGuizmo();
        auto vp = ImGui::GetMainViewport();
        ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);
        float focal = camera->focalDistance.getValue();

        ImVec2 size(128, 128);
        ImVec2 pos(vp->WorkPos.x + vp->WorkSize.x - size.x, vp->WorkPos.y);
        ImGuizmo::ViewManipulate(glm::value_ptr(viewMatrix), focal, pos, size,
                                 0x10101010);
        ImGuizmo::Manipulate(
            glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
            ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL,
            glm::value_ptr(modelMatrix));
        // ImGui::ShowDemoWindow();
        // syncImGuizmo();
    }

    void ImGuiInit()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        ImGuiIO &io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsLight();
        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowRounding = 1.f;
        style.FrameRounding = 3.f;
    }

    void ImGuiNewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuizmo::BeginFrame();

        ImGui::DockSpaceOverViewport(0, nullptr,
                                     ImGuiDockNodeFlags_PassthruCentralNode);
    }

    void ImGuiRender()
    {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiDestroy()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

GlfwCoinApp::GlfwCoinApp() { impl = new Pimpl; }

GlfwCoinApp::~GlfwCoinApp()
{
    if (impl->root) {
        impl->root->unref();
    }
    if (impl->renderManager) {
        delete impl->renderManager;
    }
    if (impl->eventManager) {
        delete impl->eventManager;
    }

    if (impl) {
        delete impl;
    }

    glfwTerminate();
}

bool GlfwCoinApp::Init()
{
    glfwSetErrorCallback([](int error, const char *description) {
        spdlog::error("{}:{}", error, description);
    });

    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    impl->window = glfwCreateWindow(1'920, 1'120, "Coin3D", NULL, NULL);
    if (!impl->window) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(impl->window, impl);
    glfwSetFramebufferSizeCallback(impl->window, impl->framebufferSizeCallback);
    glfwMakeContextCurrent(impl->window);

    SoDB::init();
    SoNodeKit::init();
    SoInteraction::init();

    impl->renderManager = new SoRenderManager;
    impl->renderManager->setAutoClipping(SoRenderManager::VARIABLE_NEAR_PLANE);
    impl->renderManager->setRenderCallback(impl->redrawCallback, impl);
    impl->renderManager->setBackgroundColor(SbColor4f(1.0f, 1.0f, 1.0f, 0.0f));
    impl->renderManager->activate();

    impl->eventManager = new SoEventManager;
    impl->eventManager->setNavigationState(SoEventManager::MIXED_NAVIGATION);

    ScXMLStateMachine *sm = ScXML::readFile(DEFAULT_NAVIGATIONFILE);
    if (sm && sm->isOfType(SoScXMLStateMachine::getClassTypeId())) {
        SoScXMLStateMachine *newsm = static_cast<SoScXMLStateMachine *>(sm);
        newsm->setSceneGraphRoot(impl->root);
        newsm->setActiveCamera(impl->camera);
        impl->eventManager->addSoScXMLStateMachine(newsm);
        newsm->initialize();
    } else {
        spdlog::warn("not a SoScXMLStateMachine: {}", DEFAULT_NAVIGATIONFILE);
        delete sm;
    }

    CreateDemoScene();

    glfwSetKeyCallback(impl->window, impl->keyCallback);
    glfwSetMouseButtonCallback(impl->window, impl->mouseClickCallback);
    glfwSetCursorPosCallback(impl->window, impl->mouseMoveCallback);
    glfwSetScrollCallback(impl->window, impl->mouseWheelCallback);

    return true;
}

void GlfwCoinApp::SetSceneGraph(SoNode *scene)
{
    if (impl->root) {
        impl->root->unref();
    }

    impl->root = new SoSeparator;
    impl->root->ref();

    auto root = impl->root;

    if (auto camera = impl->searchForCamera(root)) {
        impl->camera = camera;
    } else {
        SoPerspectiveCamera *perspectiveCamera = new SoPerspectiveCamera;
        perspectiveCamera->nearDistance = 0.01f;
        perspectiveCamera->farDistance = 100.0f;
        impl->camera = perspectiveCamera;
        root->addChild(perspectiveCamera);
    }

    SoDirectionalLight *light = new SoDirectionalLight;
    light->direction = SbVec3f(0, 0, -1);

    root->addChild(light);

    root->addChild(scene);

    impl->renderManager->setSceneGraph(root);
    impl->renderManager->setCamera(impl->camera);

    impl->eventManager->setSceneGraph(root);
    impl->eventManager->setCamera(impl->camera);
}

void GlfwCoinApp::CreateDemoScene()
{
    SoSeparator *scene = new SoSeparator;
    SoMaterial *mat = new SoMaterial;
    mat->ambientColor.setValue(1.0, 0.0, 0.0);
    mat->diffuseColor.setValue(1.0, 0.0, 0.0);
    mat->specularColor.setValue(1.0, 1.0, 1.0);
    scene->addChild(mat);
    scene->addChild(new SoCone);
    SetSceneGraph(scene);
}

void GlfwCoinApp::Run()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    impl->ImGuiInit();

    while (!glfwWindowShouldClose(impl->window)) {
        glfwPollEvents();
        impl->ImGuiNewFrame();
        impl->ImGuiDraw();

        impl->updateViewport();
        impl->idleCallback();
        impl->exposeCallback();

        impl->ImGuiRender();
        glfwSwapBuffers(impl->window);
    }

    impl->ImGuiDestroy();
}

} // namespace zen