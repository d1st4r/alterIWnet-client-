#include "StdInc.h"
#include "Hooking.h"

CallHook uiLoadHook1;
DWORD uiLoadHook1Loc = 0x60B4AC;

CallHook ffLoadHook1;
DWORD ffLoadHook1Loc = 0x506BC7;

CallHook ffLoadHook2;
DWORD ffLoadHook2Loc = 0x506B25;

struct XZoneInfo {
	const char* name;
	int type1;
	int type2;
};

typedef void (*LoadFF1_t)(XZoneInfo* data, int count, int unknown);
LoadFF1_t LoadFF1 = (LoadFF1_t)0x4E5930;

void __cdecl UILoadHook1(XZoneInfo* data, int count, int unknown) {
	XZoneInfo newData[5];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * 2);
	newData[0].name = "dlc1_ui_mp";
	newData[0].type1 = 3;
	newData[0].type2 = 0;
	newData[1].name = "dlc2_ui_mp";
	newData[1].type1 = 3;
	newData[1].type2 = 0;

	return LoadFF1(newData, 2, unknown);
}

void __declspec(naked) UILoadHook1Stub() {
	__asm jmp UILoadHook1
}

void __cdecl FFLoadHook1(XZoneInfo* data, int count, int unknown) {
	int newCount = count + 2;

	XZoneInfo newData[20];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * count);
	newData[0].type1 = 1;
	newData[1].type1 = 1;
	newData[count].name = "dlc1_ui_mp";
	newData[count].type1 = 2;
	newData[count].type2 = 0;
	newData[count + 1].name = "dlc2_ui_mp";
	newData[count + 1].type1 = 2;
	newData[count + 1].type2 = 0;
	return LoadFF1(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook1Stub() {
	__asm {
		jmp FFLoadHook1
	}
}

void __cdecl FFLoadHook2(XZoneInfo* data, int count, int unknown) {
	int newCount = count + 1;

	XZoneInfo newData[20];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * count);
	newData[count].name = "patch_alter_mp";
	newData[count].type1 = 0;
	newData[count].type2 = 0;

	return LoadFF1(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook2Stub() {
	__asm {
		jmp FFLoadHook2
	}
}

// zone\dlc patches
CallHook zoneLoadHook1;
DWORD zoneLoadHook1Loc = 0x5BC82C;

CallHook zoneLoadHook2;
DWORD zoneLoadHook2Loc = 0x4CCE08;

char zone_language[64];
char* zone_dlc = "zone\\dlc\\";
char* zone_alter = "zone\\alter\\";
char* loadedPath = "";
char* zonePath = "";

char* GetZoneLocation(const char* name) {
	char path[MAX_PATH];

	_snprintf(path, MAX_PATH, "zone\\%s\\%s.ff", "alter\\", name);
	
	if (FileExists(path)) {
		return zone_alter;
	}

	_snprintf(path, MAX_PATH, "zone\\%s\\%s.ff", "dlc\\", name);

	if (FileExists(path)) {
		return zone_dlc;
	}

	return zone_language;	
}

char* GetZonePath(const char* fileName) {
	char* language;
	DWORD getLang = 0x45CBA0;

	__asm {
		call getLang
		mov language, eax
	}

	_snprintf(zone_language, 64, "zone\\%s\\", language);

	// we do it a lot simpler than IW did.
	return GetZoneLocation(fileName);
}

void __declspec(naked) ZoneLoadHook1Stub() {
	__asm {
		mov loadedPath, esi // MAKE SURE TO EDIT THIS REGISTER FOR OTHER EXE VERSIONS
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
		retn
	}
}

void __declspec(naked) ZoneLoadHook2Stub() {
	__asm {
		mov loadedPath, eax
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
		retn
	}
}

// RAWFILE SUPPORT
// more functions
typedef void* (__cdecl * LoadModdableRawfile_t)(int a1, const char* filename);
LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x61ABC0;

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc(const char* filename, void* buffer, int bufSize) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub() {
	__asm jmp LoadModdableRawfileFunc
}

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc2(const char* filename) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub2() {
	__asm jmp LoadModdableRawfileFunc2
}

CallHook rawFileHook1;
DWORD rawFileHook1Loc = 0x632155;

CallHook rawFileHook2;
DWORD rawFileHook2Loc = 0x5FA46C;

CallHook rawFileHook3;
DWORD rawFileHook3Loc = 0x5FA4D6;

CallHook rawFileHook4;
DWORD rawFileHook4Loc = 0x6321EF;

CallHook ignoreEntityHook;
DWORD ignoreEntityHookLoc = 0x5FE8DE;

bool IgnoreEntityHookFunc(const char* entity);

typedef struct  
{
	char unknown[16];
} xAssetEntry_t;

static xAssetEntry_t xEntries[789312];

void ReallocEntries();

typedef int (__cdecl * DB_GetXAssetSizeHandler_t)();

void** DB_XAssetPool = (void**)0x7998A8;
unsigned int* g_poolSize = (unsigned int*)0x7995E8;
DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = (DB_GetXAssetSizeHandler_t*)0x799488;

void* ReallocateAssetPool(assetType_t type, unsigned int newSize)
{
	int elSize = DB_GetXAssetSizeHandlers[type]();
	void* poolEntry = malloc(newSize * elSize);
	DB_XAssetPool[type] = poolEntry;
	g_poolSize[type] = newSize;
	return poolEntry;
}

void PatchMW2_Load()
{
	// rawfile moddability
	rawFileHook1.initialize((PBYTE)rawFileHook1Loc);
	rawFileHook1.installHook(LoadModdableRawfileStub, false);

	rawFileHook2.initialize((PBYTE)rawFileHook2Loc);
	rawFileHook2.installHook(LoadModdableRawfileStub, false);

	rawFileHook3.initialize((PBYTE)rawFileHook3Loc);
	rawFileHook3.installHook(LoadModdableRawfileStub, false);

	rawFileHook4.initialize((PBYTE)rawFileHook4Loc);
	rawFileHook4.installHook(LoadModdableRawfileStub2, false);

	// fastfile loading hooks
	uiLoadHook1.initialize((PBYTE)uiLoadHook1Loc);
	uiLoadHook1.installHook(UILoadHook1Stub, false);

	ffLoadHook1.initialize((PBYTE)ffLoadHook1Loc);
	ffLoadHook1.installHook(FFLoadHook1Stub, false);

	ffLoadHook2.initialize((PBYTE)ffLoadHook2Loc);
	ffLoadHook2.installHook(FFLoadHook2Stub, false);

	zoneLoadHook1.initialize((PBYTE)zoneLoadHook1Loc);
	zoneLoadHook1.installHook(ZoneLoadHook1Stub, false);

	zoneLoadHook2.initialize((PBYTE)zoneLoadHook2Loc);
	zoneLoadHook2.installHook(ZoneLoadHook2Stub, false);

	//ReallocateAssetPool(ASSET_TYPE_WEAPON, 1400);
	return; // TODO: change this for build 159, possibly use dynamic offsets from array start
	// reallocation of image assets
	*(DWORD*)0x799618 = 0xE00 * 2; // originally 0xE00
	*(DWORD*)0x7998D8 = (DWORD)malloc((0xE00 * 2) * 0x20);

	// reallocation of loaded_sound assets
	*(DWORD*)0x799624 = 0x546 * 2; // assetID 13
	*(DWORD*)0x7998E4 = (DWORD)malloc((0x546 * 2) * 0x2C);

	// reallocation of fx assets
	*(DWORD*)0x799668 = 600 * 2; // assetID 30
	*(DWORD*)0x799928 = (DWORD)malloc((600 * 2) * 32);

	// reallocation of localize assets
	*(DWORD*)0x79965C = 7000 * 2; // assetID 27
	*(DWORD*)0x79991C = (DWORD)malloc((7000 * 2) * 8);

	// reallocation of xanim assets
	*(DWORD*)0x7995F8 = 4096 * 2; // assetID 2
	*(DWORD*)0x7998B8 = (DWORD)malloc((4096 * 2) * 88);

	// reallocation of xmodel assets
	*(DWORD*)0x799600 = 1536 * 2; // assetID 4
	*(DWORD*)0x7998C0 = (DWORD)malloc((1536 * 2) * 304);

	// reallocation of physpreset assets
	*(DWORD*)0x7995F0 = 64 * 2; // assetID 0
	*(DWORD*)0x7998B0 = (DWORD)malloc((64 * 2) * 44);

	// set GameWorldSp pool to be a random 1024 byte buffer (GameWorldMp is just a name it seems, as such useless)
	DWORD gwSP = (DWORD)malloc(1024);

	*(DWORD*)0x7998F4 = gwSP;

	// set this bit of data loaded from the GameWorld to point to new offset in GameWorldSp (seems it got added in IW4, as such 'aww')
	//*(DWORD*)0x4137C7 = gwSP + 52;

	// try not to re/unload ui FFs
	*(BYTE*)0x60DAF0 = 0xC3;

	// allow loading of IWffu (unsigned) files
	*(BYTE*)0x414AB9 = 0xEB; // main function
	*(WORD*)0x50B3C7 = 0x9090; // DB_AuthLoad_InflateInit

	// temporary stuff to not load from maps/mp/, needs to be changed to check for mp_*
	//strcpy((char*)0x70D6CC, "maps/%s.d3dbsp");

	// ignore 'node_' entities as well
	ignoreEntityHook.initialize((PBYTE)ignoreEntityHookLoc);
	ignoreEntityHook.installHook((void(*)())IgnoreEntityHookFunc, false);

	// reallocate XAsset entries
	ReallocEntries();

	*(DWORD*)0x5BE4E0 = 789312;
}

bool IgnoreEntityHookFunc(const char* entity)
{
	return (!strncmp(entity, "dyn_", 4) || !strncmp(entity, "node_", 5)/* || !strncmp(entity, "weapon_", 7)*/);
}

void ReallocEntries()
{
	int newsize = 516 * 2048;
	//newEnts = malloc(newsize);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x1352CD8;
	unsigned int origMax = 0x1352CE8;

	unsigned int difference = (unsigned int)xEntries - origMin;

	// scan the .text memory
	char* scanMin = (char*)0x401000;
	char* scanMax = (char*)0x6D5B8A;
	char* current = scanMin;

	for (; current < scanMax; current += 1) {
		unsigned int* intCur = (unsigned int*)current;

		// if the address points to something within our range of interest
		if (*intCur == origMin || *intCur == origMax) {
			// patch it
			*intCur += difference;
		}
	}
}