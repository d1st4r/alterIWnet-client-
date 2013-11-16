// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: asset restrictions
//
// Initial author: NTAuthority
// Started: 2011-05-20
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

#define CURRENT_ZONE_NAME (char*)(*(DWORD*)0x112A680 + 4)

StompHook dbAddXAssetHook;
DWORD dbAddXAssetHookLoc = 0x5BB650;
DWORD dbAddXAssetHookRet = 0x5BB657;

typedef const char* (__cdecl * DB_GetXAssetNameHandler_t)(void* asset);
DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = (DB_GetXAssetNameHandler_t*)0x799328;

bool CanWeLoadAsset(assetType_t type, void* entry)
{
	const char* name = DB_GetXAssetNameHandlers[type](entry);
	if (type == ASSET_TYPE_WEAPON)
	{
		if (!strcmp(CURRENT_ZONE_NAME, "patch_mp"))
		{
			return false;
		}
	}

	return true;
}

void __declspec(naked) DB_AddXAssetHookStub()
{
	__asm
	{
		mov eax, [esp + 4]
		mov ecx, [esp + 8]

		push ecx
		push eax
		call CanWeLoadAsset
		add esp, 08h

		test al, al
		jz doNotLoad

		mov eax, [esp + 8]
		sub esp, 14h
		jmp dbAddXAssetHookRet

doNotLoad:
		mov eax, [esp + 8]
		retn
	}
}

void PatchMW2_AssetRestrict()
{
	dbAddXAssetHook.initialize(7, (PBYTE)dbAddXAssetHookLoc);
	dbAddXAssetHook.installHook(DB_AddXAssetHookStub, true, false);
}