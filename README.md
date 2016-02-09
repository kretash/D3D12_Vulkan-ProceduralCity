### DirectX12 Procedural City ###
----
![](assets/demo.gif "")

This is the current state of my computing project. The goal of this project is to create a demo using D3D12 and hopefully Vulkan, learn how to use the APIs and optimize as much as I can. At the moment I just finished implementing the texture system and its painfully slow, that's what I will be working at the moment.

### Prerequisites ###
---
- Windows 10
- Visual Studio 2015
- Windows 10 SDK

### Build ###
---
- Edit deps/genie/premake.lua and set "windowstargetplatformversion" to your installed version ( This can also be done within VS )
- run "genie.exe 2015" or use the bat file.
- Open vs2015/D3D12 Procedural City.sln and compile.

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
