#include "julia_bindings_generator.h"

#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)

#include "core/core_constants.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "editor/editor_help.h"

#include <julia.h>

bool is_julia_keyword(String p_string) {
	// The list of keywords is taken from https://docs.julialang.org/en/v1/base/base/#Keywords
	return p_string == "baremodule" || p_string == "begin" || p_string == "break" || p_string == "catch" ||
			p_string == "const" || p_string == "continue" || p_string == "do" || p_string == "else" ||
			p_string == "elseif" || p_string == "end" || p_string == "export" || p_string == "false" ||
			p_string == "finally" || p_string == "for" || p_string == "function" || p_string == "global" ||
			p_string == "if" || p_string == "import" || p_string == "let" || p_string == "local" ||
			p_string == "macro" || p_string == "module" || p_string == "quote" || p_string == "return" ||
			p_string == "struct" || p_string == "true" || p_string == "try" || p_string == "using" ||
			p_string == "while";
}

String escape_julia_keyword(String p_string) {
	return is_julia_keyword(p_string) ? "_" + p_string : p_string;
}

static Error _save_file(const String &p_path, const StringBuilder &p_content) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_FILE_CANT_WRITE, "Cannot open file: '" + p_path + "'.");

	file->store_string(p_content.as_string());

	return OK;
}

static String fix_doc_description(const String &p_bbcode) {
	// Based on EditorHelp, specialized for godot-julia.
	return p_bbcode.dedent()
			.replace("\t", "")
			.replace("\r", "")
			.replace("Variant.Type", "VariantType")
			.replace("Variant.Operator", "VariantOperator")
			.replace("\"\"\"", "\\\"\\\"\\\"")
			.strip_edges();
}

static StringName _get_int_type_name_from_meta(GodotTypeInfo::Metadata p_meta) {
	switch (p_meta) {
		case GodotTypeInfo::METADATA_INT_IS_INT8:
			return "sbyte";
			break;
		case GodotTypeInfo::METADATA_INT_IS_INT16:
			return "short";
			break;
		case GodotTypeInfo::METADATA_INT_IS_INT32:
			return "int";
			break;
		case GodotTypeInfo::METADATA_INT_IS_INT64:
			return "long";
			break;
		case GodotTypeInfo::METADATA_INT_IS_UINT8:
			return "byte";
			break;
		case GodotTypeInfo::METADATA_INT_IS_UINT16:
			return "ushort";
			break;
		case GodotTypeInfo::METADATA_INT_IS_UINT32:
			return "uint";
			break;
		case GodotTypeInfo::METADATA_INT_IS_UINT64:
			return "ulong";
			break;
		default:
			// Assume INT64
			return "long";
	}
}

static StringName _get_float_type_name_from_meta(GodotTypeInfo::Metadata p_meta) {
	switch (p_meta) {
		case GodotTypeInfo::METADATA_REAL_IS_FLOAT:
			return "float";
			break;
		case GodotTypeInfo::METADATA_REAL_IS_DOUBLE:
			return "double";
			break;
		default:
			// Assume FLOAT64
			return "double";
	}
}

static StringName _get_type_name_from_meta(Variant::Type p_type, GodotTypeInfo::Metadata p_meta) {
	if (p_type == Variant::INT) {
		return _get_int_type_name_from_meta(p_meta);
	} else if (p_type == Variant::FLOAT) {
		return _get_float_type_name_from_meta(p_meta);
	} else {
		return Variant::get_type_name(p_type);
	}
}

bool BindingsGenerator::_arg_default_value_from_variant(const Variant &p_val, BindingsGenerator::GodotArgument &r_arg) {
	r_arg.default_value = p_val;
	r_arg.julia_default_value = p_val.operator String();

	switch (p_val.get_type()) {
		// Either Object type or Variant.
		case Variant::NIL:
			r_arg.julia_default_value = "Variant()";
			break;
		// Atomic types.
		case Variant::BOOL:
			r_arg.julia_default_value = bool(p_val) ? "true" : "false";
			break;
		case Variant::INT:
			if (r_arg.type.name != "int") {
				r_arg.julia_default_value = vformat("%s(%s)", r_arg.type.name, r_arg.julia_default_value);
			}
			break;
		case Variant::FLOAT:
			if (r_arg.type.name == "float") {
				r_arg.julia_default_value += "f0";
			}
			break;
		// TODO: Handle more complex types.
		default:
			ERR_FAIL_V_MSG(false, "Unexpected Variant type in argument: " + itos(p_val.get_type()));
			break;
	}

	// TODO: Handle nullable types.

	return true;
}

