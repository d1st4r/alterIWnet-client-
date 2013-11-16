// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#include <time.h>
#include <winsock.h>
#include <dbghelp.h>

void PatchMW2_159();

void Sys_RunInit()
{
	PatchMW2();
}

void PatchMW2()
{
	// check version
	if (!strcmp((char*)0x6E9638, "177")) { // 1.0.159 (iw4m) version-specific address
		DetermineGameFlags();
		PatchMW2_159();
		return;
	}

	TerminateProcess(GetCurrentProcess(), 0);
}

CallHook winMainInitHook;
DWORD winMainInitHookLoc = 0x4513D0;

void InstallCrashHandler();

void __declspec(naked) WinMainInitHookStub()
{
	InstallCrashHandler();

	__asm {
		jmp winMainInitHook.pOriginal
	}
}

CallHook sysInitHook;
DWORD sysInitHookLoc = 0x60BDBF;

void DoSysInit();

void __declspec(naked) SysInitHookStub()
{
	DoSysInit();

	__asm
	{
		jmp sysInitHook.pOriginal
	}
}

void PatchMW2_FFHash();
void PatchMW2_Modding();
void PatchMW2_Dedicated();
void PatchMW2_Status();
void PatchMW2_Redirect();
void PatchMW2_Servers();
void PatchMW2_Load();
void PatchMW2_MatchData();
void PatchMW2_Console();
void PatchMW2_Hello();
void PatchMW2_Experimental();
void PatchMW2_Client();
void PatchMW2_Prefix();
void PatchMW2_AssetRestrict();
void PatchMW2_ClientConsole();
void PatchMW2_NoBorder();
void PatchMW2_RemoteConsoleServer();
void PatchMW2_LogInitGame();
void PatchMW2_RemoteConsoleClient();

void PatchMW2_159()
{
	// protocol version (workaround for hacks)
	*(int*)0x4FB501 = PROTOCOL; // was 8E

	// protocol command
	*(int*)0x4D36A9 = PROTOCOL; // was 8E
	*(int*)0x4D36AE = PROTOCOL; // was 8E
	*(int*)0x4D36B3 = PROTOCOL; // was 8E

	// un-neuter Com_ParseCommandLine to allow non-connect_lobby
	*(BYTE*)0x464AE4 = 0xEB;

	// remove system pre-init stuff (improper quit, disk full)
	*(BYTE*)0x411350 = 0xC3;

	// logfile minimum at 0 (not needed in iw4m 159 executable)
	//*(BYTE*)0x60D515 = 0;

	// remove STEAMSTART checking for DRM IPC
	memset((void*)0x451145, 0x90, 5);
	*(BYTE*)0x45114C = 0xEB;

	// master server (server.alteriw.net)
	strcpy((char*)0x6D9CBC, "server.alteriw.net");

	// internal version is 99, most servers should accept it
	*(int*)0x463C61 = 99;

	// patch web1 server
	const char* webName = "http://web1.pc.iw4.iwnet.infinityward.com:13000/pc/";

	*(DWORD*)0x4D4800 = (DWORD)webName;
	*(DWORD*)0x4D481F = (DWORD)webName;

	// winmain
	winMainInitHook.initialize((PBYTE)winMainInitHookLoc);
	winMainInitHook.installHook(WinMainInitHookStub, false);

	// Sys_Init
	sysInitHook.initialize((PBYTE)sysInitHookLoc);
	sysInitHook.installHook(SysInitHookStub, false);

	// always enable system console, not just if generating reflection probes
	memset((void*)0x60BB58, 0x90, 11);

	// more detailed patches
	PatchMW2_FFHash();
	PatchMW2_Modding();
	PatchMW2_Prefix();
	PatchMW2_Status();
	PatchMW2_RemoteConsoleServer();
	PatchMW2_LogInitGame();
	PatchMW2_Redirect();
	PatchMW2_Servers();
	PatchMW2_Load();
	PatchMW2_Hello();
	PatchMW2_Experimental();
	PatchMW2_AssetRestrict();

	bool nativeConsole = GAME_FLAG(GAME_FLAG_CONSOLE);

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		PatchMW2_Dedicated();
		nativeConsole = !nativeConsole;
	}
	else
	{
		PatchMW2_Client();
		PatchMW2_ClientConsole();
		PatchMW2_NoBorder();
		PatchMW2_RemoteConsoleClient();
	}

	if (nativeConsole)
	{
		PatchMW2_Console();
	}
	else
	{
		FreeConsole();
	}
}

// smaller patches go below

// patches fastfile integrity checking
void PatchMW2_FFHash()
{
	// basic checks (hash jumps, both normal and playlist)
	*(WORD*)0x5B97A3 = 0x9090;
	*(WORD*)0x5BA493 = 0x9090;

	*(WORD*)0x5B991C = 0x9090;
	*(WORD*)0x5BA60C = 0x9090;

	*(WORD*)0x5B9962 = 0x9090;
	*(WORD*)0x5BA652 = 0x9090;

	*(WORD*)0x5B97B4 = 0x9090;
	*(WORD*)0x5BA4A4 = 0x9090;

	// some other, unknown, check
	*(BYTE*)0x5B9912 = 0xB8;
	*(DWORD*)0x5B9913 = 1;

	*(BYTE*)0x5BA602 = 0xB8;
	*(DWORD*)0x5BA603 = 1;
}

