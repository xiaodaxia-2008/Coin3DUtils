#define GLM_ENABLE_EXPERIMENTAL

#include <Inventor/SoOutput.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include <spdlog/spdlog.h>

#include <numbers>

struct AppData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
};

void PrintNode(SoNode *node)
{
    std::string buffer(1'024 * 1'024, '\0');
    SoOutput out;
    // out.openFile("output.iv");
    out.setBuffer(buffer.data(), 1'024, nullptr, 0);
    SoWriteAction wa(&out);
    wa.apply(node); // Serializes the node and its children
    out.closeFile();

    spdlog::debug("{}", buffer);
}

static glm::mat4 identity = glm::mat4(1.f);

int main()
{
    spdlog::set_level(spdlog::level::debug);
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window =
        glfwCreateWindow(1'280, 720, "Coin3D + ImGui", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    AppData ad;
    ad.model = glm::mat4(1.f);
    glm::vec3 eye(0, 0, 10);
    glm::vec3 center(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    ad.view = glm::lookAt(eye, center, up);
    float cam_dist = glm::length(eye - center);
    float near = 0.1f;
    float far = 100.f;

    const glm::mat4 grid_model = glm::rotate(glm::mat4(1.f), glm::radians(90.f),
                                             glm::vec3(1.f, 0.f, 0.f));

    constexpr float fov = glm::radians(45.0f);

    // 初始化 Coin3D
    SoDB::init(); // 必须初始化 Coin3D 数据库
    SoSeparator *root = new SoSeparator;
    root->ref(); // 防止内存泄漏
    // camera
    SoPerspectiveCamera *camera = new SoPerspectiveCamera;
    camera->heightAngle = std::numbers::pi / 4;
    // camera->nearDistance = near;
    // camera->farDistance = far;
    camera->position = SbVec3f(1, 0, 10);
    camera->orientation = SbRotation(SbVec3f(0, 0, 1), glm::radians(15.f));
    // camera->pointAt(SbVec3f(0, 0, 0), SbVec3f(1, 0, 0));
    PrintNode(camera);
    // spdlog::debug("camera position: {}",
    //               camera->position.getValue().toString().getString());
    // spdlog::debug("camera rotation: {}",
    //               camera->orientation.getValue().toString().getString());
    // spdlog::debug("camera focal: {}", camera->focalDistance.getValue());
    // camera->position.setValue(glm::value_ptr(eye));
    // camera->pointAt(SbVec3f(glm::value_ptr(center)));
    // camera->pointAt(SbVec3f(glm::value_ptr(center)),
    //                 SbVec3f(glm::value_ptr(up)));
    root->addChild(camera);

    // 添加灯光和材质
    SoDirectionalLight *light = new SoDirectionalLight;
    light->direction = SbVec3f(0, 0, -1);
    root->addChild(light);
    SoMaterial *material = new SoMaterial;
    material->ambientColor = SbColor(0.5f, 0.f, 0.f);
    material->diffuseColor = SbColor(0.5f, 0.f, 0.f);
    material->specularColor = SbColor(0.5f, 0.f, 0.f);
    material->shininess = 0.1f;
    root->addChild(material);
    SoCone *cone = new SoCone;
    root->addChild(cone);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuizmo::OPERATION current_gizmo_operation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE current_gizmo_mode = ImGuizmo::LOCAL;
    ImGuizmo::SetOrthographic(false);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGui::DockSpaceOverViewport(0, nullptr,
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        // ImGui::Begin("Settings");
        // ImGui::SliderFloat("fov", &fov, 1.f, 120.f);
        // ImGui::End();

        ImGui::Begin("Camera Matrix");
        auto camera_matrix = glm::inverse(ad.view);
        auto camera_row_major = glm::transpose(camera_matrix);
        for (int i = 0; i < 4; i++) {
            if (ImGui::InputFloat4(fmt::format("##Camera Matrix{}", i).c_str(),
                                   glm::value_ptr(camera_row_major) + i * 4)) {
                ad.view = glm::inverse(glm::transpose(camera_row_major));
            }
        }
        camera->position.setValue(camera_matrix[3][0], camera_matrix[3][1],
                                  camera_matrix[3][2]);
        glm::quat q(camera_matrix);
        camera->orientation.setValue(SbRotation(q.x, q.y, q.z, q.w));
        if (ImGui::Button("Set Coin Camera")) {
            camera->position.setValue(camera_matrix[3][0], camera_matrix[3][1],
                                      camera_matrix[3][2]);
            glm::quat q(camera_matrix);
            camera->orientation.setValue(SbRotation(q.x, q.y, q.z, q.w));
        }
        ImGui::End();

        ImGui::Begin("Model Matrix");
        auto model_row_major = glm::transpose(ad.model);
        for (int i = 0; i < 4; i++) {
            if (ImGui::InputFloat4(fmt::format("##ModelMatrix{}", i).c_str(),
                                   glm::value_ptr(model_row_major) + i * 4)) {
                ad.model = glm::transpose(model_row_major);
            }
        }
        if (ImGui::Button("Reset")) {
            ad.model = glm::mat4(1.f);
        }
        ImGui::End();

        ImGuiViewport *vp = ImGui::GetMainViewport();
        float aspect = vp->WorkSize.x / vp->WorkSize.y;
        ad.projection = glm::perspective(fov, aspect, near, far);

        // 创建 ImGui 控制面板
        ImGui::Begin("Coin3D Control");
        static float bottom_radius = 1.0f;
        static float height = 2.0f;
        ImGui::SliderFloat("Bottom Radius", &bottom_radius, 0.1f, 5.0f);
        ImGui::SliderFloat("Height", &height, 0.1f, 5.0f);
        cone->bottomRadius = bottom_radius;
        cone->height = height;
        ImGui::End();

        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        // ImGuizmo::DrawGrid(glm::value_ptr(ad.view),
        //                    glm::value_ptr(ad.projection),
        //                    glm::value_ptr(grid_model), 1000.f);
        // ImGuizmo::DrawCubes(glm::value_ptr(ad.view),
        //                     glm::value_ptr(ad.projection),
        //                     glm::value_ptr(ad.model), 1);
        ImGuizmo::Manipulate(glm::value_ptr(ad.view),
                             glm::value_ptr(ad.projection),
                             current_gizmo_operation, current_gizmo_mode,
                             glm::value_ptr(ad.model));

        float viewManipulateRight = vp->WorkPos.x + vp->WorkSize.x;
        float viewManipulateTop = vp->WorkPos.y;

        ImGuizmo::ViewManipulate(
            glm::value_ptr(ad.view), cam_dist,
            ImVec2(viewManipulateRight - 128, viewManipulateTop),
            ImVec2(128, 128), 0x10101010);

        // 渲染 Coin3D 场景
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SoGLRenderAction glAction(
            SbViewportRegion(static_cast<short>(vp->WorkSize.x),
                             static_cast<short>(vp->WorkSize.y)));
        glAction.apply(root); // 渲染场景图

        // render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 清理资源
    root->unref();
    SoDB::cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}