# godot-julia

This module adds support for [**Julia**](https://github.com/JuliaLang/julia) as a scripting language to [**Godot Engine**](https://github.com/godotengine/godot) 4.x.

## Current state

⚠ This project is a work in progress, still in a very early stage. ⚠

## Building from source

1. Clone the git repository https://github.com/rburing/godot-julia containing the module source code.

2. Open a terminal application and change its working directory to the folder containing the Godot Engine source code.

3. Compile Godot with the module, for the desired `target` (`editor`, `template_debug`, or `template_release`):

       scons target=editor custom_modules=/path/to/godot-julia/src/julia_script julia_include=/path/to/julia/include/julia julia_lib=/path/to/julia/lib

4. Generate and install the Godot bindings package `Godot.jl` for Julia, by running the Godot binary produced by the previous step in the following way:

       ./bin/godot.linuxbsd.editor.dev.x86_64 --headless --main-loop JuliaBindingsGenerator -- modules/julia_script/glue/

