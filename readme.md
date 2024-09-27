WIP

C++ port of my Java engine (https://github.com/IkeOTL/simple-game)

Requirements:
- Windows (only tested for now)
- C++ 20 capable dev environment
- CMAKE 3.30.3 (https://cmake.org/files/v3.25/cmake-3.25.2-windows-x86_64.msi)
- Vulkan SDK 1.3 (make sure to include VMA headers and GLM headers on install) (https://vulkan.lunarg.com/sdk/home#windows)
- vcpkg (https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)

Build:
- `cmake -DCMAKE_TOOLCHAIN_FILE=D:/git/vcpkg/scripts/buildsystems/vcpkg.cmake . -A x64 -B build`
- Copy contents of `examples/res` into `build/examples/res` (Todo: need to automate this)
- your `build` dir should now have a runnable project
- execute `example` project

![alt text](https://github.com/IkeOTL/kengine-cpp/blob/master/examples/res/example00.jpg?raw=true)

assets purhcased:
https://ovanisound.com/
https://syntystore.com/