CallHook execIsFSHook;
DWORD execIsFSHookLoc = 0x6098FD;

// note this function has to return 'int', as otherwise only the bottom byte will get cleared
int ExecIsFSHookFunc(const char* execFilename, const char* dummyMatch) { // dummyMatch isn't used by us
	// check if the file exists in our FS_* path
	if (FS_ReadFile(execFilename, NULL) >= 0)
	{
		return false;
	}

	return true;
}

void __declspec(naked) ExecIsFSHookStub() {
	__asm jmp ExecIsFSHookFunc
}

DWORD pureShouldBeZero = 0;

// patches fs_game/IWD script support
void PatchMW2_Modding()
{
	// remove limit on IWD file loading
	//memset((void*)0x643B94, 0x90, 6);
	*(BYTE*)0x642BF3 = 0xEB;

	// remove convar write protection (why?)
	//*(BYTE*)0x647DD4 = 0xEB;

	// remove write protection from fs_game
	*(DWORD*)0x6431EA ^= 0x800;

	// kill most of pure (unneeded in 159, 180+ messed it up)
	//memset((void*)0x45513D, 0x90, 5);
	//memset((void*)0x45515B, 0x90, 5);
	//memset((void*)0x45516C, 0x90, 5);

	//memset((void*)0x45518E, 0x90, 5);
	//memset((void*)0x45519F, 0x90, 5);

	//*(BYTE*)0x449089 = 0xEB;

	// other IWD things (pure?)
	//*(BYTE*)0x4C5E7B = 0xEB;
	//*(BYTE*)0x465107 = 0xEB;

	// default sv_pure to 0
	// TODO: implement client-side downloading/default to 1 for no-mods
	*(BYTE*)0x4D3A74 = 0;

	// remove 'impure stats' checking
	*(BYTE*)0x4BB250 = 0x33;
	*(BYTE*)0x4BB251 = 0xC0;
	*(DWORD*)0x4BB252 = 0xC3909090;

	// remove fs_game profiles
	*(WORD*)0x4A5D74 = 0x9090;

	// fs_game crash fix removing some calls
	// (NOTE: CoD4 comparison shows this is related to LoadObj weaponDefs, might fix the crash we're having)
	*(BYTE*)0x452C1D = 0xEB;

	// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
	*(WORD*)0x61AB76 = 0x9090;

	// kill filesystem init default_mp.cfg check -- IW made it useless while moving .cfg files to fastfiles
	// and it makes fs_game crash

	// not nopping everything at once, there's cdecl stack cleanup in there
	memset((void*)0x461A9E, 0x90, 5);
	memset((void*)0x461AAA, 0x90, 5);
	memset((void*)0x461AB2, 0x90, 0xB1);

	// for some reason fs_game != '' makes the game load mp_defaultweapon, which does not exist in MW2 anymore as a real asset
	// kill the call and make it act like fs_game == ''
	// UPDATE 2010-09-12: this is why CoD4 had text weapon files, those are used with fs_game.
	// CLARIFY 2010-09-27: we don't have textual weapon files, as such we should load them from fastfile as usual
	// TODO: change this into a delay-loading hook for fastfile/loadobj (2011-05-20)
	*(BYTE*)0x4081FD = 0xEB;

	// exec fixing
	execIsFSHook.initialize((PBYTE)execIsFSHookLoc);
	execIsFSHook.installHook(ExecIsFSHookStub, false);

	// exec whitelist removal (YAYFINITY WARD)
	memset((void*)0x609685, 0x90, 5);
	*(WORD*)0x60968C = 0x9090;
}

void DoWinMainInit();

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// step 1: write minidump
	char error[1024];
	char filename[MAX_PATH];
	__time64_t time;
	tm* ltime;

	_time64(&time);
	ltime = _localtime64(&time);
	strftime(filename, sizeof(filename) - 1, "iw4svr-" VERSION "-%Y%m%d%H%M%S.dmp", ltime);
	_snprintf(error, sizeof(error) - 1, "A minidump has been written to %s.", filename);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex;
		memset(&ex, 0, sizeof(ex));
		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = ExceptionInfo;
		ex.ClientPointers = FALSE;

		if (FAILED(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL)))
		{
			_snprintf(error, sizeof(error) - 1, "An error (0x%x) occurred during writing %s.", GetLastError(), filename);
		}

		CloseHandle(hFile);
	}
	else
	{
		_snprintf(error, sizeof(error) - 1, "An error (0x%x) occurred during creating %s.", GetLastError(), filename);
	}

	// step 2: exit the application
	Com_Error(0, "Fatal error (0x%08x) at 0x%08x.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);

	return 0;
}

void InstallCrashHandler() {
	DoWinMainInit();

	SetUnhandledExceptionFilter(&CustomUnhandledExceptionFilter);
}

void ClientConsole_SetAutoComplete();

