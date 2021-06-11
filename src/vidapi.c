#include <86box/vidapi.h>
#include <86box/86box.h>
#include <86box/win_sdl.h>
#include <86box/rect.h>
#ifdef USE_VNC
# include <86box/vnc.h>
#endif

static const PlatVideoAPI vid_apis[RENDERERS_NUM] = {
  {	"SDL_Software", 1, sdl_inits, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs	},
  {	"SDL_Hardware", 1, sdl_inith, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs	},
  {	"SDL_OpenGL", 1, sdl_initho, sdl_close, NULL, sdl_pause, sdl_enable, sdl_set_fs		}
#ifdef USE_VNC
 ,{	"VNC", 0, vnc_init, vnc_close, vnc_resize, vnc_pause, NULL, NULL }
#endif
};

static int vid_api_inited = 0;


/* Return the VIDAPI number for the given name. */
int vidapi(char *name)
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
char* vidapi_name(int api)
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


int
setvid(int api)
{
	int i;

	win_log("Initializing VIDAPI: api=%d\n", api);
	startblit();

	/* Close the (old) API. */
	vid_apis[vid_api].close();
	vid_api = api;

	if (vid_apis[vid_api].local)
	plat_show_window(1); //show window
	else
	plat_show_window(0); //hide window

	/* Initialize the (new) API. */
	i = vid_apis[vid_api].init();
	endblit();
	if (! i) return(0);

	device_force_redraw();

	vid_api_inited = 1;

	return(1);
}


/* Tell the renderers about a new screen resolution. */
void vidsize(int x, int y)
{
	if (!vid_api_inited || !vid_apis[vid_api].resize) return;

	startblit();
	vid_apis[vid_api].resize(x, y);
	endblit();
}


void vidapi_enable(int enable)
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


void setfullscreen(int on)
{

}
