/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file EventCallback.h
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 22:00:49, April 11, 2025
 */
#pragma once
#include "EventCallback.h"
#include "CoinAppImpl.h"

#include <Inventor/SbViewportRegion.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <spdlog/spdlog.h>

namespace zen
{
void framebufferSizeCallback(GLFWwindow *window, int w, int h)
{
    auto impl = static_cast<CoinAppImpl *>(glfwGetWindowUserPointer(window));
    SbViewportRegion vp(w, h);
    if (impl->render_manager) {
        impl->render_manager->setViewportRegion(vp);
    }
    if (impl->event_manager) {
        impl->event_manager->setViewportRegion(vp);
    }
    if (impl->camera) {
        impl->camera->aspectRatio = float(w) / float(h);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods)
{
    auto impl = static_cast<CoinAppImpl *>(glfwGetWindowUserPointer(window));

    SoKeyboardEvent event;

    if (action == GLFW_PRESS) {
        event.setState(SoButtonEvent::State::DOWN);
    } else if (action == GLFW_RELEASE) {
        event.setState(SoButtonEvent::State::UP);
    } else {
        event.setState(SoButtonEvent::State::UNKNOWN);
    }

    switch (key) {
    case GLFW_KEY_A: {
    } break;
    default:
        break;
    }
}

void mouseClickCallback(GLFWwindow *window, int button, int action, int mods)
{
    auto impl = static_cast<CoinAppImpl *>(glfwGetWindowUserPointer(window));

    auto &event = impl->mouse_button_evt;

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
        event.setButton(SoMouseButtonEvent::BUTTON2);
    } break;
    case GLFW_MOUSE_BUTTON_MIDDLE: {
        event.setButton(SoMouseButtonEvent::BUTTON3);
    } break;
    default: {
        event.setButton(SoMouseButtonEvent::ANY);
    } break;
    }

    SPDLOG_DEBUG("mouse click: {} {} {}", button, action, mods);
    impl->event_manager->processEvent(&event);
}

void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto impl = static_cast<CoinAppImpl *>(glfwGetWindowUserPointer(window));

    auto [width, height] = impl->GetWindowSize();

    auto &event = impl->location2_evt;
    SbVec2s pos(static_cast<short>(xpos), static_cast<short>(height - ypos));
    event.setPosition(pos);
    impl->mouse_button_evt.setPosition(pos);

    // SPDLOG_DEBUG("mouse move: {:.2f} {:.2f}", xpos, ypos);
    impl->event_manager->processEvent(&event);
}

void mouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto impl = static_cast<CoinAppImpl *>(glfwGetWindowUserPointer(window));
    auto &event = impl->mouse_button_evt;
    event.setState(SoButtonEvent::DOWN);
    if (yoffset > 0) {
        event.setButton(SoMouseButtonEvent::BUTTON4);
    } else {
        event.setButton(SoMouseButtonEvent::BUTTON5);
    }

    impl->event_manager->processEvent(&event);
}

} // namespace zen