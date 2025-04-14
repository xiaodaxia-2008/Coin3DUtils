/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file CoinAppImpl.cpp
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 22:13:47, April 11, 2025
 */
#include "CoinAppImpl.h"

#include "EventCallback.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/scxml/ScXML.h>
#include <Inventor/scxml/SoScXMLStateMachine.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

#include <spdlog/spdlog.h>

#include <filesystem>

namespace zen
{
CoinAppImpl::CoinAppImpl()
{
    const char DEFAULT_NAVIGATIONFILE[] = "coin:scxml/navigation/examiner.xml";

    SoDB::init();
    SoNodeKit::init();
    SoInteraction::init();

    render_manager = new SoRenderManager;
    render_manager->setAutoClipping(SoRenderManager::VARIABLE_NEAR_PLANE);
    render_manager->setRenderCallback(RenderCallback, this);
    render_manager->setBackgroundColor(SbColor4f(0.3f, 0.3f, 0.3f, 0.0f));
    render_manager->activate();

    event_manager = new SoEventManager;
    event_manager->setNavigationState(SoEventManager::MIXED_NAVIGATION);

    ScXMLStateMachine *sm = ScXML::readFile(DEFAULT_NAVIGATIONFILE);
    if (sm && sm->isOfType(SoScXMLStateMachine::getClassTypeId())) {
        SoScXMLStateMachine *newsm = static_cast<SoScXMLStateMachine *>(sm);
        newsm->setSceneGraphRoot(root);
        newsm->setActiveCamera(camera);
        event_manager->addSoScXMLStateMachine(newsm);
        newsm->initialize();
    } else {
        spdlog::warn("not a SoScXMLStateMachine: {}", DEFAULT_NAVIGATIONFILE);
        delete sm;
    }
}

CoinAppImpl::~CoinAppImpl()
{
    if (event_manager) {
        delete event_manager;
        event_manager = nullptr;
    }

    if (render_manager) {
        delete render_manager;
        render_manager = nullptr;
    }

    if (root) {
        root->unref();
        root = nullptr;
    }

    camera = nullptr;
    gizmo_transform = nullptr;
}

void CoinAppImpl::UpdateImGuizmo()
{
    if (gizmo_transform) {
        // sync transform to modelMatrix
        float qx, qy, qz, qw, x, y, z;
        gizmo_transform->translation.getValue().getValue(x, y, z);
        gizmo_transform->rotation.getValue().getValue(qx, qy, qz, qw);
        glm::mat4 m = glm::mat4_cast(glm::quat(qw, qx, qy, qz));
        m[3] = glm::vec4(x, y, z, 1.0f);
        model_matrix = m;
    }

    if (camera && camera->isOfType(SoPerspectiveCamera::getClassTypeId())) {
        auto cam = static_cast<SoPerspectiveCamera *>(camera);

        float fov = cam->heightAngle.getValue();
        float aspect = cam->aspectRatio.getValue();
        float near = cam->nearDistance.getValue();
        float far = cam->farDistance.getValue();
        projection_matrix = glm::perspective(fov, aspect, near, far);

        float qx, qy, qz, qw, x, y, z;
        cam->orientation.getValue().getValue(qx, qy, qz, qw);
        cam->position.getValue().getValue(x, y, z);
        glm::mat4 cameraMatrix = glm::mat4_cast(glm::quat(qw, qx, qy, qz));
        cameraMatrix[3] = glm::vec4(x, y, z, 1.0f);
        // view matrix is the inverse of the camera matrix
        view_matrix = glm::inverse(cameraMatrix);
    }
}

void CoinAppImpl::SyncImGuizmo()
{
    if (camera) {
        ///@note glm matrix is column major, the 3rd column is the camera
        /// position
        // camera matrix is the inverse of the view matrix
        auto cameraMatrix = glm::inverse(view_matrix);
        camera->position.setValue(cameraMatrix[3][0], cameraMatrix[3][1],
                                  cameraMatrix[3][2]);
        glm::quat q(cameraMatrix);
        camera->orientation.setValue(q.x, q.y, q.z, q.w);
    }

    if (gizmo_transform) {
        // sync transform to modelMatrix
        gizmo_transform->translation.setValue(model_matrix[3][0], model_matrix[3][1],
                                        model_matrix[3][2]);
        glm::quat q(model_matrix);
        gizmo_transform->rotation.setValue(q.x, q.y, q.z, q.w);
    }
}

std::pair<int, int> CoinAppImpl::GetWindowSize()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return {width, height};
}

