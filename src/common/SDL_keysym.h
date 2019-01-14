#ifndef SDL1_2
#ifdef SDL_R_INCL
#include <SDL_keycode.h>
#else
#include <SDL2/SDL_keycode.h>
#endif

#else // SDL1_2

#ifdef SDL_R_INCL
#include <SDL_keysym.h>
#else
#include <SDL/SDL_keysym.h>
#endif

#define KMOD_GUI KMOD_META
#define KMOD_LGUI KMOD_LMETA
#define KMOD_RGUI KMOD_RMETA
#define SDLK_LGUI SDLK_LMETA
#define SDLK_RGUI SDLK_RMETA

#endif
