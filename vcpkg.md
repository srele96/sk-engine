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

On windows:

```PowerShell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
```

On Linux(_do not use `x64-mingw-dynamic` triplet because it's not for linux._):

```terminal
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
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

## Linux troubleshooting

A list of errors and troubleshooting steps I encountered while making this project run on Linux.

### Missing GLFW3 dependencies

If the generation command produces this output, install required dev packages:

```terminal
srecko@srecko-desktop:~/Documents/projects/ct-sk-experiments/sk-engine$ cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
-- Running vcpkg install
Detecting compiler hash for triplet x64-linux...
The following packages will be built and installed:
    glfw3:x64-linux -> 3.3.8#2 -- /home/srecko/vcpkg/buildtrees/versioning_/versions/glfw3/308cd3ebdf6752e9d5eeb2933f33972c784aa8bd
    vulkan:x64-linux -> 1.1.82.1#6 -- /home/srecko/vcpkg/buildtrees/versioning_/versions/vulkan/ea62236a3c91051f5ccb340442b60a026bf160c6
    vulkan-memory-allocator:x64-linux -> 3.0.1#3 -- /home/srecko/vcpkg/buildtrees/versioning_/versions/vulkan-memory-allocator/d57ebd5ba820e5d8a5f55d797199abe4734d5af8
Restored 0 package(s) from /home/srecko/.cache/vcpkg/archives in 5.09 us. Use --debug to see more details.
Installing 1/3 glfw3:x64-linux...
Building glfw3:x64-linux...
-- Installing port from location: /home/srecko/vcpkg/buildtrees/versioning_/versions/glfw3/308cd3ebdf6752e9d5eeb2933f33972c784aa8bd
-- Using cached glfw-glfw-7482de6071d21db77a7236155da44c172a7f6c9e.tar.gz.
-- Cleaning sources at /home/srecko/vcpkg/buildtrees/glfw3/src/172a7f6c9e-7678776297.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source /home/srecko/vcpkg/downloads/glfw-glfw-7482de6071d21db77a7236155da44c172a7f6c9e.tar.gz
-- Using source at /home/srecko/vcpkg/buildtrees/glfw3/src/172a7f6c9e-7678776297.clean
GLFW3 currently requires the following libraries from the system package manager:
    xinerama
    xcursor
    xorg
    libglu1-mesa
    pkg-config

These can be installed on Ubuntu systems via sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
```

Install the required packages to build glfw3:

```terminal
sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
```

### Missing vulkan SDK

On [this](https://vulkan.lunarg.com/sdk/home#linux) page I downloaded current latest version `1.3.261.1` because it matched version I am using on Windows.

I found this list of  commands on the page to setup the vulkan SDK.

```powershell
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.3.261-jammy.list https://packages.lunarg.com/vulkan/1.3.261/lunarg-vulkan-1.3.261-jammy.list
sudo apt update
sudo apt install vulkan-sdk
```
