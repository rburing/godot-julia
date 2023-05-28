#include "julia_script_instance.h"

#include "julia_script.h"

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
	return Variant();
}

Variant JuliaScriptInstance::call_const(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	return Variant();
}

void JuliaScriptInstance::notification(int p_notification) {
}

String JuliaScriptInstance::to_string(bool *r_valid) {
	if (r_valid) {
		*r_valid = false;
	}
	return String();
}

Ref<Script> JuliaScriptInstance::get_script() const {
	return Ref<Script>();
}

void JuliaScriptInstance::property_set_fallback(const StringName &p_name, const Variant &p_value, bool *r_valid) {
}

Variant JuliaScriptInstance::property_get_fallback(const StringName &p_name, bool *r_valid) {
	return Variant();
}

ScriptLanguage *JuliaScriptInstance::get_language() {
	return nullptr;
}

JuliaScriptInstance::JuliaScriptInstance(const Ref<JuliaScript> &p_script) :
		script(p_script) {
}
