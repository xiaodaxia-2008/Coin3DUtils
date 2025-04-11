/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file CoinApp.h
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 16:47:39, April 10, 2025
 */
#pragma once

#include <functional>

class SoNode;

namespace zen
{
struct CoinAppImpl;

class CoinApp
{
  public:
    CoinApp(const char *title = "ZenView", bool create_demo_scene = true);

    ~CoinApp();

    void SetSceneGraph(SoNode *scene);
    void SetImGuiCallback(std::function<void()> callback);

    void Run();

  private:
    CoinAppImpl *impl;
};

} // namespace zen