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

       ./bin/godot.linuxbsd.editor.x86_64 --headless --main-loop JuliaBindingsGenerator -- /path/to/godot-julia/src/julia_script/glue/

## Example script

Here is a Julia script that can be attached to a `CharacterBody2D` node:

```julia
module PlayerModule

using Godot: CharacterBody2D, set_physics_process, is_on_floor, get_gravity, move_and_slide
using Godot: Input, StringName
using Godot: Vector2

const SPEED = 300.0
const DECELERATION = 1000.0
const JUMP_VELOCITY = -400.0


mutable struct Player
	character_body::CharacterBody2D
end


new(character_body::CharacterBody2D) = Player(character_body)


function _ready(self::Player)
	set_physics_process(self.character_body, true)
end


function _physics_process(self::Player, delta::Float64)
	velocity = self.character_body.velocity

	if !is_on_floor(self.character_body)
		velocity += get_gravity(self.character_body) * delta
	end

	if is_on_floor(self.character_body) && Input.is_action_pressed(StringName("ui_accept"))
		velocity = Vector2(velocity.x, JUMP_VELOCITY)
	end

	direction = Input.get_axis(StringName("ui_left"), StringName("ui_right"))
	if direction != Vector2(0,0)
		velocity = Vector2(direction * SPEED, velocity.y)
	else
		velocity = Vector2(abs(velocity.x) < DECELERATION * delta ? 0.0 : velocity.x - sign(velocity.x) * DECELERATION * delta, velocity.y)
	end

	self.character_body.velocity = velocity
	move_and_slide(self.character_body)
end

end
```
