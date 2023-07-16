# Triangle 3D

A fixture to figure out why the triangle isn't displayed as a 3D object.

## Build

Copyable build and run command from the root of the project `sk-engine`.

```PowerShell
cmake --build build; ./build/fixture/triangle-3d/triangle-3d
```

Alternatively, if you are inside the folder of this fixture, just change paths:

```PowerShell
cmake --build ../../build; ./../../build/fixture/triangle-3d/triangle-3d
```

## Reference

Drilling to understand the calling order and how it influences the OpenGL state machine, rendering, etc...

- [glEnableVertexAttribArray](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml)
- [glGenVertexArrays](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenVertexArrays.xhtml)
- [glVertexAttribPointer](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml)
- [glDrawElementsInstanced](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstanced.xhtml)
- [when-should-glvertexattribpointer-be-called](https://stackoverflow.com/questions/17149728/when-should-glvertexattribpointer-be-called)
- [Vertex_Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object)
- [Buffer_Object](https://www.khronos.org/opengl/wiki/Buffer_Object)
- [Buffer_Object#General_use](https://www.khronos.org/opengl/wiki/Buffer_Object#General_use)
- [Vertex_Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object)
- [Pixel_Buffer_Object](https://www.khronos.org/opengl/wiki/Pixel_Buffer_Object)
