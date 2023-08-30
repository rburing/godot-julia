mutable struct StringName
	data::Ptr{Nothing}
	function StringName(string::String)
		chars = transcode(UInt16, string * '\0')
		string = GodotString(C_NULL)
		@ccall godot_julia_string_new_from_utf16_chars(string::Ref{GodotString}, chars::Ref{UInt16})::Cvoid
		destroy_string(s) = @ccall godot_julia_string_destroy(s::Ref{GodotString})::Cvoid
		finalizer(destroy_string, string)
		string_name = new(C_NULL)
		@ccall godot_julia_string_name_new_from_string(string_name::Ref{StringName}, string::Ref{GodotString})::Cvoid
		destroy_string_name(s) = @ccall godot_julia_string_name_destroy(s::Ref{StringName})::Cvoid
		finalizer(destroy_string_name, string_name)
	end
end
