#include "SoFCCSysDragger.h"
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/draggers/SoTransformBoxDragger.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoTransform.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
  coin_setenv("COIN_FONT_PATH", "C:/AppSource/coin-examples", 1);

  QWidget *window = SoQt::init(argv[0]);
  Gui::SoFCCSysDragger::initClass();
  Gui::So3DAnnotation::initClass();
  auto sep = new SoSeparator;

  auto dragger = new Gui::SoFCCSysDragger;
  dragger->draggerSize = 2.f;
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

  SoQtExaminerViewer *viewer = new SoQtExaminerViewer(window);
  viewer->setSceneGraph(sep);
  viewer->show();
  SoQt::show(window);

  SoQt::mainLoop();

  delete viewer;
  return 0;
}