void BindingsGenerator::_generate_global_constants(StringBuilder &p_output) {
	for (GodotEnum &genum : global_enums) {
		Vector<GodotConstant> extraneous_constants;
		HashMap<int64_t, GodotConstant *> value_to_constant;
		// TODO: Use genum.is_flags?
		p_output.append(vformat("@enum %s begin\n", genum.julia_name));
		for (GodotConstant &gconstant : genum.constants) {
			if (!value_to_constant.has(gconstant.value)) {
				p_output.append(vformat("%s = %d\n", gconstant.name, gconstant.value));
				value_to_constant.insert(gconstant.value, &gconstant);
			} else {
				extraneous_constants.append(gconstant);
			}
		}
		p_output.append("end\n");
		for (GodotConstant &gconstant : extraneous_constants) {
			p_output.append(vformat("%s = %s\n", gconstant.name, value_to_constant[gconstant.value]->name));
		}
		// Docstrings.
		for (GodotConstant &gconstant : genum.constants) {
			p_output.append(vformat("@doc raw\"\"\"%s\"\"\"\n%s\n", fix_doc_description(gconstant.documentation->description), gconstant.name));
		}
		p_output.append("\n");
	}
}

void BindingsGenerator::_generate_string_names(StringBuilder &p_output) {
	p_output.append("mutable struct StringName\n");
	p_output.append("\tdata::Ptr{Nothing}\n");
	p_output.append("\tfunction StringName(string::String)\n");
	p_output.append("\t\tchars = transcode(UInt16, string * '\\0')\n");
	p_output.append("\t\tstring = GodotString(C_NULL)\n");
	p_output.append("\t\t@ccall godot_julia_string_new_from_utf16_chars(string::Ref{GodotString}, chars::Ref{UInt16})::Cvoid\n");
	p_output.append("\t\tdestroy_string(s) = @ccall godot_julia_string_destroy(s::Ref{GodotString})::Cvoid\n");
	p_output.append("\t\tfinalizer(destroy_string, string)\n");
	p_output.append("\t\tstring_name = new(C_NULL)\n");
	p_output.append("\t\t@ccall godot_julia_string_name_new_from_string(string_name::Ref{StringName}, string::Ref{GodotString})::Cvoid\n");
	p_output.append("\t\tdestroy_string_name(s) = @ccall godot_julia_string_name_destroy(s::Ref{StringName})::Cvoid\n");
	p_output.append("\t\tfinalizer(destroy_string_name, string_name)\n");
	p_output.append("\tend\n");
	p_output.append("end\n\n");

	HashSet<StringName> string_names;
	for (const KeyValue<StringName, GodotType> &E : object_types) {
		string_names.insert(E.value.name);
		for (const GodotMethod &godot_method : E.value.methods) {
			string_names.insert(godot_method.name);
		}
	}
	p_output.append("mutable struct StringNames\n");
	for (const StringName &string_name : string_names) {
		p_output.append(vformat("\t_%s::StringName\n", string_name));
	}
	p_output.append("\tStringNames() = new()\n");
	p_output.append("end\n\n");
	p_output.append("string_names = StringNames()\n\n");

	p_output.append("function initialize_string_names()\n");
	for (const StringName &string_name : string_names) {
		p_output.append(vformat("\tstring_names._%s = StringName(\"%s\")\n", string_name, string_name));
	}
	p_output.append("end\n\n");
}

void BindingsGenerator::_generate_variant(StringBuilder &p_output) {
	p_output.append("abstract type GodotVariant end\n\n");
	p_output.append("mutable struct Variant <: GodotVariant\n");
#ifdef REAL_T_IS_DOUBLE
	p_output.append("\tdata::NTuple{40, UInt8} # assuming real_t is double\n");
	p_output.append("\tVariant() = new(tuple(zeros(UInt8, 40)...))\n");
#else
	p_output.append("\tdata::NTuple{24, UInt8} # assuming real_t is float\n");
	p_output.append("\tVariant() = new(tuple(zeros(UInt8, 24)...))\n");
#endif
	p_output.append("end\n\n");

	p_output.append("variant_type(v::Variant) = VariantType(v.data[1])\n\n");
}

