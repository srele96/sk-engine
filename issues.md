# Issues

A collection of documents I used to troubleshoot all the issues while setting up libraries to work.

- [Glfw build guide](https://www.glfw.org/docs/3.3/build_guide.html)
- [Stackoverflow glfw cmake](https://stackoverflow.com/questions/54834826/cannot-find-package-glfw-using-cmake)
- [FindGLFW.cmake](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/cmake/modules)
- [CMake modules on gitlab repo](https://gitlab.kitware.com/cmake/cmake/-/tree/master/Modules)
- [Glad vs Glew differences](https://stackoverflow.com/questions/68821088/if-i-use-glad-and-not-glew-will-i-miss-on-something)

## Fix Visual Studio Code linter

Add to `.vscode/settings.json` to improve autocomplete:

```json
{
  "clang-tidy.compilerArgs": [
    "-IC:/Users/Srecko/Documents/MyProjects/Private/container-sk-experiments/sk-engine/include",
    "-IC:/Users/Srecko/Documents/MyProjects/Private/container-sk-experiments/sk-engine/build/vcpkg_installed/x64-windows/include"
  ]
}
```

Alternatively, add to `.vscode/c_cpp_properties.json`:

```json
{
  "configurations": [
    {
      "includePath": [
        "${workspaceFolder}\\include",
        "${workspaceFolder}\\build\\vcpkg_installed\\x64-windows\\include"
      ]
    }
  ]
}
```
