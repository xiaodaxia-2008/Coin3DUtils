add_library(FreeCADGizmo STATIC So3DAnnotation.cpp SoFCCSysDragger.cpp)
target_link_libraries(FreeCADGizmo PUBLIC CoinApp)

add_executable(FreeCADGizmoDemo FreeCADGizmoDemo.cpp)
target_link_libraries(FreeCADGizmoDemo PRIVATE FreeCADGizmo)
install(TARGETS FreeCADGizmo)