void BindingsGenerator::_generate_julia_type(const GodotType &p_godot_type, StringBuilder &p_output) {
	p_output.append(vformat("abstract type Godot%s", p_godot_type.julia_name));
	if (p_godot_type.parent_class_name != StringName()) {
		p_output.append(vformat(" <: Godot%s", object_types[p_godot_type.parent_class_name].julia_name));
	}
	p_output.append(" end\n\n");
	p_output.append(vformat("@doc raw\"\"\"%s\n\n%s\"\"\"\n", fix_doc_description(p_godot_type.class_doc->brief_description), fix_doc_description(p_godot_type.class_doc->description)));
	p_output.append(vformat("struct %s <: Godot%s\n", p_godot_type.julia_name, p_godot_type.julia_name));
	p_output.append("\tnative_ptr::Ptr{Nothing}\n");
	p_output.append("end\n\n");

	// TODO: Handle singletons.

	// TODO: Constants.

	// TODO: Enums.

	// TODO: Properties.

	// Methods.

	for (const GodotMethod &godot_method : p_godot_type.methods) {
		_generate_julia_method(p_godot_type, godot_method, p_output);
	}

	// TODO: Signals.
}

void BindingsGenerator::_generate_julia_method(const GodotType &p_godot_type, const GodotMethod &p_godot_method, StringBuilder &p_output) {
	p_output.append("let\n");
	p_output.append(vformat("\tglobal %s\n", p_godot_method.julia_name));
	p_output.append("\tmethod_bind = C_NULL\n");
	p_output.append(vformat("\tfunction %s(self::Godot%s", p_godot_method.julia_name, p_godot_type.julia_name));
	int argc = p_godot_method.arguments.size();
	for (int i = 0; i < argc; i++) {
		const GodotType *argument_type = _get_type_or_null(p_godot_method.arguments[i].type);
		// TODO: Handle argument_type == nullptr?
		p_output.append(vformat(", %s::%s", p_godot_method.arguments[i].name, argument_type->julia_name));
		if (!p_godot_method.arguments[i].julia_default_value.is_empty()) {
			p_output.append(vformat(" = %s", p_godot_method.arguments[i].julia_default_value));
		}
	}
	p_output.append(")\n");
	p_output.append("\t\tif method_bind == C_NULL\n");
	p_output.append(vformat("\t\t\tmethod_bind = @ccall godot_julia_get_method_bind(string_names._%s::Ref{StringName}, string_names._%s::Ref{StringName})::Ptr{Nothing}\n", p_godot_type.name, p_godot_method.name));
	p_output.append("\t\tend\n");
	String ret_ptrcall_typed;
	const GodotType *return_type = nullptr;
	if (p_godot_method.return_type.name == "Cvoid") {
		ret_ptrcall_typed = "C_NULL::Ptr{Nothing}";
	} else {
		return_type = _get_type_or_null(p_godot_method.return_type);
		ERR_FAIL_NULL_MSG(return_type, vformat("Return type not found: %s", p_godot_method.return_type.name));

		p_output.append(vformat("\t\tret = %s\n", return_type->ptrcall_initial));

		ret_ptrcall_typed = vformat("ret::%s", return_type->ptrcall_type);
	}
	// TODO: Handle more return types.

	// Handle arguments.
	String args_ptrcall_typed;
	if (argc == 0) {
		args_ptrcall_typed = "C_NULL::Ptr{Nothing}";
	} else {
		p_output.append("\t\targs = [");
		for (int i = 0; i < argc; i++) {
			const GodotType *arg_type = _get_type_or_null(p_godot_method.arguments[i].type);
			p_output.append(vformat(arg_type->ptrcall_input, p_godot_method.arguments[i].name));
			if (i != argc - 1) {
				p_output.append(", ");
			}
		}
		p_output.append("]\n");
		args_ptrcall_typed = "args::Ref{Ptr{Nothing}}";
	}
	// TODO: Handle more complex argument types.

	p_output.append(vformat("\t\t@ccall godot_julia_method_bind_ptrcall(method_bind::Ptr{Nothing}, self.native_ptr::Ptr{Nothing}, %s, %s)::Cvoid\n", args_ptrcall_typed, ret_ptrcall_typed));

	if (return_type != nullptr) {
		p_output.append("\t\treturn ");
		p_output.append(vformat(return_type->ptrcall_output, "ret"));
		p_output.append("\n");
	}

	p_output.append("\tend\nend\n\n");
}

