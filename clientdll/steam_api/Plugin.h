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

#if papi2
#include <iw4plugin.h>
#include <vector>

typedef struct pluginData_s
{
	HMODULE module;
	pluginDef_t* definition;
} pluginData_t;

#define MAX_PLUGINS 64

class PluginCode
{
private:
	static pluginData_t* AllocatePlugin();
public:
	static void Initialize();
	static void LoadPlugins();
	static pluginData_t* LoadPlugin(const char* filename);
};

typedef int (__cdecl * Plugin_APIVersion_t)();
typedef void (__cdecl * Plugin_Initialize_t)(pluginImport_s* import, pluginDef_s** definition, pluginHandle_t handle);
#endif