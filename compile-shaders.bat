@echo off

set PLATFORM = windows
set API = s_5_0

echo compile shaders

echo simple shader
tools\shaderc.exe ^
-f shaders\simple\simple-vert.sc -o shaders\simple\simple-vert.bin ^
--platform %PLATFORM% --type vertex --verbose -i ./ -p %API%

tools\shaderc.exe ^
-f shaders\simple\simple-frag.sc -o shaders\simple\simple-frag.bin ^
--platform %PLATFORM% --type fragment --verbose -i ./ -p %API%

PAUSE