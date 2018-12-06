#ifndef SDLCOMPAT_H
#define SDLCOMPAT_H

#ifdef SDL_R_INCL
#include <SDL2.h>
#else
#include <SDL2/SDL.h>
#endif

#if defined(WIN) || defined(LIN)
#ifdef SDL_R_INCL
#include <SDL_syswm.h>
#else
#include <SDL2/SDL_syswm.h>
#endif
#endif

#undef Above
#undef Below
#define DrawTextA DrawText

#endif // SDLCOMPAT_H
