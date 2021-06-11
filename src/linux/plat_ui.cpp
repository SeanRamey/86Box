/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		UI implementation for Linux
 *
 *
 *
 * Authors:	Sean Ramey <sramey40@gmail.com>
 */
#define UNICODE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <86box/plat.h>
#include <86box/86box.h>
#include <86box/config.h>
#include "../cpu/cpu.h"
#include <86box/device.h>
#include <86box/keyboard.h>
#include <86box/mouse.h>
#include <86box/timer.h>
#include <86box/nvr.h>
#include <86box/video.h>
#include <86box/vid_ega.h>		// for update_overscan
#include <86box/plat_midi.h>
#include <86box/plat_dynld.h>
#include <86box/ui.h>
#include <86box/version.h>
#include <86box/window.h>
#ifdef USE_DISCORD
# include <86box/win_discord.h>
#endif

#ifdef MTR_ENABLED
#include <minitrace/minitrace.h>
#endif


extern "C" {

int get_dpi(WindowHandle hwnd) {

}

int get_system_metrics(int index, int dpi) {

}

void ResizeWindowByClientArea(WindowHandle hwnd, int width, int height)
{

}

/* Set host cursor visible or not. */
void show_cursor(int val)
{

}


void notify_dlg_open(void)
{

}


void notify_dlg_closed(void)
{

}


void plat_power_off(void)
{

}


int ui_init(int nCmdShow)
{

}


char *ui_window_title(char *s)
{

}



void plat_pause(int p)
{

}


/* Tell the UI about a new screen resolution. */
void resize(int x, int y)
{

}


void plat_mouse_capture(int on)
{

}

} // extern "C"
