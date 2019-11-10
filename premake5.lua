workspace "MintServer"
	architecture "x86"
	startproject "MintServer"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "MintServer"
	location "MintServer"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "mtpch.h"
	pchsource "MintServer/src/mtpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
	}

	includedirs
	{
		"%{prj.name}/src",
	}

	links
	{

	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
		}

		postbuildcommands
		{
		}

	filter "configurations:Debug"
		defines "MT_DEBUG"
		buildoptions "/MTd"
		symbols	"On"
	
	filter "configurations:Release"
		defines "MT_RELEASE"
		buildoptions "/MT"
		symbols	"On"

	filter "configurations:Dist"
		defines "MT_DIST"
		buildoptions "/MT"
		symbols	"On"