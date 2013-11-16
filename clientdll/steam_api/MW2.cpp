// ==========================================================
// project 'secretSchemes'
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Game-specific code implementations.
//
// Initial author: NTAuthority
// Started: 2011-05-04
// ==========================================================

#include "StdInc.h"
#include "MW2.h"

// function definitions
Com_Error_t Com_Error = (Com_Error_t)0x4B22D0;
Com_Printf_t Com_Printf = (Com_Printf_t)0x402500;
Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = (Cmd_ExecuteSingleCommand_t)0x609540;
FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x4F4B90;
Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x4FC7E0;
Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x470090;
Cmd_AddServerCommand_t Cmd_AddServerCommand = (Cmd_AddServerCommand_t)0x4DCE00;
Dvar_RegisterBool_t Dvar_RegisterBool = (Dvar_RegisterBool_t)0x4CE1A0;
Dvar_RegisterInt_t Dvar_RegisterInt = (Dvar_RegisterInt_t)0x479830;
SV_GameSendServerCommand_t SV_GameSendServerCommand = (SV_GameSendServerCommand_t)0x4BC3A0;
NET_AdrToString_t NET_AdrToString = (NET_AdrToString_t)0x469880;
Com_Milliseconds_t Com_Milliseconds = (Com_Milliseconds_t)0x42A660;
Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x404B20;
Dvar_FindVar_t Dvar_FindVar = (Dvar_FindVar_t)0x4D5390;
NET_StringToAdr_t NET_StringToAdr = (NET_StringToAdr_t)0x409010;
Dvar_InfoString_Big_t Dvar_InfoString_Big = (Dvar_InfoString_Big_t)0x4D98A0;
G_LogPrintf_t G_LogPrintf = (G_LogPrintf_t)0x4B0150;
Cmd_SetAutoComplete_t Cmd_SetAutoComplete = (Cmd_SetAutoComplete_t)0x40EDC0;

// other stuff
CommandCB_t Cbuf_AddServerText_f = (CommandCB_t)0x4BB9B0;

// console commands
DWORD* cmd_id = (DWORD*)0x1AAC5D0;
DWORD* cmd_argc = (DWORD*)0x1AAC614;
DWORD** cmd_argv = (DWORD**)0x1AAC634;

/*
============
Cmd_Argc
============
*/
/*int		Cmd_Argc( void ) {
	return cmd_argc[*cmd_id];
}*/

/*
============
Cmd_Argv
============
*/
/*char	*Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= cmd_argc[*cmd_id] ) {
		return "";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);	
}*/

DWORD* cmd_id_sv = (DWORD*)0x1ACF8A0;
DWORD* cmd_argc_sv = (DWORD*)0x1ACF8E4;
DWORD** cmd_argv_sv = (DWORD**)0x1ACF904;

/*
============
Cmd_Argc
============
*/
int		Cmd_ArgcSV( void ) {
	return cmd_argc_sv[*cmd_id_sv];
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_ArgvSV( int arg ) {
	if ( (unsigned)arg >= cmd_argc_sv[*cmd_id_sv] ) {
		return "";
	}
	return (char*)(cmd_argv_sv[*cmd_id_sv][arg]);	
}

void SV_GetStatus(svstatus_t* status)
{
	if (!status) return;

	int clientCount = 0;
	BYTE* clientAddress = (BYTE*)0x31D9390;

	for (int i = 0; i < *(int*)0x31D938C; i++) {
		if (*clientAddress >= 3) {
			clientCount++;
		}

		clientAddress += 681872;
	}

	status->curClients = clientCount;
	status->maxClients = Party_NumPublicSlots();

	const char* mapname = GetStringConvar("mapname");
	strcpy(status->map, mapname);
}

typedef void (__cdecl* sendOOB_t)(int, int, int, int, int, int, const char*);
sendOOB_t OOBPrint = (sendOOB_t)0x4AEF00;

void OOBPrintT(int type, netadr_t netadr, const char* message)
{
	int* adr = (int*)&netadr;

	OOBPrint(type, *adr, *(adr + 1), *(adr + 2), 0xFFFFFFFF, *(adr + 4), message);
}

void NET_OutOfBandPrint(int type, netadr_t adr, const char* message, ...)
{
	va_list args;
	char buffer[65535];

	va_start(args, message);
	_vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	OOBPrintT(type, adr, buffer);
}

typedef struct party_s
{
	BYTE pad1[544];
	int privateSlots;
	int publicSlots;
} party_t;

static party_t** partyIngame = (party_t**)0x1081C00;

int Party_NumPublicSlots(party_t* party)
{
	return party->publicSlots + party->privateSlots;
}

int Party_NumPublicSlots()
{
	return Party_NumPublicSlots(*partyIngame);
}