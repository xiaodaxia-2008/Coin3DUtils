add_library(FCDragger OBJECT So3DAnnotation.cpp SoFCCSysDragger.cpp)
target_link_libraries(FCDragger PUBLIC Coin::Coin Qt5::Core Qt5::OpenGL OpenGL::GL)

add_executable(dragger dragger.cpp)
target_link_libraries(dragger PRIVATE SoQt::SoQt FCDragger)
install(TARGETS dragger)