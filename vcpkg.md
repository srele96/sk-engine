# Setup

Setup documentation.

## Prerequisites

- Windows OS _(I am not considering cross platform compatibility and as I am only using windows there are no guarantees it will build or run anywhere else)_.
- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/examples/manifest-mode-cmake#step-1-install-vcpkg)
- [msys2 _(I am using it to install cmake, clang, gcc, I think it's some kind of package manager for windows)_](https://www.msys2.org/)
- [cmake](https://cmake.org/)
- c++ compiler _(I have both [clang](https://clang.llvm.org/) and [gcc](https://gcc.gnu.org/), but I believe cmake uses gcc by default)_. You can download them using [cmake](https://cmake.org/).
- [Download Vulkan SDK](https://vulkan.lunarg.com/sdk/home). I found link to Vulkan SDK on [Getting Started with the Windows Vulkan SDK](https://vulkan.lunarg.com/doc/sdk/1.3.261.1/windows/getting_started.html).

## How to build

Follow instructions from [Sandbox README.md](../../../sandbox/README.md) for more information. I don't see the need to copy-paste instructions.

Generate project:

```PowerShell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
```

Build:

```PowerShell
cmake --build build
```

Run:

```powershell
./build/src/sk-engine
```

Build then run:

```powershell
cmake --build build; ./build/src/sk-engine
```
