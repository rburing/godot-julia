mutable struct StringName
	data::Ptr{Nothing}
	function StringName(string::String)
		godot_string = GodotString(string)
		string_name = new(C_NULL)
		@ccall godot_julia_string_name_new_from_string(string_name::Ref{StringName}, godot_string::Ref{GodotString})::Cvoid
		destroy_string_name(s) = @ccall godot_julia_string_name_destroy(s::Ref{StringName})::Cvoid
		finalizer(destroy_string_name, string_name)
	end
end
