
#include <climits>
#include <list>

#include "SDLCompat.h"
#ifdef LIN
#include "images.h"
#endif

#include "EventLoopSDL.h"
#include "defines.h"
#include "graphics.h"
#include "interface.h"
#include "misc.h" // mystrdup, remove after sdl2 port

#include "common/Platform.h"
#include "interface/Engine.h"
#include "gui/game/PowderToy.h" // for the_game->DeFocus(), remove once all interfaces get modernized


#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS, XA_UTF8_STRING, XA_NET_FRAME_EXTENTS;
#endif
SDL_Surface *sdl_scrn;
int sdl_scale = 1;

int savedWindowX = INT_MAX;
int savedWindowY = INT_MAX;

unsigned short sdl_mod;
int sdl_key, sdl_rkey, sdl_wheel, sdl_ascii;


// Returns 1 if the loaded position was set
// Returns 0 if something went wrong: SDL_GetWMInfo failed or the loaded position was invalid
int LoadWindowPosition(int scale)
{
#ifdef WIN
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		const SDL_VideoInfo * vidInfo = SDL_GetVideoInfo();
		int desktopWidth = vidInfo->current_w;
		int desktopHeight = vidInfo->current_h;

		int windowW = (XRES + BARSIZE) * scale;
		int windowH = (YRES + MENUSIZE) * scale;

		// Center the window on the primary desktop by default
		int newWindowX = (desktopWidth - windowW) / 2;
		int newWindowY = (desktopHeight - windowH) / 2;

		int success = 0;

		if (savedWindowX != INT_MAX && savedWindowY != INT_MAX)
		{
			POINT windowPoints[] = {
				{savedWindowX, savedWindowY},                       // Top-left
				{savedWindowX + windowW, savedWindowY + windowH}    // Bottom-right
			};

			MONITORINFO monitor;
			monitor.cbSize = sizeof(monitor);
			if (GetMonitorInfo(MonitorFromPoint(windowPoints[0], MONITOR_DEFAULTTONEAREST), &monitor) != 0)
			{
				// Only use the saved window position if it lies inside the visible screen
				if (PtInRect(&monitor.rcMonitor, windowPoints[0]) && PtInRect(&monitor.rcMonitor, windowPoints[1]))
				{
					newWindowX = savedWindowX;
					newWindowY = savedWindowY;

					success = 1;
				}
				else
				{
					// Center the window on the nearest monitor
					newWindowX = monitor.rcMonitor.left + (monitor.rcMonitor.right - monitor.rcMonitor.left - windowW) / 2;
					newWindowY = monitor.rcMonitor.top + (monitor.rcMonitor.bottom - monitor.rcMonitor.top - windowH) / 2;
				}
			}
		}

		SetWindowPos(sysInfo.window, 0, newWindowX, newWindowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

		// 1 if we didn't use the default, i.e. the position was valid
		return success;
	}
#endif

	return 0;
}

// Returns 1 if the window position was saved
int SaveWindowPosition()
{
#ifdef WIN
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		WINDOWPLACEMENT placement;
		placement.length = sizeof(placement);
		GetWindowPlacement(sysInfo.window, &placement);

		savedWindowX = (int)placement.rcNormalPosition.left;
		savedWindowY = (int)placement.rcNormalPosition.top;

		return 1;
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		Display *display;
		Window window, dummy;
		XWindowAttributes attributes;

		sysInfo.info.x11.lock_func();
		display = sysInfo.info.x11.display;
		window = sysInfo.info.x11.window;
		XSync(display, false);
		XGetWindowAttributes(display, window, &attributes);
		XTranslateCoordinates(display, window, attributes.root, 0, 0, &savedWindowX, &savedWindowY, &dummy);

		//some window managers don't include the border when getting position, so we adjust for that here
		unsigned long nitems, bytes_after;
		unsigned char *property;
		Atom type;
		int format;

		//adjust for window border. Some window managers will have get ignore the window border, so set needs to be adjusted to include it
		//this doesn't actually work though, for some reason. Possible alternative, xwininfo command?
		if (Success == XGetWindowProperty(display, window, XA_NET_FRAME_EXTENTS, 0, 16, 0, XA_CARDINAL, &type, &format, &nitems, &bytes_after, &property))
		{
			if (property)
			{
				long border_left = ((Atom*)property)[0];
				long border_top = ((Atom*)property)[2];

				savedWindowX -= border_left;
				savedWindowY -= border_top;

				XFree(property);
			}
			else
			{
				//hardcode my size :P
				savedWindowX -= 3;
				savedWindowY -= 22;
			}
		}

		sysInfo.info.x11.unlock_func();
		return 1;
	}
