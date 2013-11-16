#include "StdInc.h"
#include "Hooking.h"

CallHook uiLoadHook1;
DWORD uiLoadHook1Loc = 0x60DB4C;

CallHook ffLoadHook1;
DWORD ffLoadHook1Loc = 0x50D387;

CallHook ffLoadHook2;
DWORD ffLoadHook2Loc = 0x50D2E5;
//DWORD ffLoadHook1Loc = 0x50DD67; 

struct FFLoad1 {
	const char* name;
	int type1;
	int type2;
};

typedef void (*LoadFF1_t)(FFLoad1* data, int count, int unknown);
LoadFF1_t LoadFF1 = (LoadFF1_t)0x44A730;

typedef void (__cdecl * CommandCB_t)(void);

typedef void (__cdecl * RegisterCommand_t)(const char* name, CommandCB_t callback, DWORD* data, char);
RegisterCommand_t RegisterCommand = (RegisterCommand_t)0x50ACE0;

RegisterCommand_t RegisterCommand_l = (RegisterCommand_t)0x470090;

void __cdecl UILoadHook1(FFLoad1* data, int count, int unknown) {
	FFLoad1 newData[5];
	memcpy(&newData[0], data, sizeof(FFLoad1) * 2);
	newData[2].name = "dlc1_ui_mp";
	newData[2].type1 = 3;
	newData[2].type2 = 0;
	newData[3].name = "dlc2_ui_mp";
	newData[3].type1 = 3;
	newData[3].type2 = 0;
	//newData[4].name = "alter_ui_mp";
	//newData[4].type1 = 3;
	//newData[4].type2 = 0;

	return LoadFF1(newData, 4, unknown);
}

void __declspec(naked) UILoadHook1Stub() {
	__asm jmp UILoadHook1
}

/*void __stdcall FFLoadHook1() {
	FFLoad1 newData;
	newData.name = "dlc1_ui_mp";
	//newData.type1 = 2;
	newData.type1 = 0;
	newData.type2 = 0;

	LoadFF1(&newData, 1, 0);
}

void __declspec(naked) FFLoadHook1Stub() {
	__asm {
		pushad
		call FFLoadHook1
		popad
		jmp ffLoadHook1.pOriginal
	}
}*/

void __cdecl FFLoadHook1(FFLoad1* data, int count, int unknown) {
	int newCount = count + 2;

	FFLoad1 newData[20];
	memcpy(&newData[0], data, sizeof(FFLoad1) * count);
	newData[count].name = "dlc1_ui_mp";
	newData[count].type1 = 2;
	newData[count].type2 = 0;
	newData[count + 1].name = "dlc2_ui_mp";
	newData[count + 1].type1 = 2;
	newData[count + 1].type2 = 0;
	//newData[count + 2].name = "alter_ui_mp";
	//newData[count + 2].type1 = 2;
	//newData[count + 2].type2 = 0;
/*
	char debugString[255];
	for (int i = 0; i < newCount; i++) {
		_snprintf(debugString, 255, "file %s %d %d", newData[i].name, newData[i].type1, newData[i].type2);
		OutputDebugStringA(debugString);
	}
*/
	return LoadFF1(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook1Stub() {
	__asm {
		jmp FFLoadHook1
	}
}

void __cdecl FFLoadHook2(FFLoad1* data, int count, int unknown) {
	int newCount = count + 1;

	FFLoad1 newData[20];
	memcpy(&newData[0], data, sizeof(FFLoad1) * count);
	newData[count].name = "patch_alter_mp";
	newData[count].type1 = 0;
	newData[count].type2 = 0;

	/*
	char debugString[255];
	for (int i = 0; i < newCount; i++) {
		_snprintf(debugString, 255, "file %s %d %d", newData[i].name, newData[i].type1, newData[i].type2);
		OutputDebugStringA(debugString);
	}
	*/

	return LoadFF1(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook2Stub() {
	__asm {
		jmp FFLoadHook2
	}
}

// zone\dlc patches
CallHook zoneLoadHook1;
DWORD zoneLoadHook1Loc = 0x5BFD4C;

CallHook zoneLoadHook2;
DWORD zoneLoadHook2Loc = 0x45AC18;

CallHook zoneLoadHook3;
DWORD zoneLoadHook3Loc = 0x4A94A8;

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
	DWORD getLang = 0x4D9BA0;

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
LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x4BE4F0;

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
DWORD rawFileHook1Loc = 0x6332F5;

CallHook rawFileHook2;
DWORD rawFileHook2Loc = 0x5FCFDC;

CallHook rawFileHook3;
DWORD rawFileHook3Loc = 0x5FD046;

CallHook rawFileHook4;
DWORD rawFileHook4Loc = 0x63338F;

void PatchMW2_Load()
{
	// rawfile moddability
	rawFileHook1.initialize("aaaaa", (PBYTE)rawFileHook1Loc);
	rawFileHook1.installHook(LoadModdableRawfileStub, false);

	rawFileHook2.initialize("aaaaa", (PBYTE)rawFileHook2Loc);
	rawFileHook2.installHook(LoadModdableRawfileStub, false);

	rawFileHook3.initialize("aaaaa", (PBYTE)rawFileHook3Loc);
	rawFileHook3.installHook(LoadModdableRawfileStub, false);

	rawFileHook4.initialize("aaaaa", (PBYTE)rawFileHook4Loc);
	rawFileHook4.installHook(LoadModdableRawfileStub2, false);

	// fastfile loading hooks
	uiLoadHook1.initialize("aaaaa", (PBYTE)uiLoadHook1Loc);
	uiLoadHook1.installHook(UILoadHook1Stub, false);

	ffLoadHook1.initialize("aaaaa", (PBYTE)ffLoadHook1Loc);
	ffLoadHook1.installHook(FFLoadHook1Stub, false);

	ffLoadHook2.initialize("aaaaa", (PBYTE)ffLoadHook2Loc);
	ffLoadHook2.installHook(FFLoadHook2Stub, false);

	zoneLoadHook1.initialize("aaaaa", (PBYTE)zoneLoadHook1Loc);
	zoneLoadHook1.installHook(ZoneLoadHook1Stub, false);

	zoneLoadHook2.initialize("aaaaa", (PBYTE)zoneLoadHook2Loc);
	zoneLoadHook2.installHook(ZoneLoadHook2Stub, false);

	zoneLoadHook3.initialize("aaaaa", (PBYTE)zoneLoadHook3Loc);
	zoneLoadHook3.installHook(ZoneLoadHook2Stub, false);

	
}