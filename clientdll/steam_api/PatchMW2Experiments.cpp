// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: experimental dev code
//
// Initial author: NTAuthority
// Started: 2011-05-09
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#include <shlobj.h>
#include <direct.h>

typedef void* (__cdecl * LoadRawFile_t)(const char* filename, void* buffer, size_t buflen);
LoadRawFile_t LoadRawFile = (LoadRawFile_t)0x40BA30;

CallHook addEntryNameHook;
DWORD addEntryNameHookLoc = 0x5BB697;

StompHook endLoadingHook;
DWORD endLoadingHookLoc = 0x44AC5C;

#define MAX_RAWFILE_NAMES 32768
static const char* rawFileNames[MAX_RAWFILE_NAMES];
static int currentRawFile = 0;

#define MAX_STABLE_NAMES 32768
static const char* sTableNames[MAX_STABLE_NAMES];
static int currentSTable = 0;

#define RAWFILE_BUFSIZE 2097152
static char rawFileBuffer[RAWFILE_BUFSIZE];

typedef struct stringTable_s {
	char* fileName;
	int columns;
	int rows;
	char** data;
} stringTable_t;

stringTable_t* stringTable;

void AddEntryNameHookFunc(int type, const char* name)
{
	OutputDebugString(va("%d: %s\n", type, name));
	return;

	if (type == 37)
	{
		if (currentSTable < MAX_STABLE_NAMES)
		{
			sTableNames[currentSTable] = name;
			currentSTable++;
		}
		/*if (currentRawFile < MAX_RAWFILE_NAMES)
		{
			rawFileNames[currentRawFile] = name;
			currentRawFile++;
		}*/
	}
}

typedef void* (__cdecl * DB_FindXAssetHeader_t)(int type, const char* filename);
DB_FindXAssetHeader_t DB_FindXAssetHeader = (DB_FindXAssetHeader_t)0x493FB0;

void DumpRawFiles()
{
	/*for (int i = 0; i < currentRawFile; i++)
	{
		const char* name = rawFileNames[i];
		memset(rawFileBuffer, 0, RAWFILE_BUFSIZE);

		if (LoadRawFile(name, rawFileBuffer, RAWFILE_BUFSIZE))
		{
			char filename[512];
			char dir[512];
			size_t length = strlen(rawFileBuffer);
			sprintf(filename, "%s/%s", "raw", name);

			GetCurrentDirectoryA(sizeof(dir), dir);
			strcat(dir, "/");
			strcat(dir, filename);
			*(strrchr(dir, '/')) = '\0';

			size_t strl = strlen(dir);

			for (int i = 0; i < strl; i++)
			{
				if (dir[i] == '/') dir[i] = '\\';
			}

			SHCreateDirectoryExA(NULL, dir, NULL);

			FILE* file = fopen(filename, "w");

			if (file)
			{
				fwrite(rawFileBuffer, 1, length, file);
				fclose(file);
			}
		}
	}

	currentRawFile = 0;*/

	for (int i = 0; i < currentSTable; i++)
	{
		const char* name = sTableNames[i];
		stringTable_t* stringTable;

		if (stringTable = (stringTable_t*)DB_FindXAssetHeader(37, name))
		{
			char filename[512];
			char dir[512];
			sprintf(filename, "%s/%s", "raw", name);

			GetCurrentDirectoryA(sizeof(dir), dir);
			strcat(dir, "/");
			strcat(dir, filename);
			*(strrchr(dir, '/')) = '\0';

			size_t strl = strlen(dir);

			for (size_t i = 0; i < strl; i++)
			{
				if (dir[i] == '/') dir[i] = '\\';
			}

			SHCreateDirectoryExA(NULL, dir, NULL);

			FILE* file = fopen(filename, "w");

			if (file)
			{
				int currentColumn = 0;
				int currentRow = 0;
				int total = stringTable->columns * stringTable->rows;

				for (int i = 0; i < total; i++) {
					char* current = stringTable->data[i * 2];

					fprintf(file, "%s", current);

					bool isNext = ((i + 1) % stringTable->columns) == 0;

					if (isNext) {
						fprintf(file, "\n");
					} else {
						fprintf(file, ",");
					}

					fflush(file);
				}

				fclose(file);
			}
		}
	}

	currentSTable = 0;
}

