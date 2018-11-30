
#include "graphics/Pixel.h"

#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
struct SDL_SysWMinfo;
typedef unsigned long Atom;
extern SDL_SysWMinfo sdl_wminfo;
extern Atom XA_CLIPBOARD, XA_TARGETS, XA_UTF8_STRING, XA_NET_FRAME_EXTENTS;
#endif
extern int sdl_scale;
extern int savedWindowX;
extern int savedWindowY;

int LoadWindowPosition(int scale);

int SaveWindowPosition();

int sdl_open();

void SetSDLVideoMode(int width, int height);

int set_scale(int scale, int kiosk);

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch);

void sdl_blit_2(int x, int y, int w, int h, pixel *src, int pitch);

void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch);

int mouse_get_state(int *x, int *y);



extern unsigned short sdl_mod;
extern int sdl_key, sdl_rkey, sdl_wheel, sdl_ascii;

union SDL_Event;
int EventProcess(SDL_Event event);
int sdl_poll(void);

void limit_fps();



extern char *clipboard_text;
void clipboard_push_text(char * text);
char * clipboard_pull_text();

unsigned int GetTicks();

void BlueScreen(const char * detailMessage);
