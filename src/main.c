#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define HAVE_STDARG_H
#include <86box/86box.h>
#include <86box/config.h>
#include <86box/device.h>
#include <86box/keyboard.h>
#include <86box/mouse.h>
#include <86box/video.h>
#define GLOBAL
#include <86box/plat.h>
#include <86box/plat_midi.h>
#include <86box/ui.h>
#ifdef USE_VNC
# include <86box/vnc.h>
#endif
#include "86box/version.h"
#ifdef MTR_ENABLED
#include <minitrace/minitrace.h>
#endif

int main(int argc, char** argv)
{
	/* Set this to the default value (windowed mode). */
	video_fullscreen = 0;

	/* Set the application version ID string. */
	sprintf(emu_version, "%s v%s", EMU_NAME, EMU_VERSION);

	/* Pre-initialize the system, this loads the config file. */
	if (! pc_init(argc, argv))
	{
		return(EXIT_FAILURE);
	}
}
