@echo off
%VULKAN_BIN_PATH%\glslangValidator.exe -V test_shader.vert
%VULKAN_BIN_PATH%\glslangValidator.exe -V test_shader.frag
pause