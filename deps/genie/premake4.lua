solution "D3D12 Procedural City"
	configurations{"debug", "release"}
	location "../../vs2015"
	targetdir "../../bin"
	includedirs "../../include"
	language "C++"
	kind 'ConsoleApp'
    flags 'WinMain' 
	platforms "x64"

	libdirs{ "../../deps/FMOD" }
	links{ "fmod64_vc", "fmodL64_vc" }

	links{ "dxgi", "d3d12", "d3dcompiler" }
	windowstargetplatformversion "10.0.10586.0"

	project "core"
		kind 'ConsoleApp'
    	flags 'WinMain' 
		files { "../../include/core/**.h", "../../src/**.cc", "../../src/**.h" }
		files { "../../include/**/**.cpp", "../../include/**/**.h", "../../include/**/**/**.h", "../../include/**/**.hh",
		"../../include/**/**.cc" }

    	files  "../../assets/**"

		configuration "Debug"
			targetsuffix "-d" 
			defines { "WIN32", "_DEBUG", "WINDOWS" }
			flags { "Symbols" }

		configuration "Release"
			defines { "WIN32", "NDEBUG", "WINDOWS" }
			flags { "Optimize" }
