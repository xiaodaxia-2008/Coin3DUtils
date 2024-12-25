# SoFCDragger

This is a standalone version of coin3d dragger implemented in FreeCAD, therefore you can use it without outside the FreeCAD application.


![dragger a text](./gizmo-text.mp4)
https://github.com/xiaodaxia-2008/SoFCDragger/blob/master/gizmo-text.mp4

# How to build

use vcpkg to install `soqt` package and build with cmake.

![example](./soqt-fcdragger.jpg)

# Notes

## How to display a utf8 text

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

![coin-utf8-text](./coin-utf8-text.jpg)