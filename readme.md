WIP

C++ port of my Java engine (https://github.com/IkeOTL/simple-game)

Requirements:
- Windows (only tested for now)
- ensure submodules were pulled `git submodule update --init --recursive`
- CMAKE 3.25.2 (https://cmake.org/files/v3.25/cmake-3.25.2-windows-x86_64.msi)
- Vulkan SDK 1.3 (make sure to include VMA and GLFW on install) (https://vulkan.lunarg.com/sdk/home#windows)
- GLFW 3.3 (CMAKE installed)
  - `git clone git@github.com:glfw/glfw.git`
  - `git checkout tags/3.3.8`
  - `cmake . -A x64 -B build`
  - `cmake --build build --target install --config Release`
  - GLFW3 should now be built and installed