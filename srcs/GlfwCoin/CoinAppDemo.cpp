#include "CoinApp.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>

#include <imgui.h>

#include <spdlog/spdlog.h>

std::pair<SoSeparator *, SoTransform *> CreateDemoScene()
{
    SoSeparator *scene = new SoSeparator;
    SoMaterial *mat = new SoMaterial;
    mat->ambientColor.setValue(1.0, 0.0, 0.0);
    mat->diffuseColor.setValue(1.0, 0.0, 0.0);
    mat->specularColor.setValue(1.0, 1.0, 1.0);
    SoTransform *trans = new SoTransform;
    scene->addChild(trans);
    scene->addChild(mat);
    auto cone = new SoCone;
    scene->addChild(cone);

    auto sodepthBuffer = new SoDepthBuffer;
    sodepthBuffer->test = false;
    scene->addChild(sodepthBuffer);

    auto socb = new SoCallback;
    socb->setCallback(
        [](void *data, SoAction *action) {
            auto cone = static_cast<SoCone *>(data);
            if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
                // must invalidate the cache to trigger the callback every frame
                SoCacheElement::invalidate(action->getState());
                ImGui::Begin("Cone");
                static float bottomRadius = cone->bottomRadius.getValue();
                if (ImGui::SliderFloat("Bottom Radius", &bottomRadius, 1.f,
                                       10.0f)) {
                    cone->bottomRadius.setValue(bottomRadius);
                }
                static float height = cone->height.getValue();
                if (ImGui::SliderFloat("Height", &height, 1.f, 10.0f)) {
                    cone->height.setValue(height);
                }
                ImGui::End();
            }
        },
        cone);
    scene->addChild(socb);

    return {scene, trans};
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    zen::CoinApp app;

    auto [ scene, trans] = CreateDemoScene();

    app.SetSceneGraph(scene);
    app.SetGizmoTransform(trans);

    app.Run();

    return 0;
}

// ----------------------------------------------------------------------