const BindingsGenerator::GodotType *BindingsGenerator::_get_type_or_null(const TypeReference &p_typeref) {
	HashMap<StringName, GodotType>::ConstIterator builtin_type_match = builtin_types.find(p_typeref.name);

	if (builtin_type_match) {
		return &builtin_type_match->value;
	}

	HashMap<StringName, GodotType>::ConstIterator obj_type_match = object_types.find(p_typeref.name);

	if (obj_type_match) {
		return &obj_type_match->value;
	}

	if (p_typeref.is_enum) {
		HashMap<StringName, GodotType>::ConstIterator enum_match = enum_types.find(p_typeref.name);

		if (enum_match) {
			return &enum_match->value;
		}

		// Enum not found. Most likely because none of its constants were bound, so it's empty. That's fine. Use int instead.
		HashMap<StringName, GodotType>::ConstIterator int_match = builtin_types.find("int");
		ERR_FAIL_NULL_V_MSG(int_match, nullptr, "Integer type not found");
		return &int_match->value;
	}

	return nullptr;
}

void BindingsGenerator::_populate_object_types() {
	object_types.clear();

	// Breadth-first search from Object.
	List<StringName> class_list;
	class_list.push_front("Object");

	while (class_list.size() > 0) {
		StringName class_name = class_list.front()->get();

		List<StringName> inheriters;
		ClassDB::get_direct_inheriters_from_class(class_name, &inheriters);
		for (StringName &inheriter : inheriters) {
			class_list.push_back(inheriter);
		}

		ClassDB::APIType api_type = ClassDB::get_api_type(class_name);

		if (api_type == ClassDB::API_NONE) {
			class_list.pop_front();
			continue;
		}

		if (!ClassDB::is_class_exposed(class_name)) {
			_log("Ignoring type '%s' because it's not exposed\n", String(class_name).utf8().get_data());
			class_list.pop_front();
			continue;
		}

		if (!ClassDB::is_class_enabled(class_name)) {
			_log("Ignoring type '%s' because it's not enabled\n", String(class_name).utf8().get_data());
			class_list.pop_front();
			continue;
		}

		ClassDB::ClassInfo *class_info = ClassDB::classes.getptr(class_name);

		GodotType godot_class;
		godot_class.name = class_name;
		godot_class.julia_name = class_name;
		godot_class.ptrcall_type = "Ptr{Nothing}";
		godot_class.ptrcall_initial = "C_NULL";
		godot_class.ptrcall_input = "%s.native_ptr";
		godot_class.ptrcall_output = vformat("%s(%%s)", class_name);
		godot_class.is_object_type = true;
		godot_class.is_singleton = Engine::get_singleton()->has_singleton(class_name);
		godot_class.is_instantiable = class_info->creation_func && !godot_class.is_singleton;
		godot_class.is_ref_counted = ClassDB::is_parent_class(class_name, "RefCounted");
		godot_class.parent_class_name = ClassDB::get_parent_class(class_name);

		String doc_name = String(godot_class.name).begins_with("_") ? String(godot_class.name).substr(1) : String(godot_class.name);
		godot_class.class_doc = &EditorHelp::get_doc_data()->class_list[doc_name];

		// TODO: Properties.

		// TODO: Virtual methods.

		// Populate methods.

		List<MethodInfo> method_list;
		ClassDB::get_method_list(class_name, &method_list, true);
		method_list.sort();

		for (const MethodInfo &method_info : method_list) {
			int argc = method_info.arguments.size();

			// TODO: Handle more complex argument types.
			bool arguments_supported = true;
			for (int i = 0; i < argc; i++) {
				Variant::Type argument_type = method_info.arguments[i].type;
				if (!(argument_type == Variant::NIL ||
							argument_type == Variant::BOOL ||
							argument_type == Variant::INT ||
							argument_type == Variant::FLOAT)) {
					arguments_supported = false;
					break;
				}
			}
			if (!arguments_supported) {
				continue;
			}

			if (method_info.name.is_empty()) {
				continue;
			}

			GodotMethod godot_method;
			godot_method.name = method_info.name;
			godot_method.julia_name = escape_julia_keyword(method_info.name);

			if (method_info.flags & METHOD_FLAG_STATIC) {
				godot_method.is_static = true;
			}

			// TODO: Handle static methods.
			if (godot_method.is_static) {
				continue;
			}

			if (method_info.flags & METHOD_FLAG_VIRTUAL) {
				godot_method.is_virtual = true;
			}

			// TODO: Handle virtual methods.
			if (godot_method.is_virtual) {
				continue;
			}

			PropertyInfo return_info = method_info.return_val;

			// TODO: Handle more complex return types.
			if (!(return_info.type == Variant::NIL ||
						return_info.type == Variant::BOOL ||
						return_info.type == Variant::INT ||
						return_info.type == Variant::FLOAT)) {
				continue;
			}

			MethodBind *method_bind = godot_method.is_virtual ? nullptr : ClassDB::get_method(class_name, method_info.name);

			godot_method.is_vararg = method_bind && method_bind->is_vararg();

			// TODO: Handle vararg methods.
			if (godot_method.is_vararg) {
				continue;
			}

			// TODO: Handle Object.free special case.

			if (return_info.type == Variant::NIL && return_info.usage & PROPERTY_USAGE_NIL_IS_VARIANT) {
				godot_method.return_type.name = "Variant";
			} else if (return_info.type == Variant::NIL) {
				godot_method.return_type.name = "Cvoid";
			} else if (return_info.type == Variant::INT &&
					return_info.usage & (PROPERTY_USAGE_CLASS_IS_ENUM | PROPERTY_USAGE_CLASS_IS_BITFIELD)) {
				godot_method.return_type.name = return_info.class_name;
				godot_method.return_type.is_enum = true;
			} else {
				godot_method.return_type.name = _get_type_name_from_meta(return_info.type, method_bind ? method_bind->get_argument_meta(-1) : (GodotTypeInfo::Metadata)method_info.return_val_metadata);
			}
			// TODO: Handle more complex return types.

			// Handle arguments.
			for (int i = 0; i < argc; i++) {
				PropertyInfo arginfo = method_info.arguments[i];

				String orig_arg_name = arginfo.name;

				GodotArgument godot_arg;
				godot_arg.name = orig_arg_name;

				if (arginfo.type == Variant::INT && arginfo.usage & (PROPERTY_USAGE_CLASS_IS_ENUM | PROPERTY_USAGE_CLASS_IS_BITFIELD)) {
					godot_arg.type.name = arginfo.class_name;
					godot_arg.type.is_enum = true;
				} else if (arginfo.type == Variant::NIL) {
					godot_arg.type.name = "Variant";
				} else {
					godot_arg.type.name = _get_type_name_from_meta(arginfo.type, method_bind ? method_bind->get_argument_meta(i) : (GodotTypeInfo::Metadata)method_info.get_argument_meta(i));
				}
				// TODO: Handle more complex argument types.

				godot_arg.name = escape_julia_keyword(godot_arg.name);

				if (method_bind && method_bind->has_default_argument(i)) {
					bool defval_ok = _arg_default_value_from_variant(method_bind->get_default_argument(i), godot_arg);
					ERR_FAIL_COND_MSG(!defval_ok,
							"Cannot determine default value for argument '" + orig_arg_name + "' of method '" + godot_class.name + "." + godot_method.name + "'.");
				}

				godot_method.arguments.push_back(godot_arg);
			}

			if (godot_class.class_doc) {
				for (int i = 0; i < godot_class.class_doc->methods.size(); i++) {
					if (godot_class.class_doc->methods[i].name == godot_method.name) {
						godot_method.method_doc = &godot_class.class_doc->methods[i];
						break;
					}
				}
			}

			godot_class.methods.push_back(godot_method);
		}

		// TODO: Signals.

		// TODO: Enums and constants.

		object_types.insert(godot_class.name, godot_class);

		class_list.pop_front();
	}
}