void DoSysInit()
{
	ClientConsole_SetAutoComplete();
}

// gethostbyname hook
dvar_t** masterServerName = (dvar_t**)0x1AD8F48;

char* serverName = NULL;
char* webName = NULL;

unsigned int oneAtATimeHash(char* inpStr)
{
	unsigned int value = 0,temp = 0;
	for(size_t i=0;i<strlen(inpStr);i++)
	{
		char ctext = tolower(inpStr[i]);
		temp = ctext;
		temp += value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if(value < 2) value += 2;
	return value;
}

hostent* WINAPI custom_gethostbyname(const char* name) {
	// if the name is IWNet's stuff...
	unsigned int ip1 = oneAtATimeHash("ip1.pc.iw4.iwnet.infinityward.com");
	unsigned int log1 = oneAtATimeHash("log1.pc.iw4.iwnet.infinityward.com");
	unsigned int match1 = oneAtATimeHash("match1.pc.iw4.iwnet.infinityward.com");
	unsigned int web1 = oneAtATimeHash("web1.pc.iw4.iwnet.infinityward.com");
	unsigned int blob1 = oneAtATimeHash("blob1.pc.iw4.iwnet.infinityward.com");

	unsigned int current = oneAtATimeHash((char*)name);
	char* hostname = (char*)name;

	if (current == log1 || current == match1 || current == blob1 || current == ip1 || current == web1) {
		hostname = (*masterServerName)->current.string;
	}

	return gethostbyname(hostname);
}

void PatchMW2_Servers()
{
	// gethostbyname
	*(DWORD*)0x6D7458 = (DWORD)custom_gethostbyname;
}

// HELLO WORLD function
CallHook helloHook;
DWORD helloHookLoc = 0x60BB01;

CallHook helloHook2;
DWORD helloHook2Loc = 0x60A946;

void HelloIW(int type)
{
	Com_Printf(type, "%s built on %s %s\n", VERSIONSTRING, __DATE__, __TIME__);
	Com_Printf(type, "\"Variety is the next big thing for us. We're going in deep, and we're going in hard.\"\n");
	Com_Printf(type, "  -- Robert Bowling, Creative Strategist at Infinity Ward (during IW3 credits)\n");
}

void __declspec(naked) HelloHookStub()
{
	HelloIW(0);
	__asm retn
}

void __declspec(naked) HelloHook2Stub()
{
	HelloIW(16);
	__asm jmp helloHook2.pOriginal
}

void PatchMW2_Hello()
{
	helloHook.initialize((PBYTE)helloHookLoc);
	helloHook.installHook(HelloHookStub, false);

	helloHook2.initialize((PBYTE)helloHook2Loc);
	helloHook2.installHook(HelloHook2Stub, false);
}

// connect command is here now, not supposed to be here, please move to client patches
typedef void (__cdecl * Steam_JoinLobby_t)(CSteamID, char);
Steam_JoinLobby_t Steam_JoinLobby = (Steam_JoinLobby_t)0x49CF70;

static netadr_t currentLobbyTarget;

const char* IWClient_HandleLobbyData(const char* key)
{
	netadr_t address = currentLobbyTarget;

	if (!strcmp(key, "addr"))
	{
		return va("%d", address.ip[0] | (address.ip[1] << 8) | (address.ip[2] << 16) | (address.ip[3] << 24));
	}
	else if (!strcmp(key, "port"))
	{
		return va("%d", htons(currentLobbyTarget.port));
	}
	
	return "212";
}

void ConnectToAddress(netadr_t address)
{
	/*steamClient->currentLobby = CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	steamClient->resetThis = 0;
	steamClient->addr = address.ip[0] | (address.ip[1] << 8) | (address.ip[2] << 16) | (address.ip[3] << 24);
	steamClient->port = address.port;
	steamClient->addrLoc = steamClient->addr;
	steamClient->portLoc = steamClient->port;

	*(BYTE*)0x6726059 = 1;
	*(BYTE*)0x672605B = 1;
	*(DWORD*)0x64FD9B8 = 2;*/

	// ^ that doesn't work, and I'm lazy right now
	currentLobbyTarget = address;

	CSteamID steamIDLobby = CSteamID(1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	Steam_JoinLobby(steamIDLobby, 0);
}

typedef bool (__cdecl * NET_StringToAdr_t)(const char* string, netadr_t* adr);
extern NET_StringToAdr_t NET_StringToAdr;

void CL_Connect_f()
{
	if (Cmd_Argc() < 2)
	{
		return;
	}

	const char* str = Cmd_Argv(1);
	netadr_t adr;

	if (NET_StringToAdr(str, &adr))
	{
		ConnectToAddress(adr);
	}
}

static cmd_function_t connectCmd;

void PatchMW2_Client()
{
	Cmd_AddCommand("connect", CL_Connect_f, &connectCmd, 0);

	// cause 'does current Steam lobby match' calls in Steam_JoinLobby to be ignored (issue #8)
	*(BYTE*)0x49D007 = 0xEB;

	//memset((void*)0x4CAA5D, 0x90, 5); // function checking party heartbeat timeouts, causes random issues
}