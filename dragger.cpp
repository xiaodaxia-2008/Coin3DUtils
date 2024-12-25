#include "SoFCCSysDragger.h"
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/draggers/SoTransformBoxDragger.h>
#include <Inventor/nodes/SoSeparator.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
  QWidget *window = SoQt::init(argv[0]);
  Gui::SoFCCSysDragger::initClass();
  Gui::So3DAnnotation::initClass();

  auto *dragger = new Gui::SoFCCSysDragger;

  SoQtExaminerViewer *viewer = new SoQtExaminerViewer(window);
  viewer->setSceneGraph(dragger);
  viewer->show();
  SoQt::show(window);

  SoQt::mainLoop();

  delete viewer;
  return 0;
}