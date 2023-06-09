#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "core/typedefs.h" // for DEBUG_METHODS_ENABLED

#include "julia_language.h"
#include "julia_script.h"

#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)
#include "editor/julia_bindings_generator.h"
#endif

#include <julia.h>

JuliaLanguage *script_language_jl = nullptr;
Ref<ResourceFormatLoaderJuliaScript> resource_loader_jl;
Ref<ResourceFormatSaverJuliaScript> resource_saver_jl;

void initialize_julia_script_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	jl_init();

	GDREGISTER_CLASS(JuliaScript);
#if defined(DEBUG_METHODS_ENABLED) && defined(TOOLS_ENABLED)
	GDREGISTER_CLASS(JuliaBindingsGenerator);
#endif

	script_language_jl = memnew(JuliaLanguage);
	ScriptServer::register_language(script_language_jl);

	resource_loader_jl.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_jl);

	resource_saver_jl.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_jl);
}

void uninitialize_julia_script_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ScriptServer::unregister_language(script_language_jl);

	if (script_language_jl) {
		memdelete(script_language_jl);
	}

	ResourceLoader::remove_resource_format_loader(resource_loader_jl);
	resource_loader_jl.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_jl);
	resource_saver_jl.unref();

	jl_atexit_hook(0);
}
