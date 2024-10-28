WIP

C++ port of my Java engine (https://github.com/IkeOTL/simple-game)

Requirements:
- Windows (only tested for now)
- C++ 20 capable dev environment
- CMAKE 3.30.3 (https://cmake.org/files/v3.25/cmake-3.25.2-windows-x86_64.msi)
- Vulkan SDK 1.3 (make sure to include VMA headers and GLM headers on install) (https://vulkan.lunarg.com/sdk/home#windows)
- vcpkg (https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)

Tests:
- Visual studio:
  - https://github.com/JohnnyHendriks/TestAdapter_Catch2/blob/main/Docs/Walkthrough-vs2022.md
  - `.runsettings` is provided in the root of the repo

Build:
- `cmake -DCMAKE_TOOLCHAIN_FILE=D:/git/vcpkg/scripts/buildsystems/vcpkg.cmake . -A x64 -B build`
  - replace `DCMAKE_TOOLCHAIN_FILE` value with your vcpkg path
- Copy contents of `examples/res` into `build/examples/res` (Todo: need to automate this)
- your `build` dir should now have a runnable project
- execute `example` project

![alt text](https://github.com/IkeOTL/kengine-cpp/blob/master/examples/res/example00.jpg?raw=true)

Reminders for me:
Assets:
 - https://ovanisound.com/
 - https://syntystore.com/
 - https://gamedev.tv/