# Issues

A collection of documents I used to troubleshoot all the issues while setting up libraries to work.

- [Glfw build guide](https://www.glfw.org/docs/3.3/build_guide.html)
- [Stackoverflow glfw cmake](https://stackoverflow.com/questions/54834826/cannot-find-package-glfw-using-cmake)
- [FindGLFW.cmake](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/cmake/modules)
- [CMake modules on gitlab repo](https://gitlab.kitware.com/cmake/cmake/-/tree/master/Modules)
- [Glad vs Glew differences](https://stackoverflow.com/questions/68821088/if-i-use-glad-and-not-glew-will-i-miss-on-something)

## Fix Visual Studio Code linter

Apparently for unknown reason to me, the two below are complementary.

Add to `.vscode/settings.json`:

Enabling this removed squiggles.

```json
{
  "clang-tidy.compilerArgs": [
    "-IC:/Users/Srecko/Documents/MyProjects/Private/container-sk-experiments/sk-engine/include",
    "-IC:/Users/Srecko/Documents/MyProjects/Private/container-sk-experiments/sk-engine/build/vcpkg_installed/x64-mingw-dynamic/include",
    "-IC:/VulkanSDK/1.3.261.1/Include"
  ]
}
```

Add to `.vscode/c_cpp_properties.json`, and make sure the glob is the last, for some reason it did not work when it was first. This made the VSCode able to find vulkan includes.

```json
{
  "configurations": [
    {
      "includePath": ["C:/VulkanSDK/1.3.261.1/Include", "${workspaceFolder}/**"]
    }
  ]
}
```
