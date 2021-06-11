/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Implement threads for Linux.
 *
 *
 *
 * Authors:	Sean Ramey <sramey40@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <threads.h>
#include <86box/86box.h>
#include <86box/plat.h>

thread_t* thread_create(int (*func)(void *param), void *param)
{
	thrd_t thread_id = 0;

	int error = thrd_create(&thread_id, func, param);

	switch(error) {
		case thrd_success:
			break;
		case thrd_timedout:
			pclog("Thread %d timed out.", thread_id);
			thread_id = 0;
			break;
		case thrd_busy:
			pclog("Thread %d is busy.", thread_id);
			thread_id = 0;
			break;
		case thrd_nomem:
			fatal("Thread %d couldn't be created; Out of memory.", thread_id);
			thread_id = 0;
			break;
		case thrd_error:
		default:
			fatal("Thread %d couldn't be created. General error.", thread_id);
			thread_id = 0;
			break;
	}

    return((thread_t *)thread_id);
}


void thread_kill(thrd_t thread)
{
    int error = thrd_detach(thread);

	if(error == thrd_error)
	{
		fatal("Thread %d failed to detach properly.", thread);
	}
}


int thread_wait(thread_t *arg, int timeout)
{
    if (arg == NULL)
		return(0);

    if (timeout == -1)
		timeout = INFINITE;

    if (WaitForSingleObject(arg, timeout))
		return(1);

    return(0);
}


event_t* thread_create_event(void)
{
    win_event_t *ev = malloc(sizeof(win_event_t));

    ev->handle = CreateEvent(NULL, FALSE, FALSE, NULL);

    return((event_t *)ev);
}


void thread_set_event(event_t *arg)
{
    win_event_t *ev = (win_event_t *)arg;

    if (arg == NULL) return;

    SetEvent(ev->handle);
}


void thread_reset_event(event_t *arg)
{
    win_event_t *ev = (win_event_t *)arg;

    if (arg == NULL) return;

    ResetEvent(ev->handle);
}


int thread_wait_event(event_t *arg, int timeout)
{
    win_event_t *ev = (win_event_t *)arg;

    if (arg == NULL) return(0);

    if (ev->handle == NULL) return(0);

    if (timeout == -1)
	timeout = INFINITE;

    if (WaitForSingleObject(ev->handle, timeout)) return(1);

    return(0);
}


void thread_destroy_event(event_t *arg)
{
    win_event_t *ev = (win_event_t *)arg;

    if (arg == NULL) return;

    CloseHandle(ev->handle);

    free(ev);
}


mutex_t* thread_create_mutex(void)
{
    mutex_t *mutex = malloc(sizeof(CRITICAL_SECTION));

    InitializeCriticalSection(mutex);

    return mutex;
}


mutex_t* thread_create_mutex_with_spin_count(unsigned int spin_count)
{
    mutex_t *mutex = malloc(sizeof(CRITICAL_SECTION));

    InitializeCriticalSectionAndSpinCount(mutex, spin_count);

    return mutex;
}


int thread_wait_mutex(mutex_t *mutex)
{
    if (mutex == NULL) return(0);

    LPCRITICAL_SECTION critsec = (LPCRITICAL_SECTION)mutex;

    EnterCriticalSection(critsec);

    return 1;
}


int thread_release_mutex(mutex_t *mutex)
{
    if (mutex == NULL) return(0);

    LPCRITICAL_SECTION critsec = (LPCRITICAL_SECTION)mutex;

    LeaveCriticalSection(critsec);

    return 1;
}


void thread_close_mutex(mutex_t *mutex)
{
    if (mutex == NULL) return;

    LPCRITICAL_SECTION critsec = (LPCRITICAL_SECTION)mutex;

    DeleteCriticalSection(critsec);

    free(critsec);
}