void CoinAppImpl::UpdateViewport()
{
    auto [width, height] = GetWindowSize();
    framebufferSizeCallback(window, width, height);
}

void CoinAppImpl::Render() { render_manager->render(); }

void CoinAppImpl::IdleCallback()
{
    SoDB::getSensorManager()->processTimerQueue();
    SoDB::getSensorManager()->processDelayQueue(true);
}

void CoinAppImpl::ImGuiDraw()
{
    UpdateImGuizmo();

    auto vp = ImGui::GetMainViewport();
    ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);
    float focal = camera->focalDistance.getValue();
    auto position = camera->position.getValue();
    ImVec2 size(128, 128);
    ImVec2 pos(vp->WorkPos.x + vp->WorkSize.x - size.x, vp->WorkPos.y);
    ImGuizmo::ViewManipulate(glm::value_ptr(view_matrix), focal, pos, size,
                             0x10101010);

    if (gizmo_transform) {
        ImGuizmo::Manipulate(
            glm::value_ptr(view_matrix), glm::value_ptr(projection_matrix),
            ImGuizmo::OPERATION::TRANSLATE | ImGuizmo::OPERATION::ROTATE,
            ImGuizmo::MODE::LOCAL, glm::value_ptr(model_matrix));
    }

    // ImGui::ShowDemoWindow();

    if (imGuiCallback) {
        imGuiCallback();
    }
    SyncImGuizmo();
}

void CoinAppImpl::ImGuiInit()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (std::filesystem::exists("C:/Windows/Fonts/msyh.ttc")) {
        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 25.f);
    }

    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 1.f;
    style.FrameRounding = 3.f;
}

void CoinAppImpl::ImGuiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::BeginFrame();

    ImGui::DockSpaceOverViewport(0, nullptr,
                                 ImGuiDockNodeFlags_PassthruCentralNode);
}

void CoinAppImpl::ImGuiRender()
{
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CoinAppImpl::ImGuiDestroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void SoCallbackImGuiEventHandle(void *user, SoAction *action)
{
    auto impl = static_cast<CoinAppImpl *>(user);
    // if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
    //     // impl->ImGuiNewFrame();
    //     impl->ImGuiDraw();
    //     // impl->ImGuiRender();
    //     return;
    // }

    if (action->isOfType(SoHandleEventAction::getClassTypeId())) {
        auto handleEventAction = static_cast<SoHandleEventAction *>(action);
        auto event = handleEventAction->getEvent();
        bool isMouseButtonEvent =
            event->isOfType(SoMouseButtonEvent::getClassTypeId());
        bool isMouseMoveEvent =
            event->isOfType(SoLocation2Event::getClassTypeId());
        ImGuiIO &io = ImGui::GetIO();
        if ((isMouseButtonEvent || isMouseMoveEvent) && io.WantCaptureMouse) {
            handleEventAction->setHandled();
        }
    }

    // if (action->isOfType(SoGetBoundingBoxAction::getClassTypeId())) {
    //     auto getBoundingBoxAction =
    //         static_cast<SoGetBoundingBoxAction *>(action);

    //     ///@todo how to compute the exact gizmo bounding box ?
    //     // the gizmo axes is 100pixels long in screen space
    //     auto &modelMatrix = impl->modelMatrix;
    //     auto minPt = modelMatrix[3];
    //     auto maxPt = modelMatrix * glm::vec4(1, 1, 1, 1);

    //     if
    //     (impl->camera->isOfType(SoPerspectiveCamera::getClassTypeId())) {

    //         auto cam = static_cast<SoPerspectiveCamera *>(impl->camera);
    //         auto &vp = getBoundingBoxAction->getViewportRegion();
    //         float fov = cam->heightAngle.getValue();
    //         auto size = vp.getViewportSize();

    //         float pixelWorldSize =
    //             2.0f * glm::length(minPt) * tan(fov / 2.f) / size[1];
    //         maxPt = modelMatrix * (pixelWorldSize * glm::vec4(1, 1, 1,
    //         1));
    //     }

    //     SbBox3f box(minPt.x, minPt.y, minPt.z, maxPt.x, maxPt.y,
    //     maxPt.z); getBoundingBoxAction->extendBy(box);
    // }
}

void RenderCallback(void *user, SoRenderManager *manager) {}

SoCamera *SearchForCamera(SoNode *root)
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

} // namespace zen