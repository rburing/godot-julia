#ifndef JULIA_SCRIPT_H
#define JULIA_SCRIPT_H

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/object/script_language.h"

#include <julia.h>

class JuliaScript : public Script {
	GDCLASS(JuliaScript, Script);

	String source_code;
	bool valid = false;
	bool tool = false;

	jl_module_t *julia_module = nullptr;

protected:
	void _notification(int p_what);
	static void _bind_methods();

	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _set(const StringName &p_name, const Variant &p_value);
	void _get_property_list(List<PropertyInfo> *p_properties) const;

	void _placeholder_erased(PlaceHolderScriptInstance *p_placeholder) override {}

public:
	bool can_instantiate() const override;

	Ref<Script> get_base_script() const override; //for script inheritance
	StringName get_global_name() const override;
	bool inherits_script(const Ref<Script> &p_script) const override;

	StringName get_instance_base_type() const override; // this may not work in all scripts, will return empty if so
	ScriptInstance *instance_create(Object *p_this) override;
	PlaceHolderScriptInstance *placeholder_instance_create(Object *p_this) override { return nullptr; }
	bool instance_has(const Object *p_this) const override;

	bool has_source_code() const override;
	String get_source_code() const override;
	void set_source_code(const String &p_code) override;
	Error reload(bool p_keep_state = false) override;

#ifdef TOOLS_ENABLED
	Vector<DocData::ClassDoc> get_documentation() const override;
	PropertyInfo get_class_category() const override;
#endif // TOOLS_ENABLED

	bool has_method(const StringName &p_method) const override;
	MethodInfo get_method_info(const StringName &p_method) const override;

	bool is_tool() const override { return tool; }
	bool is_valid() const override { return valid; }

	ScriptLanguage *get_language() const override;

	bool has_script_signal(const StringName &p_signal) const override;
	void get_script_signal_list(List<MethodInfo> *r_signals) const override;

	bool get_property_default_value(const StringName &p_property, Variant &r_value) const override;

	void update_exports() override {} //editor tool
	void get_script_method_list(List<MethodInfo> *p_list) const override;
	void get_script_property_list(List<PropertyInfo> *p_list) const override;

	int get_member_line(const StringName &p_member) const override { return -1; }

	void get_constants(HashMap<StringName, Variant> *p_constants) override {}
	void get_members(HashSet<StringName> *p_constants) override {}

	bool is_placeholder_fallback_enabled() const override { return false; }

	const Variant get_rpc_config() const override;

	JuliaScript() {}
};

class ResourceFormatLoaderJuliaScript : public ResourceFormatLoader {
	GDCLASS(ResourceFormatLoaderJuliaScript, ResourceFormatLoader);

protected:
	static void _bind_methods() {}

public:
	Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	void get_recognized_extensions(List<String> *p_extensions) const override;
	bool handles_type(const String &p_type) const override;
	String get_resource_type(const String &p_path) const override;
};

class ResourceFormatSaverJuliaScript : public ResourceFormatSaver {
	GDCLASS(ResourceFormatSaverJuliaScript, ResourceFormatSaver);

protected:
	static void _bind_methods() {}

public:
	Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	bool recognize(const Ref<Resource> &p_resource) const override;
	void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;
};

#endif // JULIA_SCRIPT_H