void BindingsGenerator::_populate_builtin_types() {
	GodotType godot_type;

	// bool
	godot_type.name = "bool";
	godot_type.julia_name = "Bool";
	godot_type.ptrcall_type = "Ref{Cint}";
	godot_type.ptrcall_initial = "Ref{Cint}(0)";
	godot_type.ptrcall_input = "Ref{Cint}(%s)";
	godot_type.ptrcall_output = "Bool(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// NOTE: The expected type for all integer types in ptrcall is int64_t.

	// sbyte
	godot_type.name = "sbyte";
	godot_type.julia_name = "Int8";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "Int8(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// short
	godot_type.name = "short";
	godot_type.julia_name = "Int16";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "Int16(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// int
	godot_type.name = "int";
	godot_type.julia_name = "Int32";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "Int32(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// long
	godot_type.name = "long";
	godot_type.julia_name = "Int64";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "%s[]";
	builtin_types.insert(godot_type.name, godot_type);

	// byte
	godot_type.name = "byte";
	godot_type.julia_name = "UInt8";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "UInt8(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// ushort
	godot_type.name = "ushort";
	godot_type.julia_name = "UInt16";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "UInt16(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// uint
	godot_type.name = "uint";
	godot_type.julia_name = "UInt32";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(%s)";
	godot_type.ptrcall_output = "UInt32(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// ulong
	godot_type.name = "ulong";
	godot_type.julia_name = "UInt64";
	godot_type.ptrcall_type = "Ref{Int64}";
	godot_type.ptrcall_initial = "Ref{Int64}(0)";
	godot_type.ptrcall_input = "Ref{Int64}(reinterpret(Int64, %s))";
	godot_type.ptrcall_output = "reinterpret(UInt64, %s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// NOTE: The expected type for all floating point types in ptrcall is double.

	// float
	godot_type.name = "float";
	godot_type.julia_name = "Float32";
	godot_type.ptrcall_type = "Ref{Float64}";
	godot_type.ptrcall_initial = "Ref{Float64}(0.0)";
	godot_type.ptrcall_input = "Ref{Float64}(%s)";
	godot_type.ptrcall_output = "Float32(%s[])";
	builtin_types.insert(godot_type.name, godot_type);

	// double
	godot_type.name = "double";
	godot_type.julia_name = "Float64";
	godot_type.ptrcall_type = "Ref{Float64}";
	godot_type.ptrcall_initial = "Ref{Float64}(0.0)";
	godot_type.ptrcall_input = "Ref{Float64}(%s)";
	godot_type.ptrcall_output = "%s[]";
	builtin_types.insert(godot_type.name, godot_type);

	// TODO: Struct types.

	// Variant
	godot_type.name = "Variant";
	godot_type.julia_name = "Variant";
	godot_type.ptrcall_type = "Ref{Variant}";
	godot_type.ptrcall_initial = "Variant()";
	godot_type.ptrcall_input = "Ref{Variant}(%s)";
	godot_type.ptrcall_output = "%s";
	builtin_types.insert(godot_type.name, godot_type);
}

