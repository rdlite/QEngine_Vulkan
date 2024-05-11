@echo off
set shaderPrecompile=%1
set compileResult=%2
%VULKAN_BIN_PATH%\glslangValidator.exe -V %shaderPrecompile% -o %compileResult%

if NOT ["%errorlevel%"]==["0"] (
    pause
    exit /b %errorlevel%
)