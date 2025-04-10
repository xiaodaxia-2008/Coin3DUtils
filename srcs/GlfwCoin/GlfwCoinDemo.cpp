#include "GlfwCoinApp.h"

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    zen::GlfwCoinApp app;

    app.Init();
    app.CreateDemoScene();
    app.Run();

    return 0;
}

// ----------------------------------------------------------------------
