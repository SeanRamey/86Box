/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Define the various platform support functions.
 *
 *
 *
 * Authors:	Miran Grca, <mgrca8@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *		Copyright 2016-2019 Miran Grca.
 *		Copyright 2017-2019 Fred N. van Kempen.
 */
#ifndef EMU_PLAT_H
#define EMU_PLAT_H

#ifndef GLOBAL
#define GLOBAL
#endif

/* String ID numbers. */
#include <86box/language.h>

/* The Win32 API uses _wcsicmp. */
#ifdef _WIN32
#define wcscasecmp	_wcsicmp
#define strcasecmp	_stricmp
#endif

#if defined(UNIX) && defined(FREEBSD)
/* FreeBSD has largefile by default. */
# define fopen64        fopen
# define fseeko64       fseeko
# define ftello64       ftello
# define off64_t        off_t
#elif defined(_MSC_VER)
//# define fopen64	fopen
# define fseeko64	_fseeki64
# define ftello64	_ftelli64
# define off64_t	off_t
#endif


#ifdef _MSC_VER
# define UNUSED(arg)	arg
#else
  /* A hack (GCC-specific?) to allow us to ignore unused parameters. */
# define UNUSED(arg)	__attribute__((unused))arg
#endif

/* Return the size (in wchar's) of a wchar_t array. */
#define sizeof_w(x)	(sizeof((x)) / sizeof(wchar_t))


#ifdef __cplusplus
 "C" {
#endif

namespace Platform
{
	/* Global variables residing in the platform module. */
	int	dopause,			/* system is paused */
	doresize,			/* screen resize requested */
	is_quit,				/* system exit requested */
	mouse_capture;			/* mouse is captured in app */

	#ifdef MTR_ENABLED
	int tracing_on;
	#endif

	uint64_t	timer_freq;
	int	infocus;
	char	emu_version[200];		/* version ID string */
	int	rctrl_is_lalt;
	int	update_icons;

	int	unscaled_size_x,		/* current unscaled size X */
			unscaled_size_y;		/* current unscaled size Y */

	/* System-related functions. */
	wchar_t	*fix_exe_path(wchar_t *str);
	FILE	*plat_fopen(wchar_t *path, wchar_t *mode);
	FILE	*plat_fopen64(const wchar_t *path, const wchar_t *mode);
	void	plat_remove(wchar_t *path);
	int	plat_getcwd(wchar_t *bufp, int max);
	int	plat_chdir(wchar_t *path);
	void	plat_tempfile(wchar_t *bufp, wchar_t *prefix, wchar_t *suffix);
	void	plat_get_exe_name(wchar_t *s, int size);
	wchar_t	*plat_get_basename(const wchar_t *path);
	void	plat_get_dirname(wchar_t *dest, const wchar_t *path);
	wchar_t	*plat_get_filename(wchar_t *s);
	wchar_t	*plat_get_extension(wchar_t *s);
	void	plat_append_filename(wchar_t *dest, wchar_t *s1, wchar_t *s2);
	void	plat_put_backslash(wchar_t *s);
	void	plat_path_slash(wchar_t *path);
	int	plat_path_abs(wchar_t *path);
	int	plat_dir_check(wchar_t *path);
	int	plat_dir_create(wchar_t *path);
	uint64_t	plat_timer_read(void);
	uint32_t	plat_get_ticks(void);
	void	plat_delay_ms(uint32_t count);
	void	plat_pause(int p);
	void	plat_mouse_capture(int on);
	int	plat_vidapi(char *name);
	char	*plat_vidapi_name(int api);
	int	plat_setvid(int api);
	void	plat_vidsize(int x, int y);
	void	plat_setfullscreen(int on);
	void	plat_resize(int x, int y);
	void	plat_vidapi_enable(int enabled);


	/* Resource management. */
	void	set_language(int id);
	wchar_t	*plat_get_string(int id);


	/* Emulator start/stop support functions. */
	void	do_start(void);
	void	do_stop(void);


	/* Power off. */
	void	plat_power_off(void);


	/* Platform-specific device support. */
	void	floppy_mount(uint8_t id, wchar_t *fn, uint8_t wp);
	void	floppy_eject(uint8_t id);
	void	cdrom_mount(uint8_t id, wchar_t *fn);
	void	plat_cdrom_ui_update(uint8_t id, uint8_t reload);
	void	zip_eject(uint8_t id);
	void	zip_mount(uint8_t id, wchar_t *fn, uint8_t wp);
	void	zip_reload(uint8_t id);
	void	mo_eject(uint8_t id);
	void	mo_mount(uint8_t id, wchar_t *fn, uint8_t wp);
	void	mo_reload(uint8_t id);
	int      ioctl_open(uint8_t id, char d);
	void     ioctl_reset(uint8_t id);
	void     ioctl_close(uint8_t id);


	/* Thread support. */
	typedef void thread_t;
	typedef void event_t;
	typedef void mutex_t;

	thread_t	*thread_create(void (*thread_func)(void *param), void *param);
	void	thread_kill(thread_t *arg);
	int	thread_wait(thread_t *arg, int timeout);
	event_t	*thread_create_event(void);
	void	thread_set_event(event_t *arg);
	void	thread_reset_event(event_t *arg);
	int	thread_wait_event(event_t *arg, int timeout);
	void	thread_destroy_event(event_t *arg);

	#define MUTEX_DEFAULT_SPIN_COUNT 1024

	mutex_t	*thread_create_mutex(void);
	mutex_t	*thread_create_mutex_with_spin_count(unsigned int spin_count);
	void	thread_close_mutex(mutex_t *arg);
	int	thread_wait_mutex(mutex_t *arg);
	int	thread_release_mutex(mutex_t *mutex);

	/* Other stuff. */
	void	startblit(void);
	void	endblit(void);
	void	take_screenshot(void);

	#ifdef MTR_ENABLED
	void init_trace(void);
	void shutdown_trace(void);
	#endif

}
#ifdef __cplusplus
}
#endif


#endif	/*EMU_PLAT_H*/
