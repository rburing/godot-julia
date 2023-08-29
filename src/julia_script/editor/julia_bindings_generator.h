#ifndef JULIA_BINDINGS_GENERATOR_H
#define JULIA_BINDINGS_GENERATOR_H

#include "core/typedefs.h" // for DEBUG_METHODS_ENABLED

#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)

#include "core/doc_data.h"
#include "core/os/main_loop.h"
#include "core/string/string_builder.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/templates/list.h"

#define JULIA_PKG_NAME "Godot.jl"
#define JULIA_MODULE_NAME "Godot"

class BindingsGenerator {
	struct GodotConstant {
		String name;
		int64_t value = 0;
		const DocData::ConstantDoc *documentation;

		GodotConstant() {}

		GodotConstant(const String &p_name, int64_t p_value) {
			name = p_name;
			value = p_value;
		}
	};

	struct GodotEnum {
		StringName name;
		String julia_name;
		String julia_qualified_name;
		List<GodotConstant> constants;
		bool is_flags = false;

		_FORCE_INLINE_ bool operator==(const GodotEnum &p_ienum) const {
			return p_ienum.name == name;
		}

		GodotEnum() {}

		GodotEnum(const StringName &p_name, const String p_julia_name) {
			name = p_name;
			julia_name = p_julia_name;
		}
	};

	struct GodotProperty {
		StringName name;
		String julia_name;
		int index = 0;

		StringName setter;
		StringName getter;

		const DocData::PropertyDoc *prop_doc;
	};

	struct TypeReference {
		StringName name;
		bool is_enum = false;
		List<TypeReference> generic_type_parameters;

		TypeReference() {}

		TypeReference(const StringName &p_name) {
			name = p_name;
		}
	};

	struct GodotArgument {
		String name;
		TypeReference type;

		Variant default_value;
		String julia_default_value;

		GodotArgument() {}
	};

	struct GodotMethod {
		StringName name;
		String julia_name;

		bool is_static = false;
		bool is_virtual = false;
		bool is_vararg = false;

		TypeReference return_type;

		List<GodotArgument> arguments;

		const DocData::MethodDoc *method_doc = nullptr;

		GodotMethod() {}
	};

	struct GodotType {
		StringName name;
		String julia_name;

		bool is_enum = false;
		bool is_object_type = false;
		bool is_singleton = false;
		bool is_instantiable = false;
		bool is_ref_counted = false;

		// The reference type for the argument/return value passed into ptrcall.
		String ptrcall_type;
		// The initial value for the return value passed into ptrcall.
		String ptrcall_initial;
		// The format string to obtain the argument reference value passed into ptrcall.
		String ptrcall_input;
		// The format string to obtain the final return value from the ptrcall return value.
		String ptrcall_output;

		StringName parent_class_name;
		ClassDB::APIType api_type = ClassDB::API_NONE;

		const DocData::ClassDoc *class_doc = nullptr;

		List<GodotConstant> constants;
		List<GodotEnum> enums;
		List<GodotMethod> methods;
		List<GodotProperty> properties;

		GodotType() {}
	};

	bool initialized = false;
	bool log_print_enabled = false;

	List<GodotEnum> global_enums;
	List<GodotConstant> global_constants;

	HashMap<StringName, GodotType> builtin_types;
	HashMap<StringName, GodotType> enum_types;
	HashMap<StringName, GodotType> object_types;

	bool _arg_default_value_from_variant(const Variant &p_val, GodotArgument &r_iarg);

	void _populate_global_constants();
	void _populate_builtin_types();
	void _populate_object_types();

	const GodotType *_get_type_or_null(const TypeReference &p_typeref);

	void _generate_global_constants(StringBuilder &p_output);
	void _generate_string_names(StringBuilder &p_output);
	void _generate_variant(StringBuilder &p_output);
	void _generate_julia_type(const GodotType &p_godot_type, StringBuilder &p_output);
	void _generate_julia_method(const GodotType &p_godot_type, const GodotMethod &p_godot_method, StringBuilder &p_output);
	void _generate_julia_properties(const GodotType &p_godot_type, StringBuilder &p_output);

	template <typename... VarArgs>
	void _log(String p_format, const VarArgs... p_args);

	void _initialize();

public:
	Error generate_julia_project_file(const String &p_project_file);
	Error generate_julia_module(const String &p_module_dir);
	Error generate_julia_package(const String &p_package_dir);
	Error install_julia_package(const String &p_package_dir);
	void precompile_julia_package();

	_FORCE_INLINE_ bool is_log_print_enabled() { return log_print_enabled; }
	_FORCE_INLINE_ void set_log_print_enabled(bool p_enabled) { log_print_enabled = p_enabled; }
	_FORCE_INLINE_ bool is_initialized() { return initialized; }

	BindingsGenerator() {
		_initialize();
	}
};

class JuliaBindingsGenerator : public MainLoop {
	GDCLASS(JuliaBindingsGenerator, MainLoop);

	void initialize() override;
	bool process(double p_delta) override;
};

#endif

#endif // JULIA_BINDINGS_GENERATOR_H
