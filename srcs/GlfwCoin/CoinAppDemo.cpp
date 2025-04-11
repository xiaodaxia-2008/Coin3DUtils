#include "CoinApp.h"

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    zen::CoinApp app;

    app.Run();

    return 0;
}

// ----------------------------------------------------------------------
