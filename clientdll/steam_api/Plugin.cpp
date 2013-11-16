// ==========================================================
// iw4svr project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Basic implementation for plugin code
//
// Initial author: NTAuthority
// Started: 2011-02-10
// ==========================================================

#include "StdInc.h"

#ifdef PAPI2
/*
#include "Plugin.h"

static pluginData_t _plugins[MAX_PLUGINS];
static int _numPlugins;

void PluginCode::Initialize()
{
	return;
	Com_Printf(0, "Plugin code initializing.\n");
	LoadPlugins();
}

void PluginCode::LoadPlugins()
{
	WIN32_FIND_DATA findData;
	char filename[MAX_PATH];
	HANDLE hSearch = FindFirstFile("plugins\\*.dll", &findData);

	if (hSearch > 0)
	{
		do
		{
			_snprintf(filename, sizeof(filename), "plugins\\%s", findData.cFileName);

			LoadPlugin(filename);
		} while (FindNextFile(hSearch, &findData));

		FindClose(hSearch);
	}
}

pluginData_t* PluginCode::LoadPlugin(const char* filename)
{
	// load the library
	HMODULE module = LoadLibrary(filename);

	Com_Printf(0, "Loading plugin %s.\n", filename);

	if (!module)
	{
		Com_Printf(0, "LoadLibrary returned 0x%08x.\n", GetLastError());
		return NULL;
	}

	// check the API version
	Plugin_APIVersion_t plAPIVersion = (Plugin_APIVersion_t)GetProcAddress(module, "Plugin_APIVersion");

	if (!plAPIVersion)
	{
		Com_Printf(0, "Couldn't GPA on Plugin_APIVersion. This might not be an IW4 plugin DLL.\n");
		return NULL;
	}

	int pluginVersion = plAPIVersion();
	if (pluginVersion != IW4_PLUGIN_VERSION)
	{
		Com_Printf(0, "Incompatible API version (got: %d, expected: %d)\n", pluginVersion, IW4_PLUGIN_VERSION);
		return NULL;
	}

	// get the initialize function
	Plugin_Initialize_t plInitialize = (Plugin_Initialize_t)GetProcAddress(module, "Plugin_Initialize");

	if (!plInitialize)
	{
		Com_Printf(0, "Couldn't GPA on Plugin_Initialize.\n");
		return NULL;
	}

	// initialize data structures
	pluginData_t* data = AllocatePlugin();
	data->module = module;

	// call the initialization function
	plInitialize(/*import* /NULL, &data->definition, data);

	return data;
}

pluginData_t* PluginCode::AllocatePlugin()
{
	if (_numPlugins > MAX_PLUGINS)
	{
		Com_Error(0, "Maximum amount of plugins exceeded.");
		return NULL;
	}

	return &_plugins[_numPlugins++];
}
*/
#endif