void __declspec(naked) EndLoadingHookStub()
{
	__asm
	{
		mov eax, 050FB50h
		call eax
		call DumpRawFiles
		retn
	}
}

void __declspec(naked) AddEntryNameHookStub()
{
	__asm
	{
		push ecx
		push eax
		call AddEntryNameHookFunc
		pop eax
		pop ecx
		jmp addEntryNameHook.pOriginal
	}
}

typedef struct weaponEntry_s
{
	const char* name;
	int offset;
	int type;
} weaponEntry_t;

#define NUM_ENTRIES 672

#define WEAPON_DO_ARRAY(ar, c) \
{ \
	for (int _l_1 = 0; _l_1 < c; _l_1++) \
	{ \
		if (*(int*)data == _l_1) \
		{ \
			fprintf(file, "%s", ((char**)ar)[_l_1]); /* why do I have to explicitly define ar as being a char**? */ \
		} \
	} \
}

weaponEntry_t* weaponEntries = (weaponEntry_t*)0x795F00;

const char* SL_ConvertToString(unsigned int sl)
{
	__asm
	{
		push sl
		mov eax, 0x468680
		call eax
		add esp, 4h
		mov sl, eax
	}

	return (const char*)sl;
}

void DumpNoteTrackEntry(FILE* file, int type, char* data, char* data2)
{
	switch (type)
	{
		case 39: // notetrack-to-sound
		case 40: // notetrack-to-rumble
			{
				// as seen last time, SL is usable now.
				short* keys = *(short**)data;
				short* values = *(short**)data2;

				for (int i = 0; i < 16; i++)
				{
					short key = keys[i];
					short value = values[i];

					if (key)
					{
						fprintf(file, "%s %s%s", SL_ConvertToString(key), SL_ConvertToString(value), (i == 15 || keys[i + 1] == 0) ? "" : "\n"); // it seems like a newline is needed for all but the last tag
					}
				}
			}
			break;
	}
}