#endif

	return 0;
}

int sdl_opened = 0;
int sdl_open()
{
	//char screen_err = 0;
#ifdef WIN
	SDL_SysWMinfo SysInfo;
	HWND WindowHandle;
	HICON hIconSmall;
	HICON hIconBig;
#elif LIN
	char envStr[64];
	sprintf(envStr, "SDL_VIDEO_WINDOW_POS=%i,%i", savedWindowX, savedWindowY);
	SDL_putenv(envStr);
#endif
	//if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK)<0)
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
#ifndef ANDROID
	// scale to screen size, android does this for us automatically
	if (!sdl_opened && firstRun)
	{
		const SDL_VideoInfo *info = SDL_GetVideoInfo();
		// at least 50 pixels of buffer space just to be safe
		if (sdl_scale == 1 && info->current_w-50 > ((XRES+BARSIZE)*2) && info->current_h-50 > ((YRES+MENUSIZE)*2))
		{
			sdl_scale = 2;
			doubleScreenDialog = true;
			screenWidth = info->current_w;
			screenHeight = info->current_h;
		}
	}
#endif

#ifdef WIN
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWMInfo(&SysInfo) <= 0) {
		printf("%s : %p\n", SDL_GetError(), SysInfo.window);
		exit(-1);
	}
	WindowHandle = SysInfo.window;
	hIconSmall = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	hIconBig = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif LIN
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void*)app_icon, 48, 48, 24, 144, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
	SDL_WM_SetIcon(icon, (Uint8*)app_icon_bitmap);
	SDL_FreeSurface(icon);
#endif
	SDL_WM_SetCaption("Jacob1's Mod", "Jacob1's Mod");

	atexit(SDL_Quit);

	LoadWindowPosition(sdl_scale);
	SetSDLVideoMode((XRES + BARSIZE) * sdl_scale, (YRES + MENUSIZE) * sdl_scale);

	if (!sdl_scrn)
	{
		fprintf(stderr, "Creating window: %s\n", SDL_GetError());
		return 0;
	}
	SDL_EnableUNICODE(1);
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_VERSION(&sdl_wminfo.version);
	if(SDL_GetWMInfo(&sdl_wminfo) > 0)
	{
		SDL_GetWMInfo(&sdl_wminfo);
		sdl_wminfo.info.x11.lock_func();
		XA_CLIPBOARD = XInternAtom(sdl_wminfo.info.x11.display, "CLIPBOARD", 1);
		XA_TARGETS = XInternAtom(sdl_wminfo.info.x11.display, "TARGETS", 1);
		XA_UTF8_STRING = XInternAtom(sdl_wminfo.info.x11.display, "UTF8_STRING", 1);
		XA_NET_FRAME_EXTENTS = XInternAtom(sdl_wminfo.info.x11.display, "_NET_FRAME_EXTENTS", 0);
		sdl_wminfo.info.x11.unlock_func();
	}
	else
	{
		fprintf(stderr, "X11 setup failed, X11 window info not found");
		exit(-1);
	}
#elif WIN
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

#ifdef ANDROID
	// hide overlay buttons that just get in the way
	SDL_ANDROID_SetScreenKeyboardShown(0);
	//SDL_JoystickOpen(1);
#endif

	//if (screen_err)
	//	error_ui(vid_buf, 0, "Can't change scale factor, because screen resolution is too small");
	sdl_opened = 1;
	return 1;
}

void SetSDLVideoMode(int width, int height)
{
#ifdef PIX16
	if (kiosk_enable)
		sdl_scrn = SDL_SetVideoMode(width, height, 16, SDL_FULLSCREEN|SDL_SWSURFACE);
	else
		sdl_scrn = SDL_SetVideoMode(width, height, 16, SDL_SWSURFACE);
#else
	if (kiosk_enable)
		sdl_scrn = SDL_SetVideoMode(width, height, 32, SDL_FULLSCREEN|SDL_SWSURFACE);
	else
		sdl_scrn = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
#endif
}
int set_scale(int scale, int kiosk)
{
	if (scale > 5)
		return 0;
	int old_scale = sdl_scale, old_kiosk = kiosk_enable;
	sdl_scale = scale;
	kiosk_enable = kiosk;

	SaveWindowPosition();
#ifdef LIN
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
	if (!sdl_open())// || (size_error && !confirm_ui(vid_buf, "Confirm Size Change", "Your screen is too large, press OK to keep the size change anyway", "OK")))
	{
		sdl_scale = old_scale;
		kiosk_enable = old_kiosk;
		sdl_open();
		return 0;
	}
	return 1;
}

