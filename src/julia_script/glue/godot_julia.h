#ifndef GODOT_JULIA_H
#define GODOT_JULIA_H

#ifdef _WIN32
#define GJ_API __declspec(dllexport)
#else
#define GJ_API
#endif

#endif // GODOT_JULIA_H