void DumpEntry(FILE* file, int type, char* data)
{
	switch (type)
	{
	case 0: // string
		{
			char* str = (char*)(*(DWORD_PTR*)data);

			if (str && (int)str != 0xFFFFFFFF && (int)str > 16)
			{
				fprintf(file, "%s", str);
			}
			break;
		}
	case 4:
		{
			int number = *(int*)data;

			fprintf(file, "%d", number);
			break;
		}
	case 6:
		{
			bool boolean = *(bool*)data;

			fprintf(file, "%d", (boolean) ? 1 : 0);
			break;
		}
	case 7:
		{
			float number = *(float*)data;

			fprintf(file, "%g", number);
			break;
		}
	case 9:
		{
			int number = *(int*)data;

			fprintf(file, "%g", (number / 1000.0f));
			break;
		}
	case 16:
		WEAPON_DO_ARRAY(0x795DA0, 4)
		break;
	case 17:
		WEAPON_DO_ARRAY(0x795DB0, 12)
		break;
	case 18:
		WEAPON_DO_ARRAY(0x795E68, 2)
		break;
	case 19:
		WEAPON_DO_ARRAY(0x795E10, 4)
		break;
	case 20:
		WEAPON_DO_ARRAY(0x795E20, 11)
		break;
	case 21:
		WEAPON_DO_ARRAY(0x795E70, 3)
		break;
	case 22:
		WEAPON_DO_ARRAY(0x795E4C, 7)
		break;
	case 23:
		WEAPON_DO_ARRAY(0x795E7C, 6)
		break;
	case 24:
		WEAPON_DO_ARRAY(0x7BEFE8, *(int*)0x7BE49C)
		break;
	case 25:
		WEAPON_DO_ARRAY(0x795E94, 3)
		break;
	case 26:
		WEAPON_DO_ARRAY(0x795EA0, 4)
		break;
	case 28:
		WEAPON_DO_ARRAY(0x795EB0, 6)
		break;
	case 29:
		WEAPON_DO_ARRAY(0x795EC8, 3)
		break;
	case 30:
		WEAPON_DO_ARRAY(0x795DE0, 6)
		break;
	case 31:
		WEAPON_DO_ARRAY(0x795DF8, 6)
		break;
	case 32:
		WEAPON_DO_ARRAY(0x795ED4, 7)
		break;
	case 33:
		WEAPON_DO_ARRAY(0x795EF0, 3)
		break;
	case 34:
		WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
		break;
	case 35:
		WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
		break;
	case 36:
		WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
		break;
	case 37:
		WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
		break;
	case 13: // phys collmap
		break;
	case 10: // fx?
	case 11: // xmodel
	case 12: // material/image?
	case 14: // soundalias
	case 15: // tracer
		{
			BYTE* model = (BYTE*)(*(DWORD_PTR*)data);

			if (model)
			{
				char* str = (char*)(*(DWORD_PTR*)model);

				if (str)
				{
					fprintf(file, "%s", str);
				}
			}
			break;
		}
	case 38: // hideTags
		{
			// hopefully we'll have useful shit here, as if we don't, we're screwed
			// for some reason this code made me go deja vu - seems like I've done SL_ConvertToString before.
			short* tags = (short*)data;
			for (int i = 0; i < 32; i++)
			{
				short tag = tags[i];

				if (tag)
				{
					fprintf(file, "%s%s", SL_ConvertToString(tag), (i == 31 || tags[i + 1] == 0) ? "" : "\n"); // it seems like a newline is needed for all but the last tag
				}
			}
			break;
		}
	case 27: // bounceSound; surface sound
		{
			char*** dat = *(char****)data;
			if (dat != 0)
			{
				char bounceName[128];
				strcpy(bounceName, **dat);
				strrchr(bounceName, '_')[0] = '\0';

				fprintf(file, "%s", bounceName);
			}
			break;
		}
	default:
		fprintf(file, "lol no type %d", type);
		break;
	}
}

typedef struct 
{
	int offsetStart;
	int pointerOrigin;
} rtOffsetMap_t;

// nvm this is just impossible
/*
static rtOffsetMap_t offsetMap[] =
{
	{ 116, 4 },
	{ 1784, 12 },
	{ 1848, 16 },
	{ 1996, 120 },
	{ 2060, 128 },
	{ 2208, 132 },
	{ 2356, 140 },
	{ 2388, 144 },
	{ 2420, 148 },
	{ 2452, 152 },
	{ 2484, 588 },
	{ 2548, 1208 },
	{ 2672, 1212 },
	{ 2796, 1576 },
	{ 0, 0, 0 }
};

char* MapOffsetToPointer(char* origin, int offset)
{
	// let's say offset is 120, and origin is 0 (just to be funs)
	// pointer +4 redirects to 1000
	// we want to end up with 1004

	int i = 0;
	while (offsetMap[i].offsetStart != 0)
	{
		int max = (offsetMap[i + 1].offsetStart == 0) ? 0x7FFFFF : offsetMap[i + 1].offsetStart;

		// as 120 means it should go pointer 4, we should do so here.
		// after that we need to return a resolved pointer (wtf?) from whatever offset we came from
		if (offset >= offsetMap[i].offsetStart && offset < max)
		{
			return MapOffsetToPointer(origin, offset);
		}
	}

	return (origin + offset);
}
*/

/*
*(_DWORD *)(result + 16) = result + 1848;
*(_DWORD *)(result + 120) = result + 1996;
*(_DWORD *)(result + 128) = result + 2060;
*(_DWORD *)(result + 132) = result + 2208;
*(_DWORD *)(result + 140) = result + 2356;
*(_DWORD *)(result + 144) = result + 2388;
*(_DWORD *)(result + 148) = result + 2420;
*(_DWORD *)(result + 152) = result + 2452;
*(_DWORD *)(result + 588) = result + 2484;
*(_DWORD *)(result + 1208) = result + 2548;
*(_DWORD *)(result + 1212) = result + 2672;
*(_DWORD *)(result + 1576) = result + 2796;
*/

