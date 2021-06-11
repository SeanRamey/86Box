/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Platform main support module for Windows.
 *
 *
 *
 * Authors:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *		Copyright 2008-2019 Sarah Walker.
 *		Copyright 2016-2019 Miran Grca.
 *		Copyright 2017-2019 Fred N. van Kempen.
 */
#define UNICODE
#define NTDDI_VERSION 0x06010000
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <wchar.h>
#include <io.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include <86box/config.h>
#include <86box/device.h>
#include <86box/keyboard.h>
#include <86box/mouse.h>
#include <86box/timer.h>
#include <86box/nvr.h>
#include <86box/video.h>
#define GLOBAL
#include <86box/plat.h>
#include <86box/plat_midi.h>
#include <86box/ui.h>
#ifdef USE_VNC
# include <86box/vnc.h>
#endif
#include <86box/win_sdl.h>
#include <86box/win.h>
#include <86box/version.h>
#ifdef MTR_ENABLED
#include <minitrace/minitrace.h>
#endif

typedef struct {
	WCHAR str[512];
} rc_str_t;


/* Platform Public data, specific. */
HINSTANCE	hinstance;		/* application instance */
HANDLE		ghMutex;
LCID		lang_id;		/* current language ID used */
DWORD		dwSubLangID;
int		acp_utf8;		/* Windows supports UTF-8 codepage */


/* Local data. */
static HANDLE	thMain;
static rc_str_t	*lpRCstr2048,
		*lpRCstr4096,
		*lpRCstr4352,
		*lpRCstr4608,
		*lpRCstr5120,
		*lpRCstr5376,
		*lpRCstr5632,
		*lpRCstr5888,
		*lpRCstr6144,
		*lpRCstr7168;
static wchar_t	*argbuf;
static int	first_use = 1;
static LARGE_INTEGER	StartingTime;
static LARGE_INTEGER	Frequency;


extern int title_update;


#ifdef ENABLE_WIN_LOG
int win_do_log = ENABLE_WIN_LOG;


static void
win_log(const char *fmt, ...)
{
	va_list ap;

	if (win_do_log) {
	va_start(ap, fmt);
	pclog_ex(fmt, ap);
	va_end(ap);
	}
}
#else
#define win_log(fmt, ...)
#endif


static void
LoadCommonStrings(void)
{
	int i;

	lpRCstr2048 = (rc_str_t *)malloc(STR_NUM_2048*sizeof(rc_str_t));
	lpRCstr4096 = (rc_str_t *)malloc(STR_NUM_4096*sizeof(rc_str_t));
	lpRCstr4352 = (rc_str_t *)malloc(STR_NUM_4352*sizeof(rc_str_t));
	lpRCstr4608 = (rc_str_t *)malloc(STR_NUM_4608*sizeof(rc_str_t));
	lpRCstr5120 = (rc_str_t *)malloc(STR_NUM_5120*sizeof(rc_str_t));
	lpRCstr5376 = (rc_str_t *)malloc(STR_NUM_5376*sizeof(rc_str_t));
	lpRCstr5632 = (rc_str_t *)malloc(STR_NUM_5632*sizeof(rc_str_t));
	lpRCstr5888 = (rc_str_t *)malloc(STR_NUM_5888*sizeof(rc_str_t));
	lpRCstr6144 = (rc_str_t *)malloc(STR_NUM_6144*sizeof(rc_str_t));
	lpRCstr7168 = (rc_str_t *)malloc(STR_NUM_7168*sizeof(rc_str_t));

	for (i=0; i<STR_NUM_2048; i++)
	LoadString(hinstance, 2048+i, lpRCstr2048[i].str, 512);

	for (i=0; i<STR_NUM_4096; i++)
	LoadString(hinstance, 4096+i, lpRCstr4096[i].str, 512);

	for (i=0; i<STR_NUM_4352; i++)
	LoadString(hinstance, 4352+i, lpRCstr4352[i].str, 512);

	for (i=0; i<STR_NUM_4608; i++)
	LoadString(hinstance, 4608+i, lpRCstr4608[i].str, 512);

	for (i=0; i<STR_NUM_5120; i++)
	LoadString(hinstance, 5120+i, lpRCstr5120[i].str, 512);

	for (i=0; i<STR_NUM_5376; i++) {
	if ((i == 0) || (i > 3))
		LoadString(hinstance, 5376+i, lpRCstr5376[i].str, 512);
	}

	for (i=0; i<STR_NUM_5632; i++) {
	if ((i == 0) || (i > 3))
		LoadString(hinstance, 5632+i, lpRCstr5632[i].str, 512);
	}

	for (i=0; i<STR_NUM_5888; i++)
	LoadString(hinstance, 5888+i, lpRCstr5888[i].str, 512);

	for (i=0; i<STR_NUM_6144; i++)
	LoadString(hinstance, 6144+i, lpRCstr6144[i].str, 512);

	for (i=0; i<STR_NUM_7168; i++)
	LoadString(hinstance, 7168+i, lpRCstr7168[i].str, 512);
}


