#include "julia_script.h"

#include "julia_language.h"

void JuliaScript::_notification(int p_what) {
}

void JuliaScript::_bind_methods() {
}

bool JuliaScript::can_instantiate() const {
	return false;
}

Ref<Script> JuliaScript::get_base_script() const {
	return Ref<Script>();
}

StringName JuliaScript::get_global_name() const {
	return StringName();
}

bool JuliaScript::inherits_script(const Ref<Script> &p_script) const {
	return false;
}

StringName JuliaScript::get_instance_base_type() const {
	return StringName();
}

ScriptInstance *JuliaScript::instance_create(Object *p_this) {
	return nullptr;
}

bool JuliaScript::instance_has(const Object *p_this) const {
	return false;
}

bool JuliaScript::has_source_code() const {
	return true;
}

String JuliaScript::get_source_code() const {
	return source_code;
}

void JuliaScript::set_source_code(const String &p_code) {
	source_code = p_code;
}

Error JuliaScript::reload(bool p_keep_state) {
	return Error();
}

#ifdef TOOLS_ENABLED

Vector<DocData::ClassDoc> JuliaScript::get_documentation() const {
	return Vector<DocData::ClassDoc>();
}

PropertyInfo JuliaScript::get_class_category() const {
	return PropertyInfo();
}

#endif

bool JuliaScript::has_method(const StringName &p_method) const {
	return false;
}

MethodInfo JuliaScript::get_method_info(const StringName &p_method) const {
	return MethodInfo();
}

bool JuliaScript::is_tool() const {
	return false;
}

bool JuliaScript::is_valid() const {
	return false;
}

ScriptLanguage *JuliaScript::get_language() const {
	return JuliaLanguage::get_singleton();
}

bool JuliaScript::has_script_signal(const StringName &p_signal) const {
	return false;
}

void JuliaScript::get_script_signal_list(List<MethodInfo> *r_signals) const {
}

bool JuliaScript::get_property_default_value(const StringName &p_property, Variant &r_value) const {
	return false;
}

void JuliaScript::get_script_method_list(List<MethodInfo> *p_list) const {
}

void JuliaScript::get_script_property_list(List<PropertyInfo> *p_list) const {
}

const Variant JuliaScript::get_rpc_config() const {
	return Variant();
}

/* SCRIPT RESOURCE FORMAT LOADER */

Ref<Resource> ResourceFormatLoaderJuliaScript::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	Ref<JuliaScript> julia_script;
	julia_script.instantiate();

	Ref<FileAccess> source_file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(source_file.is_null(), julia_script, "Failed to read Julia script file '" + p_path + "'.");
	julia_script->set_source_code(source_file->get_as_text());
	julia_script->set_path(p_original_path);

	if (r_error) {
		*r_error = OK;
	}

	return julia_script;
}

void ResourceFormatLoaderJuliaScript::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("jl");
}

bool ResourceFormatLoaderJuliaScript::handles_type(const String &p_type) const {
	return p_type == "Script" || p_type == JuliaLanguage::get_singleton()->get_type();
}

String ResourceFormatLoaderJuliaScript::get_resource_type(const String &p_path) const {
	return p_path.get_extension().to_lower() == "jl" ? JuliaLanguage::get_singleton()->get_type() : "";
}

/* SCRIPT RESOURCE FORMAT SAVER */

Error ResourceFormatSaverJuliaScript::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<JuliaScript> julia_script = p_resource;
	ERR_FAIL_COND_V(julia_script.is_null(), ERR_INVALID_PARAMETER);

	String source = julia_script->get_source_code();

	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Julia script file '" + p_path + "'.");
	file->store_string(source);
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		return ERR_CANT_CREATE;
	}
	return OK;
}

bool ResourceFormatSaverJuliaScript::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<JuliaScript>(p_resource.ptr()) != nullptr;
}

void ResourceFormatSaverJuliaScript::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	if (Object::cast_to<JuliaScript>(p_resource.ptr())) {
		p_extensions->push_back("jl");
	}
}
