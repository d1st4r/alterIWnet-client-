#pragma once

#define PROTOCOL 145

#define CODE_START 0x401000
#define CODE_END 0x6D7000

#define DVAR_ARCHIVE 1

typedef enum assetType_e
{
	ASSET_TYPE_PHYSPRESET = 0,
	ASSET_TYPE_PHYS_COLLMAP = 1,
	ASSET_TYPE_XANIM = 2,
	ASSET_TYPE_XMODELSURFS = 3,
	ASSET_TYPE_XMODEL = 4,
	ASSET_TYPE_MATERIAL = 5,
	ASSET_TYPE_PIXELSHADER = 6,
	ASSET_TYPE_VERTEXSHADER = 7,
	ASSET_TYPE_VERTEXDECL = 8,
	ASSET_TYPE_TECHSET = 9,
	ASSET_TYPE_IMAGE = 10,
	ASSET_TYPE_SOUND = 11,
	ASSET_TYPE_SNDCURVE = 12,
	ASSET_TYPE_LOADED_SOUND = 13,
	ASSET_TYPE_COL_MAP_SP = 14,
	ASSET_TYPE_COL_MAP_MP = 15,
	ASSET_TYPE_COM_MAP = 16,
	ASSET_TYPE_GAME_MAP_SP = 17,
	ASSET_TYPE_GAME_MAP_MP = 18,
	ASSET_TYPE_MAP_ENTS = 19,
	ASSET_TYPE_FX_MAP = 20,
	ASSET_TYPE_GFX_MAP = 21,
	ASSET_TYPE_LIGHTDEF = 22,
	ASSET_TYPE_UI_MAP = 23,
	ASSET_TYPE_FONT = 24,
	ASSET_TYPE_MENUFILE = 25,
	ASSET_TYPE_MENU = 26,
	ASSET_TYPE_LOCALIZE = 27,
	ASSET_TYPE_WEAPON = 28,
	ASSET_TYPE_SNDDRIVERGLOBALS = 29,
	ASSET_TYPE_FX = 30,
	ASSET_TYPE_IMPACTFX = 31,
	ASSET_TYPE_AITYPE = 32,
	ASSET_TYPE_MPTYPE = 33,
	ASSET_TYPE_CHARACTER = 34,
	ASSET_TYPE_XMODELALIAS = 35,
	ASSET_TYPE_RAWFILE = 36,
	ASSET_TYPE_STRINGTABLE = 37,
	ASSET_TYPE_LEADERBOARDDEF = 38,
	ASSET_TYPE_STRUCTUREDDATADEF = 39,
	ASSET_TYPE_TRACER = 40,
	ASSET_TYPE_VEHICLE = 41,
	ASSET_TYPE_ADDON_MAP_ENTS = 42
} assetType_t;

// not complete, but enough for reading
typedef struct dvar_t
{
	const char* name;
	int unknown;
	int flags;
	int type;
	union dvar_value_t {
		float value;
		int integer;
		char* string;
		bool boolean;
	} current;
} dvar_t;

typedef struct cmd_function_s
{
	char pad[24];
} cmd_function_t;

// netadr_t
typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

typedef struct {
	netadrtype_t	type;

	BYTE	ip[4];

	unsigned short	port;

	BYTE	ipx[10];
} netadr_t;

typedef struct  
{
	int curClients;
	int maxClients;
	char map[128];
} svstatus_t;

// vars
extern dvar_t* sv_rconPassword;

// inline cmd functions
extern DWORD* cmd_id;
extern DWORD* cmd_argc;
extern DWORD** cmd_argv;

inline int	Cmd_Argc( void )
{
	return cmd_argc[*cmd_id];
}

inline char *Cmd_Argv( int arg )
{
	if ( (unsigned)arg >= cmd_argc[*cmd_id] ) {
		return "";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);	
}

// internal functions (custom)
void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char * ) );
void        Com_EndRedirect( void );

int	Cmd_ArgcSV( void );
char *Cmd_ArgvSV( int );

void SV_GetStatus(svstatus_t* status);
char* GetStringConvar(char* key);

void NET_OutOfBandPrint(int type, netadr_t adr, const char* message, ...);
int Party_NumPublicSlots();

// types
typedef void (__cdecl * CommandCB_t)(void);

// original functions
typedef void (__cdecl * Com_Error_t)(int type, char* message, ...);
extern Com_Error_t Com_Error;

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
extern Com_Printf_t Com_Printf;

typedef void (__cdecl * Cmd_ExecuteSingleCommand_t)(int controller, int a2, const char* cmd);
extern Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand;

typedef int (__cdecl * FS_ReadFile_t)(const char* qpath, void** buffer);
extern FS_ReadFile_t FS_ReadFile;

typedef int (__cdecl * Dvar_RegisterString_t)(const char*, const char*, int, const char*);
extern Dvar_RegisterString_t Dvar_RegisterString;

typedef void (__cdecl * Cmd_AddCommand_t)(const char* name, CommandCB_t callback, cmd_function_t* data, char);
extern Cmd_AddCommand_t Cmd_AddCommand;

typedef void (__cdecl * Cmd_AddServerCommand_t)(const char* name, CommandCB_t callback, cmd_function_t* data);
extern Cmd_AddServerCommand_t Cmd_AddServerCommand;

typedef dvar_t* (__cdecl * Dvar_RegisterBool_t)(const char* name, bool default, int flags, const char* description);
extern Dvar_RegisterBool_t Dvar_RegisterBool;

typedef dvar_t* (__cdecl * Dvar_RegisterInt_t)(const char* name, int default, int min, int max, int flags, const char* description);
extern Dvar_RegisterInt_t Dvar_RegisterInt;

typedef void (__cdecl * SV_GameSendServerCommand_t)(int targetEntity, int a2, const char* command);
extern SV_GameSendServerCommand_t SV_GameSendServerCommand;

typedef const char* (__cdecl * NET_AdrToString_t)(netadr_t adr);
extern NET_AdrToString_t NET_AdrToString;

typedef int (__cdecl * Com_Milliseconds_t)(void);
extern Com_Milliseconds_t Com_Milliseconds;

typedef void (__cdecl * Cbuf_AddText_t)(int a1, char* cmd);
extern Cbuf_AddText_t Cbuf_AddText;

typedef dvar_t* (__cdecl * Dvar_FindVar_t)(char*);
extern Dvar_FindVar_t Dvar_FindVar;

typedef bool (__cdecl * NET_StringToAdr_t)(const char*, netadr_t*);
extern NET_StringToAdr_t NET_StringToAdr;

typedef char* (__cdecl* Dvar_InfoString_Big_t)(int typeMask);
extern Dvar_InfoString_Big_t Dvar_InfoString_Big;

typedef void (__cdecl * G_LogPrintf_t)(char*, ...);
extern G_LogPrintf_t G_LogPrintf;

typedef void (__cdecl * Cmd_SetAutoComplete_t)(const char* name, const char* path, const char* extension);
extern Cmd_SetAutoComplete_t Cmd_SetAutoComplete;

extern CommandCB_t Cbuf_AddServerText_f;