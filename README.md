# godot-julia-script

This module adds support for [**Julia**](https://github.com/JuliaLang/julia) as a scripting language to [**Godot Engine**](https://github.com/godotengine/godot) 4.x.

## Current state

⚠ This project is an experimental work in progress, still in a very early stage. ⚠

## Building from source

1. Clone the git repository https://github.com/rburing/godot-julia-script containing the module source code.

2. Open a terminal application and change its working directory to the folder containing the Godot Engine source code.

3. Compile Godot with the module, for the desired `target` (`editor`, `template_debug`, or `template_release`):

       scons target=editor custom_modules=/path/to/godot-julia-script/src/julia_script julia_include=/path/to/julia/include/julia julia_lib=/path/to/julia/lib