void BindingsGenerator::_populate_global_constants() {
	HashMap<String, DocData::ClassDoc>::Iterator match = EditorHelp::get_doc_data()->class_list.find("@GlobalScope");

	CRASH_COND_MSG(!match, "Could not find '@GlobalScope' in DocData.");

	const DocData::ClassDoc &global_scope_doc = match->value;

	for (int i = 0; i < CoreConstants::get_global_constant_count(); i++) {
		String constant_name = CoreConstants::get_global_constant_name(i);

		const DocData::ConstantDoc *documentation = nullptr;
		for (int j = 0; j < global_scope_doc.constants.size(); j++) {
			const DocData::ConstantDoc &curr_documentation = global_scope_doc.constants[j];

			if (curr_documentation.name == constant_name) {
				documentation = &curr_documentation;
				break;
			}
		}

		int64_t constant_value = CoreConstants::get_global_constant_value(i);
		StringName enum_name = CoreConstants::get_global_constant_enum(i);

		GodotConstant gconstant(constant_name, constant_value);
		gconstant.documentation = documentation;

		if (enum_name != StringName()) {
			String julia_name = enum_name;
			// TODO: Maybe put the Variant.Type and Variant.Operator enums in a Variant submodule?
			if (julia_name == "Variant.Type") {
				julia_name = "VariantType";
			} else if (julia_name == "Variant.Operator") {
				julia_name = "VariantOperator";
			}
			GodotEnum genum(enum_name, julia_name);
			genum.is_flags = CoreConstants::is_global_constant_bitfield(i);
			List<GodotEnum>::Element *enum_match = global_enums.find(genum);
			if (enum_match) {
				enum_match->get().constants.push_back(gconstant);
			} else {
				genum.constants.push_back(gconstant);
				global_enums.push_back(genum);
			}
		} else {
			global_constants.push_back(gconstant);
		}
	}

	for (GodotEnum &godot_enum : global_enums) {
		GodotType enum_type;
		enum_type.is_enum = true;
		enum_type.name = godot_enum.name;
		enum_type.julia_name = godot_enum.julia_name;
		enum_type.ptrcall_type = "Ref{Cint}";
		enum_type.ptrcall_initial = "Ref{Cint}(0)";
		enum_type.ptrcall_input = "Ref{Cint}(%s)";
		enum_type.ptrcall_output = vformat("%s(%%s)", godot_enum.julia_name);
		enum_types.insert(enum_type.name, enum_type);
		// TODO: Handle prefixes?
	}

	// TODO: Vector2.Axis, Vector2I.Axis, Vector3.Axis, Vector3I.Axis?
}

