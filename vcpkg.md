# Setup

Follow instructions from [Sandbox README.md](../../../sandbox/README.md) for more information. I don't see the need to copy-paste instructions.

Generate project:

```PowerShell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Build:

```PowerShell
cmake --build build
```