size_t
mbstoc16s(uint16_t dst[], const char src[], int len)
{
	if (src == NULL) return 0;
	if (len < 0) return 0;

	size_t ret = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst == NULL ? 0 : len);

	if (!ret) {
	return -1;
	}

	return ret;
}


size_t
c16stombs(char dst[], const uint16_t src[], int len)
{
	if (src == NULL) return 0;
	if (len < 0) return 0;

	size_t ret = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dst == NULL ? 0 : len, NULL, NULL);

	if (!ret) {
	return -1;
	}

	return ret;
}


/* Set (or re-set) the language for the application. */
void
set_language()
{
	// LCID lcidNew = MAKELCID(id, dwSubLangID);

	// if (lang_id != lcidNew) {
	// /* Set our new language ID. */
	// lang_id = lcidNew;

	//SetThreadLocale(lang_id);
	SetThreadLocale(0x0409); // set to en-US, that's what was being done before

	/* Load the strings table for this ID. */
	LoadCommonStrings();
	}
}


char *
plat_get_string(int i)
{
	LPTSTR str;

	if ((i >= 2048) && (i <= 3071))
	str = lpRCstr2048[i-2048].str;
	else if ((i >= 4096) && (i <= 4351))
	str = lpRCstr4096[i-4096].str;
	else if ((i >= 4352) && (i <= 4607))
	str = lpRCstr4352[i-4352].str;
	else if ((i >= 4608) && (i <= 5119))
	str = lpRCstr4608[i-4608].str;
	else if ((i >= 5120) && (i <= 5375))
	str = lpRCstr5120[i-5120].str;
	else if ((i >= 5376) && (i <= 5631))
	str = lpRCstr5376[i-5376].str;
	else if ((i >= 5632) && (i <= 5887))
	str = lpRCstr5632[i-5632].str;
	else if ((i >= 5888) && (i <= 6143))
	str = lpRCstr5888[i-5888].str;
	else if ((i >= 6144) && (i <= 7167))
	str = lpRCstr6144[i-6144].str;
	else
	str = lpRCstr7168[i-7168].str;

	return((char *)str);
}

#ifdef MTR_ENABLED
void
init_trace(void)
{
	mtr_init("trace.json");
	mtr_start();
}

void
shutdown_trace(void)
{
	mtr_stop();
	mtr_shutdown();
}
#endif

