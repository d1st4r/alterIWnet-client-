// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: console-based console
//
// Initial author: NTAuthority
// Started: 2011-01-20
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#undef MOUSE_MOVED
#include <curses.h>

void DoWinMainInit() {}

// patch locations
#define HOOK_Sys_CreateConsole 0x4305E0
#define HOOK_Sys_DestroyConsole 0x4528A0
#define HOOK_Sys_Print 0x4B2080
#define HOOK_Sys_Error 0x43D570
#define HOOK_Sys_ConsoleInput 0x4859A5

// patch handlers
StompHook createConsoleHook;
StompHook destroyConsoleHook;
StompHook sysPrintHook;
StompHook sysErrorHook;
CallHook consoleInputHook;

// patch functions
#define WIDTH 80
#define HEIGHT 25
#define OUTPUT_HEIGHT 250
#define OUTPUT_MAX_TOP (OUTPUT_HEIGHT - (HEIGHT - 2))
static WINDOW* outputWindow;
static WINDOW* inputWindow;
static WINDOW* infoWindow;

static int currentOutputTop = 0;
static int currentOutBuffer = 0; // for initial counting of output buffer

void RefreshOutput()
{
	prefresh(outputWindow, (currentOutputTop >= 1) ? currentOutputTop - 1 : 0, 0, 1, 0, HEIGHT - 2, WIDTH);
}

void ScrollOutput(int amount)
{
	currentOutputTop += amount;

	if (currentOutputTop > OUTPUT_MAX_TOP)
	{
		currentOutputTop = OUTPUT_MAX_TOP;
	}
	else if (currentOutputTop < 0)
	{
		currentOutputTop = 0;
	}

	// make it only scroll the top if there's more than HEIGHT lines
	if (currentOutBuffer >= 0)
	{
		currentOutBuffer += amount;
		
		if (currentOutBuffer >= (HEIGHT))
		{
			currentOutBuffer = -1;
		}

		if (currentOutputTop < HEIGHT)
		{
			currentOutputTop = 0;
		}
	}
}

void Sys_CreateConsole()
{
	initscr();
	raw();
	noecho();

	//outputWindow = newwin(HEIGHT - 2, WIDTH, 1, 0);
	outputWindow = newpad(OUTPUT_HEIGHT, WIDTH);
	inputWindow = newwin(1, WIDTH, HEIGHT - 1, 0);
	infoWindow = newwin(1, WIDTH, 0, 0);

	scrollok(outputWindow, true);
	scrollok(inputWindow, true);
	nodelay(inputWindow, true);
	keypad(inputWindow, true);

	if (has_colors())
	{
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_WHITE);
		init_pair(2, COLOR_WHITE, COLOR_BLACK);
		init_pair(3, COLOR_RED, COLOR_BLACK);
		init_pair(4, COLOR_GREEN, COLOR_BLACK);
		init_pair(5, COLOR_YELLOW, COLOR_BLACK);
		init_pair(6, COLOR_BLUE, COLOR_BLACK);
		init_pair(7, COLOR_CYAN, COLOR_BLACK);
		init_pair(8, COLOR_RED, COLOR_BLACK);
		init_pair(9, COLOR_WHITE, COLOR_BLACK);
		init_pair(10, COLOR_WHITE, COLOR_BLACK);
		//init_pair(2, COLOR_WHITE, COLOR_BLACK);
	}

	wbkgd(infoWindow, COLOR_PAIR(1));

	wrefresh(infoWindow);
	//wrefresh(outputWindow);
	wrefresh(inputWindow);
	RefreshOutput();
}

void Sys_DestroyConsole()
{
	delwin(outputWindow);
	delwin(inputWindow);
	delwin(infoWindow);
	endwin();
}

static bool didWeHaveConsole = false;
static int lastRefresh = 0;

void Sys_Print(const char* message)
{
	const char* p = message;
	while (*p != '\0')
	{
		if (*p == '\n')
		{
			ScrollOutput(1);
		}

		if (*p == '^')
		{
			char color;
			p++;

			color = (*p - '0');

			if (color < 9 && color > 0)
			{
				wattron(outputWindow, COLOR_PAIR(color + 2));
				p++;
				continue;
			}
		}

		waddch(outputWindow, *p);

		p++;
	}

	wattron(outputWindow, COLOR_PAIR(9));

	int currentTime = GetTickCount();

	if (!didWeHaveConsole)
	{
		RefreshOutput();
	}
	else if ((currentTime - lastRefresh) > 100)
	{
		RefreshOutput();
		lastRefresh = currentTime;
	}
}

