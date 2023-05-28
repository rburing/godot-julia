#ifndef JULIA_SCRIPT_REGISTER_TYPES_H
#define JULIA_SCRIPT_REGISTER_TYPES_H

#include "modules/register_module_types.h"

void initialize_julia_script_module(ModuleInitializationLevel p_level);
void uninitialize_julia_script_module(ModuleInitializationLevel p_level);

#endif // JULIA_SCRIPT_REGISTER_TYPES_H