/* Create a console if we don't already have one. */
static void
CreateConsole(int init)
{
	HANDLE h;
	FILE *fp;
	fpos_t p;
	int i;

	if (! init) {
	if (force_debug)
		FreeConsole();
	return;
	}

	/* Are we logging to a file? */
	p = 0;
	(void)fgetpos(stdout, &p);
	if (p != -1) return;

	/* Not logging to file, attach to console. */
	if (! AttachConsole(ATTACH_PARENT_PROCESS)) {
	/* Parent has no console, create one. */
	if (! AllocConsole()) {
		/* Cannot create console, just give up. */
		return;
	}
	}
	fp = NULL;
	if ((h = GetStdHandle(STD_OUTPUT_HANDLE)) != NULL) {
	/* We got the handle, now open a file descriptor. */
	if ((i = _open_osfhandle((intptr_t)h, _O_TEXT)) != -1) {
		/* We got a file descriptor, now allocate a new stream. */
		if ((fp = _fdopen(i, "w")) != NULL) {
			/* Got the stream, re-initialize stdout without it. */
			(void)freopen("CONOUT$", "w", stdout);
			setvbuf(stdout, NULL, _IONBF, 0);
			fflush(stdout);
		}
	}
	}

	if (fp != NULL) {
	fclose(fp);
	fp = NULL;
	}
}


static void
CloseConsole(void)
{
	CreateConsole(0);
}


/* Process the commandline, and create standard argc/argv array. */
static int
ProcessCommandLine(char ***argv)
{
	char **args;
	int argc_max;
	int i, q, argc;

	if (acp_utf8) {
	i = strlen(GetCommandLineA()) + 1;
	argbuf = (char *)malloc(i);
	strcpy(argbuf, GetCommandLineA());
	} else {
	i = c16stombs(NULL, GetCommandLineW(), 0) + 1;
	argbuf = (char *)malloc(i);
	c16stombs(argbuf, GetCommandLineW(), i);
	}

	argc = 0;
	argc_max = 64;
	args = (char **)malloc(sizeof(char *) * argc_max);
	if (args == NULL) {
	free(argbuf);
	return(0);
	}

	/* parse commandline into argc/argv format */
	i = 0;
	while (argbuf[i]) {
	while (argbuf[i] == ' ')
		  i++;

	if (argbuf[i]) {
		if ((argbuf[i] == '\'') || (argbuf[i] == '"')) {
			q = argbuf[i++];
			if (!argbuf[i])
				break;
		} else
			q = 0;

		args[argc++] = &argbuf[i];

		if (argc >= argc_max) {
			argc_max += 64;
			args = realloc(args, sizeof(char *)*argc_max);
			if (args == NULL) {
				free(argbuf);
				return(0);
			}
		}

		while ((argbuf[i]) && ((q)
			? (argbuf[i]!=q) : (argbuf[i]!=' '))) i++;

		if (argbuf[i]) {
			argbuf[i] = 0;
			i++;
		}
	}
	}

	args[argc] = NULL;
	*argv = args;

	return(argc);
}


/* For the Windows platform, this is the start of the application. */
int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszArg, int nCmdShow)
{
	char **argv = NULL;
	int	argc, i;
	char * AppID = L"86Box.86Box\0";

	SetCurrentProcessExplicitAppUserModelID(AppID);

	/* Check if Windows supports UTF-8 */
	if (GetACP() == CP_UTF8)
	acp_utf8 = 1;
	else
	acp_utf8 = 0;

	/* Set this to the default value (windowed mode). */
	video_fullscreen = 0;

	/* We need this later. */
	hinstance = hInst;

	/* Set the application version ID string. */
	sprintf(emu_version, "%s v%s", EMU_NAME, EMU_VERSION);

	/* First, set our (default) language. */
	set_language();

	/* Process the command line for options. */
	argc = ProcessCommandLine(&argv);

	/* Pre-initialize the system, this loads the config file. */
	if (! pc_init(argc, argv))
	{
		/* Detach from console. */
		if (force_debug)
		{
			CreateConsole(0);
		}

		if (source_hwnd)
		{
			PostMessage((HWND) (uintptr_t) source_hwnd, WM_HAS_SHUTDOWN, (WPARAM) 0, (LPARAM) hwndMain);
		}


		free(argbuf);
		free(argw);
		return(EXIT_FAILURE);
	}

	/* Enable crash dump services. */
	if (enable_crashdump)
	{
		InitCrashDump();
	}

	/* Create console window. */
	if (force_debug)
	{
		CreateConsole(1);
		atexit(CloseConsole);
	}

	/* Handle our GUI. */
	i = ui_init(nCmdShow);

	free(argbuf);
	free(argv);
	return(i);
}


