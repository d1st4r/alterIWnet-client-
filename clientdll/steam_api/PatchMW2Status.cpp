// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: status queries
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

#pragma unmanaged
// get convar string
char* GetStringConvar(char* key) {
	dvar_t* var = Dvar_FindVar(key);
	
	if (!var) return "";

	return var->current.string;
}

// getstatus/getinfo OOB packets
CallHook oobHandlerHook;
DWORD oobHandlerHookLoc = 0x6267EB;

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x469AC0;

dvar_t** sv_privateClients = (dvar_t**)0x2098D8C;

void HandleGetInfoOOB(netadr_t from, void* msg) {
	int clientCount = 0;
	BYTE* clientAddress = (BYTE*)0x31D9390;
	char infostring[MAX_INFO_STRING];

	for (int i = 0; i < *(int*)0x31D938C; i++) {
		if (*clientAddress >= 3) {
			clientCount++;
		}

		clientAddress += 681872;
	}

	infostring[0] = 0;

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	Info_SetValueForKey(infostring, "hostname", GetStringConvar("sv_hostname"));
	Info_SetValueForKey(infostring, "gamename", "IW4");
	Info_SetValueForKey(infostring, "protocol", va("%i", PROTOCOL));
	Info_SetValueForKey(infostring, "mapname", GetStringConvar("mapname"));
	Info_SetValueForKey(infostring, "clients", va("%i", clientCount));
	Info_SetValueForKey(infostring, "sv_privateClients", va("%i", (*sv_privateClients)->current.integer));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", Party_NumPublicSlots()));
	Info_SetValueForKey(infostring, "gametype", GetStringConvar("g_gametype"));
	Info_SetValueForKey(infostring, "pure", "1");
	Info_SetValueForKey(infostring, "fs_game", GetStringConvar("fs_game"));
	Info_SetValueForKey(infostring, "shortversion", VERSION);

	bool hardcore = 0;
	dvar_t* g_hardcore = 0;
	g_hardcore = Dvar_FindVar("g_hardcore");
	
	if (g_hardcore)
	{
		hardcore = g_hardcore->current.boolean;
	}

	Info_SetValueForKey(infostring, "hc", va("%i", hardcore));

	NET_OutOfBandPrint(1, from, "infoResponse\n%s", infostring);
}

void HandleGetStatusOOB(netadr_t from, void* msg) {
	char infostring[8192];
	char player[1024];
	char status[2048];
	int playerLength = 0;
	int statusLength = 0;
	BYTE* clientAddress = (BYTE*)0x31D9390;

	//strncpy(infostring, Dvar_InfoString_Big(1028), 8192);
	strncpy(infostring, Dvar_InfoString_Big(1024), 1024);

	char* hostname = GetStringConvar("sv_hostname");

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	//Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *(int*)0x31D938C));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", Party_NumPublicSlots()));
	Info_SetValueForKey(infostring, "protocol", va("%i", PROTOCOL));
	Info_SetValueForKey(infostring, "shortversion", VERSION);

	for (int i = 0; i < *(int*)0x31D938C; i++) {
		if (*clientAddress >= 3) { // connected
			int score = SV_GameClientNum_Score(i);
			int ping = *(WORD*)(clientAddress + 135880);
			char* name = (char*)(clientAddress + 135844);

			_snprintf(player, sizeof(player), "%i %i \"%s\"\n", score, ping, name);

			playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) ) {
				break;
			}

			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}

		clientAddress += 681872;
	}

	status[statusLength] = '\0';

	NET_OutOfBandPrint(1, from, "statusResponse\n%s\n%s", infostring, status);
}

void HandleCustomOOB(const char* commandName, netadr_t from, void* msg) {
	if (!strcmp(commandName, "getinfo")) {
		return HandleGetInfoOOB(from, msg);
	}

	if (!strcmp(commandName, "getstatus")) {
		return HandleGetStatusOOB(from, msg);
	}
}

void __declspec(naked) OobHandlerHookStub() {
	__asm {
		// esp + 408h
		push esi
		mov eax, [esp + 40Ch + 14h]
		push eax
		mov eax, [esp + 410h + 10h]
		push eax
		mov eax, [esp + 414h + 0Ch]
		push eax
		mov eax, [esp + 418h + 08h]
		push eax
		mov eax, [esp + 41Ch + 04h]
		push eax
		push edi
		call HandleCustomOOB
		add esp, 1Ch
		jmp oobHandlerHook.pOriginal
	}
}

// entry point
void PatchMW2_Status()
{
	oobHandlerHook.initialize((PBYTE)oobHandlerHookLoc);
	oobHandlerHook.installHook(OobHandlerHookStub, false);

	// remove SV_WaitServer; might be optional these days
	//memset((void*)0x446EF6, 0x90, 5);
}