mutable struct StringName
	data::Ptr{Nothing}
	function StringName(godot_string::GodotString)
		string_name = new(C_NULL)
		@ccall godot_julia_string_name_new_from_string(string_name::Ref{StringName}, godot_string::Ref{GodotString})::Cvoid
		destroy_string_name(s) = @ccall godot_julia_string_name_destroy(s::Ref{StringName})::Cvoid
		finalizer(destroy_string_name, string_name)
	end
	StringName(string::String) = StringName(GodotString(string))
end

string_names = Dict{Symbol, StringName}()

function get_string_name!(s::Symbol)
	global string_names
	get!(string_names, s) do
		StringName(GodotString(String(s)))
	end
end
