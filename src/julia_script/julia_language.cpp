#include "julia_language.h"

#include "julia_script.h"

JuliaLanguage *JuliaLanguage::singleton = nullptr;

void JuliaLanguage::_bind_methods() {
}

String JuliaLanguage::get_name() const {
	return "Julia";
}

/* LANGUAGE FUNCTIONS */

void JuliaLanguage::init() {
}

String JuliaLanguage::get_type() const {
	return "JuliaScript";
}

String JuliaLanguage::get_extension() const {
	return "jl";
}

void JuliaLanguage::finish() {
}

/* EDITOR FUNCTIONS */

void JuliaLanguage::get_reserved_words(List<String> *p_words) const {
}

bool JuliaLanguage::is_control_flow_keyword(String p_string) const {
	return false;
}

void JuliaLanguage::get_comment_delimiters(List<String> *p_delimiters) const {
}

void JuliaLanguage::get_string_delimiters(List<String> *p_delimiters) const {
}

Ref<Script> JuliaLanguage::make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const {
	Ref<JuliaScript> julia_script;
	julia_script.instantiate();
	// TODO: Implement script templates.
	return julia_script;
}

bool JuliaLanguage::validate(const String &p_script, const String &p_path, List<String> *r_functions, List<ScriptError> *r_errors, List<Warning> *r_warnings, HashSet<int> *r_safe_lines) const {
	// TODO: Implement validation.
	return true;
}

String JuliaLanguage::validate_path(const String &p_path) const {
	return "";
}

Script *JuliaLanguage::create_script() const {
	return memnew(JuliaScript);
}

int JuliaLanguage::find_function(const String &p_function, const String &p_code) const {
	return 0;
}

String JuliaLanguage::make_function(const String &p_class, const String &p_name, const PackedStringArray &p_args) const {
	return "";
}

void JuliaLanguage::auto_indent_code(String &p_code, int p_from_line, int p_to_line) const {
}

void JuliaLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {
}

void JuliaLanguage::add_named_global_constant(const StringName &p_name, const Variant &p_value) {
}

void JuliaLanguage::remove_named_global_constant(const StringName &p_name) {
}

/* MULTITHREAD FUNCTIONS */

void JuliaLanguage::thread_enter() {
}

void JuliaLanguage::thread_exit() {
}

/* DEBUGGER FUNCTIONS */

String JuliaLanguage::debug_get_error() const {
	return "";
}

int JuliaLanguage::debug_get_stack_level_count() const {
	return 0;
}

int JuliaLanguage::debug_get_stack_level_line(int p_level) const {
	return 0;
}

String JuliaLanguage::debug_get_stack_level_function(int p_level) const {
	return "";
}

String JuliaLanguage::debug_get_stack_level_source(int p_level) const {
	return "";
}

void JuliaLanguage::debug_get_stack_level_locals(int p_level, List<String> *p_locals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
}

void JuliaLanguage::debug_get_stack_level_members(int p_level, List<String> *p_members, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
}

void JuliaLanguage::debug_get_globals(List<String> *p_globals, List<Variant> *p_values, int p_max_subitems, int p_max_depth) {
}

String JuliaLanguage::debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems, int p_max_depth) {
	return "";
}

Vector<ScriptLanguage::StackInfo> JuliaLanguage::debug_get_current_stack_info() {
	return Vector<StackInfo>();
}

void JuliaLanguage::reload_all_scripts() {
}

void JuliaLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
}

/* LOADER FUNCTIONS */

void JuliaLanguage::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("jl");
}

void JuliaLanguage::get_public_functions(List<MethodInfo> *p_functions) const {
}

void JuliaLanguage::get_public_constants(List<Pair<String, Variant>> *p_constants) const {
}

void JuliaLanguage::get_public_annotations(List<MethodInfo> *p_annotations) const {
}

void JuliaLanguage::profiling_start() {
}

void JuliaLanguage::profiling_stop() {
}

int JuliaLanguage::profiling_get_accumulated_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0;
}

int JuliaLanguage::profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max) {
	return 0;
}

void JuliaLanguage::frame() {
}

bool JuliaLanguage::handles_global_class_type(const String &p_type) const {
	return false;
}

String JuliaLanguage::get_global_class_name(const String &p_path, String *r_base_type, String *r_icon_path) const {
	return "";
}

JuliaLanguage::JuliaLanguage() {
	ERR_FAIL_COND_MSG(singleton, "Julia singleton already exists.");
	singleton = this;
	string_names._script_source = StaticCString::create("script/source");
}

JuliaLanguage::~JuliaLanguage() {
	singleton = nullptr;
}