template <typename... VarArgs>
void BindingsGenerator::_log(String p_format, const VarArgs... p_args) {
	if (log_print_enabled) {
		print_line(vformat(p_format, p_args...));
	}
}

void BindingsGenerator::_initialize() {
	initialized = false;
	EditorHelp::generate_doc(false);
	_populate_global_constants();
	_populate_builtin_types();
	_populate_object_types();
	initialized = true;
}

Error BindingsGenerator::generate_julia_project_file(const String &p_project_file) {
	ERR_FAIL_COND_V(!initialized, ERR_UNCONFIGURED);

	StringBuilder project_source;
	project_source.append("name = \"Godot\"\n");
	project_source.append("uuid = \"7de006e5-8474-4dac-b065-c8f25e0b26d0\"\n");
	project_source.append("version = \"0.1.0\"\n");
	project_source.append("authors = [\"Ricardo Buring <ricardo.buring@gmail.com>\"]\n");
	return _save_file(p_project_file, project_source);
}

Error BindingsGenerator::generate_julia_module(const String &p_module_dir) {
	ERR_FAIL_COND_V(!initialized, ERR_UNCONFIGURED);

	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	ERR_FAIL_COND_V(da.is_null(), ERR_CANT_CREATE);

	if (!DirAccess::exists(p_module_dir)) {
		Error err = da->make_dir_recursive(p_module_dir);
		ERR_FAIL_COND_V_MSG(err != OK, ERR_CANT_CREATE, "Cannot create directory '" + p_module_dir + "'.");
	}

	da->change_dir(p_module_dir);

	// Generate source file for global scope constants and enums.
	{
		StringBuilder constants_source;
		_generate_global_constants(constants_source);
		String output_file = p_module_dir.path_join("constants.jl");
		Error save_err = _save_file(output_file, constants_source);
		if (save_err != OK) {
			return save_err;
		}
	}

	// Generate source file for string names.
	{
		StringBuilder strings_source;
		_generate_string_names(strings_source);
		String output_file = p_module_dir.path_join("StringName.jl");
		Error save_err = _save_file(output_file, strings_source);
		if (save_err != OK) {
			return save_err;
		}
	}

	// Generate source file for variant.
	{
		StringBuilder variant_source;
		_generate_variant(variant_source);
		String output_file = p_module_dir.path_join("Variant.jl");
		Error save_err = _save_file(output_file, variant_source);
		if (save_err != OK) {
			return save_err;
		}
	}

	// Generate source files for object types.
	for (const KeyValue<StringName, GodotType> &E : object_types) {
		const GodotType &godot_type = E.value;

		if (godot_type.api_type == ClassDB::API_EDITOR) {
			continue;
		}

		StringBuilder object_type_source;
		_generate_julia_type(godot_type, object_type_source);
		String output_file = p_module_dir.path_join(godot_type.julia_name + ".jl");
		Error save_err = _save_file(output_file, object_type_source);
		if (save_err != OK) {
			return save_err;
		}
	}

	// Generate top-level module file.
	{
		StringBuilder module_source;
		module_source.append("module Godot\n\n");
		module_source.append("include(\"constants.jl\");\n\n");

		// TODO: Separate this.
		module_source.append("mutable struct GodotString\n\tcowdata::Ptr{Char}\nend\n\n");

		module_source.append("include(\"StringName.jl\");\n\n");

		module_source.append("include(\"Variant.jl\");\n\n");

		for (const KeyValue<StringName, GodotType> &E : object_types) {
			module_source.append(vformat("include(\"%s.jl\");\n", E.value.julia_name));
		}

		module_source.append("\nend # module\n");
		String output_file = p_module_dir.path_join("Godot.jl");
		Error save_err = _save_file(output_file, module_source);
		if (save_err != OK) {
			return save_err;
		}
	}

	return OK;
}

