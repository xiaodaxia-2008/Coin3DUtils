find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

add_library(GlfwCoinApp STATIC GlfwCoinApp.cpp)
target_link_libraries(GlfwCoinApp PUBLIC
    glfw
    imgui::imgui
    imguizmo::imguizmo
    Coin::Coin
    OpenGL::GL
    spdlog::spdlog
)

add_executable(GlfwCoinDemo GlfwCoinDemo.cpp)
target_link_libraries(GlfwCoinDemo PRIVATE GlfwCoinApp)
