#include "godot_julia.h"

#include "core/object/class_db.h"
#include "core/object/method_bind.h"
#include "core/string/string_name.h"
#include "core/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

GJ_API void godot_julia_string_new_from_utf16_chars(String *r_string, const char16_t *p_chars) {
	memnew_placement(r_string, String());
	r_string->parse_utf16(p_chars);
}

GJ_API void godot_julia_string_destroy(String *p_string) {
	p_string->~String();
}

GJ_API void godot_julia_string_name_new_from_string(StringName *r_string_name, const String *p_name) {
	memnew_placement(r_string_name, StringName(*p_name));
}

GJ_API void godot_julia_string_name_destroy(StringName *p_string_name) {
	p_string_name->~StringName();
}

GJ_API MethodBind *godot_julia_get_method_bind(const StringName *p_classname, const StringName *p_methodname) {
	return ClassDB::get_method(*p_classname, *p_methodname);
}

GJ_API void godot_julia_method_bind_ptrcall(MethodBind *p_method_bind, Object *p_instance, const void **p_args, void *p_ret) {
	p_method_bind->ptrcall(p_instance, p_args, p_ret);
}

#ifdef __cplusplus
}
#endif
