#include "julia_bindings_generator.h"

#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)

#include "core/core_constants.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "editor/editor_help.h"

#include <julia.h>

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
			.strip_edges();
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
		p_output.append(vformat("\t%s::StringName\n", string_name));
	}
	p_output.append("\tStringNames() = new()\n");
	p_output.append("end\n\n");
	p_output.append("string_names = StringNames()\n\n");

	p_output.append("function initialize_string_names()\n");
	for (const StringName &string_name : string_names) {
		p_output.append(vformat("\tstring_names.%s = StringName(\"%s\")\n", string_name, string_name));
	}
	p_output.append("end\n\n");
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
	// TODO: Return type.

	// TODO: Arguments.

	p_output.append(vformat("function %s(self::Godot%s)\n", p_godot_method.julia_name, p_godot_type.julia_name));
	p_output.append(vformat("\tmethod_bind = @ccall godot_julia_get_method_bind(string_names.%s::Ref{StringName}, string_names.%s::Ref{StringName})::Ptr{Nothing}\n", p_godot_type.name, p_godot_method.name));
	p_output.append("\tret = GodotVariant((0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0))\n");
	p_output.append("\t@ccall godot_julia_method_bind_ptrcall(method_bind::Ptr{Nothing}, self.native_ptr::Ptr{Nothing}, C_NULL::Ptr{Nothing}, ret::Ref{GodotVariant})::Cvoid\n");
	p_output.append("end\n\n");

	// TODO: Clean up strings.
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

			// TODO: Handle arguments.
			if (argc != 0) {
				continue;
			}

			if (method_info.name.is_empty()) {
				continue;
			}

			GodotMethod godot_method;
			godot_method.name = method_info.name;
			godot_method.julia_name = method_info.name;

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

			// TODO: Handle return types.
			if (return_info.type != Variant::NIL) {
				continue;
			}

			MethodBind *method_bind = godot_method.is_virtual ? nullptr : ClassDB::get_method(class_name, method_info.name);

			godot_method.is_vararg = method_bind && method_bind->is_vararg();

			// TODO: Handle vararg methods.
			if (godot_method.is_vararg) {
				continue;
			}

			// TODO: Handle arguments.

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

		// TODO: Avoid this.
		module_source.append("mutable struct GodotVariant\n\tidk::NTuple{20, UInt8} # assuming real_t is float\nend\n\n");

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
}

bool JuliaBindingsGenerator::process(double p_delta) {
	return true; // Exit.
}

#endif
