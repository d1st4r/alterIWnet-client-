// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: remote console support
//          (server-side component)
//
// Initial author: NTAuthority
// Started: 2011-05-21 (copied from PatchMW2Status.cpp)
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

dvar_t* sv_rconPassword;
netadr_t redirectAddress;

void SV_FlushRedirect( char *outputbuf ) {
	NET_OutOfBandPrint(1, redirectAddress, "print\n%s", outputbuf);
}

#define SV_OUTPUTBUF_LENGTH ( 16384 - 16 )
char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

extern DWORD* cmd_id_sv;
extern dvar_t* aiw_remoteKick;

void SVC_RemoteCommand(netadr_t from, void* msg)
{
	bool valid;
	unsigned int time;
	char remaining[1024] = {0};
	size_t current = 0;
	static unsigned int lasttime = 0;

	remaining[0] = '\0';

	time = Com_Milliseconds();
	if (time < (lasttime + 100))
	{
		return;
	}
	lasttime = time;

	if (!sv_rconPassword)
	{
		return;
	}

	if (!strlen(sv_rconPassword->current.string) || strcmp(Cmd_Argv(1), sv_rconPassword->current.string))
	{
		valid = false;
		Com_Printf(1, "Bad rcon from %s:\n%s\n", NET_AdrToString(from), Cmd_Argv(2));
	}
	else
	{
		valid = true;
		Com_Printf(1, "Rcon from %s:\n%s\n", NET_AdrToString(from), Cmd_Argv(2));
	}

	// start redirecting all print outputs to the packet
	redirectAddress = from;
	Com_BeginRedirect(sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if (!valid)
	{
		if (!strlen(sv_rconPassword->current.string))
		{
			Com_Printf(0, "The server must set 'rcon_password' for clients to use 'rcon'.\n");
		}
		else
		{
			Com_Printf(0, "Invalid password.\n");
		}
	}
	else
	{
		remaining[0] = 0;

		if (Cmd_Argc() > 2)
		{
			for (int i = 2; i < Cmd_Argc(); i++)
			{
				current = Com_AddToString(Cmd_Argv(i), remaining, current, sizeof(remaining), true);
				current = Com_AddToString(" ", remaining, current, sizeof(remaining), false);
			}
		}
		else
		{
			memset(remaining, 0, sizeof(remaining));
			strncpy(remaining, Cmd_Argv(2), sizeof(remaining) - 1);
		}

		Cmd_ExecuteSingleCommand(0, 0, remaining);
	}

	Com_EndRedirect();

	if (strlen(remaining) > 0)
	{
		Com_Printf(0, "handled rcon: %s\n", remaining);
	}
}

// patch component

// TODO: make this more generic for actual 'getServersResponse' code
CallHook gsrCmpHook;
DWORD gsrCmpHookLoc = 0x5AA709;

int GsrCmpHookFunc(const char* a1, const char* a2)
{
	int result = _strnicmp(a1, "rcon", 4);

	return result;
}

void __declspec(naked) GsrCmpHookStub()
{
	__asm jmp GsrCmpHookFunc
}

bool wasGetServers;

void __declspec(naked) SVC_RemoteCommandStub()
{
	__asm
	{
		push ebp //C54
		// esp = C54h?
		mov eax, [esp + 0C54h + 14h]
		push eax
		mov eax, [esp + 0C58h + 10h]
		push eax
		mov eax, [esp + 0C5Ch + 0Ch]
		push eax
		mov eax, [esp + 0C60h + 08h]
		push eax
		mov eax, [esp + 0C64h + 04h]
		push eax
		call SVC_RemoteCommand
		add esp, 14h
		add esp, 4h
		mov al, 1
		//C50
		pop edi //C4C
		pop esi //C48
		pop ebp //C44
		pop ebx //C40
		add esp, 0C40h
		retn
	}
}

void PatchMW2_RemoteConsoleServer()
{
	// maximum size in NET_OutOfBandPrint
	*(DWORD*)0x4AEF08 = 0x1FFFC;
	*(DWORD*)0x4AEFA3 = 0x1FFFC;

	// client-side OOB handler
	*(int*)0x5AA715 = ((DWORD)SVC_RemoteCommandStub) - 0x5AA713 - 6;

	gsrCmpHook.initialize((PBYTE)gsrCmpHookLoc);
	gsrCmpHook.installHook(GsrCmpHookStub, false);
}