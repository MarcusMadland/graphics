project "mrender"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../binaries/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")
	objdir ("../intermediate/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")

	files
	{
		"include/**.hpp",
		"src/**.cpp",
	}

	includedirs
	{
		"include"
	}