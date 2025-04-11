find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

add_library(CoinApp STATIC
    CoinApp.cpp
    CoinAppImpl.cpp
    EventCallback.cpp
)
target_link_libraries(CoinApp PUBLIC
    glfw
    imgui::imgui
    imguizmo::imguizmo
    Coin::Coin
    OpenGL::GL
    spdlog::spdlog
    glm::glm-header-only
    Eigen3::Eigen
)

target_compile_definitions(CoinApp PUBLIC $<BUILD_INTERFACE:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>)
target_include_directories(CoinApp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_executable(CoinAppDemo CoinAppDemo.cpp)
target_link_libraries(CoinAppDemo PRIVATE CoinApp)
