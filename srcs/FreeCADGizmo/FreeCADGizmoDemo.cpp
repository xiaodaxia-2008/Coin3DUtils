#include "SoFCCSysDragger.h"
#include <CoinApp.h>
#include <Inventor/draggers/SoTransformBoxDragger.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoTransform.h>

#include <filesystem>
#include <print>
#include <source_location>

int main(int argc, char **argv) {
  auto src_dir =
      std::filesystem::path(std::source_location::current().file_name())
          .parent_path();
  auto src_dir_str = src_dir.u8string();
  auto src_dir_cstr = (char *)src_dir_str.c_str();
  std::println("{}", src_dir_cstr);

  coin_setenv("COIN_FONT_PATH", src_dir_cstr, 1);
  zen::CoinApp app;

  Gui::SoFCCSysDragger::initClass();
  Gui::So3DAnnotation::initClass();
  auto sep = new SoSeparator;

  auto dragger = new Gui::SoFCCSysDragger;
  dragger->draggerSize = 1.f;
  sep->addChild(dragger);

  auto transform = new SoTransform;
  transform->translation.connectFrom(&dragger->translation);
  transform->rotation.connectFrom(&dragger->rotation);
  sep->addChild(transform);

  auto font = new SoFont;
  font->name = "NotoSansSC-Regular.ttf";
  font->size = 10;
  auto text = new SoText3;
  text->string = "Alçapão, Hello, 你好！";
  text->justification = SoText3::CENTER;
  sep->addChild(font);
  sep->addChild(text);

  app.SetSceneGraph(sep);
  app.Run();

  return 0;
}