// lolnope again
/*
char* GetPointerForOffset(char* origin, int offset)
{
	char* retptr = origin;

	if (offset >= 1784 && offset < 1848)
	{
		retptr = *(BYTE**)(retptr + 12);
		retptr += (offset - 1784);
	}

	if (offset >= 1848 && offset < 1996)
	{
		retptr = *(BYTE**)(retptr + 16);
		retptr += (offset - 1848);
	}

	if (offset >= 116)
	{
		retptr = *(BYTE**)(retptr + 4);
		retptr += (offset - 116);
	}

	if (offset >= 1996 && offset < 2060)
	{
		retptr = *(BYTE**)(retptr + 4);
		retptr += (offset - 1996);
	}
}
*/

#define NUM_OFFSET_MAPS 14

static rtOffsetMap_t offsetMap[] =
{
	{ 116, 4 },
	{ 1784, 12 },
	{ 1848, 16 },
	{ 1996, 120 },
	{ 2060, 128 },
	{ 2208, 132 },
	{ 2356, 140 },
	{ 2388, 144 },
	{ 2420, 148 },
	{ 2452, 152 },
	{ 2484, 588 },
	{ 2548, 1208 },
	{ 2672, 1212 },
	{ 2796, 1576 },
	{ 0, 0 }
};

char* MapOffsetToPointer(char* origin, int offset)
{
	for (int i = (NUM_OFFSET_MAPS - 1); i >= 0; i--)
	{
		rtOffsetMap_t* current = &offsetMap[i];
		rtOffsetMap_t* next = &offsetMap[i + 1];

		int max = next->offsetStart;
		if (max == 0) max = 0xFFFFFF;

		if (offset >= current->offsetStart && offset < max)
		{
			char* pointer = *(char**)MapOffsetToPointer(origin, current->pointerOrigin);
			return (pointer + (offset - current->offsetStart));
		}
	}

	return (origin + offset);
}

void DumpWeaponFile(FILE* file, char* data)
{
	for (int i = 0; i < NUM_ENTRIES; i++)
	{
		weaponEntry_t* entry = &weaponEntries[i];

		char* ptr = MapOffsetToPointer(data, entry->offset);//(data + entry->offset);

		// quick patch to not export empty models (causes too large weaponfiles)
		if (entry->type == 11 && *(int*)ptr == 0)
		{
			continue;
		}

		// same for empty strings
		if (entry->type == 0)
		{
			char* str = (char*)(*(DWORD_PTR*)ptr);

			if (str && (int)str != 0xFFFFFFFF && (int)str > 16 && *str != 0)
			{
				// yeah I know, but it's 3:45 AM and I'm too lazy to invert this
			}
			else
			{
				continue;
			}
		}

		fprintf(file, "%s\\", entry->name);

		/*if (entry->offset > 116)
		{
			ptr = *(BYTE**)(data + 4);
			ptr += (entry->offset - 116);
		}*/

		if (entry->type == 39 || entry->type == 40)
		{
			DumpNoteTrackEntry(file, entry->type, MapOffsetToPointer(data, 140), MapOffsetToPointer(data, 144));
		}
		else
		{
			DumpEntry(file, entry->type, ptr);
		}

		if (i < (NUM_ENTRIES - 1))
		{
			fprintf(file, "\\");
		}
	}
}