void
main_thread(void *param)
{
	uint32_t old_time, new_time;
	int drawits, frames;

	title_update = 1;
	old_time = GetTickCount();
	drawits = frames = 0;
	while (!is_quit) {
	/* See if it is time to run a frame of code. */
	new_time = GetTickCount();
	drawits += (new_time - old_time);
	old_time = new_time;
	if (drawits > 0 && !dopause) {
		/* Yes, so do one frame now. */
		drawits -= 10;
		if (drawits > 50)
			drawits = 0;

		/* Run a block of code. */
		pc_run();

		/* Every 200 frames we save the machine status. */
		if (++frames >= 200 && nvr_dosave) {
			nvr_save();
			nvr_dosave = 0;
			frames = 0;
		}
	} else	/* Just so we dont overload the host OS. */
		Sleep(1);

	/* If needed, handle a screen resize. */
	if (doresize && !video_fullscreen) {
		vidsize(scrnsz_x, scrnsz_y);
		doresize = 0;
	}
	}
}


/*
 * We do this here since there is platform-specific stuff
 * going on here, and we do it in a function separate from
 * main() so we can call it from the UI module as well.
 */
void
do_start(void)
{
	LARGE_INTEGER qpc;

	/* We have not stopped yet. */
	is_quit = 0;

	/* Initialize the high-precision timer. */
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&qpc);
	timer_freq = qpc.QuadPart;
	win_log("Main timer precision: %llu\n", timer_freq);

	/* Start the emulator, really. */
	thMain = thread_create(main_thread, &is_quit);
	SetThreadPriority(thMain, THREAD_PRIORITY_HIGHEST);
}


/* Cleanly stop the emulator. */
void
do_stop(void)
{
	is_quit = 1;

	plat_delay_ms(100);

	if (source_hwnd)
	PostMessage((HWND) (uintptr_t) source_hwnd, WM_HAS_SHUTDOWN, (WPARAM) 0, (LPARAM) hwndMain);

	pc_close();

	thMain = NULL;

	/* Wait a while so things can shut down. */
	plat_delay_ms(200);
}


void
plat_get_exe_name(char *s, size_t size)
{
	char temp[MAX_PATH];

	if (acp_utf8)
	GetModuleFileNameA(hinstance, s, size);
	else {
	GetModuleFileNameW(hinstance, temp, sizeof_w(temp));
	c16stombs(s, temp, size);
	}
}


int
plat_getcwd(char *bufp, int max)
{
	char *temp;

	if (acp_utf8)
	(void)_getcwd(bufp, max);
	else {
	temp = malloc(max * sizeof(char));
	(void)_wgetcwd(temp, max);
	c16stombs(bufp, temp, max);
	free(temp);
	}

	return(0);
}

int
plat_chdir(char *path)
{
	char *temp;
	int len, ret;

	if (acp_utf8)
	return(_chdir(path));
	else {
	len = mbstoc16s(NULL, path, 0) + 1;
	temp = malloc(len * sizeof(wchar_t));
	mbstoc16s(temp, path, len);

	ret = _wchdir(temp);

	free(temp);
	return ret;
	}
}


FILE *
plat_fopen(const char *path, const char *mode)
{
	char *pathw, *modew;
	int len;
	FILE *fp;

	if (acp_utf8)
	return fopen(path, mode);
	else {
	len = mbstoc16s(NULL, path, 0) + 1;
	pathw = malloc(sizeof(wchar_t) * len);
	mbstoc16s(pathw, path, len);

	len = mbstoc16s(NULL, mode, 0) + 1;
	modew = malloc(sizeof(wchar_t) * len);
	mbstoc16s(modew, mode, len);

	fp = _wfopen(pathw, modew);

	free(pathw);
	free(modew);

	return fp;
	}
}


