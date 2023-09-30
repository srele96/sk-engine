# Vulkan triangle

## Compile shaders

Every time shaders are modified, they need to be recompiled.

```powershell
./compile.bat -files ./shader.vert, ./shader.frag
```

If in the root directory, if you want to make sure the shaders are always compiled, then binary is built, and run, you can run:

Always first compile the shaders, because build system copies them to the build directory.

```powershell
./fixture/vulkan-triangle/compile.bat -files ./fixture/vulkan-triangle/shader.vert, ./fixture/vulkan-triangle/shader.frag; cmake --build build; .\build\fixture\vulkan-triangle\vulkan-triangle.exe
```
