// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: console redirection
//
// Initial author: NTAuthority
// Started: 2010-09-29
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

// code
static char *rd_buffer;
static unsigned int rd_buffersize;
static void ( *rd_flush )( char *buffer );

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char *) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

// com_printf to com_printmessage
CallHook printfHook;
DWORD printfHookLoc = 0x402540;

// 'local' vars
int level;
const char* msg;
int wtf;

void __declspec(naked) PrintfHookStub()
{
	__asm push eax
	__asm mov eax, [esp + 8h]
	__asm mov level, eax
	__asm mov eax, [esp + 0Ch]
	__asm mov msg, eax
	__asm mov eax, [esp + 10h]
	__asm mov wtf, eax
	__asm pop eax

	if (rd_buffer)
	{
		// quick fix for some stack fuckups here
		__asm pushad
		if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
			rd_flush( rd_buffer );
			*rd_buffer = 0;
		}
		strncat( rd_buffer, msg, rd_buffersize );

		__asm popad
		__asm retn
	}

	__asm jmp printfHook.pOriginal
}

void PatchMW2_Redirect()
{
	printfHook.initialize((PBYTE)printfHookLoc);
	printfHook.installHook(PrintfHookStub, false);
}