void
plat_remove(char *path)
{
	char *temp;
	int len;

	if (acp_utf8)
	remove(path);
	else {
	len = mbstoc16s(NULL, path, 0) + 1;
	temp = malloc(len * sizeof(char));
	mbstoc16s(temp, path, len);

	_wremove(temp);

	free(temp);
	}
}


/* Make sure a path ends with a trailing (back)slash. */
void
plat_path_slash(char *path)
{
	if ((path[wcslen(path)-1] != L'\\') &&
	(path[wcslen(path)-1] != L'/')) {
	wcscat(path, L"\\");
	}
}


/* Make sure a path ends with a trailing (back)slash. */
void
plat_path_slash_a(char *path)
{
	if ((path[strlen(path)-1] != '\\') &&
	(path[strlen(path)-1] != '/')) {
	strcat(path, "\\");
	}
}


/* Check if the given path is absolute or not. */
int
plat_path_abs(char *path)
{
	if ((path[1] == L':') || (path[0] == L'\\') || (path[0] == L'/'))
	return(1);

	return(0);
}


/* Check if the given path is absolute or not. */
int
plat_path_abs_a(char *path)
{
	if ((path[1] == ':') || (path[0] == '\\') || (path[0] == '/'))
	return(1);

	return(0);
}


/* Return the last element of a pathname. */
char *
plat_get_basename(const char *path)
{
	int c = (int)strlen(path);

	while (c > 0) {
	if (path[c] == '/' || path[c] == '\\')
	   return((char *)&path[c]);
	   c--;
	}

	return((char *)path);
}


/* Return the 'directory' element of a pathname. */
void
plat_get_dirname(char *dest, const char *path)
{
	int c = (int)wcslen(path);
	char *ptr;

	ptr = (char *)path;

	while (c > 0) {
	if (path[c] == L'/' || path[c] == L'\\') {
		ptr = (char *)&path[c];
		break;
	}
	 c--;
	}

	/* Copy to destination. */
	while (path < ptr)
	*dest++ = *path++;
	*dest = L'\0';
}


/* Return the 'directory' element of a pathname. */
void
plat_get_dirname_a(char *dest, const char *path)
{
	int c = (int)strlen(path);
	char *ptr;

	ptr = (char *)path;

	while (c > 0) {
	if (path[c] == '/' || path[c] == '\\') {
		ptr = (char *)&path[c];
		break;
	}
 	c--;
	}

	/* Copy to destination. */
	while (path < ptr)
	*dest++ = *path++;
	*dest = '\0';
}


char *
plat_get_filename(char *s)
{
	int c = strlen(s) - 1;

	while (c > 0) {
	if (s[c] == '/' || s[c] == '\\')
	   return(&s[c+1]);
	   c--;
	}

	return(s);
}


char *
plat_get_extension(char *s)
{
	int c = wcslen(s) - 1;

	if (c <= 0)
	return(s);

	while (c && s[c] != L'.')
		c--;

	if (!c)
	return(&s[wcslen(s)]);

	return(&s[c+1]);
}


void
plat_append_filename(char *dest, char *s1, char *s2)
{
	wcscat(dest, s1);
	plat_path_slash(dest);
	wcscat(dest, s2);
}


void
plat_append_filename_a(char *dest, char *s1, char *s2)
{
	strcat(dest, s1);
	plat_path_slash_a(dest);
	strcat(dest, s2);
}


void
plat_put_backslash(char *s)
{
	int c = strlen(s) - 1;

	if (s[c] != '/' && s[c] != '\\')
	   s[c] = '/';
}


