#include "julia_script.h"

#include "julia_language.h"
#include "julia_script_instance.h"

#include "core/os/thread.h"

void JuliaScript::_notification(int p_what) {
}

void JuliaScript::_bind_methods() {
}

bool JuliaScript::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == JuliaLanguage::get_singleton()->string_names._script_source) {
		r_ret = get_source_code();
		return true;
	}
	return false;
}

bool JuliaScript::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == JuliaLanguage::get_singleton()->string_names._script_source) {
		set_source_code(p_value);
		reload();
		return true;
	}
	return false;
}

void JuliaScript::_get_property_list(List<PropertyInfo> *p_properties) const {
	p_properties->push_back(PropertyInfo(Variant::STRING, JuliaLanguage::get_singleton()->string_names._script_source, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
}

bool JuliaScript::can_instantiate() const {
#ifdef TOOLS_ENABLED
	bool extra_cond = tool || ScriptServer::is_scripting_enabled();
#else
	bool extra_cond = true;
#endif
	return valid && extra_cond;
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
#ifdef DEBUG_ENABLED
	CRASH_COND(!valid);
#endif

	JuliaScriptInstance *instance = memnew(JuliaScriptInstance(Ref<JuliaScript>(this), p_this));
	if (!instance->julia_instance) {
		memdelete(instance);
		// At this point an error message was already printed in the JuliaScriptInstance constructor.
		return nullptr;
	}

	// TODO: Check inheritance.

	instance->owner->set_script_instance(instance);

	return instance;
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
	valid = false;
	julia_module = nullptr;

	// TODO: Handle reloading requests from threads.
	ERR_FAIL_COND_V_MSG(Thread::get_caller_id() != Thread::get_main_id(), FAILED, "Tried to reload Julia script " + get_path() + " from a separate thread; this is not supported for now.");

	jl_value_t *julia_module_maybe = jl_eval_string(source_code.utf8());
	if (jl_exception_occurred()) {
		// None of these allocate, so a gc-root (JL_GC_PUSH) is not necessary.
		jl_value_t *exception_str = jl_call2(jl_get_function(jl_base_module, "sprint"),
				jl_get_function(jl_base_module, "showerror"),
				jl_exception_occurred());
		ERR_FAIL_V_MSG(FAILED, "Julia script " + get_path() + " throws an exception: " + jl_string_ptr(exception_str));
	}

	ERR_FAIL_COND_V_MSG(!jl_is_module(julia_module_maybe), FAILED, "Julia script " + get_path() + " is not a module");

	jl_function_t *julia_new_maybe = jl_get_function((jl_module_t *)julia_module_maybe, "new");
	ERR_FAIL_NULL_V_MSG(julia_new_maybe, FAILED, "Julia script " + get_path() + " module does not a have a 'new' function");

	jl_function_t *methods = jl_get_function(jl_base_module, "methods");
	jl_value_t *julia_new_methods = jl_call1(methods, julia_new_maybe);
	jl_function_t *length = jl_get_function(jl_base_module, "length");
	jl_value_t *julia_new_methods_count = jl_call1(length, julia_new_methods);
	ERR_FAIL_COND_V_MSG(jl_unbox_int64(julia_new_methods_count) != 1, FAILED, "Julia script " + get_path() + " module's 'new' function should have exactly one method definition");

	jl_function_t *getindex = jl_get_function(jl_base_module, "getindex");
	jl_value_t *julia_new_method = jl_call2(getindex, julia_new_methods, jl_box_int64(1));
	jl_value_t *julia_new_signature = jl_get_field(julia_new_method, "sig");
	jl_value_t *julia_new_parameters = jl_get_field(julia_new_signature, "parameters");
	ERR_FAIL_COND_V_MSG(jl_svec_len(julia_new_parameters) != 2, FAILED, "Julia script " + get_path() + " module's 'new' method should take exactly one argument");

	jl_value_t *julia_new_param_type_maybe = jl_svec_data(julia_new_parameters)[1];
	ERR_FAIL_COND_V_MSG(!jl_is_structtype(julia_new_param_type_maybe), FAILED, "Julia script " + get_path() + " module's 'new' method should take a struct type as its argument");

	valid = true;
	julia_module = (jl_module_t *)julia_module_maybe;
	julia_new = julia_new_maybe;
	julia_new_param_type = (jl_datatype_t *)julia_new_param_type_maybe;

	// Rooting to protect from the garbage collector.
	jl_binding_t *b_module = jl_get_binding_wr(jl_main_module, julia_module->name, 1);
	jl_checked_assignment(b_module, (jl_value_t *)julia_module);

	jl_binding_t *b_instances = jl_get_binding_wr(julia_module, jl_symbol("#GODOT_INSTANCES#"), 1);
	julia_instances = jl_eval_string("IdDict()");
	jl_checked_assignment(b_instances, julia_instances);

	// TODO: Update script class info.

	return OK;
}

#ifdef TOOLS_ENABLED

Vector<DocData::ClassDoc> JuliaScript::get_documentation() const {
	return Vector<DocData::ClassDoc>();
}

String JuliaScript::get_class_icon_path() const {
	return "";
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

	julia_script->reload();

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
