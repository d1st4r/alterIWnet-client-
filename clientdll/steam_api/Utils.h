// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Various generic utility functions.
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

//void Trace(char* source, char* message, ...);
//void Trace2(char* message, ...);
#define Trace(source, message, ...) Trace2("[" source "] " message, __VA_ARGS__)
#define Trace2(message, ...) Com_Printf(0, message, __VA_ARGS__)

bool FileExists(const char* file);

const char* va(const char* format, ...);

size_t Com_AddToString(const char* source, char* buffer, size_t current, size_t length, bool escapeSpaces);

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

void Info_RemoveKey( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );

// flag settings
#define GAME_FLAG_DEDICATED		(1 << 0)
#define GAME_FLAG_CONSOLE		(1 << 1)

#define GAME_FLAG(x)			((_gameFlags & x) == x)

extern unsigned int _gameFlags;
void DetermineGameFlags();