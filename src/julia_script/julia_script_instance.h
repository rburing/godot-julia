#ifndef JULIA_SCRIPT_INSTANCE_H
#define JULIA_SCRIPT_INSTANCE_H

#include "core/object/script_language.h"

#include <julia.h>

class JuliaScript;

class JuliaScriptInstance : ScriptInstance {
	friend class JuliaScript;

	Object *owner = nullptr;
	Ref<JuliaScript> script;
	jl_value_t *julia_instance;

public:
	bool set(const StringName &p_name, const Variant &p_value) override;
	bool get(const StringName &p_name, Variant &r_ret) const override;
	void get_property_list(List<PropertyInfo> *p_properties) const override;
	Variant::Type get_property_type(const StringName &p_name, bool *r_is_valid = nullptr) const override;

	bool property_can_revert(const StringName &p_name) const override;
	bool property_get_revert(const StringName &p_name, Variant &r_ret) const override;

	Object *get_owner() override { return nullptr; }
	void get_property_state(List<Pair<StringName, Variant>> &state) override;

	void get_method_list(List<MethodInfo> *p_list) const override;
	bool has_method(const StringName &p_method) const override;

	Variant callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) override;

	Variant call_const(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) override; // implement if language supports const functions

	void notification(int p_notification) override;
	String to_string(bool *r_valid) override;

	//this is used by script languages that keep a reference counter of their own
	//you can make make Ref<> not die when it reaches zero, so deleting the reference
	//depends entirely from the script

	void refcount_incremented() override {}
	bool refcount_decremented() override { return true; } //return true if it can die

	Ref<Script> get_script() const override;

	bool is_placeholder() const override { return false; }

	void property_set_fallback(const StringName &p_name, const Variant &p_value, bool *r_valid) override;
	Variant property_get_fallback(const StringName &p_name, bool *r_valid) override;

	ScriptLanguage *get_language() override;

	JuliaScriptInstance(const Ref<JuliaScript> &p_script);
	virtual ~JuliaScriptInstance() {}
};

#endif // JULIA_SCRIPT_INSTANCE_H
