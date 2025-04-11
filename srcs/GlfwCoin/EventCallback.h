/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file EventCallback.h
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 22:00:49, April 11, 2025
 */
#pragma once
#include <GLFW/glfw3.h>

namespace zen
{
void framebufferSizeCallback(GLFWwindow *window, int w, int h);

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);

void mouseClickCallback(GLFWwindow *window, int button, int action, int mods);

void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos);

void mouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset);

} // namespace zen