Error BindingsGenerator::generate_julia_package(const String &p_package_dir) {
	ERR_FAIL_COND_V(!initialized, ERR_UNCONFIGURED);

	// TODO: Convert to absolute path?
	String package_dir = p_package_dir;

	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	ERR_FAIL_COND_V(da.is_null(), ERR_CANT_CREATE);

	if (!DirAccess::exists(package_dir)) {
		Error err = da->make_dir_recursive(package_dir);
		ERR_FAIL_COND_V(err != OK, ERR_CANT_CREATE);
	}

	String project_file = package_dir.path_join("Project.toml");
	Error project_file_err = generate_julia_project_file(project_file);
	if (project_file_err != OK) {
		ERR_PRINT("Generation of the Julia project file failed.");
		return project_file_err;
	}

	String module_dir = package_dir.path_join("src");
	Error module_err = generate_julia_module(module_dir);
	if (module_err != OK) {
		ERR_PRINT("Generation of the Julia module failed.");
		return module_err;
	}

	_log("The Julia package " + String(JULIA_PKG_NAME) + " was successfully generated.\n");

	return OK;
}

Error BindingsGenerator::install_julia_package(const String &p_package_dir) {
	// TODO: Don't use Pkg.develop, and build a sysimage instead?
	jl_eval_string("using Pkg");
	jl_eval_string(("Pkg.develop(path=\"" + p_package_dir + "\")").utf8());

	if (jl_exception_occurred()) {
		// None of these allocate, so a gc-root (JL_GC_PUSH) is not necessary.
		jl_value_t *exception_str = jl_call2(jl_get_function(jl_base_module, "sprint"),
				jl_get_function(jl_base_module, "showerror"),
				jl_exception_occurred());
		ERR_FAIL_V_MSG(FAILED, jl_string_ptr(exception_str));
	}

	_log("\nThe Julia package " + String(JULIA_PKG_NAME) + " was successfully installed.\n");

	return OK;
}

void BindingsGenerator::precompile_julia_package() {
	jl_eval_string("Pkg.precompile()");
}

void JuliaBindingsGenerator::initialize() {
	List<String> cmdline_args = OS::get_singleton()->get_cmdline_user_args();

	if (cmdline_args.size() != 1) {
		ERR_PRINT("The glue directory must be passed as a user argument, after -- or ++.");
		return;
	}

	String glue_dir_path = cmdline_args[0];

	BindingsGenerator bindings_generator;
	bindings_generator.set_log_print_enabled(true);

	if (!bindings_generator.is_initialized()) {
		ERR_PRINT("Failed to initialize the bindings generator");
		return;
	}

	CRASH_COND(glue_dir_path.is_empty());

	if (bindings_generator.generate_julia_package(glue_dir_path.path_join(JULIA_PKG_NAME)) != OK) {
		ERR_PRINT("Failed to generate the Julia package.");
		return;
	}

	if (bindings_generator.install_julia_package(glue_dir_path.path_join(JULIA_PKG_NAME)) != OK) {
		ERR_PRINT("Failed to install the Julia package.");
		return;
	}

	bindings_generator.precompile_julia_package();
}

bool JuliaBindingsGenerator::process(double p_delta) {
	return true; // Exit.
}

#endif
