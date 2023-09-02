#include "julia_script_instance.h"

#include "julia_language.h"
#include "julia_script.h"
#include "julia_variant.h"

bool JuliaScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	return false;
}

bool JuliaScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	return false;
}

void JuliaScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
}

Variant::Type JuliaScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	return Variant::Type();
}

void JuliaScriptInstance::validate_property(PropertyInfo &p_property) const {
}

bool JuliaScriptInstance::property_can_revert(const StringName &p_name) const {
	return false;
}

bool JuliaScriptInstance::property_get_revert(const StringName &p_name, Variant &r_ret) const {
	return false;
}

void JuliaScriptInstance::get_property_state(List<Pair<StringName, Variant>> &state) {
}

void JuliaScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
}

bool JuliaScriptInstance::has_method(const StringName &p_method) const {
	return false;
}

Variant JuliaScriptInstance::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	jl_function_t *function = jl_get_function(script->julia_module, String(p_method).utf8());
	if (!function) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Put arguments on the stack and call the function.
	jl_value_t **args = (jl_value_t **)alloca(sizeof(jl_value_t *) * (p_argcount + 1));
	JL_GC_PUSHARGS(args, p_argcount + 1);
	args[0] = julia_instance;
	for (int i = 0; i < p_argcount; i++) {
		args[i + 1] = julia_value_from_variant(p_args[i]);
	}
	jl_value_t *julia_ret = jl_call(function, args, p_argcount + 1);
	JL_GC_POP();

	if (jl_exception_occurred()) {
		// None of these allocate, so a gc-root (JL_GC_PUSH) is not necessary.
		jl_value_t *exception_str = jl_call2(jl_get_function(jl_base_module, "sprint"),
				jl_get_function(jl_base_module, "showerror"),
				jl_exception_occurred());
		ERR_FAIL_V_MSG(Variant(), "Julia method " + p_method + " in " + script->get_path() + " throws an exception: " + jl_string_ptr(exception_str));
	}

	return variant_from_julia_value(julia_ret);
}

Variant JuliaScriptInstance::call_const(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	return Variant();
}

void JuliaScriptInstance::notification(int p_notification, bool p_reversed) {
}

String JuliaScriptInstance::to_string(bool *r_valid) {
	if (r_valid) {
		*r_valid = false;
	}
	return String();
}

Ref<Script> JuliaScriptInstance::get_script() const {
	return script;
}

void JuliaScriptInstance::property_set_fallback(const StringName &p_name, const Variant &p_value, bool *r_valid) {
}

Variant JuliaScriptInstance::property_get_fallback(const StringName &p_name, bool *r_valid) {
	return Variant();
}

ScriptLanguage *JuliaScriptInstance::get_language() {
	return JuliaLanguage::get_singleton();
}

JuliaScriptInstance::JuliaScriptInstance(const Ref<JuliaScript> &p_script, Object *p_owner) :
		script(p_script), owner(p_owner) {
	jl_value_t *base_obj = jl_new_struct((jl_datatype_t *)p_script->julia_new_param_type,
			jl_box_voidpointer((void *)owner));
	julia_instance = jl_call1(script->julia_new, base_obj);
	if (jl_exception_occurred()) {
		// None of these allocate, so a gc-root (JL_GC_PUSH) is not necessary.
		jl_value_t *exception_str = jl_call2(jl_get_function(jl_base_module, "sprint"),
				jl_get_function(jl_base_module, "showerror"),
				jl_exception_occurred());
		ERR_FAIL_MSG("Julia script " + script->get_path() + " module's new() method throws an exception: " + jl_string_ptr(exception_str));
		// This invalid JuliaScriptInstance will be deleted in JuliaScript::instance_create.
	}

	// Rooting to protect from the garbage collector.
	jl_function_t *setindex = jl_get_function(jl_base_module, "setindex!");
	jl_call3(setindex, script->julia_instances, julia_instance, julia_instance);
}

JuliaScriptInstance::~JuliaScriptInstance() {
	// Unrooting to release to the garbage collector.
	jl_function_t *delete_shriek = jl_get_function(jl_base_module, "delete!");
	jl_call2(delete_shriek, script->julia_instances, julia_instance);
}
