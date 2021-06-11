#ifndef VIDAPI_H
#define VIDAPI_H

#ifdef USE_VNC
#define RENDERERS_NUM		4
#else
#define RENDERERS_NUM		3
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern int vidapi(char *name);
extern char	*vidapi_name(int api);
extern int setvid(int api);
extern void	vidsize(int x, int y);
extern void	setfullscreen(int on);
extern void	resize(int x, int y);
extern void	vidapi_enable(int enabled);

typedef struct PlatVideoAPI {
    const char	*name;
    int		local;
    int		(*init)(void);
    void	(*close)(void);
    void	(*resize)(int x, int y);
    int		(*pause)(void);
    void	(*enable)(int enable);
    void	(*set_fs)(int fs);
} PlatVideoAPI;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VIDAPI_H
