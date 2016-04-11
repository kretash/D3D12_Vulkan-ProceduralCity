### DirectX12 and Vulkan Procedural City ###
----
![](assets/demo.gif "")

This is the end of this project. Although there are features I wanted to implement there is no more time. The whole project has been a great learning experience.

The city is rendered with ExecuteIndirect, I store all geometries in a single buffer and have created a system that allows me to reserve and delete geometries from said buffer. All buildings, although similar are a unique instance generated based on a seed. Each building has a set of textures, diffuse, normal and specular generated with the same seed. The texture manger will load textures and decide their LOD. The biggest textures are 1024x510 and the smallest 128x64.

Since I started this project in September I thought about porting it to Vulkan. I have never supported 2 different graphics APIs before, but I separated the C++ code and the D3D12 with intentions to have an easier job if I get the change to port it to Vulkan.

### Controls ###
---
- WASD movement
- QE height movement
- Shift speed modifier
- F1 show/hide interface
- ESC quit
- F4 halt render manager
- F5 continue render manager

### Prerequisites ###
---
- Windows 10
- Visual Studio 2015
- Windows 10 SDK
- LunarG SDK 1.0.3 (Only necessary to run vulkan validation)

### Build ###
---
- Edit deps/genie/premake.lua and set "windowstargetplatformversion" to your installed version ( This can also be done within VS )
- run "genie.exe 2015" or use the bat file.
- Open vs2015/D3D12 Procedural City.sln and compile.

### Bugs ###
---
- Hot swapping from Vulkan to D3D12 is highly unstable
- Will not run well on low end machines or machines with low VRAM

### Dependencies ###
---
- [FMOD](http://www.fmod.org/)
- OpenSimplexNoise, PerlinNoise and SimplexNoise.
- [Rapidjson](https://github.com/miloyip/rapidjson)
- [Nothings STB](https://github.com/nothings/stb)
- [Tiny OBJ](https://github.com/syoyo/tinyobjloader)

### License ###
---
MIT License
