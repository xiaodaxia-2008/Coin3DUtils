#include "SoFCCSysDragger.h"
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/draggers/SoTransformBoxDragger.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
  coin_setenv("COIN_FONT_PATH", "C:/AppSource/coin-examples", 1);

  QWidget *window = SoQt::init(argv[0]);
  Gui::SoFCCSysDragger::initClass();
  Gui::So3DAnnotation::initClass();
  auto sep = new SoSeparator;

  auto *dragger = new Gui::SoFCCSysDragger;
  sep->addChild(dragger);
  auto font = new SoFont;
  font->name = "NotoSansSC-Regular.ttf";
  font->size = 10;
  auto text = new SoText3;
  text->string = "Alçapão, Hello, 你好！";
  sep->addChild(font);
  sep->addChild(text);

  SoQtExaminerViewer *viewer = new SoQtExaminerViewer(window);
  viewer->setSceneGraph(sep);
  viewer->show();
  SoQt::show(window);

  SoQt::mainLoop();

  delete viewer;
  return 0;
}