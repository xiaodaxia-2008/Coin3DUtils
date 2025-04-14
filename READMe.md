# Coin3D Utils

## Integrate Coin3D with Imgui

The follow is an example integrate coin3d with glfw, [imgui](https://github.com/ocornut/imgui) and [imguizomo](https://github.com/CedricGuillemet/ImGuizmo).

![coin3d-imgui](./docs/imgui-gizmo.gif)

## Standalone FreeCAD Gizmo

This is a standalone version of coin3d dragger implemented in FreeCAD, therefore you can use it without the FreeCAD library.


![dragger a text](./docs/gizmo-text.gif)

## Display UTF-8 Text

Generally you can use `SoText2` to display a utf8 text, but the default font only contains latin charset, therefore an extra font is required. Set the font with `SoFont` node, put the font ttf file in the runtime path or set the `COIN_FONT_PATH` environment variable (`coin_setenv` may be helpful). See [here](https://www.coin3d.org/coin/classSoFont.html#details) for more details. 

```c++
auto font = new SoFont;
font->name = "NotoSansSC-Regular.ttf";

auto text = new SoText2;
text->string = "Alçapão, Hello, 你好！";

auto sep = new SoSeparator;
sep->addChild(font);
sep->addChild(text);
```

## Pybind11 Bindings for coin3d

This is a todo.  

Use [litgen](https://github.com/pthom/litgen) to generate pybind11 python bindings.

# How to Build
install vcpkg.

create a `CMakeUserPresets.json` file as follows:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "default",
            "inherits": "vcpkg",
            "environment": {
                "VCPKG_ROOT": "<Path to vcpkg root>"
            }
        }
    ]
}
```

then 

```
mkdir build
cd build
cmake .. --preset default
cmake --build .
```