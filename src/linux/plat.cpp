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
 */

#include <cstdarg>
#include <cstdint>

// needed before the #include of <stdio.h> for safe versions of printf, such as printf_s or sprintf_s
#ifdef __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <clocale>
#include <thread>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <86box/86box.h>
#include <86box/plat.h>
#include <86box/utility.h>
#include <86box/timer.h>
#include <86box/nvr.h>
#include <86box/vidapi.h>

namespace Platform
{
extern "C"
{

	/* Set (or re-set) the language for the application. */
	void set_language(void)
	{
		// just set to guaranteed available POSIX or C locale
		std::setlocale(LC_ALL, "C");
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

	void main_thread(void *param)
	{
		uint32_t old_time, new_time;
		int drawits, frames;
		extern int title_update;
		title_update = 1;
		old_time = plat_timer_read();
		drawits = frames = 0;
		while (!is_quit) {
		/* See if it is time to run a frame of code. */
		new_time = plat_timer_read();
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
			plat_delay_ms(1);

		/* If needed, handle a screen resize. */
		if (doresize && !video_fullscreen) {
			vidsize(scrnsz_x, scrnsz_y);
			doresize = 0;
		}
		}
	}

	static std::thread mainThread;

	void do_start(void)
	{
		// We have not stopped yet.
		is_quit = 0;

		// Start the emulator, really.
		mainThread = std::thread(main_thread, &is_quit);
	}


	/* Cleanly stop the emulator. */
	void do_stop(void)
	{
		is_quit = 1;

		plat_delay_ms(100);

		// wait for main thread to finish.
		mainThread.join();

		pc_close();
	}

	void plat_get_exe_name(char *s, size_t size)
	{
		ssize_t bytesRead = readlink("/proc/self/exe", s, size-1);
		if(bytesRead == -1)
		{
			fatal("Failed to read full pathname.");
		}

		s[bytesRead] = '\0'; // append null char to string, since readlink() doesn't.
	}


	int plat_getcwd(char *buf, size_t size)
	{
		getcwd(buf, size);

		return 0;
	}


	int plat_chdir(const char *path)
	{
		return chdir(path);
	}


	FILE* plat_fopen(const char *path, const char *mode)
	{
		return fopen(path, mode);
	}


	void plat_remove(char *path)
	{
		remove(path);
	}


	/* Make sure a path ends with a trailing (back)slash. */
	void plat_path_slash(char *path)
	{
		if ((path[strlen(path)-1] != '\\') && (path[strlen(path)-1] != '/'))
		{
			strcat(path, "\\");
		}
	}


	/* Check if the given path is absolute or not. */
	int plat_path_abs(char *path)
	{
		if ((path[1] == ':') || (path[0] == '\\') || (path[0] == '/'))
		{
			return(1);
		}

		return(0);
	}


	/* Return the last element of a pathname. */
	char* plat_get_basename(const char *path)
	{
		size_t c = strlen(path);

		while (c > 0)
		{
			if (path[c] == '/' || path[c] == '\\')
			{
				return((char *)&path[c]);
			}
			c--;
		}

		return((char *)path);
	}


	/* Return the 'directory' element of a pathname. */
	void plat_get_dirname(char *dest, const char *path)
	{
		size_t c = strlen(path);
		char *ptr;

		ptr = (char *)path;

		while (c > 0)
		{
			if (path[c] == '/' || path[c] == '\\')
			{
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


	char* plat_get_filename(char *s)
	{
		size_t c = strlen(s) - 1;

		while (c > 0)
		{
			if (s[c] == '/' || s[c] == '\\')
			{
				return(&s[c+1]);
				c--;
			}
		}

		return(s);
	}


	char* plat_get_extension(char *s)
	{
		size_t c = strlen(s) - 1;

		if (c <= 0)
		{
			return(s);
		}

		while (c && s[c] != '.')
		{
			c--;
		}

		if (!c)
		{
			return(&s[strlen(s)]);
		}

		return(&s[c+1]);
	}


	void plat_append_filename(char *dest, char *s1, char *s2)
	{
		strcat(dest, s1);
		plat_path_slash(dest);
		strcat(dest, s2);
	}


	void plat_put_backslash(char *s)
	{
		size_t c = strlen(s) - 1;

		if (s[c] != '/' && s[c] != '\\')
		{
			s[c] = '/';
		}

	}


	int plat_dir_check(const char *path)
	{

		struct stat s = {0};

		if (!stat(path, &s))
		{
			fatal("plat_dir_check() failed to check if %s is a directory or not.", path);
		}

		return S_ISDIR(s.st_mode);
	}


	int plat_dir_create(const char *path)
	{
		int error = 0;
		if(!mkdir(path, NULL))
		{
			// mkdir() returns -1 on error.
			error = 1;
		}

		return error;
	}


	uint64_t plat_timer_read(void)
	{
		clock_t ticks = clock();
		return (uint64_t)ticks;

		// maybe another possible way? Posix specific
		// struct timespec time{0};
		// clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
		// return (uint64_t)time.tv_nsec;
	}


	uint32_t plat_get_elapsed_msec(void)
	{
		// remember, static vars initialized only once
		static int first_use = 1;
		static clock_t starting_time;

		if (first_use) {
			starting_time = clock();
			first_use = 0;
		}

		clock_t now_time = clock();

		// double dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
		// double posix_dur = 1000.0*ts2.tv_sec + 1e-6*ts2.tv_nsec
		// 				- (1000.0*ts1.tv_sec + 1e-6*ts1.tv_nsec);
		// double posix_wall = 1000.0*tw2.tv_sec + 1e-6*tw2.tv_nsec
		// 				- (1000.0*tw1.tv_sec + 1e-6*tw1.tv_nsec);

		// printf("CPU time used (per clock(): %.2f ms\n", dur);
		// printf("CPU time used (per clock_gettime()): %.2f ms\n", posix_dur);
		// printf("Wall time passed: %.2f ms\n", posix_wall);
	}


	void plat_delay_ms(uint32_t count)
	{
		useconds_t ucount = (useconds_t)count/1000; // convert to microseconds
		usleep(ucount);
	}


	// options:
	// 0 = hide window
	// 1 = show window
	// everything else = show window
	void plat_show_window(int options)
	{
		switch (options)
		{
			case 0:
				options = 0;
				break;
			case 1:
				options = 1;
				break;
			default:
				options = 1;
				break;
		}

		ShowWindow(hwndRenderer, options);
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

	void plat_mouse_close()
	{
		// do nothing on linux
	}

} // extern "C"
} // namespace Platform
