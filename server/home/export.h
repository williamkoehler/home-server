#pragma once
#ifdef _WIN32
#ifdef HOME_COMPILE
#define HOME_EXPORT __declspec(dllexport)
#else
#define HOME_EXPORT __declspec(dllimport)
#endif
#else
#define HOME_EXPORT
#endif

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT
#endif