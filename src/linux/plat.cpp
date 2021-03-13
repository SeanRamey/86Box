/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Linux platform specific functions.
 *
 *
 *
 * Authors:	Sean Ramey <sramey40@gmail.com>
 *
 *		Copyright 2021 Sean Ramey
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <clocale>
#include <thread>

#include <86box/86box.h>
#include <86box/plat.h>

namespace Platform
{
    static const struct {
        const char	*name;
        int		local;
        int		(*init)(void *);
        void	(*close)(void);
        void	(*resize)(int x, int y);
        int		(*pause)(void);
        void	(*enable)(int enable);
        void	(*set_fs)(int fs);
    } vid_apis[RENDERERS_NUM] = {
    {	"SDL_Software", 1, (int(*)(void*))sdl_inits, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs	},
    {	"SDL_Hardware", 1, (int(*)(void*))sdl_inith, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs	},
    {	"SDL_OpenGL", 1, (int(*)(void*))sdl_initho, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs	}
    #ifdef USE_VNC
    ,{	"VNC", 0, vnc_init, vnc_close, vnc_resize, vnc_pause, NULL, NULL				}
    #endif
    };


    /* Set (or re-set) the language for the application. */
    void set_language(int id)
    {
        // just set to guaranteed available POSIX or C locale
        setlocale(LC_ALL, "");
    }

    #ifdef MTR_ENABLED
    void init_trace(void)
    {
        mtr_init("trace.json");
        mtr_start();
    }

    void shutdown_trace(void)
    {
        mtr_stop();
        mtr_shutdown();
    }
    #endif

    /*
    * We do this here since there is platform-specific stuff
    * going on here, and we do it in a function separate from
    * main() so we can call it from the UI module as well.
    */
    void do_start(void)
    {
        // We have not stopped yet.
        is_quit = 0;

        // Start the emulator, really.
        std::thread mainThread(pc_thread, &is_quit);
    }


    /* Cleanly stop the emulator. */
    void do_stop(void)
    {
        is_quit = 1;

        plat_delay_ms(100);

        pc_close(mainThread);

        mainThread = NULL;
    }


    void plat_get_exe_name(wchar_t *s, int size)
    {
        GetModuleFileName(hinstance, s, size);
    }


    void plat_tempfile(wchar_t *bufp, wchar_t *prefix, wchar_t *suffix)
    {
        SYSTEMTIME SystemTime;
        char temp[1024];

        if (prefix != NULL)
        sprintf(temp, "%ls-", prefix);
        else
        strcpy(temp, "");

        GetSystemTime(&SystemTime);
        sprintf(&temp[strlen(temp)], "%d%02d%02d-%02d%02d%02d-%03d%ls",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
        SystemTime.wMilliseconds,
        suffix);
        mbstowcs(bufp, temp, strlen(temp)+1);
    }


    int plat_getcwd(wchar_t *bufp, int max)
    {
        (void)_wgetcwd(bufp, max);

        return(0);
    }


    int plat_chdir(wchar_t *path)
    {
        return(_wchdir(path));
    }


    FILE* plat_fopen(wchar_t *path, wchar_t *mode)
    {
        return(_wfopen(path, mode));
    }


    /* Open a file, using Unicode pathname, with 64bit pointers. */
    FILE* plat_fopen64(const wchar_t *path, const wchar_t *mode)
    {
        return(_wfopen(path, mode));
    }


    void plat_remove(wchar_t *path)
    {
        _wremove(path);
    }


    /* Make sure a path ends with a trailing (back)slash. */
    void plat_path_slash(wchar_t *path)
    {
        if ((path[wcslen(path)-1] != L'\\') &&
        (path[wcslen(path)-1] != L'/')) {
        wcscat(path, L"\\");
        }
    }


    /* Check if the given path is absolute or not. */
    int plat_path_abs(wchar_t *path)
    {
        if ((path[1] == L':') || (path[0] == L'\\') || (path[0] == L'/'))
        return(1);

        return(0);
    }


    /* Return the last element of a pathname. */
    wchar_t* plat_get_basename(const wchar_t *path)
    {
        int c = (int)wcslen(path);

        while (c > 0) {
        if (path[c] == L'/' || path[c] == L'\\')
        return((wchar_t *)&path[c]);
        c--;
        }

        return((wchar_t *)path);
    }


    /* Return the 'directory' element of a pathname. */
    void plat_get_dirname(wchar_t *dest, const wchar_t *path)
    {
        int c = (int)wcslen(path);
        wchar_t *ptr;

        ptr = (wchar_t *)path;

        while (c > 0) {
        if (path[c] == L'/' || path[c] == L'\\') {
            ptr = (wchar_t *)&path[c];
            break;
        }
        c--;
        }

        /* Copy to destination. */
        while (path < ptr)
        *dest++ = *path++;
        *dest = L'\0';
    }


    wchar_t* plat_get_filename(wchar_t *s)
    {
        int c = wcslen(s) - 1;

        while (c > 0) {
        if (s[c] == L'/' || s[c] == L'\\')
        return(&s[c+1]);
        c--;
        }

        return(s);
    }


    wchar_t* plat_get_extension(wchar_t *s)
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


    void plat_append_filename(wchar_t *dest, wchar_t *s1, wchar_t *s2)
    {
        wcscat(dest, s1);
        plat_path_slash(dest);
        wcscat(dest, s2);
    }


