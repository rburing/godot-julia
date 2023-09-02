mutable struct GodotString
	cowdata::Ptr{Char}
	function GodotString(string::String)
		chars = transcode(UInt16, string * '\0')
		godot_string = new(C_NULL)
		@ccall godot_julia_string_new_from_utf16_chars(godot_string::Ref{GodotString}, chars::Ref{UInt16})::Cvoid
		destroy_string(s) = @ccall godot_julia_string_destroy(s::Ref{GodotString})::Cvoid
		finalizer(destroy_string, godot_string)
	end
end