int
plat_dir_check(char *path)
{
	DWORD dwAttrib;
	int len;
	char *temp;

	if (acp_utf8)
	dwAttrib = GetFileAttributesA(path);
	else {
	len = mbstoc16s(NULL, path, 0);
	temp = malloc(len * sizeof(char));
	mbstoc16s(temp, path, len);

	dwAttrib = GetFileAttributesW(temp);

	free(temp);
	}

	return(((dwAttrib != INVALID_FILE_ATTRIBUTES &&
	   (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) ? 1 : 0);
}


int
plat_dir_create(const char *path)
{
	int ret, len;
	char *temp;
	int error 0;

	if (acp_utf8)
	{
		if(SHCreateDirectoryExA(NULL, path, NULL) != ERROR_SUCCESS)
		{
			ui_msgbox(MBX_ERROR, "Couldn't create directory \"%s\"", path);
			error = 1;
		}
	}
	else
	{
		len = mbstoc16s(NULL, path, 0) + 1;
		temp = malloc(len * sizeof(char));
		mbstoc16s(temp, path, len);

		if(SHCreateDirectoryExW(NULL, temp, NULL) != ERROR_SUCCESS)
		{
			ui_msgbox(MBX_ERROR, "Couldn't create directory \"%s\"", temp);
			error = 1;
		}

		free(temp);

		return error;
	}
}


uint64_t
plat_timer_read(void)
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);

	return(li.QuadPart);
}


uint32_t
plat_get_elapsed_msec(void)
{
	LARGE_INTEGER EndingTime, ElapsedMicroseconds;

	if (first_use) {
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);
	first_use = 0;
	}


	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

	/* We now have the elapsed number of ticks, along with the
	   number of ticks-per-second. We use these values
	   to convert to the number of elapsed microseconds.
	   To guard against loss-of-precision, we convert
	   to microseconds *before* dividing by ticks-per-second. */
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	return (uint32_t) (ElapsedMicroseconds.QuadPart / 1000);
}


void
plat_delay_ms(uint32_t count)
{
	Sleep(count);
}


/* Return the VIDAPI number for the given name. */
int
vidapi(char *name)
{
	int i;

	/* Default/System is SDL Hardware. */
	if (!strcasecmp(name, "default") || !strcasecmp(name, "system")) return(1);

	/* If DirectDraw or plain SDL was specified, return SDL Software. */
	if (!strcasecmp(name, "ddraw") || !strcasecmp(name, "sdl")) return(1);

	for (i = 0; i < RENDERERS_NUM; i++) {
	if (vid_apis[i].name &&
		!strcasecmp(vid_apis[i].name, name)) return(i);
	}

	/* Default value. */
	return(1);
}


/* Return the VIDAPI name for the given number. */
char *
vidapi_name(int api)
{
	char *name = "default";

	switch(api) {
	case 0:
		name = "sdl_software";
		break;
	case 1:
		break;
	case 2:
		name = "sdl_opengl";
		break;

#ifdef USE_VNC
	case 3:
		name = "vnc";
		break;
#endif
	default:
		fatal("Unknown renderer: %i\n", api);
		break;
	}

	return(name);
}


// options:
// 0 = hide window
// 1 = show window
// everything else = show window
void
plat_show_window(int options)
{
	switch (options)
	{
		case 0:
			options = SW_HIDE;
			break;
		case 1:
			options = SW_SHOW;
			break;
		default:
			options = SW_SHOW;
			break;
	}

	ShowWindow(hwndRender, options);
}


void
take_screenshot(void)
{
	startblit();
	screenshots++;
	endblit();
	device_force_redraw();
}


/* LPARAM interface to plat_get_string(). */
LPARAM win_get_string(int id)
{
	char *ret;

	ret = plat_get_string(id);
	return ((LPARAM) ret);
}


void	/* plat_ */
startblit(void)
{
	WaitForSingleObject(ghMutex, INFINITE);
}


void	/* plat_ */
endblit(void)
{
	ReleaseMutex(ghMutex);
}

void
plat_mouse_close()
{
	win_mouse_close();
}