void Sys_Error(const char* format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);

	//exit(0);
	TerminateProcess(GetCurrentProcess(), 0xDEADDEAD);
}

static char consoleLineBuffer[1024];
static int consoleLineBufferIndex = 0;

static char consoleLineBuffer2[1024];

void ShowConsolePrompt()
{
	wattron(inputWindow, COLOR_PAIR(10) | A_BOLD);
	wprintw(inputWindow, "%s> ", VERSIONSTRING);
}

void UpdateConsoleStatus()
{
	svstatus_t status;

	SV_GetStatus(&status);

	wclear(infoWindow);
	wprintw(infoWindow, "IW4 Dedicated Server : %d/%d players : map %s", status.curClients, status.maxClients, status.map);
	wnoutrefresh(infoWindow);
}

const char* Sys_ConsoleInput()
{
	if (!didWeHaveConsole)
	{
		ShowConsolePrompt();
		wrefresh(inputWindow);
		didWeHaveConsole = true;
	}

	int currentTime = GetTickCount();
	if ((currentTime - lastRefresh) > 250)
	{
		RefreshOutput();
		lastRefresh = currentTime;
	}

	UpdateConsoleStatus();

	int c = wgetch(inputWindow);

	if (c == ERR)
	{
		return NULL;
	}

	switch (c)
	{
		case '\r':
			wattron(outputWindow, COLOR_PAIR(10) | A_BOLD);
			wprintw(outputWindow, "]%s\n", consoleLineBuffer);
			wattroff(outputWindow, A_BOLD);
			wclear(inputWindow);
			ShowConsolePrompt();
			wrefresh(inputWindow);
			
			ScrollOutput(1);
			RefreshOutput();

			if (consoleLineBufferIndex)
			{
				strcpy(consoleLineBuffer2, consoleLineBuffer);
				strcat(consoleLineBuffer, "\n");
				consoleLineBufferIndex = 0;
				return consoleLineBuffer;
			}

			break;
		case 'c' - 'a' + 1: // ctrl-c
		case 27:
			consoleLineBuffer[0] = '\0';
			consoleLineBufferIndex = 0;
			wclear(inputWindow);
			ShowConsolePrompt();
			wrefresh(inputWindow);
			break;
		case 8: // backspace
			if (consoleLineBufferIndex > 0)
			{
				consoleLineBufferIndex--;
				consoleLineBuffer[consoleLineBufferIndex] = '\0';

				wprintw(inputWindow, "%c %c", (char)c, (char)c);
				wrefresh(inputWindow);
			}
			break;
		case KEY_PPAGE:
			ScrollOutput(-1);
			RefreshOutput();
			break;
		case KEY_NPAGE:
			ScrollOutput(1);
			RefreshOutput();
			break;
		case KEY_UP:
			wclear(inputWindow);
			ShowConsolePrompt();
			wprintw(inputWindow, "%s", consoleLineBuffer2);
			wrefresh(inputWindow);

			strcpy(consoleLineBuffer, consoleLineBuffer2);
			consoleLineBufferIndex = strlen(consoleLineBuffer);
			break;
		default:
			if (c <= 127 && consoleLineBufferIndex < 1022)
			{
				// temporary workaround for issue #9, find out what overwrites our index later on
				consoleLineBufferIndex = strlen(consoleLineBuffer);

				consoleLineBuffer[consoleLineBufferIndex++] = (char)c;
				consoleLineBuffer[consoleLineBufferIndex] = '\0';
				wprintw(inputWindow, "%c", (char)c);
				wrefresh(inputWindow);
			}
			break;
	}

	return NULL;
}

// patch entry point
void PatchMW2_Console()
{
	createConsoleHook.initialize(5, (PBYTE)HOOK_Sys_CreateConsole);
	createConsoleHook.installHook(Sys_CreateConsole, true, false);

	destroyConsoleHook.initialize(5, (PBYTE)HOOK_Sys_DestroyConsole);
	destroyConsoleHook.installHook(Sys_DestroyConsole, true, false);

	// TODO: get rid of weird casts
	sysPrintHook.initialize(5, (PBYTE)HOOK_Sys_Print);
	sysPrintHook.installHook((void (__cdecl *)(void))Sys_Print, true, false);

	sysErrorHook.initialize(5, (PBYTE)HOOK_Sys_Error);
	sysErrorHook.installHook((void (__cdecl *)(void))Sys_Error, true, false);

	consoleInputHook.initialize((PBYTE)HOOK_Sys_ConsoleInput);
	consoleInputHook.installHook((void (__cdecl *)(void))Sys_ConsoleInput, false);

	//PatchMW2_DebugAllocations();
}