    void plat_put_backslash(wchar_t *s)
    {
        int c = wcslen(s) - 1;

        if (s[c] != L'/' && s[c] != L'\\')
        s[c] = L'/';
    }


    int plat_dir_check(wchar_t *path)
    {
        DWORD dwAttrib = GetFileAttributes(path);

        return(((dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) ? 1 : 0);
    }


    int plat_dir_create(wchar_t *path)
    {
        return((int)SHCreateDirectory(hwndMain, path));
    }


    uint64_t plat_timer_read(void)
    {
        LARGE_INTEGER li;

        QueryPerformanceCounter(&li);

        return(li.QuadPart);
    }


    uint32_t plat_get_ticks(void)
    {
        return(GetTickCount());
    }


    void plat_delay_ms(uint32_t count)
    {
        Sleep(count);
    }


    /* Return the VIDAPI number for the given name. */
    int plat_vidapi(char *name)
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
    char* plat_vidapi_name(int api)
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


    int plat_setvid(int api)
    {
        int i;

        win_log("Initializing VIDAPI: api=%d\n", api);
        startblit();

        /* Close the (old) API. */
        vid_apis[vid_api].close();
        vid_api = api;

        if (vid_apis[vid_api].local)
        ShowWindow(hwndRender, SW_SHOW);
        else
        ShowWindow(hwndRender, SW_HIDE);

        /* Initialize the (new) API. */
        i = vid_apis[vid_api].init((void *)hwndRender);
        endblit();
        if (! i) return(0);

        device_force_redraw();

        vid_api_inited = 1;

        return(1);
    }


    /* Tell the renderers about a new screen resolution. */
    void plat_vidsize(int x, int y)
    {
        if (!vid_api_inited || !vid_apis[vid_api].resize) return;

        startblit();
        vid_apis[vid_api].resize(x, y);
        endblit();
    }


    void plat_vidapi_enable(int enable)
    {
        int i = 1;

        if (!vid_api_inited || !vid_apis[vid_api].enable)
        return;

        vid_apis[vid_api].enable(enable != 0);

        if (! i)
        return;

        if (enable)
        device_force_redraw();
    }


    int get_vidpause(void)
    {
        return(vid_apis[vid_api].pause());
    }


    void plat_setfullscreen(int on)
    {
        RECT rect;
        int temp_x, temp_y;
        int dpi = win_get_dpi(hwndMain);

        /* Are we changing from the same state to the same state? */
        if ((!!on) == (!!video_fullscreen))
        return;

        if (on && video_fullscreen_first) {
        video_fullscreen |= 2;
        if (ui_msgbox_header(MBX_INFO | MBX_DONTASK, (wchar_t *) IDS_2134, (wchar_t *) IDS_2052) == 10) {
            video_fullscreen_first = 0;
            config_save();
        }
        video_fullscreen &= 1;
        }

        /* OK, claim the video. */
        win_mouse_close();

        /* Close the current mode, and open the new one. */
        video_fullscreen = on | 2;
        if (vid_apis[vid_api].set_fs)
        vid_apis[vid_api].set_fs(on);
        if (!on) {
        plat_resize(scrnsz_x, scrnsz_y);
        if (vid_resize) {
            /* scale the screen base on DPI */
            if (window_remember) {
                MoveWindow(hwndMain, window_x, window_y, window_w, window_h, TRUE);
                GetClientRect(hwndMain, &rect);

                temp_x = rect.right - rect.left + 1;
                temp_y = rect.bottom - rect.top + 1 - sbar_height;
            } else {
                if (dpi_scale) {
                    temp_x = MulDiv(unscaled_size_x, dpi, 96);
                    temp_y = MulDiv(unscaled_size_y, dpi, 96);
                } else {
                    temp_x = unscaled_size_x;
                    temp_y = unscaled_size_y;
                }

                /* Main Window. */
                ResizeWindowByClientArea(hwndMain, temp_x, temp_y + sbar_height);
            }

            /* Render window. */
            MoveWindow(hwndRender, 0, 0, temp_x, temp_y, TRUE);
            GetWindowRect(hwndRender, &rect);

            /* Status bar. */
            MoveWindow(hwndSBAR, 0, rect.bottom, temp_x, 17, TRUE);

            if (mouse_capture)
                ClipCursor(&rect);

            scrnsz_x = unscaled_size_x;
            scrnsz_y = unscaled_size_y;
        }
        }
        video_fullscreen &= 1;
        video_force_resize_set(1);
        if (!on)
        doresize = 1;

        win_mouse_init();

        /* Release video and make it redraw the screen. */
        device_force_redraw();

        /* Send a CTRL break code so CTRL does not get stuck. */
        keyboard_input(0, 0x01D);

        /* Finally, handle the host's mouse cursor. */
        /* win_log("%s full screen, %s cursor\n", on ? "enter" : "leave", on ? "hide" : "show"); */
        show_cursor(video_fullscreen ? 0 : -1);

        /* This is needed for OpenGL. */
        plat_vidapi_enable(0);
        plat_vidapi_enable(1);
    }


    void take_screenshot(void)
    {
        startblit();
        screenshots++;
        endblit();
        device_force_redraw();
    }


    void startblit(void)
    {
        WaitForSingleObject(ghMutex, INFINITE);
    }


    void endblit(void)
    {
        ReleaseMutex(ghMutex);
    }

}