void DumpWeaponTypes(FILE* file)
{
	for (int i = 0; i < NUM_ENTRIES; i++)
	{
		weaponEntry_t* entry = &weaponEntries[i];

		//fprintf(file, "%s\\", entry->name);

		//DumpEntry(file, entry->type, (data + entry->offset));

		fprintf(file, "+0x%x:\t%s", entry->offset, entry->name);

		switch (entry->type)
		{
		case 0:
			fprintf(file, " (string)");
			break;
		case 7:
			fprintf(file, " (float)");
			break;
		case 8:
			fprintf(file, " (float / 17.6)");
			break;
		case 9:
			fprintf(file, " (float / 1000.0)");
			break;
		case 11:
			fprintf(file, " (xmodel)");
			break;
		case 0xD:
			fprintf(file, " (phys collmap)");
			break;
		default:
			fprintf(file, " (type %d)", entry->type);
			break;
		}

		fprintf(file, "\n");
	}
}

StompHook weaponFileHook;
DWORD weaponFileHookLoc = 0x581900;

void* WeaponFileHookFunc(const char* filename)
{
	char* file = (char*)DB_FindXAssetHeader(0x1C, filename);

	_mkdir("raw\\weapons");
	_mkdir("raw\\weapons\\mp");

	char dumpfile[512];
	strcpy(dumpfile, "raw\\weapons\\mp\\");
	strcat(dumpfile, filename);

	FILE* dump = fopen(dumpfile, "w");
	fprintf(dump, "WEAPONFILE\\");

	DumpWeaponFile(dump, file);

	fclose(dump);

	dump = fopen("raw\\weaponFields.txt", "w");
	DumpWeaponTypes(dump);
	fclose(dump);

	//TerminateProcess(GetCurrentProcess(), 0);

	return file;
}

void __declspec(naked) WeaponFileHookStub()
{
	__asm jmp WeaponFileHookFunc
}

StompHook scriptOddLogHook;
DWORD scriptOddLogHookLoc = 0x4938D0;

StompHook scriptOddLogHook2;
DWORD scriptOddLogHook2Loc = 0x49D3F0;

void ScriptOddLogHookFunc(int meh, char* mehh)
{
	Com_Printf(0, "sd: %s\n", mehh);
}

void __declspec(naked) ScriptOddLogHookStub()
{
	__asm jmp ScriptOddLogHookFunc
}

// HELL YEAH PART 2 OF ODD LOG HOOK
void ScriptOddLogHook2Func(char* mehh)
{
	Com_Printf(0, "sd: %s\n", mehh);
}

void __declspec(naked) ScriptOddLogHook2Stub()
{
	__asm jmp ScriptOddLogHook2Func // if it breaks spawning, firstly go 'wtf'.
}

// debug hook to find whoever fucked with my party state
CallHook psClearHook;
DWORD psClearHookLoc = 0x48569C;

int curret;

void __declspec(naked) PsClearHookStub()
{
	__asm
	{
		mov eax, [esp + 14h]
		mov curret, eax
	}

	Com_Printf(0, "GUILTY AS CHARGED AT 0x%x\n", curret);

	__asm retn
}

void PatchMW2_Experimental()
{
	//psClearHook.initialize((PBYTE)psClearHookLoc);
	//psClearHook.installHook(PsClearHookStub, false);

	//weaponFileHook.initialize(5, (PBYTE)weaponFileHookLoc);
	//weaponFileHook.installHook(WeaponFileHookStub, true, false);

	//addEntryNameHook.initialize((PBYTE)addEntryNameHookLoc);
	//addEntryNameHook.installHook(AddEntryNameHookStub, false);

	/*endLoadingHook.initialize(5, (PBYTE)endLoadingHookLoc);
	endLoadingHook.installHook(EndLoadingHookStub, true, false);*/

	/*scriptOddLogHook.initialize(5, (PBYTE)scriptOddLogHookLoc);
	scriptOddLogHook.installHook(ScriptOddLogHookStub, true, false);*/

	//scriptOddLogHook2.initialize(5, (PBYTE)scriptOddLogHook2Loc);
	//scriptOddLogHook2.installHook(ScriptOddLogHook2Stub, true, false);//*/

	// BAD HACK: load any weapon without cache in G_GetWeaponByName
	// wow, this was needed in T5 as well. wonder why.
	// (nope, didn't fix it)
	//*(WORD*)0x43DF17 = 0x9090;
}