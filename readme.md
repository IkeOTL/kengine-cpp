WIP

C++ port of my Java engine (https://github.com/IkeOTL/simple-game)

Requirements:
- vcpkg potentially not sure if i want it yet
- Windows (only tested for now)
- C++ 20 capable dev environment
- ensure submodules were pulled `git submodule update --init --recursive`
- CMAKE 3.25.2 (https://cmake.org/files/v3.25/cmake-3.25.2-windows-x86_64.msi)
- Vulkan SDK 1.3 (make sure to include VMA headers and GLM headers on install) (https://vulkan.lunarg.com/sdk/home#windows)
- GLFW 3.3 (CMAKE installed)
  - `git clone git@github.com:glfw/glfw.git`
  - `cd glfw`
  - `git checkout tags/3.3.8`
  - `cmake . -A x64 -B build`
  - In an elevated terminal: `cmake --build build --target install --config Release`
  - GLFW3 should now be built and installed


Build:
- `cmake -DCMAKE_TOOLCHAIN_FILE=D:/git/vcpkg/scripts/buildsystems/vcpkg.cmake . -A x64 -B build`
- `cmake . -A x64 -B build`
  - if CMAKE cant find GLFW hint its install dir on your system `cmake -Dglfw3_DIR="C:/Program Files/GLFW/lib/cmake/glfw3" . -A x64 -B build`
- Copy contents of `examples/res` into `build/examples/res` (Todo: need to automate this)
- your `build` dir should now have a runnable project
- execute `example` project


![alt text](https://github.com/IkeOTL/kengine-cpp/blob/master/examples/res/example00.jpg?raw=true)