void DrawPixel(pixel * vid, pixel color, int x, int y)
{
	if (x >= 0 && x < (XRES+BARSIZE) && y >= 0 && y < (YRES+MENUSIZE))
		vid[x+y*(XRES+BARSIZE)] = color;
}
// draws a custom cursor, used to make 3D mode work properly (normal cursor ruins the effect)
void DrawCursor(pixel * vid)
{
	/*Point mouse = Engine::Ref().GetLastMousePosition();
	int mousex = mouse.X;
	int mousey = mouse.Y;*/
	int mousex, mousey;
	mouse_get_state(&mousex, &mousey);
	for (int j = 0; j <= 9; j++)
	{
		for (int i = 0; i <= j; i++)
		{
			if (i == 0 || i == j)
				DrawPixel(vid, 0xFFFFFFFF, mousex+i, mousey+j);
			else
				DrawPixel(vid, 0xFF000000, mousex+i, mousey+j);
		}
	}
	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+10);
	for (int i = 0; i < 5; i++)
	{
		DrawPixel(vid, 0xFF000000, mousex+1+i, mousey+10);
		DrawPixel(vid, 0xFFFFFFFF, mousex+6+i, mousey+10);
	}
	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+1, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+2, mousey+11);
	DrawPixel(vid, 0xFFFFFFFF, mousex+3, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+4, mousey+11);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+11);
	DrawPixel(vid, 0xFFFFFFFF, mousex+6, mousey+11);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+1, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+2, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+4, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+12);
	DrawPixel(vid, 0xFF000000, mousex+6, mousey+12);
	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+12);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+1, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+4, mousey+13);
	DrawPixel(vid, 0xFF000000, mousex+5, mousey+13);
	DrawPixel(vid, 0xFF000000, mousex+6, mousey+13);
	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+13);

	DrawPixel(vid, 0xFFFFFFFF, mousex, mousey+14);
	for (int i = 0; i < 2; i++)
	{
		DrawPixel(vid, 0xFFFFFFFF, mousex+5, mousey+14+i);
		DrawPixel(vid, 0xFF000000, mousex+6, mousey+14+i);
		DrawPixel(vid, 0xFF000000, mousex+7, mousey+14+i);
		DrawPixel(vid, 0xFFFFFFFF, mousex+8, mousey+14+i);

		DrawPixel(vid, 0xFFFFFFFF, mousex+6, mousey+16+i);
		DrawPixel(vid, 0xFF000000, mousex+7, mousey+16+i);
		DrawPixel(vid, 0xFF000000, mousex+8, mousey+16+i);
		DrawPixel(vid, 0xFFFFFFFF, mousex+9, mousey+16+i);
	}

	DrawPixel(vid, 0xFFFFFFFF, mousex+7, mousey+18);
	DrawPixel(vid, 0xFFFFFFFF, mousex+8, mousey+18);
}

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch)
{
	pixel *dst;
	int j, depth3d = Engine::Ref().Get3dDepth();
	if (SDL_MUSTLOCK(sdl_scrn))
		if (SDL_LockSurface(sdl_scrn)<0)
			return;
	dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
	if (depth3d)
	{
		if (!vid3d)
			vid3d = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
		memcpy(vid3d, src, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		DrawCursor(vid3d);
		src = vid3d;
	}
	if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
	{
		//pixel format conversion, used for strange formats (OS X specifically)
		int i;
		unsigned int red, green, blue;
		pixel px, lastpx, nextpx;
		SDL_PixelFormat *fmt = sdl_scrn->format;
		if (depth3d)
		{
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
				{
					lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
					nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
					int redshift = PIXB(lastpx) + PIXG(lastpx);
					if (redshift > 255)
						redshift = 255;
					int blueshift = PIXR(nextpx) + PIXG(nextpx);
					if (blueshift > 255)
						blueshift = 255;
					red = ((int)(PIXR(lastpx)*.69f+redshift*.3f)>>fmt->Rloss)<<fmt->Rshift;
					green = ((int)(PIXG(nextpx)*.3f)>>fmt->Gloss)<<fmt->Gshift;
					blue = ((int)(PIXB(nextpx)*.69f+blueshift*.3f)>>fmt->Bloss)<<fmt->Bshift;
					dst[i] = red|green|blue;
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
		else
		{
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
				{
					px = src[i];
					red = (PIXR(px)>>fmt->Rloss)<<fmt->Rshift;
					green = (PIXG(px)>>fmt->Gloss)<<fmt->Gshift;
					blue = (PIXB(px)>>fmt->Bloss)<<fmt->Bshift;
					dst[i] = red|green|blue;
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
	}
	else
	{
		int i;
		if (depth3d)
		{
			pixel lastpx, nextpx;
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
				{
					lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
					nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
					int redshift = PIXB(lastpx) + PIXG(lastpx);
					if (redshift > 255)
						redshift = 255;
					int blueshift = PIXR(nextpx) + PIXG(nextpx);
					if (blueshift > 255)
						blueshift = 255;
					dst[i] = PIXRGB((int)(PIXR(lastpx)*.69f+redshift*.3f), (int)(PIXG(nextpx)*.3f), (int)(PIXB(nextpx)*.69f+blueshift*.3f));
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
		else
		{
			for (j=0; j<h; j++)
			{
				memcpy(dst, src, w*PIXELSIZE);
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
	}
	if (SDL_MUSTLOCK(sdl_scrn))
		SDL_UnlockSurface(sdl_scrn);
	SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit_2(int x, int y, int w, int h, pixel *vid, int pitch)
{
	pixel *dst, *src = vid;
	pixel px, lastpx, nextpx;
	int j, depth3d = Engine::Ref().Get3dDepth();
	int i, k, sx;
	if (SDL_MUSTLOCK(sdl_scrn))
		if (SDL_LockSurface(sdl_scrn)<0)
			return;
	dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
	if (depth3d)
	{
		if (!vid3d)
			vid3d = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
		memcpy(vid3d, src, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		DrawCursor(vid3d);
		src = vid3d;
	}
	if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
	{
		//pixel format conversion, used for strange formats (OS X specifically)
		SDL_PixelFormat *fmt = sdl_scrn->format;
		int red, green, blue;
		for (j=0; j<h; j++)
		{
			for (k=0; k<sdl_scale; k++)
			{
				for (i=0; i<w; i++)
				{
					if (depth3d)
					{
						lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
						nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
						int redshift = PIXB(lastpx) + PIXG(lastpx);
						if (redshift > 255)
							redshift = 255;
						int blueshift = PIXR(nextpx) + PIXG(nextpx);
						if (blueshift > 255)
							blueshift = 255;
						red = ((int)(PIXR(lastpx)*.69f+redshift*.3f)>>fmt->Rloss)<<fmt->Rshift;
						green = ((int)(PIXG(nextpx)*.3f)>>fmt->Gloss)<<fmt->Gshift;
						blue = ((int)(PIXB(nextpx)*.69f+blueshift*.3f)>>fmt->Bloss)<<fmt->Bshift;
					}
					else
					{
						px = src[i];
						red = (PIXR(px)>>fmt->Rloss)<<fmt->Rshift;
						green = (PIXG(px)>>fmt->Gloss)<<fmt->Gshift;
						blue = (PIXB(px)>>fmt->Bloss)<<fmt->Bshift;
					}
					for (sx = 0; sx < sdl_scale; sx++)
						dst[i*sdl_scale+sx] = red|green|blue;
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
			}
			src+=pitch;
		}
	}
	else
	{
		for (j=0; j<h; j++)
		{
			for (k=0; k<sdl_scale; k++)
			{
				for (i=0; i<w; i++)
				{
					px = src[i];
					if (depth3d)
					{
						lastpx = i >= depth3d && i < w+depth3d ? src[i-depth3d] : 0;
						nextpx = i >= -depth3d && i < w-depth3d ? src[i+depth3d] : 0;
						int redshift = PIXB(lastpx) + PIXG(lastpx);
						if (redshift > 255)
							redshift = 255;
						int blueshift = PIXR(nextpx) + PIXG(nextpx);
						if (blueshift > 255)
							blueshift = 255;
						px = PIXRGB((int)(PIXR(lastpx)*.69f+redshift*.3f), (int)(PIXG(nextpx)*.3f), (int)(PIXB(nextpx)*.69f+blueshift*.3f));
					}
					for (sx = 0; sx < sdl_scale; sx++)
						dst[i*sdl_scale+sx] = px;
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
			}
			src+=pitch;
		}
	}
	if (SDL_MUSTLOCK(sdl_scrn))
		SDL_UnlockSurface(sdl_scrn);
	SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch)
{
	if (sdl_scale >= 2)
		sdl_blit_2(x, y, w, h, src, pitch);
	else
		sdl_blit_1(x, y, w, h, src, pitch);
}

int mouse_get_state(int *x, int *y)
{
	//Wrapper around SDL_GetMouseState to divide by sdl_scale
	Uint8 sdl_b;
	int sdl_x, sdl_y;
	sdl_b = SDL_GetMouseState(&sdl_x, &sdl_y);
	*x = sdl_x/sdl_scale;
	*y = sdl_y/sdl_scale;
	return sdl_b;
}


SDLKey MapNumpad(SDLKey key)
{
	switch(key)
	{
	case SDLK_KP8:
		return SDLK_UP;
	case SDLK_KP2:
		return SDLK_DOWN;
	case SDLK_KP6:
		return SDLK_RIGHT;
	case SDLK_KP4:
		return SDLK_LEFT;
	case SDLK_KP7:
		return SDLK_HOME;
	case SDLK_KP1:
		return SDLK_END;
	case SDLK_KP_PERIOD:
		return SDLK_DELETE;
	case SDLK_KP0:
	case SDLK_KP3:
	case SDLK_KP9:
		return SDLK_UNKNOWN;
	default:
		return key;
	}
}

int EventProcess(SDL_Event event)
{
	if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
	{
		if (event.key.keysym.unicode == 0)
		{
			// If unicode is zero, this could be a numpad key with numlock off, or numlock on and shift on (unicode is set to 0 by SDL or the OS in these circumstances. If numlock is on, unicode is the relevant digit character).
			// For some unknown reason, event.key.keysym.mod seems to be unreliable on some computers (keysum.mod&KEY_MOD_NUM is opposite to the actual value), so check keysym.unicode instead.
			// Note: unicode is always zero for SDL_KEYUP events, so this translation won't always work properly for keyup events.
			SDLKey newKey = MapNumpad(event.key.keysym.sym);
			if (newKey != event.key.keysym.sym)
			{
				event.key.keysym.sym = newKey;
				event.key.keysym.unicode = 0;
			}
		}
	}
	switch (event.type)
	{
	case SDL_KEYDOWN:
		sdl_key = event.key.keysym.sym;
		sdl_ascii = event.key.keysym.unicode;
		sdl_mod = static_cast<unsigned short>(SDL_GetModState());
		if (event.key.keysym.sym=='q' && (sdl_mod & (KMOD_CTRL|KMOD_META)))
		{
			if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
			{
				has_quit = 1;
				return 1;
			}
		}
		break;
	case SDL_KEYUP:
		sdl_rkey = event.key.keysym.sym;
		sdl_mod = static_cast<unsigned short>(SDL_GetModState());
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_WHEELUP)
			sdl_wheel++;
		if (event.button.button == SDL_BUTTON_WHEELDOWN)
			sdl_wheel--;
		break;
	case SDL_VIDEORESIZE:
		// screen resize event, we are supposed to call SDL_SetVideoMode with the new size. Ignore this entirely and call it with the old size :)
		// if we don't do this, the touchscreen calibration variables won't ever be set properly
		SetSDLVideoMode((XRES + BARSIZE) * sdl_scale, (YRES + MENUSIZE) * sdl_scale);
		break;
	case SDL_QUIT:
		if (fastquit)
			has_quit = 1;
		return 1;
	case SDL_SYSWMEVENT:
#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
		if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
			break;
		sdl_wminfo.info.x11.lock_func();
		XEvent xe = event.syswm.msg->event.xevent;
		if (xe.type==SelectionClear)
		{
			if (clipboard_text!=NULL) {
				free(clipboard_text);
				clipboard_text = NULL;
			}
		}
		else if (xe.type==SelectionRequest)
		{
			XEvent xr;
			xr.xselection.type = SelectionNotify;
			xr.xselection.requestor = xe.xselectionrequest.requestor;
			xr.xselection.selection = xe.xselectionrequest.selection;
			xr.xselection.target = xe.xselectionrequest.target;
			xr.xselection.property = xe.xselectionrequest.property;
			xr.xselection.time = xe.xselectionrequest.time;
			if (xe.xselectionrequest.target==XA_TARGETS)
			{
				// send list of supported formats
				Atom targets[] = {XA_TARGETS, XA_STRING, XA_UTF8_STRING};
				xr.xselection.property = xe.xselectionrequest.property;
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
			}
			// TODO: Supporting more targets would be nice
			else if ((xe.xselectionrequest.target==XA_STRING || xe.xselectionrequest.target==XA_UTF8_STRING) && clipboard_text)
			{
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, xe.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)clipboard_text, strlen(clipboard_text)+1);
			}
			else
			{
				// refuse clipboard request
				xr.xselection.property = None;
			}
			XSendEvent(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, 0, 0, &xr);
		}
		sdl_wminfo.info.x11.unlock_func();
#elif WIN
		switch (event.syswm.msg->msg)
		{
		case WM_USER+614:
			if (!ptsaveOpenID && !saveURIOpen && num_tabs < 24-GetNumMenus() && main_loop)
				ptsaveOpenID = event.syswm.msg->lParam;
			//If we are already opening a save, we can't have it do another one, so just start it in a new process
			else
			{
				char *exename = Platform::ExecutableName(), args[64];
				sprintf(args, "ptsave noopen:%i", event.syswm.msg->lParam);
				if (exename)
				{
					ShellExecute(NULL, "open", exename, args, NULL, SW_SHOWNORMAL);
					free(exename);
				}
				//I doubt this will happen ... but as a last resort just open it in this window anyway
				else
					saveURIOpen = event.syswm.msg->lParam;
			}
			break;
		}
#else
		break;
#endif
	}
	return 0;
}

int FPSwait = 0, fastquit = 0;
int sdl_poll(void)
{
	SDL_Event event;
	sdl_key=sdl_rkey=sdl_wheel=sdl_ascii=0;
	if (has_quit)
		return 1;
	loop_time = SDL_GetTicks();
	if (main_loop > 0)
	{
		main_loop--;
		if (main_loop == 0)
		{
			SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
			FPSwait = 2;
			the_game->OnDefocus();
		}
		else
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	}

	while (SDL_PollEvent(&event))
	{
		int ret = EventProcess(event);
		if (ret)
			return ret;
	}

	sdl_mod = static_cast<unsigned short>(SDL_GetModState());
	limit_fps();
	sendNewEvents = true;
	return 0;
}

int pastFPS = 0;
float FPSB2 = 60.0f;
double frameTimeAvg = 0.0, correctedFrameTimeAvg = 60.0;
void limit_fps()
{
	int frameTime = SDL_GetTicks() - currentTime;

	frameTimeAvg = frameTimeAvg * .8 + frameTime * .2;
	if (limitFPS > 2)
	{
		double offset = 1000.0 / limitFPS - frameTimeAvg;
		if (offset > 0)
			//millisleep((Uint32)(offset + 0.5));
			SDL_Delay((Uint32)(offset + 0.5));
	}

	int correctedFrameTime = SDL_GetTicks() - currentTime;
	correctedFrameTimeAvg = correctedFrameTimeAvg * 0.95 + correctedFrameTime * 0.05;
	elapsedTime = currentTime-pastFPS;
	if (elapsedTime >= 200)
	{
		if (!FPSwait)
			FPSB2 = 1000.0f/(float)correctedFrameTimeAvg;
		if (main_loop && FPSwait > 0)
			FPSwait--;
		pastFPS = currentTime;
	}
	currentTime = SDL_GetTicks();
}



char * clipboardtext = NULL;
void clipboard_push_text(char * text)
{
	if (clipboardtext)
	{
		free(clipboardtext);
		clipboardtext = NULL;
	}
	clipboardtext = mystrdup(text);
#ifdef ANDROID
	SDL_SetClipboardText(text);
#elif MACOSX
	writeClipboard(text);
#elif WIN
	if (OpenClipboard(NULL))
	{
		HGLOBAL cbuffer;
		char * glbuffer;

		EmptyClipboard();

		cbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text)+1);
		glbuffer = (char*)GlobalLock(cbuffer);

		strcpy(glbuffer, text);

		GlobalUnlock(cbuffer);
		SetClipboardData(CF_TEXT, cbuffer);
		CloseClipboard();
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	if (clipboard_text!=NULL) {
		free(clipboard_text);
		clipboard_text = NULL;
	}
	clipboard_text = mystrdup(text);
	sdl_wminfo.info.x11.lock_func();
	XSetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
	XFlush(sdl_wminfo.info.x11.display);
	sdl_wminfo.info.x11.unlock_func();
#else
	printf("Not implemented: put text on clipboard \"%s\"\n", text);
#endif
}

char * clipboard_pull_text()
{
#ifdef ANDROID
	if (!SDL_HasClipboardText())
		return mystrdup("");
	char *data = SDL_GetClipboardText();
	if (!data)
		return mystrdup("");
	char *ret = mystrdup(data);
	SDL_free(data);
	return ret;
#elif MACOSX
	char * data = readClipboard();
	if (!data)
		return mystrdup("");
	return mystrdup(data);
#elif WIN
	if (OpenClipboard(NULL))
	{
		HANDLE cbuffer;
		char * glbuffer;

		cbuffer = GetClipboardData(CF_TEXT);
		glbuffer = (char*)GlobalLock(cbuffer);
		GlobalUnlock(cbuffer);
		CloseClipboard();
		if(glbuffer!=NULL){
			return mystrdup(glbuffer);
		} //else {
		//	return mystrdup("");
		//}
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	char *text = NULL;
	Window selectionOwner;
	sdl_wminfo.info.x11.lock_func();
	selectionOwner = XGetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD);
	if (selectionOwner != None)
	{
		unsigned char *data = NULL;
		Atom type;
		int format, result;
		unsigned long len, bytesLeft;
		XConvertSelection(sdl_wminfo.info.x11.display, XA_CLIPBOARD, XA_UTF8_STRING, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
		XFlush(sdl_wminfo.info.x11.display);
		sdl_wminfo.info.x11.unlock_func();
		while (1)
		{
			SDL_Event event;
			SDL_WaitEvent(&event);
			if (event.type == SDL_SYSWMEVENT)
			{
				XEvent xevent = event.syswm.msg->event.xevent;
				if (xevent.type == SelectionNotify && xevent.xselection.requestor == sdl_wminfo.info.x11.window)
					break;
				else
					EventProcess(event);
			}
		}
		sdl_wminfo.info.x11.lock_func();
		XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, 0, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
		if (data)
		{
			XFree(data);
			data = NULL;
		}
		if (bytesLeft)
		{
			result = XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, bytesLeft, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
			if (result == Success)
			{
				text = strdup((const char*) data);
				XFree(data);
			}
			else
			{
				printf("Failed to pull from clipboard\n");
				return mystrdup("?");
			}
		}
		else
			return mystrdup("");
		XDeleteProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD);
	}
	sdl_wminfo.info.x11.unlock_func();
	return text;
#else
	printf("Not implemented: get text from clipboard\n");
#endif
	if (clipboardtext)
		return mystrdup(clipboardtext);
	return mystrdup("");
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
}

void BlueScreen(const char * detailMessage)
{
	//std::string errorDetails = "Details: " + std::string(detailMessage);
	SDL_Event event;
	const char * errorHelp = "An unrecoverable fault has occurred, please report this to jacob1:\n"
		" http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=11117\n"
		" OR the built in bug reporter.\n\n"
		"Note: TPT will now restart and reload your work";
	int positionX = (XRES+BARSIZE)/2-textwidth(errorHelp)/2-50, positionY = (YRES+MENUSIZE)/2-100;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 17, 114, 169, 210);

	drawtext(vid_buf, positionX, positionY, "ERROR", 255, 255, 255, 255);
	drawtext(vid_buf, positionX, positionY + 14, detailMessage, 255, 255, 255, 255);
	drawtext(vid_buf, positionX, positionY  + 28, errorHelp, 255, 255, 255, 255);

	pixel* vid_buf2 = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	memcpy(vid_buf2, vid_buf, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
	std::vector<Point> food;
	std::list<Point> tron;
	unsigned int tronSize = 5, score = 0;
	int tronDirection = 2;
	char scoreString[20];
	bool gameRunning = false, gameLost = false;
	for (int i = 0; i < 10; i++)
		food.push_back(Point(rand()%(XRES+BARSIZE-6)+3, rand()%(YRES+MENUSIZE-6)+3));
	for (unsigned int i = 0; i < tronSize; i++)
		tron.push_back(Point(20, 10+i));

	//Death loop
	while(1)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				Platform::DoRestart(true);
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					if (tronDirection != 2)
						tronDirection = 0;
					break;
				case SDLK_RIGHT:
					if (tronDirection != 3)
						tronDirection = 1;
					break;
				case SDLK_DOWN:
					if (tronDirection != 0)
						tronDirection = 2;
					break;
				case SDLK_LEFT:
					if (tronDirection != 1)
						tronDirection = 3;
					break;
				default:
#ifdef ANDROID
					Platform::DoRestart(true);
#endif
					break;
				}
				gameRunning = true;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button != SDL_BUTTON_WHEELUP && event.button.button != SDL_BUTTON_WHEELDOWN)
				{
					int mx = event.motion.x/sdl_scale, my = event.motion.y/sdl_scale;
					if (my < (YRES+MENUSIZE)/4)
					{
						if (tronDirection != 2)
							tronDirection = 0;
					}
					else if (my > YRES+MENUSIZE-(YRES+MENUSIZE)/4)
					{
						if (tronDirection != 0)
							tronDirection = 2;
					}
					else if (mx < (XRES+BARSIZE)/4)
					{
						if (tronDirection != 1)
							tronDirection = 3;
					}
					else if (mx > XRES+BARSIZE-(XRES+BARSIZE)/4)
					{
						if (tronDirection != 3)
							tronDirection = 1;
					}
					gameRunning = true;
				}
			}
		}
		if (gameRunning && !gameLost)
		{
			Point tronHead = ((Point)tron.back());
			tronHead.X -= (tronDirection-2)%2;
			tronHead.Y += (tronDirection-1)%2;
			if (tronHead.X > 1 && tronHead.X < XRES+BARSIZE-2 && tronHead.Y > 1 && tronHead.Y < YRES+MENUSIZE-2)
				tron.push_back(tronHead);
			else
				gameLost = true;

			sprintf(scoreString, "Score: %i", score);
			drawtext(vid_buf, XRES-BARSIZE-50, 10, scoreString, 255, 255, 255, 255);
			for (std::vector<Point>::iterator iter = food.begin(); iter != food.end(); ++iter)
			{
				Point moo = (Point)*iter;
				vid_buf[moo.X + moo.Y*(XRES+BARSIZE)] = PIXRGB(255, 0, 0);
				vid_buf[moo.X + moo.Y*(XRES+BARSIZE)+1] = PIXRGB(100, 0, 0);
				vid_buf[moo.X + moo.Y*(XRES+BARSIZE)-1] = PIXRGB(100, 0, 0);
				vid_buf[moo.X + (moo.Y+1)*(XRES+BARSIZE)] = PIXRGB(100, 0, 0);
				vid_buf[moo.X + (moo.Y-1)*(XRES+BARSIZE)] = PIXRGB(100, 0, 0);
				if (tronHead.X >= moo.X - 3 && tronHead.X <= moo.X + 3 && tronHead.Y >= moo.Y - 3 && tronHead.Y <= moo.Y + 3)
				{
					food.erase(iter, iter+1);
					food.push_back(Point(rand()%(XRES+BARSIZE-6)+3, rand()%(YRES+MENUSIZE-6)+3));
					food.push_back(Point(rand()%(XRES+BARSIZE-6)+3, rand()%(YRES+MENUSIZE-6)+3));
					tronSize += 20;
					score++;
					break;
				}
			}
			if (!(rand()%20))
				tronSize++;
			int i = tronSize;
			for (std::list<Point>::iterator iter = tron.begin(); iter != tron.end(); ++iter)
			{
				Point point = *iter;
				vid_buf[point.X + point.Y*(XRES+BARSIZE)] = PIXRGB(0, 255-(int)(200.0f/tronSize*i), 0);
				i--;
				if (iter != --tron.end() && point == tronHead)
					gameLost = true;
			}
			while (tron.size() > tronSize)
				tron.pop_front();
		}
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf, XRES+BARSIZE);
		if (gameRunning && !gameLost)
			memcpy(vid_buf, vid_buf2, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		SDL_Delay(5);
	}
}
