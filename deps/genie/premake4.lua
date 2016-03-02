solution "D3D12 & Vulkan Procedural City"
	configurations{"debug", "release"}
	location "../../vs2015"
	targetdir "../../bin"
	includedirs "../../include"
	language "C++"
	kind 'ConsoleApp'
    flags 'WinMain' 
	platforms "x64"

	libdirs{ "../FMOD" }
	links{ "fmod64_vc", "fmodL64_vc" }

	links{ "dxgi", "d3d12", "d3dcompiler" }

	libdirs{ "../vulkan" }
	links{ "vulkan-1" }

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
			defines { "_CRT_SECURE_NO_WARNINGS", "WIN32", "_DEBUG", "DEBUG", "VK_PROTOTYPES",
			 "VK_USE_PLATFORM_WIN32_KHR", "_USE_MATH_DEFINES", "NOMINMAX", "WINDOWS" }
			flags { "Symbols" }

		configuration "Release"
			defines { "_CRT_SECURE_NO_WARNINGS", "WIN32", "NDEBUG", "VK_PROTOTYPES", 
			"VK_USE_PLATFORM_WIN32_KHR", "_USE_MATH_DEFINES", "NOMINMAX", "WINDOWS" }
			flags { "Optimize" }
