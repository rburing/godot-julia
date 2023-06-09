#ifndef JULIA_BINDINGS_GENERATOR_H
#define JULIA_BINDINGS_GENERATOR_H

#include "core/typedefs.h" // for DEBUG_METHODS_ENABLED

#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)

#include "core/doc_data.h"
#include "core/os/main_loop.h"
#include "core/string/string_builder.h"
#include "core/string/ustring.h"
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

	bool initialized = false;
	bool log_print_enabled = false;

	List<GodotEnum> global_enums;
	List<GodotConstant> global_constants;

	void _populate_global_constants();

	void _generate_global_constants(StringBuilder &p_output);

	template <typename... VarArgs>
	void _log(String p_format, const VarArgs... p_args);

	void _initialize();

public:
	Error generate_julia_project_file(const String &p_project_file);
	Error generate_julia_module(const String &p_module_dir);
	Error generate_julia_package(const String &p_package_dir);
	Error install_julia_package(const String &p_package_dir);

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
