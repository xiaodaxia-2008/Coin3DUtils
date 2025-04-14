/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file CoinAppImpl.h
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 22:08:57, April 11, 2025
 */
#pragma once

#include <GLFW/glfw3.h>

#include <Inventor/SoEventManager.h>
#include <Inventor/SoRenderManager.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <utility>

namespace zen
{

struct CoinAppImpl {
    GLFWwindow *window{nullptr};
    SoRenderManager *render_manager{nullptr};
    SoEventManager *event_manager{nullptr};
    SoCamera *camera{nullptr};
    SoSeparator *root{nullptr};
    SoTransform *gizmo_transform{nullptr};
    SoMouseButtonEvent mouse_button_evt;
    SoLocation2Event location2_evt;

    glm::mat4 view_matrix{1.0f};
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 model_matrix{1.0f};

    std::function<void()> imGuiCallback;

    CoinAppImpl();
    ~CoinAppImpl();

    void UpdateImGuizmo();
    void SyncImGuizmo();

    std::pair<int, int> GetWindowSize();

    void UpdateViewport();
    void Render();
    void IdleCallback();

    void ImGuiDraw();
    void ImGuiInit();
    void ImGuiNewFrame();
    void ImGuiRender();
    void ImGuiDestroy();
};

void RenderCallback(void *user, SoRenderManager *manager);

SoCamera *SearchForCamera(SoNode *root);

void SoCallbackImGuiEventHandle(void *user, SoAction *action);

} // namespace zen