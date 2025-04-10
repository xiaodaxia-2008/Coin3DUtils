find_package(glfw3 CONFIG REQUIRED)

add_library(GlfwCoinApp STATIC GlfwCoinApp.cpp)
target_link_libraries(GlfwCoinApp PUBLIC glfw Coin::Coin OpenGL::GL spdlog::spdlog)

add_executable(GlfwCoinDemo GlfwCoinDemo.cpp)
target_link_libraries(GlfwCoinDemo PRIVATE GlfwCoinApp)