find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

add_library(GlfwCoinApp STATIC GlfwCoinApp.cpp)
target_link_libraries(GlfwCoinApp PUBLIC
    glfw
    imgui::imgui
    imguizmo::imguizmo
    Coin::Coin
    OpenGL::GL
    spdlog::spdlog
    glm::glm-header-only
    Eigen3::Eigen
)

target_compile_definitions(GlfwCoinApp PUBLIC $<BUILD_INTERFACE:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>)

add_executable(GlfwCoinDemo GlfwCoinDemo.cpp)
target_link_libraries(GlfwCoinDemo PRIVATE GlfwCoinApp)
