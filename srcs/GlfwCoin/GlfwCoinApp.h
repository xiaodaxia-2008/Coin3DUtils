/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file GlfwCoinApp.h
 * @author Zen Shawn
 * @email xiaozisheng2008@hotmail.com
 * @date 16:47:39, April 10, 2025
 */
#pragma once

class SoNode;

namespace zen
{
class GlfwCoinApp
{
  public:
    GlfwCoinApp();

    ~GlfwCoinApp();

    bool Init();

    void SetSceneGraph(SoNode *scene);

    void CreateDemoScene();

    void Run();

  private:
    struct Pimpl;
    Pimpl *impl;
};

} // namespace zen