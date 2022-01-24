/**
 * Powder Toy - user interface
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/profile/ProfileViewer.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <sstream>
#include <set>
#include <cmath>
#include <ctime>
#include <dirent.h>
#ifdef WIN
#include <direct.h>
#define getcwd _getcwd
#ifdef _MSC_VER
#undef rmdir
#define rmdir _rmdir //deprecated in visual studio
#else
#include <windows.h>
#endif
#include <shellapi.h>
#endif
#if defined(LIN) || defined(MACOSX)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "cJSON.h"
#include "legacy_console.h"
#include "defines.h"
#include "EventLoopSDL.h"
#include "font.h"
#include "graphics.h"
#include "interface.h"
#include "hud.h"
#include "images.h"
#include "luaconsole.h"
#include "misc.h"
#include "powder.h"
#include "powdergraphics.h"
#include "save_legacy.h"
#include "update.h"

#include "common/Format.h"
#include "common/Platform.h"
#include "game/Authors.h"
#include "game/Request.h"
#include "game/Favorite.h"
#include "game/Menus.h"
#include "game/Save.h"
#include "game/Sign.h"
#include "game/ToolTip.h"
#include "interface/Engine.h"
#include "json/json.h"
#include "simulation/SnapshotHistory.h"
#include "simulation/Tool.h"
#include "simulation/WallNumbers.h"
#include "simulation/ToolNumbers.h"
#include "simulation/GolNumbers.h"

#include "gui/dialogs/ConfirmPrompt.h"
#include "gui/game/PowderToy.h"
#include "gui/gol/GolWindow.h"
#include "simulation/elements/ANIM.h"
#include "simulation/elements/LIFE.h"

int svf_login = 0;
int svf_admin = 0;
int svf_mod = 0;
char svf_user[64] = "";
char svf_user_id[64] = "";
char svf_session_id[64] = "";
char svf_session_key[64] = "";

int svf_open = 0;
int svf_own = 0;
int svf_myvote = 0;
int svf_publish = 0;
char svf_filename[255] = "";
int svf_fileopen = 0;
char svf_id[16] = "";
std::string svf_version = "";
char svf_name[64] = "";
char svf_description[255] = "";
char svf_author[64] = "";
char svf_tags[256] = "";

char *search_ids[GRID_X*GRID_Y];
char *search_dates[GRID_X*GRID_Y];
int   search_votes[GRID_X*GRID_Y];
int   search_publish[GRID_X*GRID_Y];
int	  search_scoredown[GRID_X*GRID_Y];
int	  search_scoreup[GRID_X*GRID_Y];
char *search_names[GRID_X*GRID_Y];
char *search_owners[GRID_X*GRID_Y];
void *search_thumbs[GRID_X*GRID_Y];
int   search_thsizes[GRID_X*GRID_Y];

int search_own = 0;
int search_fav = 0;
int search_date = 0;
int search_page = 0;
int p1_extra = 0;
char search_expr[256] = "";

char server_motd[512] = "";

char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int hud_menunum = 0;
int dateformat = 7;
int show_ids = 0;

bool drawgrav_enable = false;

void ui_edit_init(ui_edit *ed, int x, int y, int w, int h)
{
	ed->x = x;
	ed->y = y;
	ed->w = w;
	ed->h = h;
	ed->nx = 1;
	ed->def[0] = '\0';
	strcpy(ed->str, "");
#ifdef TOUCHUI
	ed->focus = 0;
#else
	ed->focus = 1;
#endif
	ed->alwaysFocus = 0;
	ed->hide = 0;
	ed->multiline = 0;
	ed->limit = 255;
	ed->cursor = ed->cursorstart = 0;
	ed->highlightstart = ed->highlightlength = 0;
	ed->resizable = ed->resizespeed = 0;
	ed->lastClick = ed->numClicks = 0;
	ed->overDelete = 0;
	ed->autoCorrect = 1;
}

int ui_edit_draw(pixel *vid_buf, ui_edit *ed)
{
	int cx, i, cy, ret = 12;
	char echo[1024], *str, highlightstr[1024];

	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = (char)0x8D;
		echo[i] = 0;
		str = echo;
	}
	else
		str = ed->str;

	if (ed->str[0])
	{
		int deletecolor = 127+ed->overDelete*64;
		if (ed->multiline) {
			ret = drawtextwrap(vid_buf, ed->x, ed->y, ed->w-14, 0, str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlightwrap(vid_buf, ed->x, ed->y, ed->w-14, 0, ed->str, ed->highlightstart, ed->highlightlength);
			}
			drawtext(vid_buf, ed->x+ed->w-10, ed->y, "\xAA", deletecolor, deletecolor, deletecolor, 255);
		} else {
			ret = drawtext(vid_buf, ed->x, ed->y, str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlight(vid_buf, ed->x+textwidth(str)-textwidth(&str[ed->highlightstart]), ed->y, highlightstr);
			}
			drawtext(vid_buf, ed->x+ed->w-10, ed->y, "\xAA", deletecolor, deletecolor, deletecolor, 255);
		}
	}
	else if (!ed->focus)
		drawtext(vid_buf, ed->x, ed->y, ed->def, 128, 128, 128, 255);
	if (ed->focus && ed->numClicks < 2)
	{
		if (ed->multiline) {
			textnpos(str, ed->cursor, ed->w-14, &cx, &cy);
		} else {
			cx = textnwidth(str, ed->cursor);
			cy = 0;
		}

		for (i=-3; i<9; i++)
			drawpixel(vid_buf, ed->x+cx, ed->y+i+cy, 255, 255, 255, 255);
	}
	if (ed->resizable && ed->multiline)
	{
		int diff = ((ret+2)-ed->h)/5;
		if (diff == 0)
			ed->h = ret+2;
		else
			ed->h += diff;
		ret = ed->h-2;
	}
	return ret;
}

void findWordPosition(const char *s, int position, int *cursorStart, int *cursorEnd, const char *spaces)
{
	int wordLength = 0, totalLength = 0, strLength = strlen(s);
	while (totalLength < strLength)
	{
		wordLength = strcspn(s, spaces);
		if (totalLength + wordLength >= position)
		{
			*cursorStart = totalLength;
			*cursorEnd = totalLength+wordLength;
			return;
		}
		s += wordLength+1;
		totalLength += wordLength+1;
	}
	*cursorStart = totalLength;
	*cursorEnd = totalLength+wordLength;
}

void ui_edit_process(int mx, int my, int mb, int mbq, ui_edit *ed)
{
	char ch, ts[2], echo[1024], *str = ed->str;
	int l, i;

	if (ed->alwaysFocus && !ed->focus)
	{
		ed->focus = 1;
	}
	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = (char)0x8D;
		echo[i] = 0;
		str = echo;
	}
	if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11) //on top of the delete button
	{
		if (!ed->overDelete && !mb)
			ed->overDelete = 1;
		else if (mb && !mbq)
			ed->overDelete = 2;
		if (!mb && mbq && ed->overDelete == 2)
		{
#ifndef TOUCHUI
			ed->focus = 1;
#endif
			ed->cursor = 0;
			ed->str[0] = 0;
			if (!ed->numClicks)
				ed->numClicks = 1;
		}
	}
	else if (mb && (ed->focus || !mbq) && mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+(ed->multiline?ed->h:11)) //clicking / dragging over textbox
	{
#ifdef TOUCHUI
		if (ed->focus == 0)
		{
			char buffer[1024];
			memcpy(buffer, ed->str, 1024);
			Platform::GetOnScreenKeyboardInput(buffer, 1024, ed->autoCorrect);
			if (!(!ed->multiline && textwidth(buffer) > ed->w-14) && !((int)strlen(buffer)>ed->limit) && !(ed->multiline && ed->limit != 1023 && ((textwidth(buffer))/(ed->w-14)*12) > ed->h))
				memcpy(ed->str, buffer, 1024);
		}
#endif
		ed->focus = 1;
		ed->overDelete = 0;
		ed->cursor = textposxy(str, ed->w-14, mx-ed->x, my-ed->y);
		if (!ed->numClicks)
			ed->numClicks = 1;
	}
	else if (mb && !mbq) //a first click anywhere outside
	{
		if (!ed->alwaysFocus && ed->focus)
		{
			ed->focus = 0;
			ed->cursor = ed->cursorstart = 0;
		}
	}
	else //when a click has moved outside the textbox
	{
		if (ed->numClicks && mb && (ed->focus || !mbq))
		{
			if (my <= ed->y-5)
				ed->cursor = 0;
			else if (my >= ed->y+ed->h)
				ed->cursor = strlen(ed->str);
		}
		ed->overDelete = 0;
	}
	if (ed->focus && sdl_key)
	{
		l = strlen(ed->str);
		switch (sdl_key)
		{
		case SDLK_HOME:
			ed->cursorstart = 0;
			if (!(sdl_mod & KMOD_SHIFT))
				ed->cursor = 0;
			break;
		case SDLK_END:
			ed->cursorstart = l;
			if (!(sdl_mod & KMOD_SHIFT))
				ed->cursor = l;
			break;
		case SDLK_LEFT:
			if (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))
			{
				if (ed->cursor)
					ed->cursor--;
				else if (ed->cursorstart < ed->cursor && ed->cursorstart)
					ed->cursorstart--;
			}
			else
			{
				if (ed->cursor > 0)
					ed->cursor--;
				if (ed->cursor > 0 && ed->str[ed->cursor-1] == '\b')
				{
					ed->cursor--;
				}
				ed->cursorstart = ed->cursor;
			}
			break;
		case SDLK_RIGHT:
			if (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))
			{
				if (ed->cursor < l)
					ed->cursor++;
				else if (ed->cursorstart > ed->cursor && ed->cursorstart < l)
					ed->cursorstart++;
			}
			else
			{
				if (ed->cursor < l && ed->str[ed->cursor] == '\b')
				{
					ed->cursor++;
				}
				if (ed->cursor < l)
					ed->cursor++;
				ed->cursorstart = ed->cursor;
			}
			break;
		case SDLK_DELETE:
			if (sdl_mod & (KMOD_CTRL|KMOD_GUI))
			{
				int start, end;
				const char *spaces = " .,!?\n";
				findWordPosition(ed->str, ed->cursor+1, &start, &end, spaces);
				if (end > ed->cursor)
				{
					memmove(ed->str+ed->cursor, ed->str+end, l-ed->cursor);
					break;
				}
			}
			if (ed->highlightlength)
			{
				memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
				ed->cursor = ed->highlightstart;
			}
			else if (ed->cursor < l)
			{
				if (ed->str[ed->cursor] == '\b')
				{
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+2, l-ed->cursor);
				}
				else
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			}
			ed->cursorstart = ed->cursor;
			break;
		case SDLK_BACKSPACE:
			if (sdl_mod & (KMOD_CTRL|KMOD_GUI))
			{
				int start, end;
				const char *spaces = " .,!?\n";
				findWordPosition(ed->str, ed->cursor-1, &start, &end, spaces);
				if (start < ed->cursor)
				{
					memmove(ed->str+start, ed->str+ed->cursor, l-ed->cursor+1);
					ed->cursorstart = ed->cursor = start;
					break;
				}
			}
			if (ed->highlightlength)
			{
				memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
				ed->cursor = ed->highlightstart;
			}
			else if (ed->cursor > 0)
			{
				ed->cursor--;
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
				if (ed->cursor > 0 && ed->str[ed->cursor-1] == '\b')
				{
					ed->cursor--;
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
				}
			}
			ed->cursorstart = ed->cursor;
			break;
		default:
			if(sdl_mod & (KMOD_CTRL|KMOD_GUI) && sdl_key=='c')//copy
			{
				if (ed->highlightlength)
				{
					char highlightstr[1024];
					strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
					highlightstr[ed->highlightlength] = 0;
					Engine::Ref().ClipboardPush(highlightstr);
				}
				else if (l)
					Engine::Ref().ClipboardPush(ed->str);
				break;
			}
			else if(sdl_mod & (KMOD_CTRL|KMOD_GUI) && sdl_key=='v')//paste
			{
				std::string paste = Engine::Ref().ClipboardPull();
				if (!paste.length())
					return;
				int pl = paste.length();
				if ((textwidth(str)+textwidth(paste.c_str()) > ed->w-14 && !ed->multiline) || (pl+(int)strlen(ed->str)>ed->limit) || (float)(((textwidth(str)+textwidth(paste.c_str()))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
				{
					break;
				}
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				memmove(ed->str+ed->cursor+pl, ed->str+ed->cursor, l-ed->cursor+1);
				memcpy(ed->str+ed->cursor,paste.c_str(),pl);
				ed->cursor += pl;
				ed->cursorstart = ed->cursor;
				break;
			}
			else if(sdl_mod & (KMOD_CTRL|KMOD_GUI) && sdl_key=='a')//highlight all
			{
				ed->cursorstart = 0;
				ed->cursor = l;
			}
			if ((sdl_mod & (KMOD_CTRL|KMOD_GUI)) && (svf_admin || svf_mod))
			{
				if (ed->cursor > 1 && ed->str[ed->cursor-2] == '\b')
					break;
				if (sdl_key=='w' || sdl_key=='g' || sdl_key=='o' || sdl_key=='r' || sdl_key=='l' || sdl_key=='b' || sdl_key=='t')// || sdl_key=='p')
					ch = sdl_key;
				else
					break;
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				if ((textwidth(str) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				memmove(ed->str+ed->cursor+2, ed->str+ed->cursor, l+2-ed->cursor);
				ed->str[ed->cursor] = '\b';
				ed->str[ed->cursor+1] = ch;
				ed->cursor+=2;
				ed->cursorstart = ed->cursor;
			}
			if (sdl_textinput.length() != 1)
				break;
			for (std::string::value_type c : sdl_textinput)
			{
				if (!(c >= ' ' && c < 127 && l < ed->limit))
					break;
			}
			for (std::string::value_type c : sdl_textinput)
			{
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				ch = c;
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((textwidth(str)+textwidth(ts) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str)+textwidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
				ed->cursorstart = ed->cursor;
			}
			break;
		}
	}
	if (mb && !mbq && ed->focus && mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+(ed->multiline?ed->h:11))
	{
		int clickTime = GetTicks()-ed->lastClick;
		ed->lastClick = GetTicks();
		if (clickTime < 300)
		{
			ed->clickPosition = ed->cursor;
			if (ed->numClicks == 2)
				ed->numClicks = 3;
			else
				ed->numClicks = 2;
		}
		ed->cursorstart = ed->cursor;
	}
	else if (!mb && GetTicks()-ed->lastClick > 300)
		ed->numClicks = 0;
	if (ed->numClicks >= 2)
	{
		int start = 0, end = strlen(ed->str);
		const char *spaces = " .,!?_():;~\n";
		if (ed->numClicks == 3)
			spaces = "\n";
		findWordPosition(ed->str, ed->cursor, &start, &end, spaces);
		if (start < ed->clickPosition)
		{
			ed->cursorstart = start;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = end;
		}
		else
		{
			ed->cursorstart = end;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = start;
		}
	}
}

void ui_label_init(ui_label *label, int x, int y, int w, int h)
{
	label->x = x;
	label->y = y;
	label->w = w;
	label->h = h;
	label->maxHeight = 0;
	strcpy(label->str, "");
	label->focus = 0;
	label->multiline = 1;
	label->cursor = label->cursorstart = 0;
	label->highlightstart = label->highlightlength = 0;
	label->lastClick = label->numClicks = 0;
}

int ui_label_draw(pixel *vid_buf, ui_label *ed)
{
	char highlightstr[1024];
	int ret = 0, heightlimit = ed->maxHeight;

	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}

	if (ed->str[0])
	{
		if (ed->multiline) {
			ret = drawtextwrap(vid_buf, ed->x, ed->y, ed->w-14, heightlimit, ed->str, 255, 255, 255, 185);
			if (ed->maxHeight)
				ed->h = ret;
			if (ed->highlightlength)
			{
				drawhighlightwrap(vid_buf, ed->x, ed->y, ed->w-14, heightlimit, ed->str, ed->highlightstart, ed->highlightlength);
			}
		} else {
			ret = drawtext(vid_buf, ed->x, ed->y, ed->str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &ed->str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlight(vid_buf, ed->x+textwidth(ed->str)-textwidth(&ed->str[ed->highlightstart]), ed->y, highlightstr);
			}
		}
	}
	return ret;
}

void ui_label_process(int mx, int my, int mb, int mbq, ui_label *ed)
{
	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	
	if (mb && (ed->focus || !mbq) && mx>=ed->x && mx<ed->x+ed->w && my>=ed->y-2 && my<ed->y+(ed->multiline?ed->h:11))
	{
		ed->focus = 1;
		ed->cursor = textposxy(ed->str, ed->w-14, mx-ed->x, my-ed->y);
		if (!ed->numClicks)
			ed->numClicks = 1;
	}
	else if (mb && !mbq)
	{
		ed->focus = 0;
		ed->cursor = ed->cursorstart = 0;
	}
	else if (mb && (ed->focus || !mbq))
	{
		if (my <= ed->y-2)
			ed->cursor = 0;
		else if (my >= ed->y+ed->h)
			ed->cursor = strlen(ed->str);
	}

	if (ed->focus && sdl_key)
	{
		int l = strlen(ed->str);
		if (sdl_key == SDLK_HOME && (sdl_mod & KMOD_SHIFT))
		{
			ed->cursorstart = 0;
		}
		else if (sdl_key == SDLK_END && (sdl_mod & KMOD_SHIFT))
		{
			ed->cursorstart = l;
		}
		else if (sdl_key == SDLK_LEFT)
		{
			if (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))
			{
				if (ed->cursor)
					ed->cursor--;
				else if (ed->cursorstart < ed->cursor && ed->cursorstart)
					ed->cursorstart--;
			}
		}
		else if (sdl_key == SDLK_RIGHT)
		{
			if (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))
			{
				if (ed->cursor < l)
					ed->cursor++;
				else if (ed->cursorstart > ed->cursor && ed->cursorstart < l)
					ed->cursorstart++;
			}
		}
		else if(sdl_mod & (KMOD_CTRL|KMOD_GUI) && sdl_key=='c')//copy
		{
			if (ed->highlightlength)
			{
				char highlightstr[1024];
				strncpy(highlightstr, &ed->str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				Engine::Ref().ClipboardPush(highlightstr);
			}
			else if (l)
				Engine::Ref().ClipboardPush(ed->str);
		}
		else if(sdl_mod & (KMOD_CTRL|KMOD_GUI) && sdl_key=='a')//highlight all
		{
			ed->cursorstart = 0;
			ed->cursor = l;
		}
	}
	if (mb && !mbq && ed->focus)
	{
		int clickTime = GetTicks()-ed->lastClick;
		ed->lastClick = GetTicks();
		if (clickTime < 300)
		{
			ed->clickPosition = ed->cursor;
			if (ed->numClicks == 2)
				ed->numClicks = 3;
			else
				ed->numClicks = 2;
		}
		ed->cursorstart = ed->cursor;
	}
	else if (!mb && GetTicks()-ed->lastClick > 300)
		ed->numClicks = 0;
	if (ed->numClicks >= 2)
	{
		int start = 0, end = strlen(ed->str);
		const char *spaces = " .,!?_():;~\n";
		if (ed->numClicks == 3)
			spaces = "\n";
		findWordPosition(ed->str, ed->cursor, &start, &end, spaces);
		if (start < ed->clickPosition)
		{
			ed->cursorstart = start;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = end;
		}
		else
		{
			ed->cursorstart = end;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = start;
		}
	}
}

void ui_list_process(pixel * vid_buf, int mx, int my, int mb, int mbq, ui_list *ed)
{
	int i, ystart, selected = 0;
	if (ed->selected > ed->count || ed->selected < -1)
	{
		ed->selected = -1;
	}
	if (mx > ed->x && mx < ed->x+ed->w && my > ed->y && my < ed->y+ed->h)
	{
		ed->focus = 1;
		if (!mb && mbq)
		{
			ystart = ed->y-(ed->count*8);
			if (ystart < 5)
				ystart = 5;
			while (!sdl_poll() && !selected)
			{
				mbq = mb;
				mb = mouse_get_state(&mx, &my);
				for (i = 0; i < ed->count; i++)
				{
					if (mx > ed->x && mx < ed->x+ed->w && my > (ystart + i*16) && my < (ystart + i * 16) + 16)
					{
						if (!mb && mbq)
						{
							ed->selected = i;
							selected = 1;
						}
						fillrect(vid_buf, ed->x, ystart + i * 16, ed->w, 16, 255, 255, 255, 25);
						drawtext(vid_buf, ed->x + 4, ystart + i * 16 + 5, ed->items[i], 255, 255, 255, 255);
					}
					else
					{
						drawtext(vid_buf, ed->x + 4, ystart + i * 16 + 5, ed->items[i], 192, 192, 192, 255);
					}
					draw_line(vid_buf, ed->x, ystart + i * 16, ed->x+ed->w, ystart + i * 16, 128, 128, 128, XRES+BARSIZE);
				}
				drawrect(vid_buf, ed->x, ystart, ed->w, ed->count*16, 255, 255, 255, 255);
				sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
				clearrect(vid_buf, ed->x-1, ystart-1, ed->w+3, (ed->count*16)+3);

				if (!selected && !mb && mbq)
					break;
				if (sdl_key == SDLK_RETURN)
					break;
				if (sdl_key == SDLK_ESCAPE)
					break;
			}
			
			if (ed->selected!=-1)
				strcpy(ed->str, ed->items[ed->selected]);
		}
	}
	else
	{
		ed->focus = 0;
	}
}

void ui_list_draw(pixel *vid_buf, ui_list *ed)
{
	if(ed->selected > ed->count || ed->selected < -1)
	{
		ed->selected = -1;
	}
	if (ed->focus)
	{
		drawrect(vid_buf, ed->x, ed->y, ed->w, ed->h, 255, 255, 255, 255);
	}
	else
	{
		drawrect(vid_buf, ed->x, ed->y, ed->w, ed->h, 192, 192, 192, 255);
	}
	if(ed->selected!=-1)
	{
		drawtext(vid_buf, ed->x+4, ed->y+5, ed->items[ed->selected], 255, 255, 255, 255);
	}
	else
	{
		drawtext(vid_buf, ed->x+4, ed->y+5, ed->def, 192, 192, 192, 255);
	}
}

void ui_checkbox_draw(pixel *vid_buf, ui_checkbox *ed)
{
	int w = 12;
	if (ed->checked)
	{
		drawtext(vid_buf, ed->x+3, ed->y+3, "\xCF", 128, 128, 128, 255);
	}
	if (ed->focus)
	{
		drawrect(vid_buf, ed->x, ed->y, w, w, 255, 255, 255, 255);
	}
	else
	{
		drawrect(vid_buf, ed->x, ed->y, w, w, 128, 128, 128, 255);
	}
}

void ui_checkbox_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
	int w = 12;

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_radio_draw(pixel *vid_buf, ui_checkbox *ed)
{
	if (ed->checked)
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<10)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 128, 128, 128, 255);
	}
	if (ed->focus)
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<40 && (nx*nx+ny*ny)>28)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 255, 255, 255, 255);
	}
	else
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<40 && (nx*nx+ny*ny)>28)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 128, 128, 128, 255);
	}
}

void ui_radio_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
	int w = 12;

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_copytext_draw(pixel *vid_buf, ui_copytext *ed)
{
	int g = 180, i = 0;
	if (!ed->state)
	{
		if (ed->hover)
			i = 0;
		else
			i = 100;
		g = 255;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Click the box below to copy the save ID")/2), ed->y-12, "Click the box below to copy the save ID", 255, 255, 255, 255-i);
	}
	else
	{
		i = 0;
		if (ed->state == 2)
			g = 230;
		else
			g = 190;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Copied!")/2), ed->y-12, "Copied!", 255, 255, 255, 255-i);
	}

	drawrect(vid_buf, ed->x, ed->y, ed->width, ed->height, g, 255, g, 255-i);
	drawrect(vid_buf, ed->x+1, ed->y+1, ed->width-2, ed->height-2, g, 255, g, 100-i);
	drawtext(vid_buf, ed->x+6, ed->y+5, ed->text, g, 255, g, 230-i);
}

void ui_copytext_process(int mx, int my, int mb, int mbq, ui_copytext *ed)
{
	if (my>=ed->y && my<=ed->y+ed->height && mx>=ed->x && mx<=ed->x+ed->width)
	{
		if (mb && !mbq)
		{
			Engine::Ref().ClipboardPush(ed->text);
			ed->state = 2;
		}
		ed->hover = 1;
	}
	else
		ed->hover = 0;

	if (ed->state == 2 && !(mb && ed->hover))
		ed->state = 1;
}

void ui_richtext_draw(pixel *vid_buf, ui_richtext *ed)
{
	ed->str[511] = 0;
	ed->printstr[511] = 0;
	drawtext(vid_buf, ed->x, ed->y, ed->printstr, 255, 255, 255, 255);
}

int markup_getregion(char *text, char *action, char *data, char *atext){
	int datamarker = 0;
	int terminator = 0;
	int minit;
	if (sregexp(text, "^{a:.*|.*}")==0)
	{
		for (minit=3; text[minit-1] != '|'; minit++)
			datamarker = minit + 1;
		for (minit=datamarker; text[minit-1] != '}'; minit++)
			terminator = minit + 1;
		if (terminator-datamarker-1 > 0 && datamarker-4 > 0)
		{
			strncpy(data, text+3, datamarker-4);
			strncpy(atext, text+datamarker, terminator-datamarker-1);
		}
		else
			return 0;
		*action = text[1];
		return terminator;
	}
	else
	{
		return 0;
	}	
}

void ui_richtext_settext(char *text, ui_richtext *ed)
{
	int pos = 0, action = 0, ppos = 0, ipos = 0;
	memset(ed->printstr, 0, 512);
	memset(ed->str, 0, 512);
	strcpy(ed->str, text);
	//strcpy(ed->printstr, text);
	for(action = 0; action < 6; action++){
		ed->action[action] = 0;	
		memset(ed->actiondata[action], 0, 256);
		memset(ed->actiontext[action], 0, 256);
	}
	action = 0;
	for(pos = 0; pos<512; ){
		if(!ed->str[pos])
			break;
		if(ed->str[pos] == '{'){
			int mulen = 0;
			mulen = markup_getregion(ed->str+pos, &ed->action[action], ed->actiondata[action], ed->actiontext[action]);
			if(mulen){
				ed->regionss[action] = ipos;
				ed->regionsf[action] = ipos + strlen(ed->actiontext[action]);
				//printf("%c, %s, %s [%d, %d]\n", ed->action[action], ed->actiondata[action], ed->actiontext[action], ed->regionss[action], ed->regionsf[action]);
				strcpy(ed->printstr+ppos, ed->actiontext[action]);
				ppos+=strlen(ed->actiontext[action]);
				ipos+=strlen(ed->actiontext[action]);
				pos+=mulen;
				action++;			
			} 
			else
			{
				pos++;			
			}
		} else {
			ed->printstr[ppos] = ed->str[pos];
			ppos++;
			pos++;
			ipos++;
			if(ed->str[pos] == '\b'){
				ipos-=2;			
			}
		}
	}
	ed->printstr[ppos] = 0;
	//printf("%s\n", ed->printstr);
}

void ui_richtext_process(int mx, int my, int mb, int mbq, ui_richtext *ed)
{
	int action = 0;
	int currentpos = 0;
	if(mx>ed->x && mx < ed->x+textwidth(ed->printstr) && my > ed->y && my < ed->y + 10 && mb && !mbq){
		currentpos = textwidthx(ed->printstr, mx-ed->x);
		for(action = 0; action < 6; action++){
			if(currentpos >= ed->regionss[action] && currentpos <= ed->regionsf[action])
			{	
				//Do action
				if(ed->action[action]=='a'){
					//Open link
					Platform::OpenLink(ed->actiondata[action]);
				}
				break;
			}
		}
	}
}

void error_ui(pixel *vid_buf, int err, std::string txt)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	char *msg;

	msg = (char*)malloc(txt.length() + 16);
	if (err)
		sprintf(msg, "%03d %s", err, txt.c_str());
	else
		sprintf(msg, "%s", txt.c_str());
	textheight = textwrapheight(msg, 240);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, 243, 51+textheight);
		drawrect(vid_buf, x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		if (err)
			drawtext(vid_buf, x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
		else
			drawtext(vid_buf, x0+8, y0+8, "Error:", 255, 64, 32, 255);
		drawtextwrap(vid_buf, x0+8, y0+26, 224, 0, msg, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+textheight+37, "Dismiss", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+textheight+32, 240, 16, 192, 192, 192, 255);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq)
		{
			if (mx>=x0 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
				break;
			//else if (mx < x0 || my < y0 || mx > x0+240 || my > y0+textheight+48)
			//	break;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	free(msg);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

typedef struct int_pair
{
	int first, second;
} int_pair;

int int_pair_cmp (const void * a, const void * b)
{
	int_pair *ap = (int_pair*)a;
	int_pair *bp = (int_pair*)b;
	return ( ap->first - bp->first );
}

#include "simulation/Simulation.h"
void element_search_ui(pixel *vid_buf, Tool ** selectedLeft, Tool ** selectedRight)
{
	int windowHeight = 300, windowWidth = 240;
	int x0 = (XRES-windowWidth)/2, y0 = (YRES-windowHeight)/2, b = 1, mx, my;
	volatile int bq; // MSVC will optimize this away, causing the check for hover to never run
	int toolx = 0, tooly = 0, i, xoff, yoff, found;
	int_pair tempInts[PT_NUM];
	int selectedl = -1;
	int selectedr = -1;
	int firstResult = -1, hover = -1;

	ui_edit ed;
	ui_edit_init(&ed, x0+12, y0+30, windowWidth - 20, 14);
	ed.autoCorrect = 0;
	strcpy(ed.def, "[element name]");

	std::string toolTip = "";
	int toolTipAlpha = 0;

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, windowWidth+3, windowHeight+3);
		drawrect(vid_buf, x0, y0, windowWidth, windowHeight, 192, 192, 192, 255);
		
		drawtext(vid_buf, x0+8, y0+8, "\xE6 Element Search", 255, 255, 255, 255);

		drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);
		
		drawrect(vid_buf, ed.x-4, (ed.y-5)+20, ed.w+4, windowHeight-((ed.y-5)), 192, 192, 192, 255);
		xoff = (ed.x-4)+6;
		yoff = ((ed.y-5)+20)+4;
		toolx = 0;
		tooly = 0;
		
		drawtext(vid_buf, xoff+toolx+4, yoff+tooly+3, "Matches:", 255, 255, 255, 180);
		draw_line(vid_buf, xoff+toolx+2, yoff+tooly+14, xoff+toolx+5+(ed.w-16), yoff+tooly+14, 180, 180, 180, XRES+BARSIZE);
		tooly += 17;
		
		//Covert input to lower case
		std::string searchLower = Format::ToLower(ed.str);
		
		firstResult = -1;
		hover = -1;
		for(i = 0; i < PT_NUM; i++)
		{
#ifndef NOMOD
			if (i == PT_EXPL && !explUnlocked)
				continue;
#endif
			std::string nameLower = Format::ToLower(globalSim->elements[i].Name);
			if (nameLower.find(searchLower) != nameLower.npos && globalSim->elements[i].Enabled)
			{
				Tool* foundTool = GetToolFromIdentifier(globalSim->elements[i].Identifier);
				if (!foundTool)
					continue;
				if(firstResult==-1)
					firstResult = i;
				toolx += draw_tool_xy(vid_buf, toolx+xoff, tooly+yoff, foundTool)+5;
				if (!bq && mx>=xoff+toolx-32 && mx<xoff+toolx && my>=yoff+tooly && my<yoff+tooly+15)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
					hover = i;
				}
				else if (i == selectedl || foundTool == *selectedLeft)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
				}
				else if (i==selectedr || foundTool == *selectedRight)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 55, 55, 255, 255);
				}
				if (Favorite::Ref().IsFavorite(foundTool->GetIdentifier()))
					drawtext(vid_buf, xoff+toolx-32, yoff+tooly-1, "\xED", 255, 205, 50, 255);
				if(toolx > ed.w-4)
				{
					tooly += 18;
					toolx = 0;
				}
				if(tooly>windowHeight-((ed.y-5)+20))
					break;;
			}
		}
		
		if(toolx>0)
		{
			toolx = 0;
			tooly += 18;
		}
		
		if(tooly<windowHeight-((ed.y-5)+20))
		{
			drawtext(vid_buf, xoff+toolx+4, yoff+tooly+3, "Related:", 255, 255, 255, 180);
			draw_line(vid_buf, xoff+toolx+2, yoff+tooly+14, xoff+toolx+5+(ed.w-16), yoff+tooly+14, 180, 180, 180, XRES+BARSIZE);
			tooly += 17;
			
			found = 0;
			for(i = 0; i < PT_NUM; i++)
			{
#ifndef NOMOD
				if (i == PT_EXPL && !explUnlocked)
					continue;
#endif
				std::string descLower = Format::ToLower(globalSim->elements[i].Description);
				if (descLower.find(searchLower) != descLower.npos && globalSim->elements[i].Enabled)
				{
					tempInts[found].first = 1;
					tempInts[found++].second = i;
				}
			}
			
			qsort(tempInts, found, sizeof(int_pair), int_pair_cmp);
			
			for(i = 0; i < found; i++)
			{
				Tool* foundTool = GetToolFromIdentifier(globalSim->elements[tempInts[i].second].Identifier);
				if (!foundTool)
					continue;
				if(firstResult==-1)
					firstResult = tempInts[i].second;
				toolx += draw_tool_xy(vid_buf, toolx+xoff, tooly+yoff, foundTool)+5;
				if (!bq && mx>=xoff+toolx-32 && mx<xoff+toolx && my>=yoff+tooly && my<yoff+tooly+15)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
					hover = tempInts[i].second;
				}
				else if (tempInts[i].second == selectedl || foundTool == *selectedLeft)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
				}
				else if (tempInts[i].second == selectedr || foundTool == *selectedRight)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 55, 55, 255, 255);
				}
				if(toolx > ed.w-4)
				{
					tooly += 18;
					toolx = 0;
				}
				if(tooly>windowHeight-((ed.y-5)+18))
					break;
			}
		}
		
		if(b==1 && hover!=-1)
		{
			selectedl = hover;
#ifndef TOUCHUI
			break;
#endif
		}
		if(b==4 && hover!=-1)
		{
			selectedr = hover;
#ifndef TOUCHUI
			break;
#endif
		}
		if (hover != -1)
		{
			toolTip = globalSim->elements[hover].Description;
			if (toolTipAlpha < 120)
				toolTipAlpha += 2;
		}
		else if (toolTipAlpha > 0)
			toolTipAlpha--;
		
		drawtext(vid_buf, x0+5, y0+windowHeight-12, "Dismiss", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+windowHeight-16, windowWidth, 16, 192, 192, 192, 255);
		fillrect(vid_buf, -1, YRES-11, XRES + 1, 12, 0, 0, 0, 255);
		if (toolTipAlpha)
			drawtext(vid_buf, 10, YRES-9, toolTip.c_str(), 255, 255, 255, toolTipAlpha > 51 ? 255 : toolTipAlpha*5);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq)
		{
			if (mx>=x0 && mx<x0+windowWidth && my>=y0+windowHeight-16 && my<=y0+windowHeight)
			{
#ifndef TOUCHUI
				selectedl = -1;
				selectedr = -1;
#endif
				break;
			}
#ifndef TOUCHUI
			else if (mx < x0 || mx > x0+windowWidth || my < y0 || my > y0+windowHeight)
				break;
#endif
		}

		if (sdl_key==SDLK_RETURN)
		{
			if(selectedl==-1)
				selectedl = firstResult;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
#ifndef TOUCHUI
			selectedl = -1;
			selectedr = -1;
#endif
			break;
		}
	}

	if (selectedl != -1)
	{
		if ((sdl_mod & (KMOD_CTRL|KMOD_GUI)) && (sdl_mod & KMOD_SHIFT) && !(sdl_mod & KMOD_ALT))
		{
			Favorite::Ref().AddFavorite(globalSim->elements[selectedl].Identifier);
			save_presets();
		}
		else
		{
			*selectedLeft = GetToolFromIdentifier(globalSim->elements[selectedl].Identifier);
			Favorite::Ref().AddRecent(globalSim->elements[selectedl].Identifier);
		}
		FillMenus();
	}
	if (selectedr != -1)
	{
		if ((sdl_mod & (KMOD_CTRL|KMOD_GUI)) && (sdl_mod & KMOD_SHIFT) && !(sdl_mod & KMOD_ALT))
		{
			if (Favorite::Ref().IsFavorite(globalSim->elements[selectedr].Identifier))
			{
				Favorite::Ref().RemoveFavorite(globalSim->elements[selectedr].Identifier);
				save_presets();
			}
		}
		else
		{
			*selectedRight = GetToolFromIdentifier(globalSim->elements[selectedr].Identifier);
			Favorite::Ref().AddRecent(globalSim->elements[selectedr].Identifier);
		}
		FillMenus();
	}
	
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

char *input_ui(pixel *vid_buf, const char *title, const char *prompt, const char *text, const char *shadow)
{
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;

	ui_edit ed;
	ui_edit_init(&ed, x0+12, y0+50, xsize-20, 14);
	strncpy(ed.def, shadow, 32);
	ed.focus = 0;
	strncpy(ed.str, text, 254);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, xsize+3, ysize+3);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, title, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, prompt, 255, 255, 255, 255);
		
		drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);

		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq)
		{
			if (mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
				break;
			//else if (mx < x0 || my < y0 || mx > x0+xsize || my > y0+ysize)
			//	break;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	return mystrdup(ed.str);
}

void info_ui(pixel *vid_buf, std::string top, std::string txt)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, 243, 63);
		drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top.c_str(), 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, txt.c_str(), 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+49, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

void info_box(pixel *vid_buf, const char *msg)
{
	int w = textwidth(msg)+16;
	int x0=((XRES+BARSIZE)-w)/2,y0=((YRES+MENUSIZE)-24)/2;
	
	clearrect(vid_buf, x0-1, y0-1, w+3, 27);
	drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
#ifndef RENDERER
	sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
#endif
}

void info_box_overlay(pixel *vid_buf, char *msg)
{
	int w = textwidth(msg)+16;
	int x0=(XRES-w)/2,y0=(YRES-24)/2;
	
	clearrect(vid_buf, x0-1, y0-1, w+3, 27);
	drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
}

void copytext_ui(pixel *vid_buf, const char *top, const char *txt, const char *copytxt)
{
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int buttonx = 0;
	int buttony = 0;
	int buttonwidth = 0;
	int buttonheight = 0;
	ui_copytext ed;

	buttonwidth = textwidth(copytxt)+12;
	buttonheight = 10+8;
	buttony = y0+50;
	buttonx = x0+(xsize/2)-(buttonwidth/2);

	ed.x = buttonx;
	ed.y = buttony;
	ed.width = buttonwidth;
	ed.height = buttonheight;
	ed.hover = 0;
	ed.state = 0;
	strcpy(ed.text, copytxt);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, xsize+3, ysize+3);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);

		ui_copytext_draw(vid_buf, &ed);
		ui_copytext_process(mx, my, b, bq, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}
bool confirm_ui(pixel *vid_buf, const char *top, const char *msg, const char *btn)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	bool ret = false;

	textheight = textwrapheight((char*)msg, 224);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, 243, 51+textheight);
		drawrect(vid_buf, x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 255, 216, 32, 255);
		drawtextwrap(vid_buf, x0+8, y0+26, 224, 0, msg, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+textheight+37, "Cancel", 255, 255, 255, 255);
		drawtext(vid_buf, x0+165, y0+textheight+37, btn, 255, 216, 32, 255);
		drawrect(vid_buf, x0, y0+textheight+32, 160, 16, 192, 192, 192, 255);
		drawrect(vid_buf, x0+160, y0+textheight+32, 80, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
		{
			ret = true;
			break;
		}
		
		if (b && !bq)
		{
			if (mx>=x0 && mx<x0+160 && my>=y0+textheight+32 && my<=y0+textheight+48)
				break;
			//else if (mx < x0 || mx > x0+160 || my < y0 || my > y0+textheight+48)
			//	break;
		}

		if (sdl_key==SDLK_RETURN)
		{
			ret = true;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	return ret;
}

bool login_ui(pixel *vid_buf)
{
	int x0=(XRES+BARSIZE-192)/2,y0=(YRES+MENUSIZE-80)/2,b=1,bq,mx,my;
	ui_edit ed1,ed2;
	bool signIn = false;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed1, x0+25, y0+25, 158, 14);
	strcpy(ed1.def, "[user name]");
	ed1.cursor = ed1.cursorstart = strlen(svf_user);
	strcpy(ed1.str, svf_user);
	if (ed1.cursor)
		ed1.focus = 0;
	ed1.autoCorrect = false;

	ui_edit_init(&ed2, x0+25, y0+45, 158, 14);
	strcpy(ed2.def, "[password]");
	ed2.hide = 1;
	if (!ed1.cursor)
		ed2.focus = 0;
	ed2.autoCorrect = false;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
		clearrect(vid_buf, x0+1, y0+1, 191, 79);
		drawtext(vid_buf, x0+8, y0+8, "Server login:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+23, "\x8B", 32, 64, 128, 255);
		drawtext(vid_buf, x0+12, y0+23, "\x8A", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
		drawtext(vid_buf, x0+11, y0+44, "\x8C", 160, 144, 32, 255);
		drawtext(vid_buf, x0+11, y0+44, "\x84", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+40, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed1);
		ui_edit_draw(vid_buf, &ed2);
		if (svf_login)
			drawtext(vid_buf, x0+5, y0+69, "Sign out", 255, 255, 255, 255);
		else
			drawtext(vid_buf, x0+5, y0+69, "Register Online", 255, 255, 255, 255);
		drawtext(vid_buf, x0+187-textwidth("Sign in"), y0+69, "Sign in", 255, 255, 55, 255);
		drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);
		drawrect(vid_buf, x0, y0+64, 96, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed1);
		ui_edit_process(mx, my, b, bq, &ed2);

		if (b && !bq)
		{
			// sign out or "Register Online" button, depending on whether you are logged in
			if (mx>=x0 && mx<x0+96 && my>=y0+64 && my<=y0+80)
			{
				if (svf_login)
					goto fail;
				else
				{
					Platform::OpenLink("https://powdertoy.co.uk/Register.html");
				}
			}
			// sign in
			if (mx>=x0+97 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			{
				signIn = true;
				break;
			}
			// out of bounds, exit without doing anything
			if (mx < x0 || my < y0 || mx > x0+192 || my > y0+80)
				return false;
		}

		if (sdl_key == SDLK_RETURN)
		{
			if (!ed1.focus)
			{
				signIn = true;
				break;
			}
			ed1.focus = 0;
			ed2.focus = 1;
		}
		else if (sdl_key == SDLK_TAB)
		{
			ed2.focus = ed1.focus;
			ed1.focus = !ed1.focus;
		}
		else if (sdl_key == SDLK_ESCAPE)
		{
			if (!ed1.focus && !ed2.focus)
				return false;
			ed1.focus = 0;
			ed2.focus = 0;
		}
	}

	if (!signIn)
		return false;
	if (strchr(ed1.str, '@'))
	{
		error_ui(vid_buf, 0, "Use your Powder Toy account to log in, not your email. If you don't have a Powder Toy account, you can create one at https://powdertoy.co.uk/Register.html");
		return false;
	}

	strncpy(svf_user, ed1.str, 64);

	// new scope because of goto warning
	{
		int status;
		std::string data = Request::SimpleAuth("https://" SERVER "/Login.json", &status, svf_user_id, svf_session_id, {
			{ "name", svf_user },
			{ "pass", ed2.str },
		});
		if (status == 200 && !data.empty())
		{
			cJSON *root, *tmpobj;//, *notificationarray, *notificationobj;
			if ((root = cJSON_Parse(data.c_str())))
			{
				tmpobj = cJSON_GetObjectItem(root, "Status");
				if (tmpobj && tmpobj->type == cJSON_Number && tmpobj->valueint == 1)
				{
					if((tmpobj = cJSON_GetObjectItem(root, "Username")) && tmpobj->type == cJSON_String)
						strncpy(svf_user, tmpobj->valuestring, 64);
					if((tmpobj = cJSON_GetObjectItem(root, "UserID")) && tmpobj->type == cJSON_Number)
						sprintf(svf_user_id, "%i", tmpobj->valueint);
					if((tmpobj = cJSON_GetObjectItem(root, "SessionID")) && tmpobj->type == cJSON_String)
						strncpy(svf_session_id, tmpobj->valuestring, 64);
					if((tmpobj = cJSON_GetObjectItem(root, "SessionKey")) && tmpobj->type == cJSON_String)
						strncpy(svf_session_key, tmpobj->valuestring, 64);
					if((tmpobj = cJSON_GetObjectItem(root, "Elevation")) && tmpobj->type == cJSON_String)
					{
						char * elevation = tmpobj->valuestring;
						if (!strcmp(elevation, "Mod"))
						{
							svf_admin = 0;
							svf_mod = 1;
						}
						else if (!strcmp(elevation, "Admin"))
						{
							svf_admin = 1;
							svf_mod = 0;
						}
						else
						{
							svf_admin = 0;
							svf_mod = 0;
						}
					}
					/*notificationarray = cJSON_GetObjectItem(root, "Notifications");
					notificationobj = cJSON_GetArrayItem(notificationarray, 0);
					while (notificationobj)
					{
						i++;
						if((tmpobj = cJSON_GetObjectItem(notificationarray, "Text")) && tmpobj->type == cJSON_String)
							if (strstr(tmpobj->valuestring, "message"))
								svf_messages++;
						notificationobj = cJSON_GetArrayItem(notificationarray, i);
					}*/
	
					svf_login = 1;
					save_presets();
				}
				else
				{
					tmpobj = cJSON_GetObjectItem(root, "Error");
					if (tmpobj && tmpobj->type == cJSON_String)
					{
						if (strlen(tmpobj->valuestring))
							error_ui(vid_buf, 0, tmpobj->valuestring);
						else
							error_ui(vid_buf, 0, "Unknown error while logging in");
					}
					else
						error_ui(vid_buf, 0, "Could not read Error response");
					goto fail;
				}
			}
			else
			{
				error_ui(vid_buf, 0, "Could not read response");
				goto fail;
			}
		}
		else
		{
			error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
			goto fail;
		}
		return true;
	}

fail:
	strcpy(svf_user, "");
	strcpy(svf_user_id, "");
	strcpy(svf_session_id, "");
	svf_login = 0;
	svf_own = 0;
	svf_admin = 0;
	svf_mod = 0;
	save_presets();
	return false;
}

int stamp_ui(pixel *vid_buf, int *reorder)
{
	int b=1,bq,mx,my,d=-1,i,j,k,x,gx,gy,y,w,h,r=-1,stamp_page=0,per_page=GRID_X*GRID_Y,page_count,numdelete=0,lastdelete;
	char page_info[64];
	// stamp_count-1 to avoid an extra page when there are per_page stamps on each page
	page_count = (stamp_count-1)/per_page+1;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, 0, 0, XRES+BARSIZE, YRES+MENUSIZE);
		k = stamp_page*per_page;//0;
		r = -1;
		d = -1;
		if (!stamps[k].name[0])
			drawtext(vid_buf, (XRES-textwidth("Use 's' to save stamps"))/2, YRES/2-6, "Use 's' to save stamps", 255, 255, 255, 255);
		for (j=0; j<GRID_Y; j++)
			for (i=0; i<GRID_X; i++)
			{
				if (stamps[k].name[0])
				{
					gx = ((XRES/GRID_X)*i) + (XRES/GRID_X-XRES/GRID_S)/2;
					gy = ((((YRES-MENUSIZE+20)+15)/GRID_Y)*j) + ((YRES-MENUSIZE+20)/GRID_Y-(YRES-MENUSIZE+20)/GRID_S+10)/2 + 18;
					x = (XRES*i)/GRID_X + XRES/(GRID_X*2);
					y = (YRES*j)/GRID_Y + YRES/(GRID_Y*2);
					gy -= 20;
					w = stamps[k].thumb_w;
					h = stamps[k].thumb_h;
					x -= w/2;
					y -= h/2;
					if (stamps[k].thumb)
					{
						draw_image(vid_buf, stamps[k].thumb, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h, 255);
						xor_rect(vid_buf, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h);
					}
					else
					{
						drawtext(vid_buf, gx+8, gy+((YRES/GRID_S)/2)-4, "Error loading stamp", 255, 255, 255, 255);
					}
					if ((mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4) || stamps[k].dodelete)
					{
						if (mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4)
							d = k;
						drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x85", 255, 48, 32, 255);
					}
					else
					{
						if (mx>=gx && mx<gx+(XRES/GRID_S) && my>=gy && my<gy+(YRES/GRID_S) && stamps[k].thumb)
						{
							r = k;
							drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 210, 255);
						}
						else
						{
							drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						}
						drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x85", 150, 48, 32, 255);
					}
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(stamps[k].name)/2, gy+YRES/GRID_S+7, stamps[k].name, 192, 192, 192, 255);
					drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x86", 255, 255, 255, 255);
				}
				k++;
			}

		if (numdelete)
		{
			drawrect(vid_buf,(XRES/2)-19,YRES+MENUSIZE-18,37,16,255,255,255,255);
			drawtext(vid_buf, (XRES/2)-14, YRES+MENUSIZE-14, "Delete", 255, 255, 255, 255);
			if (b == 1 && bq == 0 && mx > (XRES/2)-20 && mx < (XRES/2)+19 && my > YRES+MENUSIZE-19 && my < YRES+MENUSIZE-1)
			{
				sprintf(page_info, "%d stamp%s", numdelete, (numdelete == 1)?"":"s");
				if (confirm_ui(vid_buf, "Do you want to delete?", page_info, "Delete"))
					del_stamp(lastdelete);
				for (i=0; i<STAMP_MAX; i++)
					stamps[i].dodelete = 0;
				numdelete = 0;
				d = lastdelete = -1;
			}
		}
		else
		{
			sprintf(page_info, "Page %d of %d", stamp_page+1, page_count);

			drawtext(vid_buf, (XRES/2)-(textwidth(page_info)/2), YRES+MENUSIZE-14, page_info, 255, 255, 255, 255);
		}

		if (stamp_page)
		{
			drawtext(vid_buf, 4, YRES+MENUSIZE-14, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}
		if (stamp_page<page_count-1)
		{
			drawtext(vid_buf, XRES-15, YRES+MENUSIZE-14, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}
		drawtext(vid_buf, XRES-60, YRES+MENUSIZE-14, "Rescan", 255, 255, 255, 255);
		drawrect(vid_buf, XRES-65, YRES+MENUSIZE-18, 40, 16, 255, 255, 255, 255);

		if (b==1&&bq==0&&d!=-1)
		{
			if (sdl_mod & (KMOD_CTRL|KMOD_GUI))
			{
				if (!stamps[d].dodelete)
				{
					stamps[d].dodelete = 1;
					numdelete++;
					lastdelete=d;
				}
				else
				{
					stamps[d].dodelete = 0;
					numdelete--;
				}
			}
			else if (!numdelete && confirm_ui(vid_buf, "Do you want to delete?", stamps[d].name, "Delete"))
			{
				del_stamp(d);
			}
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b==1&&r!=-1)
			break;
		if (b==4&&r!=-1)
		{
			r = -1;
			break;
		}

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel>0)
		{
			if (stamp_page)
			{
				stamp_page --;
			}
			sdl_wheel = 0;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel<0)
		{
			if (stamp_page<page_count-1)
			{
				stamp_page ++;
			}
			sdl_wheel = 0;
		}
		if (b && !bq && mx >= XRES-65 && mx <= XRES-25 && my >= YRES+MENUSIZE-18 && my < YRES+MENUSIZE-2 && confirm_ui(vid_buf, "Rescan stamps?", "Rescanning stamps will find all stamps in your stamps/ directory and overwrite stamps.def", "OK"))
		{
			rescan_stamps();
			page_count = (stamp_count - 1) / per_page + 1;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			r = -1;
			break;
		}
	}
	if (sdl_mod & (KMOD_CTRL|KMOD_GUI))
		*reorder = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	for (i=0; i<STAMP_MAX; i++)
		stamps[i].dodelete = 0;
	return r;
}

void tag_list_ui(pixel *vid_buf)
{
	int y,d,x0=(XRES-192)/2,y0=(YRES-256)/2,b=1,bq,mx,my,vp,vn;
	char *p,*q,s;
	char *tag=NULL;
	const char *op=NULL;
	struct strlist *vote=NULL,*down=NULL;

	ui_edit ed;
	ui_edit_init(&ed, x0+25, y0+221, 158, 14);
	strcpy(ed.def, "[new tag]");
	ed.focus = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	fillrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		op = tag = NULL;

		drawrect(vid_buf, x0, y0, 192, 256, 192, 192, 192, 255);
		clearrect(vid_buf, x0+1, y0+1, 191, 255);
		drawtext(vid_buf, x0+8, y0+8, "Manage tags:    \bgTags are only to \nbe used to improve search results", 255, 255, 255, 255);
		p = svf_tags;
		s = svf_tags[0] ? ' ' : 0;
		y = 36 + y0;
		while (s)
		{
			q = strchr(p, ' ');
			if (!q)
				q = p+strlen(p);
			s = *q;
			*q = 0;
			if (svf_own || svf_admin || svf_mod)
			{
				drawtext(vid_buf, x0+21, y+1, "\x85", 160, 48, 32, 255);
				drawtext(vid_buf, x0+21, y+1, "\x86", 255, 255, 255, 255);
				d = 14;
				if (b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12)
				{
					op = "delete";
					tag = mystrdup(p);
				}
			}
			else
				d = 0;
			if (svf_login)
			{
				vp = strlist_find(&vote, p);
				vn = strlist_find(&down, p);
				if ((!vp && !vn && !svf_own) || svf_admin || svf_mod)
				{
					drawtext(vid_buf, x0+d+20, y-1, "\x88", 32, 144, 32, 255);
					drawtext(vid_buf, x0+d+20, y-1, "\x87", 255, 255, 255, 255);
					if (b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12)
					{
						op = "vote";
						tag = mystrdup(p);
						strlist_add(&vote, p);
					}
					drawtext(vid_buf, x0+d+34, y-1, "\x88", 144, 48, 32, 255);
					drawtext(vid_buf, x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
					if (b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12)
					{
						op = "down";
						tag = mystrdup(p);
						strlist_add(&down, p);
					}
				}
				if (vp)
					drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted!", 48, 192, 48, 255);
				if (vn)
					drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted.", 192, 64, 32, 255);
			}
			drawtext(vid_buf, x0+d+48, y, p, 192, 192, 192, 255);
			*q = s;
			p = q+1;
			y += 16;
		}
		
		drawtext(vid_buf, x0+12, y0+221, "\xEE", 32, 144, 32, 255);
		drawtext(vid_buf, x0+12, y0+221, "\xEF", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+216, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed);
		drawtext(vid_buf, x0+5, y0+245, "Close", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+240, 192, 16, 192, 192, 192, 255);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed);

		if (b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
			break;

		if (op)
		{
			d = execute_tagop(vid_buf, op, tag);
			free(tag);
			op = tag = NULL;
			if (d)
				break;
		}

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232)
		{
			d = execute_tagop(vid_buf, "add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = ed.cursorstart = 0;
			if (d)
				break;
		}

		if (!b && bq && (mx < x0 || my < y0 || mx > x0+192 || my > y0+256))
			break;

		if (sdl_key == SDLK_RETURN)
		{
			if (svf_login)
			{
				if (!ed.focus)
					break;
				d = execute_tagop(vid_buf, "add", ed.str);
				strcpy(ed.str, "");
				ed.cursor = ed.cursorstart = 0;
				if (d)
					break;
			}
			else
			{
				error_ui(vid_buf, 0, "Not Authenticated");
				break;
			}
		}
		if (sdl_key == SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			strcpy(ed.str, "");
			ed.cursor = ed.cursorstart = 0;
			ed.focus = 0;
		}
	}
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	sdl_key = 0;

	strlist_free(&vote);
}

int save_name_ui(pixel *vid_buf)
{
	int x0=(XRES-420)/2,y0=(YRES-78-YRES/4)/2,b=1,bq,mx,my,ths,idtxtwidth,nd=0;
	void *th;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	ui_edit ed;
	ui_edit ed2;
	ui_checkbox cbPublish;
	ui_checkbox cbPaused;
	ui_copytext ctb;

	th = build_thumb(&ths, 0);

#ifndef NOMOD
	bool can_publish = true;
	if (check_save(2,0,0,XRES,YRES,0))
		can_publish = false;
#endif

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed, x0+25, y0+25, 158, 14);
	strcpy(ed.def, "[simulation name]");
	ed.cursor = ed.cursorstart = strlen(svf_name);
	strcpy(ed.str, svf_name);

	ui_edit_init(&ed2, x0+13, y0+45, 170, 115);
	strcpy(ed2.def, "[simulation description]");
	ed2.focus = 0;
	ed2.cursor = ed2.cursorstart = strlen(svf_description);
	ed2.multiline = 1;
	strcpy(ed2.str, svf_description);

	ctb.x = 0;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(svf_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, svf_id);

	cbPublish.x = x0+10;
	cbPublish.y = y0+74+YRES/4;
	cbPublish.focus = 0;
	cbPublish.checked = svf_publish;

	cbPaused.x = x0+110;
	cbPaused.y = y0+74+YRES/4;
	cbPaused.focus = 0;
	cbPaused.checked = sys_pause || framerender;
	
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, (unsigned char*)save_to_server_image, 0, 0, 0.7f);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		drawrect(vid_buf, x0, y0, 420, 110+YRES/4, 192, 192, 192, 255); // rectangle around entire thing
		clearrect(vid_buf, x0+1, y0+1, 419, 109+YRES/4);
		if (strcmp(svf_name, ed.str) || !svf_own)
			drawtext(vid_buf, x0+8, y0+8, "Upload new simulation:", 255, 255, 255, 255);
		else
			drawtext(vid_buf, x0+8, y0+8, "Modify simulation properties:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+9, y0+25, "\x82", 192, 192, 192, 255);
		drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255); //rectangle around title box

		drawrect(vid_buf, x0+8, y0+40, 176, 124, 192, 192, 192, 255); //rectangle around description box

		ui_edit_draw(vid_buf, &ed);
		ui_edit_draw(vid_buf, &ed2);

		drawrect(vid_buf, x0+(205-XRES/3)/2-2+205, y0+40, XRES/3+3, YRES/3+3, 128, 128, 128, 255); //rectangle around thumbnail
		render_thumb(th, ths, 0, vid_buf, x0+(205-XRES/3)/2+205, y0+42, 3);

#ifndef NOMOD
		if (!can_publish)
		{
			drawtext(vid_buf, x0+235, y0+180, "\xE4", 255, 255, 0, 255);
			drawtext(vid_buf, x0+251, y0+182, "Warning: uses mod elements", 192, 192, 192, 255);
		}
#endif

		ui_checkbox_draw(vid_buf, &cbPublish);
		drawtext(vid_buf, cbPublish.x+24, cbPublish.y+3, "Publish?", 192, 192, 192, 255);

		ui_checkbox_draw(vid_buf, &cbPaused);
		drawtext(vid_buf, cbPaused.x+24, cbPaused.y+3, "Paused?", 192, 192, 192, 255);

		drawtext(vid_buf, x0+5, y0+99+YRES/4, "Save simulation", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+94+YRES/4, 192, 16, 192, 192, 192, 255);

		draw_line(vid_buf, x0+192, y0, x0+192, y0+110+YRES/4, 150, 150, 150, XRES+BARSIZE);

		if (svf_id[0])
		{
			//Save ID text and copybox
			idtxtwidth = textwidth("Current save ID: ");
			idtxtwidth += ctb.width;
			ctb.x = textwidth("Current save ID: ")+(XRES+BARSIZE-idtxtwidth)/2;
			drawtext(vid_buf, (XRES+BARSIZE-idtxtwidth)/2, YRES+MENUSIZE-15, "Current save ID: ", 255, 255, 255, 255);

			ui_copytext_draw(vid_buf, &ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, bq, &ed);
		ui_edit_process(mx, my, b, bq, &ed2);
		ui_checkbox_process(mx, my, b, bq, &cbPublish);
		ui_checkbox_process(mx, my, b, bq, &cbPaused);

		if ((b && !bq && mx>=x0 && mx<x0+192 && my>=y0+94+YRES/4 && my<y0+110+YRES/4) || sdl_key==SDLK_RETURN)
		{
			bool cont = true;
			if (cbPublish.checked && strlen(svf_author) && strcmp(svf_user, svf_author))
			{
				std::string message = "This save was created by ";
				message += svf_author;
				message += ", you're about to publish this under your own name. If you haven't been given permission "
				           "by the author to do so, please uncheck the publish box, otherwise continue";
				cont = confirm_ui(vid_buf, "Publish", message.c_str(), "Continue");
			}
			if (cont)
			{
				if (th) free(th);
				if (!ed.str[0])
					return 0;
				nd = strcmp(svf_name, ed.str) || !svf_own;
				strncpy(svf_name, ed.str, 63);
				svf_name[63] = 0;
				strncpy(svf_description, ed2.str, 254);
				strncpy(svf_author, svf_user, 63);
				svf_description[254] = 0;
				if (nd)
				{
					strcpy(svf_id, "");
					strcpy(svf_tags, "");
				}
				svf_open = 1;
				svf_own = 1;
				svf_publish = cbPublish.checked;
				sys_pause = cbPaused.checked;
				svf_filename[0] = 0;
				svf_fileopen = 0;
				free(old_vid);
				return nd+1;
			}
		}
		if (b && !bq && (mx < x0 || my < y0 || mx > x0+420 || my > y0+110+YRES/4)
			&& !(mx >= ctb.x && mx <= ctb.x + ctb.width && my >= ctb.y && my <= ctb.y + ctb.height)
		)
			break;
		if (sdl_key == SDLK_ESCAPE)
		{
			break;
		}
	}
	if (th) free(th);
	free(old_vid);
	return 0;
}

//calls menu_ui_v2 when needed, draws fav. menu on bottom
void old_menu_v2(int active_menu, int x, int y, int b, int bq)
{
	if (active_menu > SC_FAV)
	{
		Tool *over = menu_draw(x, y, b, bq, active_menu);
		if (over)
		{
			int height = (int)(ceil((float)menuSections[active_menu]->tools.size()/16.0f)*18);
			int textY = (((YRES/GetNumMenus())*active_menu)+((YRES/GetNumMenus())/2))-(height/2)+(FONT_H/2)+25+height;
			menu_draw_text(over, textY);
			if (b && !bq)
				menu_select_element(b, over);
		}
	}
	else
	{
		Tool *over = menu_draw(x, y, b, bq, SC_FAV);
		if (over)
		{
			menu_draw_text(over, YRES-9);
			if (b && !bq)
				menu_select_element(b, over);
		}
	}
	int numMenus = GetNumMenus();
	for (int i = 0; i < numMenus; i++)
	{
		if (i < SC_FAV)
			drawchar(vid_buf, menuStartPosition+1, /*(12*i)+2*/((YRES/numMenus)*i)+((YRES/numMenus)/2)+5, menuSections[i]->icon, 255, 255, 255, 255);
	}
	//check mouse position to see if it is on a menu section
	for (int i = 0; i < numMenus; i++)
	{
		if (!b && i < SC_FAV && x>=menuStartPosition+1 && x<XRES+BARSIZE-1 && y>= ((YRES/numMenus)*i)+((YRES/numMenus)/2)+3 && y<((YRES/numMenus)*i)+((YRES/numMenus)/2)+17)
			menu_ui_v2(vid_buf, i); //draw the elements in the current menu if mouse inside
	}
}

//old menu function, with the elements drawn on the side
void menu_ui_v2(pixel *vid_buf, int i)
{
	int b=1, bq, mx, my, y, sy, numMenus = GetNumMenus(), someStrangeYValue = (((YRES/numMenus)*i)+((YRES/numMenus)/2))+3;
	int rows = (int)ceil((float)menuSections[i]->tools.size()/16.0f);
	int height = (int)(ceil((float)menuSections[i]->tools.size()/16.0f)*18);
	int width = (int)restrict_flt(menuSections[i]->tools.size()*31.0f, 0, 16*31);
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!old_vid)
		return;
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
	active_menu = i;
	sy = y = (((YRES/numMenus)*i)+((YRES/numMenus)/2))-(height/2)+(FONT_H/2)+6;
	//wipe out existing toolTips, to prevent weirdness inside here
	for (int j = toolTips.size()-1; j >= 0; j--)
		delete toolTips[j];
	toolTips.clear();

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		DrawToolTips();
		fillrect(vid_buf, menuStartPosition-width-21, y-5, width+16, height+16+rows, 0, 0, 0, 100);
		drawrect(vid_buf, menuStartPosition-width-21, y-5, width+16, height+16+rows, 255, 255, 255, 255);
		fillrect(vid_buf, menuStartPosition-3, someStrangeYValue, 15, FONT_H+4, 0, 0, 0, 100);
		drawrect(vid_buf, menuStartPosition-5, someStrangeYValue, 16, FONT_H+4, 255, 255, 255, 255);
		drawrect(vid_buf, menuStartPosition-5, someStrangeYValue+1, 1, FONT_H+2, 0, 0, 0, 255);
		drawchar(vid_buf, menuStartPosition+1, /*(12*i)+2*/((YRES/numMenus)*i)+((YRES/numMenus)/2)+5, menuSections[i]->icon, 255, 255, 255, 255);
		if (i) //if not in walls
			drawtext(vid_buf, 12, 12, "\bgPress 'o' to return to the original menu", 255, 255, 255, 255);
		
		Tool *over = menu_draw(mx, my, b, bq, active_menu);
		if (over)
		{
			int height = (int)(ceil((float)menuSections[active_menu]->tools.size()/16.0f)*18);
			int textY = (((YRES/GetNumMenus())*active_menu)+((YRES/GetNumMenus())/2))-(height/2)+(FONT_H/2)+25+height;
			menu_draw_text(over, textY);
		}

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (!(mx>=(menuStartPosition-menuIconWidth-width)-4 && my>=sy-5 && my<sy+height+14) || (mx >= menuStartPosition-menuIconWidth+13 && !(my >= someStrangeYValue && my <= someStrangeYValue+FONT_H+4)))
		{
			break;
		}

		if (over)
			menu_select_element(b, over);

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	free(old_vid);

	//wipe out existing toolTips again, to prevent them from persisting once interface closes
	for (int i = toolTips.size()-1; i >= 0; i--)
		delete toolTips[i];
	toolTips.clear();
}

//current menu function
#ifdef TOUCHUI
Tool *originalOver = NULL;
int originalmx = 0, currentScroll = 0;
bool draggingMenu = false;
#endif
void menu_ui_v3(pixel *vid_buf, int i, int b, int bq, int mx, int my)
{
	Tool* over = menu_draw(mx, my, b, bq, i);
	if (over)
	{
		menu_draw_text(over, YRES-9);
#ifdef TOUCHUI
		if (b && !bq)
			originalOver = over;
		else if (!b && bq && over == originalOver)
			menu_select_element(bq, over);
		else if (!b && !bq)
			originalOver = NULL;
#else
		if (b && !bq)
			menu_select_element(b, over);
#endif
	}
#ifdef TOUCHUI
	if (mx>=menuStartPosition && mx<XRES+BARSIZE-1)
		currentScroll = 0;
#endif
}
#ifdef TOUCHUI
void scrollbar(int fwidth, int scroll, int y)
{
	/*int scrollSize = (int)((float)(menuStartPosition-menuIconWidth)/fwidth * (menuStartPosition-menuIconWidth));
	int scrollbarx = (int)((float)scroll*XRES/(XRES-8-fwidth)/menuStartPosition * (scrollSize-menuStartPosition));
	fillrect(vid_buf, scrollbarx+(menuStartPosition-scrollSize), y+19, scrollSize, 3, 200, 200, 200, 255);*/
}

#else
int scrollbar(int fwidth, int mx, int y)
{
	int scrollSize, scrollbarx;
	float overflow, location;
	if (mx > menuStartPosition)
		mx = menuStartPosition;
	//if (mx < 15) //makes scrolling a little nicer at edges but apparently if you put hundreds of elements in a menu it makes the end not show ...
	//	mx = 15;
	scrollSize = (int)(((float)(menuStartPosition-menuIconWidth))/((float)fwidth) * ((float)menuStartPosition-menuIconWidth));
	scrollbarx = (int)(((float)mx/((float)menuStartPosition))*(float)(menuStartPosition-scrollSize));
	if (scrollSize+scrollbarx>menuStartPosition)
		scrollbarx = menuStartPosition-scrollSize;
	fillrect(vid_buf, scrollbarx, y+19, scrollSize, 3, 200, 200, 200, 255);

	overflow = (float)fwidth-menuStartPosition+10;
	location = ((float)menuStartPosition-3)/((float)(mx-(menuStartPosition-2)));
	return (int)(overflow / location);
}
#endif

Tool* menu_draw(int mx, int my, int b, int bq, int i)
{
	int x, y, xoff=0, fwidth = menuSections[i]->tools.size()*31;
	Tool* over = NULL;
	if (!old_menu || i >= SC_FAV)
	{
		x = menuStartPosition-35;
		y = YRES+1;
	}
	else
	{
		int i2 = i;
		int height = (int)(ceil((float)menuSections[i]->tools.size()/16.0f)*18);

		if (i == SC_FAV2 || i == SC_HUD)
			i2 = SC_FAV;
		x = menuStartPosition-39;
		y = (((YRES/GetNumMenus())*i2)+((YRES/GetNumMenus())/2))-(height/2)+(FONT_H/2)+11;
	}

	//draw everything in the decoration editor
	if (i == SC_DECO)
	{
		decoration_editor(vid_buf, b, bq, mx, my);

		int presetx = 6;
		for (int n = 0; n < NUM_COLOR_PRESETS; n++)
		{
			if (mx>=presetx-1 && mx<presetx+27 && my>=y && my< y+15)
			{
				drawrect(vid_buf, presetx-1, y-1, 29, 17, 255, 55, 55, 255);
				over =  GetToolFromIdentifier(colorlist[n].identifier);
			}
			draw_tool_button(vid_buf, presetx, y, PIXPACK(colorlist[n].colour), "");
			presetx += 31;
		}
	}
#ifndef NOMOD
	else if (i == SC_HUD)
	{
		fwidth = 0;
		for (int n = 0; n < HUD_NUM; n++)
		{
			if (hud_menu[n].menunum == hud_menunum || n == 0)
			{
				fwidth += 31;
			}
		}
	}
#endif

	//fancy scrolling
	if ((!old_menu || i >= SC_FAV) && fwidth > menuStartPosition)
	{
#ifdef TOUCHUI
		if (!draggingMenu || (b && !bq))
		{
			originalmx = mx;
			xoff = currentScroll;
			if (b && !bq && my > YRES && mx < menuStartPosition)
				draggingMenu = true;
		}
		else if ((b || bq) && (my > YRES || draggingMenu))
		{
			xoff = currentScroll+originalmx-mx;
			if (xoff < -fwidth+XRES-8)
				xoff = -fwidth+XRES-8;
			if (xoff > 0)
				xoff = 0;
			if (!b)
				currentScroll = xoff;
		}
		else
			xoff = currentScroll;
		scrollbar(fwidth, xoff, y);
#else
		xoff = scrollbar(fwidth, mx, y);
#endif
	}

#ifdef TOUCHUI
	bool overflowsLeft = false;
#endif
	//main loop, draws the tools and figures out which tool you are hovering over / selecting
	for (std::vector<Tool*>::iterator iter = menuSections[i]->tools.begin(), end = menuSections[i]->tools.end(); iter != end; ++iter)
	{
		Tool* current = *iter;
#ifndef NOMOD
		//special cases for fake menu things in my mod
		if (i == SC_FAV || i == SC_FAV2 || i == SC_HUD)
		{
			last_fav_menu = i;
			if (i == SC_HUD && hud_menu[current->GetID()].menunum != hud_menunum && current->GetID() != 0)
				continue;
		}
#endif
		//if it's offscreen to the right or left, draw nothing
		if (x-xoff > menuStartPosition-28 || x-xoff < -26)
		{
			x -= 31;
#ifdef TOUCHUI
			overflowsLeft = true;
#endif
			continue;
		}
		if (old_menu && x <= menuStartPosition-17*31 && i < SC_FAV)
		{
			x = menuStartPosition-39;
			y += 19;
		}
		x -= draw_tool_xy(vid_buf, x-xoff, y, current)+5;

#ifndef NOMOD
		if (i == SC_HUD) //HUD menu is special and should hopefully be removed someday ...
		{
			if (mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
				over = current;
			}
			else if (current->GetID() >= HUD_REALSTART && currentHud[current->GetID()-HUD_REALSTART] && hud_menu[current->GetID()].name.find("#") == hud_menu[current->GetID()].name.npos)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 0, 255);
			}
		}
		else
#endif
		{
			//draw rectangles around selected tools
			if (activeTools[0]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
			else if (activeTools[1]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 55, 55, 255, 255);
			else if (activeTools[2]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);

			//if mouse inside button
			if (mx>=x+32-xoff && mx<x+58-xoff && my>=y && my<y+15)
			{
				over = current;
#ifndef TOUCHUI
				//draw rectangles around hovered on tools
				if ((sdl_mod & KMOD_ALT) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & KMOD_SHIFT) && ((ElementTool*)current)->GetID() >= 0)
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
				else if ((sdl_mod & KMOD_SHIFT) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & KMOD_ALT) && current->GetType() != INVALID_TOOL)
					drawtext(vid_buf, x+30-xoff, y-1, "\xED", 255, 205, 50, 255);
				else
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
#endif
			}
			if (Favorite::Ref().IsFavorite(current->GetIdentifier()))
				drawtext(vid_buf, x+30-xoff, y-1, "\xED", 255, 205, 50, 255);
		}
	}
#ifdef TOUCHUI
	if ((overflowsLeft && x-xoff < 15) || x-xoff < -31)
		for (int i = 0; i <= 15; i++)
		{
			for (int j = -1; j <= 15; j++)
				blendpixel(vid_buf, i, y+j, 0, 0, 0, 255-16*i);
		}
	if (fwidth > menuStartPosition && std::abs(originalmx-mx) > 10)
	{
		originalOver = NULL;
		return NULL;
	}
#endif
	return over;
}

void menu_draw_text(Tool* over, int y)
{
	if (the_game->IsinsideRenderOptions())
		return;
	std::stringstream toolTip;
	int toolID = over->GetID();
	if (over->GetType() == HUD_MENU_BUTTON)
	{
		if (hud_menu[toolID].name.find("#") == hud_menu[toolID].name.npos)
			toolTip << over->GetDescription();
		else
		{
			toolTip << over->GetDescription() << currentHud[toolID-HUD_REALSTART] << " decimal places";
		}
	}
	else if (over->GetType() == FAV_MENU_BUTTON)
	{
		toolTip << over->GetDescription();
		if (toolID == FAV_ROTATE)
		{
			if (globalSim->msRotation)
				toolTip << "on";
			else
				toolTip << "off";
		}
		if (toolID == FAV_HEAT)
		{
			if (!heatmode)
				toolTip << "normal: -273.15C - 9725.85C";
			else if (heatmode == 1)
				toolTip << "automatic: " << lowesttemp-273 << "C - " << highesttemp-273 << "C";
			else
				toolTip << "manual: " << lowesttemp-273 << "C - " << highesttemp-273 << "C";
		}
		else if (toolID == FAV_REAL)
		{
			if (realistic)
				toolTip << "on";
			else
				toolTip << "off";
		}
		else if (toolID == FAV_FIND2)
		{
			if (finding &0x8)
				toolTip << "on";
			else
				toolTip << "off";
		}
		else if (toolID == FAV_DATE)
		{
			char *time;
			converttotime("1300000000", &time, -1, -1, -1);
			toolTip << time;
		}
	}
	else
		toolTip << over->GetDescription();
	if (toolTip.str().size())
		UpdateToolTip(toolTip.str(), Point(XRES - textwidth(toolTip.str().c_str()) - BARSIZE, y), ELEMENTTIP, -1);
}

void menu_select_element(int b, Tool* over)
{
	if (the_game->IsinsideRenderOptions())
		return;
	int toolID = over->GetID();
	if (b==1 && over)
	{
		if (over->GetType() == FAV_MENU_BUTTON)
		{
			if (toolID == FAV_MORE)
				active_menu = SC_FAV2;
			else if (toolID == FAV_BACK)
				active_menu = SC_FAV;
			else if (toolID == FAV_FIND)
			{
				if (finding & 0x4)
					finding &= 0x8;
				else if (finding &	0x2)
					finding |= 0x4;
				else if (finding &	0x1)
					finding |= 0x2;
				else
					finding |= 0x1;
			}
			else if (toolID == FAV_INFO)
				drawinfo = !drawinfo;
			else if (toolID == FAV_ROTATE)
				globalSim->msRotation = !globalSim->msRotation;
			else if (toolID == FAV_HEAT)
				heatmode = (heatmode + 1)%3;
			else if (toolID == FAV_LUA)
#ifdef LUACONSOLE
				ReadLuaCode();
#else
				error_ui(vid_buf, 0, "Lua console not enabled");
#endif
			else if (toolID == FAV_CUSTOMHUD)
				active_menu = SC_HUD;
			else if (toolID == FAV_REAL)
			{
				realistic = !realistic;
				if (realistic)
					globalSim->elements[PT_FIRE].HeatConduct = 1;
				else
					globalSim->elements[PT_FIRE].HeatConduct = 88;
			}
			else if (toolID == FAV_FIND2)
			{
				if (finding & 0x8)
					finding &= ~0x8;
				else
					finding |= 0x8;
			}
			else if (toolID == FAV_DATE)
			{
				if (dateformat == 5)
					dateformat = 12;
				else if (dateformat == 11)
					dateformat = 13;
				else if (dateformat == 12)
					dateformat = 0;
				else if (dateformat >= 13)
					dateformat = 6;
				else
					dateformat = dateformat + 1;
			}
			else if (toolID == FAV_SECR)
			{
				secret_els = !secret_els;
				explUnlocked = true;
				FillMenus();
			}
		}
		else if (over->GetType() == HUD_MENU_BUTTON)
		{
			if (toolID == HUD_BACK)
			{
				if (hud_menunum != 0)
					hud_menunum = 0;
				else
					active_menu = SC_FAV2;
			}
			else if (toolID == HUD_RESET)
			{
				HudDefaults();
				SetCurrentHud();
				save_presets();
			}
			else if (toolID < HUD_REALSTART)
			{
				hud_menunum = toolID;
			}
			else
			{
				char hud_curr[16];
				sprintf(hud_curr,"%i",currentHud[toolID-HUD_REALSTART]);
				if (hud_menu[toolID].name.find("#") != hud_menu[toolID].name.npos)
				{
					int decimals = atoi(input_ui(vid_buf,hud_menu[toolID].name.c_str(),"Enter number of decimal places",hud_curr,""));
					if (decimals <= 10)
						currentHud[toolID-HUD_REALSTART] = decimals;
					else
						error_ui(vid_buf, 0, "# of Decimal places cannot be more than 10");
				}
				else
					currentHud[toolID-HUD_REALSTART] = !currentHud[toolID-HUD_REALSTART];
				if (DEBUG_MODE)
					memcpy(debugHud,currentHud,sizeof(debugHud));
				else
					memcpy(normalHud,currentHud,sizeof(normalHud));
				save_presets();
			}
		}
		else if (over->GetType() == DECO_PRESET)
		{
			ARGBColour newDecoColor = colorlist[((DecoPresetTool*)over)->GetID()].colour;
			if (newDecoColor != decocolor)
				decocolor = newDecoColor;
			else
			{
				if (activeTools[0]->GetIdentifier() != "DEFAULT_DECOR_SET")
				{
					activeTools[0] = GetToolFromIdentifier("DEFAULT_DECOR_SET");
				}
			}
			currR = COLR(decocolor), currG = COLG(decocolor), currB = COLB(decocolor), currA = COLA(decocolor);
			RGB_to_HSV(currR, currG, currB, &currH, &currS, &currV);
		}
		else if ((sdl_mod & (KMOD_ALT)) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & (KMOD_SHIFT)) && ((ElementTool*)over)->GetID() >= 0)
		{
			activeTools[2] = over;
		}
		else if ((sdl_mod & (KMOD_SHIFT)) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & (KMOD_ALT)))
		{
			Favorite::Ref().AddFavorite(over->GetIdentifier());
			FillMenus();
			save_presets();
		}
		else
		{
#ifdef TOUCHUI
			if (activeTools[0] == over && over->GetType() == GOL_TOOL && over->GetID() >= NGOL)
			{
				auto *cgol = static_cast<LIFE_ElementDataContainer&>(*globalSim->elementData[PT_LIFE]).GetCustomGOLByRule(toolID);
				int cgolRule = cgol->rule;
				auto *confirmPrompt = new ConfirmPrompt([cgolRule](bool b) {
					if (b)
					{
						static_cast<LIFE_ElementDataContainer&>(*globalSim->elementData[PT_LIFE]).RemoveCustomGOL(cgolRule);
						FillMenus();
						save_presets();
					}
				}, "Remove custom GOL type", "Do you want to remove " + cgol->nameString + "?");
				Engine::Ref().ShowWindow(confirmPrompt);
			}
#endif
			activeTools[0] = over;
			if (((ToolTool*)over)->GetID() == TOOL_PROP)
				openProp = true;
			if (((ToolTool*)over)->GetID() == TOOL_GOL)
			{
				auto *golWindow = new GolWindow(true);
				Engine::Ref().ShowWindow(golWindow);
			}
			Favorite::Ref().AddRecent(over->GetIdentifier());
			FillMenus();
		}
	}
	if (b==4 && over)
	{
		if (over->GetType() == FAV_MENU_BUTTON)
		{
			if (toolID == FAV_HEAT)
			{
				heatmode = 2;
				lowesttemp = atoi(input_ui(vid_buf,"Manual Heat Display","Enter a Minimum Temperature in Celcius","",""))+273;
				highesttemp = atoi(input_ui(vid_buf,"Manual Heat Display","Enter a Maximum Temperature in Celcius","",""))+273;
			}
			else if (toolID == FAV_DATE)
			{
				if (dateformat == 12)
					dateformat = 13;
				else if (dateformat == 13)
					dateformat = 12;
				else if (dateformat < 6)
					dateformat += 6;
				else
					dateformat -= 6;
			}
		}
		else if (over->GetType() == HUD_MENU_BUTTON)
		{
		}
		else if ((sdl_mod & (KMOD_ALT)) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & (KMOD_SHIFT)) && ((ElementTool*)over)->GetID() >= 0)
		{
			activeTools[2] = over;
		}
		else if ((sdl_mod & (KMOD_SHIFT)) && (sdl_mod & (KMOD_CTRL|KMOD_GUI)) && !(sdl_mod & (KMOD_ALT)))
		{
			bool removed = Favorite::Ref().RemoveFavorite(over->GetIdentifier());
			removed = Favorite::Ref().RemoveRecent(over->GetIdentifier()) || removed;

			if (!removed && over->GetType() == GOL_TOOL && over->GetID() >= NGOL)
			{
				auto *cgol = static_cast<LIFE_ElementDataContainer&>(*globalSim->elementData[PT_LIFE]).GetCustomGOLByRule(toolID);
				int cgolRule = cgol->rule;
				auto *confirmPrompt = new ConfirmPrompt([cgolRule](bool b) {
					if (b)
					{
						static_cast<LIFE_ElementDataContainer&>(*globalSim->elementData[PT_LIFE]).RemoveCustomGOL(cgolRule);
						FillMenus();
						save_presets();
					}
				}, "Remove custom GOL type", "Are you sure you want to remove " + cgol->nameString + "?");
				Engine::Ref().ShowWindow(confirmPrompt);
			}

			FillMenus();
			save_presets();
		}
		else
		{
			activeTools[1] = over;
			if (((ToolTool*)over)->GetID() == TOOL_PROP)
				openProp = true;
			if (((ToolTool*)over)->GetID() == TOOL_GOL)
			{
				auto *golWindow = new GolWindow(false);
				Engine::Ref().ShowWindow(golWindow);
			}
			Favorite::Ref().AddRecent(over->GetIdentifier());
			FillMenus();
		}
	}
}

char tabNames[10][255];
pixel* tabThumbnails[10];
int quickoptionsThumbnailFade = 0;
int clickedQuickoption = -1, hoverQuickoption = -1;
void QuickoptionsMenu(pixel *vid_buf, int b, int bq, int x, int y)
{
	int i = 0;
	bool isQuickoptionClicked = false, switchTabsOverride = false;
	//normal quickoptions
	if (!show_tabs && !(sdl_mod & (KMOD_CTRL|KMOD_GUI)))
	{
		while(quickmenu[i].icon!=NULL)
		{
			if(quickmenu[i].type == QM_TOGGLE)
			{
				drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				if (((i == 3 && globalSim->grav->IsEnabled()) || (i !=3 && *(quickmenu[i].variable))) || clickedQuickoption == i)
				{
					fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
					drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 0, 0, 0, 255);
				}
				else
				{
					fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
					drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 255, 255, 255, 255);
				}
				if(x >= (XRES+BARSIZE)-16 && x <= (XRES+BARSIZE)-2 && y >= (i*16)+1 && y <= (i*16)+15)
				{
					UpdateToolTip(quickmenu[i].name, Point(menuStartPosition-5-textwidth(quickmenu[i].name), (i*16)+6), QTIP, -1);

					if (b == 1 && !bq)
					{
						if (clickedQuickoption == -1)
							clickedQuickoption = i;
						isQuickoptionClicked = true;
					}
					else if (b == 1 || bq == 1)
					{
						if (clickedQuickoption == i)
							isQuickoptionClicked = true;
					}
				}
			}
			i++;
		}
	}
	//tab menu
	else
	{
		while(i < num_tabs + 2 && i < 23-GetNumMenus())
		{
			char num[10];
			sprintf(num, "%d", i);
			drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
			if (i == 0)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 0, 0, 0, 255);
			}
			else if (i == num_tabs + 1)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-12, (i*16)+5, "\xEF", 255, 255, 255, 255);
			}
			else if(tab_num == i)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, num, 0, 0, 0, 255);
			}
			else
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, num, 255, 255, 255, 255);
			}
			if(x >= (XRES+BARSIZE)-16 && x <= (XRES+BARSIZE)-2 && y >= (i*16)+1 && y <= (i*16)+15)
			{
				std::string toolTip;
				if (i == 0)
					toolTip = quickmenu[i].name;
				else if (i == num_tabs + 1)
					toolTip = "Add tab \bg(ctrl+n)";
				else if (tab_num == i)
				{
					if (strlen(svf_name))
						toolTip = svf_name;
					else if (strlen(svf_filename))
						toolTip = svf_filename;
					else
						toolTip = "Untitled Simulation (current)";
				}
				else
					toolTip = tabNames[i-1];
				UpdateToolTip(toolTip, Point(menuStartPosition-5-textwidth(toolTip.c_str()), (i*16)+6), QTIP, -1);

				if (i > 0 && i <= num_tabs && tab_num != i)
				{
					quickoptionsThumbnailFade += 2;
					if (quickoptionsThumbnailFade > 12)
						quickoptionsThumbnailFade = 12;
					hoverQuickoption = i;
				}
				if (b && !bq)
				{
					if (clickedQuickoption == -1)
						clickedQuickoption = i;
					isQuickoptionClicked = true;
				}
				else if (b || bq)
				{
					if (clickedQuickoption == i)
						isQuickoptionClicked = true;
				}
			}
			i++;
		}
	}
	if (sdl_key == SDLK_TAB && (sdl_mod & KMOD_CTRL))
	{
		if (sdl_mod & KMOD_SHIFT)
		{
			if (tab_num > 1)
			{
				clickedQuickoption = tab_num - 1;
				switchTabsOverride = true;
			}
		}
		else if (tab_num < 22-GetNumMenus())
		{
			clickedQuickoption = tab_num + 1;
			switchTabsOverride = true;
		}
	}
	else if (!isQuickoptionClicked)
		clickedQuickoption = -1;
	if ((clickedQuickoption >= 0 && !b && bq) || switchTabsOverride)
	{
		if (!show_tabs && !switchTabsOverride && !(sdl_mod & (KMOD_CTRL|KMOD_GUI)))
		{
			if (bq == 1)
			{
				if (clickedQuickoption == 3)
				{
					if (!globalSim->grav->IsEnabled())
						globalSim->grav->StartAsync();
					else
						globalSim->grav->StopAsync();
				}
				else if (clickedQuickoption == 6)
				{
					the_game->OpenConsole();
				}
				else
					*(quickmenu[clickedQuickoption].variable) = !(*(quickmenu[clickedQuickoption].variable));
			}
		}
		else
		{
			if (bq == 1 || switchTabsOverride)
			{
				//toggle show_tabs
				if (clickedQuickoption == 0)
					*(quickmenu[clickedQuickoption].variable) = !(*(quickmenu[clickedQuickoption].variable));
				//start a new tab
				else if (clickedQuickoption == num_tabs + 1)
				{
					tab_save(tab_num);
					num_tabs++;
					tab_num = num_tabs;
					if (sdl_mod & (KMOD_CTRL|KMOD_GUI))
						NewSim();
					tab_save(tab_num);
				}
				//load a different tab
				else if (tab_num != clickedQuickoption)
				{
					tab_save(tab_num);
					tab_load(clickedQuickoption);
					tab_num = clickedQuickoption;
				}
				//clicked current tab, do nothing
				else
				{
				}
			}
			else if (bq == 4 && clickedQuickoption > 0 && clickedQuickoption <= num_tabs)
			{
				if (num_tabs > 1)
				{
					char name[30], newname[30];
					//delete the tab that was closed, free thumbnail
					sprintf(name, "tabs%s%d.stm", PATH_SEP, clickedQuickoption);
					remove(name);
					free(tabThumbnails[clickedQuickoption-1]);
					tabThumbnails[clickedQuickoption-1] = NULL;

					//rename all the tabs and move other variables around
					for (i = clickedQuickoption; i < num_tabs; i++)
					{
						sprintf(name, "tabs%s%d.stm", PATH_SEP, i+1);
						sprintf(newname, "tabs%s%d.stm", PATH_SEP, i);
						rename(name, newname);
						strncpy(tabNames[i-1], tabNames[i], 254);
						tabThumbnails[i-1] = tabThumbnails[i];
					}

					num_tabs--;
					tabThumbnails[num_tabs] = NULL;
					if (clickedQuickoption == tab_num)
					{
						if (tab_num > 1)
							tab_num --;
						//if you deleted current tab, load the new one
						tab_load(tab_num);
					}
					else if (tab_num > clickedQuickoption && tab_num > 1)
						tab_num--;
				}
				else
				{
					NewSim();
					tab_save(tab_num);
				}
			}
		}
	}

	if (quickoptionsThumbnailFade && tabThumbnails[hoverQuickoption-1])
	{
		drawrect(vid_buf, (XRES+BARSIZE)/3-1, YRES/3-1, (XRES+BARSIZE)/3+2, YRES/3+1, 0, 0, 255, quickoptionsThumbnailFade*21);
		draw_image(vid_buf, tabThumbnails[hoverQuickoption-1], (XRES+BARSIZE)/3, YRES/3, (XRES+BARSIZE)/3+1, YRES/3, quickoptionsThumbnailFade*21);
		quickoptionsThumbnailFade--;
	}
}

struct thumbDownloadInfo
{
	Request *req;
	char *imgId;
	int pos;

	thumbDownloadInfo(Request *req, char *imgId, int pos):
		req(req),
		imgId(imgId),
		pos(pos)
	{}
};

int search_ui(pixel *vid_buf)
{
	int uih=0,nyu,nyd,b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,dap,own,last_own=search_own,last_fav=search_fav,page_count=0,last_page=0,last_date=0,j,w,h,st=0,lv;
	int is_p1=0, exp_res=GRID_X*GRID_Y, tp, last_p1_extra=0, motdswap = rand()%2;
#ifdef TOUCHUI
	const int xOffset = 10;
	int initialOffset = 0;
	bool dragging = false;
#else
	const int xOffset = 0;
	int nmp = -1;
#endif
	int touchOffset = 0;
	bool touchDragged = false; // when true, ignore clicks on saves
	int thumb_drawn[GRID_X*GRID_Y];
	pixel *v_buf = (pixel *)malloc(((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
	pixel *bthumb_rsdata = NULL;
	float ry;
	ui_edit ed;
	ui_richtext motd;
	int searchFailureCode = -1;


	Request *saveListDownload = NULL;
	char *last = NULL;
	int search = 0;
	std::list<thumbDownloadInfo> thumbDownloads;

	if (!v_buf)
		return 0;
	memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

	//memset(img_id, 0, sizeof(img_id));

	memset(search_ids, 0, sizeof(search_ids));
	memset(search_dates, 0, sizeof(search_dates));
	memset(search_names, 0, sizeof(search_names));
	memset(search_scoreup, 0, sizeof(search_scoreup));
	memset(search_scoredown, 0, sizeof(search_scoredown));
	memset(search_publish, 0, sizeof(search_publish));
	memset(search_owners, 0, sizeof(search_owners));
	memset(search_thumbs, 0, sizeof(search_thumbs));
	memset(search_thsizes, 0, sizeof(search_thsizes));

	memset(thumb_drawn, 0, sizeof(thumb_drawn));

	do_open = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed, 65+xOffset, 13, XRES-200, 14);
	strcpy(ed.def, "[search terms]");
	ed.cursor = ed.cursorstart = strlen(search_expr);
	strcpy(ed.str, search_expr);

	motd.x = 20;
	motd.y = 33;
	motd.str[0] = 0;
	motd.printstr[0] = 0;

	sdl_wheel = 0;

	while (!sdl_poll())
	{
		uih = 0;
		bq = b;
		mxq = mx;
		myq = my;
		b = mouse_get_state(&mx, &my);

		if (mx!=mxq || my!=myq || sdl_wheel || b)
			mmt = 0;
		else if (mmt<TIMEOUT)
			mmt++;

		clearrect(vid_buf, 0, 0, XRES+BARSIZE, YRES+MENUSIZE);

		if (touchOffset == 0)
			memcpy(vid_buf, v_buf, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
		else
		{
			for (int y = 0; y < YRES+MENUSIZE; y++)
			{
				if (touchOffset < 0)
				{
					memcpy(vid_buf+y*(XRES+BARSIZE), v_buf+y*(XRES+BARSIZE)-touchOffset, (XRES+BARSIZE+touchOffset)*PIXELSIZE);
				}
				else
				{
					memcpy(vid_buf+y*(XRES+BARSIZE)+touchOffset, v_buf+y*(XRES+BARSIZE), (XRES+BARSIZE-touchOffset)*PIXELSIZE);
				}
			}
		}

		drawtext(vid_buf, 11+xOffset, 13, "Search:", 192, 192, 192, 255);
		if (!saveListDownload || saveListDownload->CheckStarted())
			drawtext(vid_buf, 51+xOffset, 11, "\x8E", 192, 160, 32, 255);
		else
			drawtext(vid_buf, 51+xOffset, 11, "\x8E", 32, 64, 160, 255);
		drawtext(vid_buf, 51+xOffset, 11, "\x8F", 255, 255, 255, 255);
		drawrect(vid_buf, 48+xOffset, 8, XRES-182, 16, 192, 192, 192, 255);

		if (!svf_login || search_fav)
		{
			search_own = 0;
			drawrect(vid_buf, XRES-64+16+xOffset, 8, 56, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x94", 96, 80, 16, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x93", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-46+16+xOffset, 13, "My Own", 128, 128, 128, 255);
		}
		else if (search_own)
		{
			fillrect(vid_buf, XRES-65+16+xOffset, 7, 58, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x94", 192, 160, 64, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x93", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-46+16+xOffset, 13, "My Own", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-64+16+xOffset, 8, 56, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x94", 192, 160, 32, 255);
			drawtext(vid_buf, XRES-61+16+xOffset, 11, "\x93", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-46+16+xOffset, 13, "My Own", 255, 255, 255, 255);
		}

		if(!svf_login)
		{
			search_fav = 0;
			drawrect(vid_buf, XRES-134+xOffset, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130+xOffset, 11, "\xCC", 120, 120, 120, 255);
		}
		else if (search_fav)
		{
			fillrect(vid_buf, XRES-134+xOffset, 7, 18, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-130+xOffset, 11, "\xCC", 192, 160, 64, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-134+xOffset, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130+xOffset, 11, "\xCC", 192, 160, 32, 255);
		}

		if(search_fav)
		{
			search_date = 0;
			drawrect(vid_buf, XRES-129+16+xOffset, 8, 60, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA9", 44, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA8", 32, 44, 32, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA7", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-111+16+xOffset, 13, "By votes", 128, 128, 128, 255);
		}
		else if (search_date)
		{
			fillrect(vid_buf, XRES-130+16+xOffset, 7, 62, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA6", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-111+16+xOffset, 13, "By date", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-129+16+xOffset, 8, 60, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA9", 144, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA8", 32, 144, 32, 255);
			drawtext(vid_buf, XRES-126+16+xOffset, 11, "\xA7", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-111+16+xOffset, 13, "By votes", 255, 255, 255, 255);
		}

		ui_edit_draw(vid_buf, &ed);

		if (page_count)
		{
			char pagecount[17];
			sprintf(pagecount,"Page %i",search_page+1);
			drawtext(vid_buf, (XRES-textwidth(pagecount))/2+xOffset, YRES+MENUSIZE-10, pagecount, 255, 255, 255, 255);
		}

#ifndef TOUCHUI
		if (search_page)
		{
			drawtext(vid_buf, 4+xOffset, YRES+MENUSIZE-16, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1+xOffset, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}
		else if (page_count > exp_res && !(search_own || search_fav || search_date))
		{
			if (p1_extra)
				drawtext(vid_buf, 5+xOffset, YRES+MENUSIZE-15, "\x86", 255, 255, 255, 255);
			else
				drawtext(vid_buf, 5+xOffset, YRES+MENUSIZE-15, "\xEF", 255, 255, 255, 255);
			drawrect(vid_buf, 1+xOffset, YRES+MENUSIZE-20, 15, 15, 255, 255, 255, 255);
		}
		if (page_count > exp_res)
		{
			drawtext(vid_buf, XRES-15+xOffset, YRES+MENUSIZE-16, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18+xOffset, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}

		if ((!b && bq && mx>=1+xOffset && mx<=17+xOffset && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel>0)
		{
			if (search_page)
				search_page--;
			else if (!(search_own || search_fav || search_date) && !sdl_wheel)
				p1_extra = !p1_extra;
			sdl_wheel = 0;
			uih = 1;
		}
		if ((!b && bq && mx>=XRES-18+xOffset && mx<=XRES-1+xOffset && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel<0)
		{
			if (page_count>exp_res)
			{
				search_page ++;
				page_count = exp_res;
			}
			sdl_wheel = 0;
			uih = 1;
		}
#endif

		tp = -1;
		if (is_p1)
		{	
			//Message of the day
			ui_richtext_process(mx, my, b, bq, &motd);
			ui_richtext_draw(vid_buf, &motd);
			//Popular tags
			drawtext(vid_buf, (XRES-textwidth("Popular tags:"))/2+xOffset+touchOffset, 49, "Popular tags:", 255, 192, 64, 255);
			for (gj=0; gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14); gj++)
				for (gi=0; gi<(GRID_X+1); gi++)
				{
					pos = gi+(GRID_X+1)*gj;
					if (pos>TAG_MAX || !tag_names[pos])
						break;
					if (tag_votes[0])
						i = 127+(128*tag_votes[pos])/tag_votes[0];
					else
						i = 192;
					w = textwidth(tag_names[pos]);
					if (w>XRES/(GRID_X+1)-5)
						w = XRES/(GRID_X+1)-5;
					gx = (XRES/(GRID_X+1))*gi+xOffset+touchOffset;
					gy = gj*13 + 62;
					if (mx>=gx && mx<gx+(XRES/((GRID_X+1)+1)) && my>=gy && my<gy+14)
					{
						j = (i*5)/6;
						tp = pos;
					}
					else
						j = i;
					drawtextmax(vid_buf, gx+(XRES/(GRID_X+1)-w)/2, gy, XRES/(GRID_X+1)-5, tag_names[pos], j, j, i, 255);
				}
		}

		mp = dp = -1;
		dap = -1;
		st = 0;
		for (gj=0; gj<GRID_Y; gj++)
			for (gi=0; gi<GRID_X; gi++)
			{
				if (is_p1)
				{
					pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
					if (pos<0)
						break;
				}
				else
					pos = gi+GRID_X*gj;
				if (!search_ids[pos])
					break;
				gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2+xOffset+touchOffset;
				gy = ((((YRES-(MENUSIZE-20))+15)/GRID_Y)*gj) + ((YRES-(MENUSIZE-20))/GRID_Y-(YRES-(MENUSIZE-20))/GRID_S+10)/2 + 18;
				if (textwidth(search_names[pos]) > XRES/GRID_X-10)
				{
					char *tmp = (char*)malloc(strlen(search_names[pos])+4);
					strcpy(tmp, search_names[pos]);
					j = textwidthx(tmp, XRES/GRID_X-15);
					strcpy(tmp+j, "...");
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(tmp)/2, gy+YRES/GRID_S+7, tmp, 192, 192, 192, 255);
					free(tmp);
				}
				else
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(search_names[pos])/2, gy+YRES/GRID_S+7, search_names[pos], 192, 192, 192, 255);
				j = textwidth(search_owners[pos]);
				if (mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
						my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+29)
				{
					st = 1;
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
				}
				else
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
				if (search_thumbs[pos]&&thumb_drawn[pos]==0)
				{
					//render_thumb(search_thumbs[pos], search_thsizes[pos], 1, v_buf, gx, gy, GRID_S);
					int finh, finw;
					pixel *thumb_rsdata = NULL;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[pos], search_thsizes[pos], &finw, &finh);
					if(thumb_imgdata!=NULL){
						thumb_rsdata = resample_img(thumb_imgdata, finw, finh, XRES/GRID_S, YRES/GRID_S);
						draw_image(v_buf, thumb_rsdata, gx-touchOffset, gy, XRES/GRID_S, YRES/GRID_S, 255);
						free(thumb_imgdata);
						free(thumb_rsdata);
					}
					thumb_drawn[pos] = 1;
				}
				own = (svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin || svf_mod));
				if (mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy && my<=gy+YRES/GRID_S+29)
					mp = pos;
				if ((own || search_fav) && mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4)
				{
					mp = -1;
					dp = pos;
				}
				if (!search_dates[pos] && mx>=gx-6 && mx<=gx+4 && my>=gy+YRES/GRID_S-4 && my<=gy+YRES/GRID_S+6)
				{
					mp = -1;
					dap = pos;
				}
				drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, YRES/GRID_S+3, 128, 128, 128, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, 1+(YRES/GRID_S+3)/2, 0, 107, 10, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 6, 1+(YRES/GRID_S+3)/2, 107, 10, 0, 255);

				if (mp==pos && !st)
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
				else
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
				if (own || search_fav)
				{
					if (dp == pos)
						drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x85", 255, 48, 32, 255);
					else
						drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x85", 160, 48, 32, 255);
					drawtext(vid_buf, gx+XRES/GRID_S-3, gy-4, "\x86", 255, 255, 255, 255);
				}
				if (!search_publish[pos])
				{
					drawtext(vid_buf, gx-6, gy-6, "\xCD", 255, 255, 255, 255);
					drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (!search_dates[pos])
				{
					fillrect(vid_buf, gx-5, gy+YRES/GRID_S-3, 7, 8, 255, 255, 255, 255);
					if (dap == pos) {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 200, 100, 80, 255);
					} else {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 160, 70, 50, 255);
					}
					//drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (true)
				{
					char ts[64];
					sprintf(ts+1, "%d", search_votes[pos]);
					ts[0] = (char)0xBB;
					for (j=1; ts[j]; j++)
						ts[j] = (char)0xBC;
					ts[j-1] = (char)0xB9;
					ts[j] = (char)0xBA;
					ts[j+1] = 0;
					w = gx+XRES/GRID_S-2-textwidth(ts);
					h = gy+YRES/GRID_S-11;
					drawtext(vid_buf, w, h, ts, 16, 72, 16, 255);
					for (j=0; ts[j]; j++)
						ts[j] -= 14;
					drawtext(vid_buf, w, h, ts, 192, 192, 192, 255);
					sprintf(ts, "%d", search_votes[pos]);
					for (j=0; ts[j]; j++)
						if (ts[j] != '-')
							ts[j] += 127;
					drawtext(vid_buf, w+3, h, ts, 255, 255, 255, 255);
				}
				if (search_scoreup[pos]>0||search_scoredown[pos]>0)
				{
					lv = (search_scoreup[pos]>search_scoredown[pos]?search_scoreup[pos]:search_scoredown[pos]);

					if (((YRES/GRID_S+3)/2)>lv)
					{
						ry = ((float)((YRES/GRID_S+3)/2)/(float)lv);
						if (lv<8)
						{
							ry =  ry/(8-lv);
						}
						nyu = (int)(search_scoreup[pos]*ry);
						nyd = (int)(search_scoredown[pos]*ry);
					}
					else
					{
						ry = ((float)lv/(float)((YRES/GRID_S+3)/2));
						nyu = (int)(search_scoreup[pos]/ry);
						nyd = (int)(search_scoredown[pos]/ry);
					}


					fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-1+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 57, 187, 57, 255);
					fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 4, nyd, 187, 57, 57, 255);
					//drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 0, 107, 10, 255);
					//drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)+1, 4, nyd, 107, 10, 0, 255);
				}
			}

#ifndef TOUCHUI
		if (mp!=-1 && mmt>=TIMEOUT/5 && !st && my<YRES+MENUSIZE-25)
		{
			gi = mp % GRID_X;
			gj = mp / GRID_X;
			if (is_p1)
				gj += GRID_Y-GRID_P;
			gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2+xOffset+touchOffset;
			gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
			i = w = textwidth(search_names[mp]);
			h = YRES/GRID_Z+30;
			if (w<XRES/GRID_Z) w=XRES/GRID_Z;
			gx += XRES/(GRID_S*2)-w/2;
			gy += YRES/(GRID_S*2)-h/2;
			if (gx<2) gx=2;
			if (gx+w>=XRES-2) gx=XRES-3-w;
			if (gy<32) gy=32;
			if (gy+h>=YRES+(MENUSIZE-2)) gy=YRES+(MENUSIZE-3)-h;
			clearrect(vid_buf, gx-1, gy-2, w+3, h-1);
			drawrect(vid_buf, gx-2, gy-3, w+4, h, 160, 160, 192, 255);
			if (search_thumbs[mp]){
				if(mp != nmp && bthumb_rsdata){
					free(bthumb_rsdata);
					bthumb_rsdata = NULL;
				}
				if(!bthumb_rsdata){
					int finh, finw;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[mp], search_thsizes[mp], &finw, &finh);
					if(thumb_imgdata!=NULL){
						bthumb_rsdata = resample_img(thumb_imgdata, finw, finh, XRES/GRID_Z, YRES/GRID_Z, false);
						free(thumb_imgdata);
					}
				}
				draw_image(vid_buf, bthumb_rsdata, gx+(w-(XRES/GRID_Z))/2, gy, XRES/GRID_Z, YRES/GRID_Z, 255);
				nmp = mp;
			}
			drawtext(vid_buf, gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
			drawtext(vid_buf, gx+(w-textwidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
		}
#endif
		if (saveListDownload && saveListDownload->CheckStarted())
		{
			fillrect(vid_buf, 0, 30, XRES+BARSIZE, YRES+MENUSIZE-30, 0, 0, 0, 150);
			drawtext(vid_buf, (XRES+BARSIZE-textwidth("Loading ..."))/2, (YRES+MENUSIZE)/2, "Loading ...", 255, 255, 255, 255);
		}
		else if (searchFailureCode != -1)
		{
			std::string errorMsg = "\boHTTP Error " + Format::NumberToString<int>(searchFailureCode) + ": " + Request::GetStatusCodeDesc(searchFailureCode);
			fillrect(vid_buf, 0, 30, XRES+BARSIZE, YRES+MENUSIZE-30, 0, 0, 0, 150);
			drawtext(vid_buf, (XRES+BARSIZE-textwidth(errorMsg.c_str()))/2, (YRES+MENUSIZE)/2, errorMsg.c_str(), 255, 255, 255, 255);
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed);

		if (sdl_key==SDLK_RETURN)
		{
			if (search_ids[0] && !search_ids[1])
			{
				bq = 1;
				b = 0;
				mp = 0;
			}
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			strcpy(search_expr, ed.str);
			goto finish;
		}

#ifdef TOUCHUI
		if (dragging)
		{
			if (!b)
			{
				if (touchOffset > (XRES+BARSIZE)/3 && search_page)
				{
					search_page--;
					uih = 1; // not sure what this does
					bq = 0;
				}
				else if (touchOffset < -(XRES+BARSIZE)/3 && page_count>exp_res)
				{
					search_page++;
					page_count = exp_res;
					uih = 1; // not sure what this does
					bq = 0;
				}
				else
				{
					touchOffset = 0;
				}
				dragging = false;
			}
			else
			{
				touchOffset = mx-initialOffset;
				if (touchOffset > 0 && !search_page)
				{
					touchOffset = 0;
					touchDragged = true;
				}
				else if (touchOffset < 0 && page_count<=exp_res)
				{
					touchOffset = 0;
					touchDragged = true;
				}
				else if (std::abs(touchOffset) > 20)
					touchDragged = true;
			}
		}
		// Delay a frame before setting this to false
		 if (touchDragged && !bq)
			touchDragged = false;
		if (b && !bq && my >= 30 && do_open == 0)
		{
			dragging = true;
			initialOffset = mx;
		}
#endif
		if (!b && bq && !touchDragged)
		{
			if (mx>=XRES-64+16+xOffset && mx<=XRES-8+16+xOffset && my>=8 && my<=24 && svf_login && !search_fav)
			{
				search_own = !search_own;
			}
			else if (mx>=XRES-129+16+xOffset && mx<=XRES-65+16+xOffset && my>=8 && my<=24 && !search_fav)
			{
				search_date = !search_date;
			}
			else if (mx>=XRES-134+xOffset && mx<=XRES-134+16+xOffset && my>=8 && my<=24 && svf_login)
			{
				search_fav = !search_fav;
				search_own = 0;
				search_date = 0;
			}
			else if (dp!=-1)
			{
				if (search_fav){
					if(confirm_ui(vid_buf, "Remove from favorites?", search_names[dp], "Remove")){
						execute_unfav(vid_buf, search_ids[dp]);
						if (last)
						{
							free(last);
							last = NULL;
						}
					}
				} else {
					if (confirm_ui(vid_buf, "Do you want to delete?", search_names[dp], "Delete"))
					{
						execute_delete(vid_buf, search_ids[dp]);
						if (last)
						{
							free(last);
							last = NULL;
						}
					}
				}
			}
			else if (dap!=-1)
			{
				sprintf(ed.str, "history:%s", search_ids[dap]);
			}
			else if (tp!=-1)
			{
				strncpy(ed.str, tag_names[tp], 255);
			}
			else if (mp!=-1 && st)
			{
				sprintf(ed.str, "user:%s", search_owners[mp]);
			}
			else if ((mp!=-1 && !st && !uih) || do_open==1)
			{
				strcpy(search_expr, ed.str);
				if (open_ui(vid_buf, search_ids[mp], search_dates[mp]?search_dates[mp]:NULL, sdl_mod&(KMOD_CTRL|KMOD_GUI)) || do_open==1) {
					goto finish;
				}
			}
		}

		if (do_open == 1)
		{
			mp = 0;
		}

		if (!last)
		{
			search = 1;
		}
		else if ((strcmp(last, ed.str) && (!strcmp(ed.str, "") || strlen(ed.str) > 3)) || last_own!=search_own || last_date!=search_date || last_page!=search_page || last_fav!=search_fav || last_p1_extra!=p1_extra)
		{
			search = 1;
			if ((strcmp(last, ed.str) && (strcmp(ed.str, "") || strlen(ed.str) > 3)) || last_own!=search_own || last_fav!=search_fav || last_date!=search_date)
			{
				search_page = 0;
				page_count = 0;
			}
			free(last);
			last = NULL;
		}
		else
			search = 0;

		if (search && (!saveListDownload || !saveListDownload->CheckStarted()))
		{
			std::stringstream uri;
			int start, count;
			last = mystrdup(ed.str);
			last_own = search_own;
			last_date = search_date;
			last_page = search_page;
			last_fav = search_fav;
			last_p1_extra = p1_extra;

			bool byvotes = !search_own && !search_date && !search_fav && !*last;
			if (byvotes)
			{
				if (search_page)
				{
					start = (search_page-1)*GRID_X*GRID_Y + GRID_X*GRID_P;
					count = GRID_X*GRID_Y;
				}
				else
				{
					start = 0;
					count = p1_extra ? GRID_X*GRID_Y : GRID_X*GRID_P;
				}
			}
			else
			{
				start = search_page*GRID_X*GRID_Y;
				count = GRID_X*GRID_Y;
			}
			exp_res = count; //-1 so that it can know if there is an extra save and show the next page button
			uri << SCHEME << SERVER << "/Search.api?Start=" << start << "&Count=" << count+1 << "&ShowVotes=true";
			if (byvotes)
				uri << "&t=" << ((GRID_Y-GRID_P)*YRES)/(GRID_Y*14)*GRID_X; //what does this even mean? ...
			uri << "&Query=" << Format::URLEncode(last);
			if (search_own)
				uri << Format::URLEncode(" user:") + Format::URLEncode(svf_user);
			if (search_fav)
				uri << Format::URLEncode(" cat:favs");
			if (search_date)
				uri << Format::URLEncode(" sort:date");

			if (saveListDownload)
				saveListDownload->Cancel();
			saveListDownload = new Request(uri.str());
			if (svf_login)
				saveListDownload->AuthHeaders(svf_user_id, svf_session_id);
			saveListDownload->Start();
			search = 0;
		}

		if (saveListDownload && saveListDownload->CheckStarted() && saveListDownload->CheckDone())
		{
			int status;
			std::string resultsStr = saveListDownload->Finish(&status);
			const char *results = resultsStr.c_str();
			is_p1 = (exp_res < GRID_X*GRID_Y);
			touchOffset = 0;
			if (status == 200)
			{
				page_count = search_results((char*)results, true);
				memset(thumb_drawn, 0, sizeof(thumb_drawn));
				memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
#ifndef TOUCHUI
				nmp = -1;
#endif
				
				if (is_p1)
				{
					if (motdswap)
						sprintf(server_motd,"Links: \bt{a:https://powdertoy.co.uk|Powder Toy main page}\bg, \bt{a:https://powdertoy.co.uk/Discussions/Categories/Index.html|Forums}\bg, \bt{a:https://github.com/ThePowderToy/The-Powder-Toy|Official TPT github}\bg, \bt{a:https://github.com/jacob1/The-Powder-Toy/tree/c++|Jacob1's Mod github}");
					motdswap = !motdswap;
				}
				ui_richtext_settext(server_motd, &motd);
				motd.x = (XRES-textwidth(motd.printstr))/2;
				searchFailureCode = -1;
			}
			else
				searchFailureCode = status;
			for (auto requestPair : thumbDownloads)
			{
				free(requestPair.imgId);
				requestPair.req->Cancel();
			}
			thumbDownloads.clear();
			for (pos=0; pos<GRID_X*GRID_Y; pos++)
			{
				std::stringstream uri;
				char *imgID = nullptr;
				if (search_dates[pos])
				{
					std::stringstream tempID;
					tempID << search_ids[pos] << "_" << search_dates[pos];
					imgID = mystrdup(tempID.str().c_str());
					uri << STATICSCHEME << STATICSERVER << "/" << search_ids[pos] << "_" << search_dates[pos] << "_small.pti";
				}
				else if (search_ids[pos])
				{
					imgID = mystrdup(search_ids[pos]);
					uri << STATICSCHEME STATICSERVER "/" << search_ids[pos] << "_small.pti";
				}
				if (imgID && !thumb_cache_find(imgID, search_thumbs + pos, search_thsizes + pos))
				{
					Request *req = new Request(uri.str());
					req->Start();
					thumbDownloads.push_back(thumbDownloadInfo(req, imgID, pos));
				}
				saveListDownload = nullptr;
			}
		}

		for (auto iter = thumbDownloads.begin(), end = thumbDownloads.end(); iter != end;)
		{
			if (iter->req->CheckDone())
			{
				int status;
				std::string thumbStr = iter->req->Finish(&status);
				char *thumb = (char*)malloc(thumbStr.size() + 1);// mystrdup(thumbStr.c_str());
				memcpy(thumb, thumbStr.c_str(), thumbStr.size());
				int len = thumbStr.size();
				if (thumb && status == 200)
				{
					thumb_cache_add(iter->imgId, thumb, len);
					int pos = iter->pos;
					search_thumbs[pos] = thumb;
					search_thsizes[pos] = len;
				}
				free(iter->imgId);
				iter = thumbDownloads.erase(iter);
			}
			else
				++iter;
		}
	}

	strcpy(search_expr, ed.str);
finish:
	if (last)
		free(last);
	if (saveListDownload)
		saveListDownload->Cancel();
	for (auto requestPair : thumbDownloads)
	{
		free(requestPair.imgId);
		requestPair.req->Cancel();
	}
			
	if(bthumb_rsdata){
		free(bthumb_rsdata);
		bthumb_rsdata = NULL;
	}

	search_results((char*)"", 0);

	free(v_buf);
	return 0;
}

int report_ui(pixel* vid_buf, char *save_id, bool bug)
{
	int b=1,bq,mx,my,messageHeight;
	ui_edit ed;
	const char *message;
	if (bug)
		message = "Report bugs and feedback here. Do not suggest new elements or features, or report bugs with downloaded scripts.";
	else
		message = "Things to consider when reporting:\n"\
				  "\bw1) \bgWhen reporting stolen saves, please include the ID of the original save.\n"
				  "\bw2) \bgDo not ask for saves to be removed from front page unless they break the rules.\n"
				  "\bw3) \bgYou may report saves for comments and tags too (including your own saves)";
	messageHeight = (int)(textwrapheight((char*)message, XRES+BARSIZE-410)/2);

	ui_edit_init(&ed, 209, 159+messageHeight, (XRES+BARSIZE-400)-18, (YRES+MENUSIZE-300)-36);
	if (bug)
		strcpy(ed.def, "Feedback");
	else
		strcpy(ed.def, "Report details");
	ed.focus = 0;
	ed.multiline = 1;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll())
	{
		fillrect(vid_buf, 200, 150-messageHeight, (XRES+BARSIZE-400), (YRES+MENUSIZE-300)+messageHeight*2, 0,0,0, 255);
		drawrect(vid_buf, 200, 150-messageHeight, (XRES+BARSIZE-400), (YRES+MENUSIZE-300)+messageHeight*2, 255, 255, 255, 255);

		drawtextwrap(vid_buf, 205, 155-messageHeight, (XRES+BARSIZE-410), 0, message, 255, 255, 255, 255);

		drawrect(vid_buf, 205, 155+messageHeight, (XRES+BARSIZE-400)-10, (YRES+MENUSIZE-300)-28, 255, 255, 255, 170);

		bq = b;
		b = mouse_get_state(&mx, &my);


		drawrect(vid_buf, 200, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 255);
		drawtext(vid_buf, 213, (YRES+MENUSIZE-150)-13+messageHeight, "Cancel", 255, 255, 255, 255);

		drawrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 255);
		if (bug)
			drawtext(vid_buf, (XRES+BARSIZE-400)+163, (YRES+MENUSIZE-150)-13+messageHeight, "Send", 255, 255, 255, 255);
		else
			drawtext(vid_buf, (XRES+BARSIZE-400)+163, (YRES+MENUSIZE-150)-13+messageHeight, "Report", 255, 255, 255, 255);
		if (mx>(XRES+BARSIZE-400)+150 && my>(YRES+MENUSIZE-150)-18+messageHeight && mx<(XRES+BARSIZE-400)+200 && my<(YRES+MENUSIZE-150)+messageHeight)
		{
			if (b)
			{
				fillrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 100);
			}
			else if (!b && bq == 1)
			{
				int ret = 0;
				if (bug)
					ret = execute_bug(vid_buf, ed.str);
				else
					ret = execute_report(vid_buf, save_id, ed.str);
				if (ret)
				{
					if (bug)
						info_ui(vid_buf, "Success", "Feedback has been sent");
					else
						info_ui(vid_buf, "Success", "This save has been reported");
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
				fillrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 40);
		}
		if (mx>200 && my>(YRES+MENUSIZE-150)-18+messageHeight && mx<250 && my<(YRES+MENUSIZE-150)+messageHeight)
		{
			fillrect(vid_buf, 200, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 40);
			if (!b && bq)
				return 0;
			else if (b)
				fillrect(vid_buf, 200, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 100);
			else
				fillrect(vid_buf, 200, (YRES+MENUSIZE-150)-18+messageHeight, 50, 18, 255, 255, 255, 40);
		}
		ui_edit_draw(vid_buf, &ed);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		ui_edit_process(mx, my, b, bq, &ed);

		if (b && !bq && (mx < 200 || my < 150-messageHeight || mx > (XRES+BARSIZE-200) || my > (YRES+MENUSIZE-150)+messageHeight))
			return 0;
		if (sdl_key == SDLK_ESCAPE)
		{
			return 0;
		}
	}
	return 0;
}

void converttotime(const char *timestamp, char **timestring, int show_day, int show_year, int show_time)
{
	int curr_tm_year, curr_tm_yday;
	char *tempstring = (char*)calloc(63,sizeof(char*));
	struct tm * stamptime, * currtime;
	time_t stamptime2 = atoi(timestamp), currtime2 = time(NULL);
	currtime = localtime(&currtime2);
	curr_tm_year = currtime->tm_year; curr_tm_yday = currtime->tm_yday;
	stamptime = localtime(&stamptime2);
	*timestring = (char*)calloc(63,sizeof(char*));

	if (dateformat >= 12 && (show_day || show_year))
	{
		sprintf(*timestring, "%.4i-%.2i-%.2i", stamptime->tm_year+1900, stamptime->tm_mon+1, stamptime->tm_mday);
	}
	else
	{
		if (show_day == 1 || (show_day != 0 && (stamptime->tm_yday != curr_tm_yday || stamptime->tm_year != curr_tm_year))) //Different day or year, show date
		{
			if (dateformat%6 < 3) //Show weekday
			{
				sprintf(tempstring, "%s", asctime(stamptime));
				tempstring[4] = 0;
				strappend(*timestring,tempstring);
			}
			if (dateformat%3 == 1) // MM/DD/YY
			{
				sprintf(tempstring,"%i/%i",stamptime->tm_mon+1,stamptime->tm_mday);

			}
			else if (dateformat%3 == 2) // DD/MM/YY
			{
				sprintf(tempstring,"%i/%i",stamptime->tm_mday,stamptime->tm_mon+1);
			}
			else //Ex. Sun Jul 4
			{
				//sprintf(tempstring,asctime(stamptime));
				//tempstring[7] = 0;
				strncpy(tempstring,asctime(stamptime)+4,4);
				strappend(*timestring,tempstring);
				sprintf(tempstring,"%i",stamptime->tm_mday);
			}
			strappend(*timestring,tempstring);
		}
		if (show_year == 1 || (show_year != 0 && stamptime->tm_year != curr_tm_year)) //Show year
		{
			if (dateformat%3 != 0)
			{
				sprintf(tempstring,"/%i",(stamptime->tm_year+1900)%100);
				strappend(*timestring,tempstring);
			}
			else
			{
				sprintf(tempstring," %i",stamptime->tm_year+1900);
				strappend(*timestring,tempstring);
			}
		}
	}
	if (show_time == 1 || (show_time != 0 && (dateformat < 6 || dateformat == 12 || (stamptime->tm_yday == curr_tm_yday && stamptime->tm_year == curr_tm_year)))) //Show time
	{
		int hour = stamptime->tm_hour%12;
		const char *ampm = "AM";
		if (stamptime->tm_hour > 11)
			ampm = "PM";
		if (hour == 0)
			hour = 12;
		sprintf(tempstring,"%s%i:%.2i:%.2i %s",strlen(*timestring)==0?"":" ",hour,stamptime->tm_min,stamptime->tm_sec,ampm);
		strappend(*timestring,tempstring);
	}
	free(tempstring);
	//strncpy(*timestring, asctime(stamptime), 63);
}

std::set<std::string> swearWords;
bool swearInitialized = false;
void InitializeSwearing()
{
	if (swearInitialized)
		return;
	swearWords.insert("fuck");
	swearWords.insert("shit ");
	swearWords.insert("asshole");
	swearWords.insert("dick");
	swearWords.insert("cunt");
	swearWords.insert(" nigger");
	swearWords.insert("faggot");
	swearWords.insert("dumbass");
	swearInitialized = true;
}
bool CheckSwearing(std::string text)
{
	for (std::set<std::string>::iterator iter = swearWords.begin(), end = swearWords.end(); iter != end; iter++)
	{
		if (text.find(*iter) != text.npos)
			return true;
	}
	return false;
}

int scrollSpeed = 15;
int scrollSpeedMomentum = 2;
float scrollDeceleration = 0.98f;

int open_ui(pixel *vid_buf, char *save_id, char *save_date, int instant_open)
{
	int b=1,bq,mx,my,cc=0,ccy=0,cix=0;
	int hasdrawninfo=0,hasdrawncthumb=0,hasdrawnthumb=0,authoritah=0,myown=0,queue_open=0,retval=0,bc=255,openable=1;
	int comment_scroll = 0, comment_page = 0, disable_scrolling = 0;
	float comment_scroll_float = 0.0f;
	Save *save = nullptr;
#ifdef TOUCHUI
	int lastY = 0;
	bool scrolling = false;
#else
	int dofocus = 0;
	bool clickOutOfBounds = false;
#endif
	int nyd,nyu,lv;
	float ryf, scroll_velocity = 0.0f;

	save_info *info = (save_info*)calloc(sizeof(save_info), 1);
	int lasttime = TIMEOUT, saveTotal, saveDone, infoTotal, infoDone, downloadDone, downloadTotal;
	int info_ready = 0, data_ready = 0, thumb_data_ready = 0;
	pixel *save_pic = NULL;
	pixel *save_pic_thumb = NULL;
	char *thumb_data = NULL;
	char viewcountbuffer[11];
	int thumb_data_size = 0;
	ui_edit ed;
	ui_copytext ctb;

	const char *profileToOpen = "";
	bool fake404save = false;

	std::string commentWarning;
	bool commentWarningShown = false;
	size_t commentLen = 0; // used to not recheck comment for warnings
	InitializeSwearing();

	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!old_vid || !info)
		return 0;
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	viewcountbuffer[0] = 0;

	fillrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 0, 0, 0, 255);
	drawrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 255, 255, 255, 255);
	drawrect(vid_buf, 50, 50, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 155);
	drawrect(vid_buf, 50+(XRES/2)+1, 50, XRES+BARSIZE-100-((XRES/2)+1), YRES+MENUSIZE-100, 155, 155, 155, 255);
	drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);

	ui_edit_init(&ed, 57+(XRES/2)+1, YRES+MENUSIZE-83, XRES+BARSIZE-114-((XRES/2)+1), 14);
	strcpy(ed.def, "Add comment");
#ifndef TOUCHUI
	ed.focus = svf_login?1:0;
#endif
	ed.multiline = 1;
	ed.resizable = 1;
	ed.limit = 1023;

	ctb.x = 100;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(save_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, save_id);

	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	//Try to load the thumbnail from the cache
	bool thumbFound;
	if(save_date)
	{
		char * id_d_temp = (char*)malloc(strlen(save_id)+strlen(save_date)+2);
		strcpy(id_d_temp, save_id);
		strappend(id_d_temp, "_");
		strappend(id_d_temp, save_date);
		
		thumbFound = thumb_cache_find(id_d_temp, (void**)(&thumb_data), &thumb_data_size);
		free(id_d_temp);
	}
	else
	{
		thumbFound = thumb_cache_find(save_id, (void**)(&thumb_data), &thumb_data_size);
	}
	if (!thumbFound)
	{
		thumb_data = NULL;	
	}
	else
	{
		//We found a thumbnail in the cache, we'll draw this one while we wait for the full image to load.
		int finw, finh;
		pixel *thumb_imgdata = ptif_unpack(thumb_data, thumb_data_size, &finw, &finh);
		if(thumb_imgdata!=NULL){
			save_pic_thumb = resample_img(thumb_imgdata, finw, finh, XRES/2, YRES/2, false);
			//draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);	
		}
		free(thumb_imgdata);
		//rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
	}

	Request *saveDataDownload, *saveInfoDownload, *thumbnailDownload = NULL, *commentsDownload = NULL;
	//Begin Async loading of data
	if (save_date)
	{
		// We're loading a historical save
		std::stringstream uri;
		uri << STATICSCHEME << STATICSERVER << "/" << save_id << "_" << save_date << ".cps";
		saveDataDownload = new Request(uri.str());

		uri.str("");
		uri << STATICSCHEME << STATICSERVER << "/" << save_id << "_" << save_date << ".info";
		saveInfoDownload = new Request(uri.str());

		if (!instant_open)
		{
			uri.str("");
			uri << STATICSCHEME << STATICSERVER << "/" << save_id << "_" << save_date << "_large.pti";
			thumbnailDownload = new Request(uri.str());
		}
	}
	else
	{
		//We're loading a normal save
		std::stringstream uri;
		uri << STATICSCHEME << STATICSERVER << "/" << save_id << ".cps";
		saveDataDownload = new Request(uri.str());

		uri.str("");
		uri << STATICSCHEME << STATICSERVER << "/" << save_id << ".info";
		saveInfoDownload = new Request(uri.str());

		if (!instant_open)
		{
			uri.str("");
			uri << STATICSCHEME << STATICSERVER << "/" << save_id << "_large.pti";
			thumbnailDownload = new Request(uri.str());
		}
	}
	//authenticate requests
	if (svf_login)
	{
		saveDataDownload->AuthHeaders(svf_user_id, svf_session_id);
		saveInfoDownload->AuthHeaders(svf_user_id, svf_session_id);
	}
	saveDataDownload->Start();
	saveInfoDownload->Start();
	if (!instant_open)
	{
		std::stringstream uri;
		uri << SCHEME << SERVER << "/Browse/Comments.json?ID=" << save_id << "&Start=0&Count=20";
		commentsDownload = new Request(uri.str());
		commentsDownload->Start();

		thumbnailDownload->Start();
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		if (saveDataDownload)
		{
			if (saveDataDownload->CheckDone())
			{
				int imgh, imgw, status;
				std::string data = saveDataDownload->Finish(&status);
				saveDataDownload = nullptr;
				saveDone = saveTotal = data.length();
				if (status == 200)
				{
					pixel *full_save;
					if (data.empty())
					{
						error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
						break;
					}
					full_save = prerender_save((char*)data.c_str(), data.length(), &imgw, &imgh);
					if (full_save)
					{
						//save_pic = rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
						data_ready = 1;
						free(full_save);
					}
					else
					{
						error_ui(vid_buf, 0, "Save may be from a newer version");
						break;
					}
				}
				save = new Save(data.c_str(), data.length());
			}
			else
				saveDataDownload->CheckProgress(&saveTotal, &saveDone);
		}
		if (saveInfoDownload)
		{
			if (saveInfoDownload->CheckDone())
			{
				int status;
				std::string info_data = saveInfoDownload->Finish(&status);
				saveInfoDownload = nullptr;
				infoDone = info_data.length();
				if (status == 200 || !infoDone)
				{
					info_ready = info_parse(info_data.c_str(), info);
					sprintf(viewcountbuffer, "%d", info->downloadcount);
					if (info_ready <= 0)
					{
						error_ui(vid_buf, 0, "Save info not found");
						break;
					}
					else if (!fake404save && !strlen(info->author))
					{
						if (info->title) free(info->title);
						if (info->name) free(info->name);
						if (info->author) free(info->author);
						if (info->date) free(info->date);
						if (info->description) free(info->description);
						if (info->tags) free(info->tags);
						info->title = mystrdup("Save doesn't exist");
						info->name = mystrdup("Save doesn't exist");
						info->author = mystrdup("FourOhFour");
						info->date = mystrdup("December 2010");
						info->description = mystrdup("I DUNNO LOL");
						info->tags = mystrdup("");

						if (saveDataDownload)
							saveDataDownload->Cancel();
						std::stringstream uri;
						uri << STATICSCHEME << STATICSERVER << "/2157797.cps";
						saveDataDownload = new Request(uri.str());
						if (svf_login)
							saveDataDownload->AuthHeaders(svf_user_id, svf_session_id);
						saveDataDownload->Start();

						if (!instant_open)
						{
							uri.str("");
							uri << STATICSCHEME << STATICSERVER << "/2157797_large.pti";
							thumbnailDownload = new Request(uri.str());
							thumbnailDownload->Start();
						}

						data_ready = 0;
						fake404save = true;
					}
				}
			}
			else
				saveInfoDownload->CheckProgress(&infoTotal, &infoDone);
		}
		if (thumbnailDownload && thumbnailDownload->CheckDone())
		{
			int imgh, imgw, status;
			std::string thumb_data_full = thumbnailDownload->Finish(&status);
			thumbnailDownload = nullptr;
			if (status == 200)
			{
				pixel *full_thumb;
				if (thumb_data_full.empty())
				{
					//error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
					//break;
				}
				else
				{
					full_thumb = ptif_unpack((char*)thumb_data_full.c_str(), thumb_data_full.length(), &imgw, &imgh);
					if (full_thumb)
					{
						save_pic = resample_img(full_thumb, imgw, imgh, XRES/2, YRES/2);
						thumb_data_ready = 1;
						free(full_thumb);
					}
				}
			}
		}
		if (commentsDownload && commentsDownload->CheckStarted() && info_ready && commentsDownload->CheckDone())
		{
			int status;
			std::string comment_data = commentsDownload->Finish(&status);
			commentsDownload = nullptr;
			if (status == 200)
			{
				cJSON *root, *commentobj, *tmpobj;
				for (int i = comment_page*20; i < comment_page*20+20 && i < NUM_COMMENTS; i++)
				{
					info->comments[i].str[0] = 0;
					if (info->commentauthors[i]) { free(info->commentauthors[i]); info->commentauthors[i] = NULL; }
					if (info->commentauthorsunformatted[i]) { free(info->commentauthorsunformatted[i]); info->commentauthorsunformatted[i] = NULL; }
					if (info->commentauthorIDs[i]) { free(info->commentauthorIDs[i]); info->commentauthorIDs[i] = NULL; }
					if (info->commenttimestamps[i]) { free(info->commenttimestamps[i]); info->commenttimestamps[i] = NULL; }
				}
				if (!comment_data.empty() && (root = cJSON_Parse(comment_data.c_str())))
				{
					if (comment_page == 0)
						info->comment_count = cJSON_GetArraySize(root);
					else
						info->comment_count += cJSON_GetArraySize(root);
					if (info->comment_count > NUM_COMMENTS)
						info->comment_count = NUM_COMMENTS;
					for (int i = comment_page*20; i < info->comment_count; i++)
					{
						commentobj = cJSON_GetArrayItem(root, i%20);
						if (commentobj)
						{
							if ((tmpobj = cJSON_GetObjectItem(commentobj, "FormattedUsername")) && tmpobj->type == cJSON_String)
							{
								info->commentauthors[i] = (char*)calloc(63,sizeof(char*));
								if (!strcmp(tmpobj->valuestring, "jacobot"))
									sprintf(info->commentauthors[i], "\bt%s", tmpobj->valuestring);
								else
									strncpy(info->commentauthors[i], tmpobj->valuestring, 63);
							}
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Username")) && tmpobj->type == cJSON_String) { info->commentauthorsunformatted[i] = (char*)calloc(63,sizeof(char*)); strncpy(info->commentauthorsunformatted[i], tmpobj->valuestring, 63); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "UserID")) && tmpobj->type == cJSON_String) { info->commentauthorIDs[i] = (char*)calloc(16,sizeof(char*)); strncpy(info->commentauthorIDs[i], tmpobj->valuestring, 16); }
							//if((tmpobj = cJSON_GetObjectItem(commentobj, "Gravatar")) && tmpobj->type == cJSON_String) { info->commentauthors[i] = (char*)calloc(63,sizeof(char*)); strncpy(info->commentauthors[i], tmpobj->valuestring, 63); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Text")) && tmpobj->type == cJSON_String)  { strncpy(info->comments[i].str, tmpobj->valuestring, 1023); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Timestamp")) && tmpobj->type == cJSON_String) { converttotime(tmpobj->valuestring, &info->commenttimestamps[i], -1, -1, -1); }
						}
					}
					cJSON_Delete(root);
				}
			}
			disable_scrolling = 0;
		}
		bool commentsDownloadStarted = commentsDownload && commentsDownload->CheckStarted();
		if (!instant_open)
		{
			if (save_pic_thumb!=NULL && !hasdrawncthumb) {
				draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);
				free(save_pic_thumb);
				save_pic_thumb = NULL;
				hasdrawncthumb = 1;
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (thumb_data_ready && !hasdrawnthumb) {
				draw_image(vid_buf, save_pic, 51, 51, XRES/2, YRES/2, 255);
				free(save_pic);
				save_pic = NULL;
				hasdrawnthumb = 1;
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (info_ready && !hasdrawninfo) {
				//Render all the save information
				cix = drawtext(vid_buf, 60, (YRES/2)+60, info->name, 255, 255, 255, 255);
				cix = drawtext(vid_buf, 60, (YRES/2)+72, "Author:", 255, 255, 255, 155);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->author, 255, 255, 255, 255);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, "Date Updated:", 255, 255, 255, 155);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->date, 255, 255, 255, 255);
				if(info->downloadcount){
					drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer)-textwidth("Views:")-4, (YRES/2)+72, "Views:", 255, 255, 255, 155);
					drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer), (YRES/2)+72, viewcountbuffer, 255, 255, 255, 255);
				}
				drawtextwrap(vid_buf, 62, (YRES/2)+86, (XRES/2)-24, 0, info->description, 255, 255, 255, 200);

				//Draw the score bars
				if (info->voteup>0||info->votedown>0)
				{
					lv = (info->voteup>info->votedown)?info->voteup:info->votedown;
					lv = (lv>10)?lv:10;

					if (50>lv)
					{
						ryf = 50.0f/((float)lv);
						//if(lv<8)
						//{
						//	ry =  ry/(8-lv);
						//}
						nyu = (int)(info->voteup*ryf);
						nyd = (int)(info->votedown*ryf);
					}
					else
					{
						ryf = ((float)lv)/50.0f;
						nyu = (int)(info->voteup/ryf);
						nyd = (int)(info->votedown/ryf);
					}
					nyu = nyu>50?50:nyu;
					nyd = nyd>50?50:nyd;

					fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 0, 107, 10, 255);
					fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 107, 10, 0, 255);
					drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 128, 128, 128, 255);
					drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 128, 128, 128, 255);

					fillrect(vid_buf, 48+(XRES/2)-nyu, (YRES/2)+54, nyu, 4, 57, 187, 57, 255);
					fillrect(vid_buf, 48+(XRES/2)-nyd, (YRES/2)+60, nyd, 4, 187, 57, 57, 255);
				}

				hasdrawninfo = 1;
				myown = svf_login && !strcmp(info->author, svf_user);
				authoritah = svf_login && (!strcmp(info->author, svf_user) || svf_admin || svf_mod);
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (info_ready) // draw the comments
			{
				int commentNum = 0;
				ccy = 0;
				info->comments[0].y = 72+comment_scroll;
				clearrect(vid_buf, 52+(XRES/2), 51, XRES+BARSIZE-100-((XRES/2)+2), YRES+MENUSIZE-101);
				for (cc=0; cc<info->comment_count; cc++)
				{
					 //Try not to draw off the screen
					if (ccy + 72 + comment_scroll<YRES+MENUSIZE-56)
					{
						if (ccy+comment_scroll >= 0 && info->commentauthors[cc]) //Don't draw above the screen either
						{
							int r = 255, g = 255, bl = 255;
							if (!strcmp(info->commentauthors[cc], svf_user))
							{
								bl = 100;
							}
							else if (!strcmp(info->commentauthors[cc], info->author))
							{
								g = 100;
								bl = 100;
							}

							if (show_ids && info->commentauthorIDs[cc]) //Draw author id
							{
								drawtext(vid_buf, 265+(XRES/2)-textwidth(info->commentauthorIDs[cc]), ccy+60+comment_scroll, info->commentauthorIDs[cc], 255, 255, 0, 255);
								if (!b && bq && mx > 265+(XRES/2)-textwidth(info->commentauthorIDs[cc]) && mx < 265+(XRES/2) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
									show_ids = 0;
							}
							else if (info->commenttimestamps[cc]) //, or draw timestamp
							{
								drawtext(vid_buf, 265+(XRES/2)-textwidth(info->commenttimestamps[cc]), ccy+60+comment_scroll, info->commenttimestamps[cc], 255, 255, 0, 255);
								if (!b && bq && mx > 265+(XRES/2)-textwidth(info->commenttimestamps[cc]) && mx < 265+(XRES/2) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
									show_ids = 1;
							}
							drawtext(vid_buf, 61+(XRES/2), ccy+60+comment_scroll, info->commentauthors[cc], r, g, bl, 255); //Draw author

							if (!b && bq && mx > 61+(XRES/2) && mx < 61+(XRES/2)+textwidth(info->commentauthors[cc]) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
							{
								if (sdl_mod & (KMOD_CTRL|KMOD_GUI)) //open profile
								{
									/*char link[128];
									strcpy(link, SCHEME SERVER "/User.html?Name=");
									strcaturl(link, info->commentauthorsunformatted[cc]);
									open_link(link);*/
									//profileToOpen = info->commentauthorsunformatted[cc];
								}
								else if (sdl_mod & KMOD_SHIFT) //, or search for a user's saves
								{
									sprintf(search_expr,"user:%s", info->commentauthorsunformatted[cc]);
									search_own = 0;
									search_page = 0;
									search_ui(vid_buf);
									retval = 1;
									goto finish;
								}
								else //copy name to comment box
								{
									if (strlen(ed.str) + strlen(info->commentauthorsunformatted[cc]) < 1023)
									{
										strappend(ed.str, info->commentauthorsunformatted[cc]);
										strappend(ed.str, ": ");
#ifndef TOUCHUI
										dofocus = 1;
#endif
									}
								}
							}
						}

						ccy += 12;
						if (ccy + 72 + comment_scroll<YRES+MENUSIZE-56) // Check again if the comment is off the screen, incase the author line made it too long
						{
							int change, commentboxy = YRES+MENUSIZE-70-ed.h+2-5;
							if (ccy+comment_scroll < 0) // if above screen set height to negative, how long until it can start being drawn
								info->comments[cc].maxHeight = ccy+comment_scroll-10;
							else                        // else set how much can be drawn until it goes off the screen
								info->comments[cc].maxHeight = YRES+MENUSIZE-41 - (ccy + 72 + comment_scroll);

							change = ui_label_draw(vid_buf, &info->comments[cc]); // draw the comment
#ifndef TOUCHUI
							ui_label_process(mx, my, b, bq, &info->comments[cc]); // process copying
#endif

							if (svf_login && !b && bq && mx > 50+(XRES/2)+1 && mx < 50 + XRES+BARSIZE-100 && my > commentboxy - 2 && my < commentboxy + ed.h+2) // defocus comments that are under textbox
								info->comments[cc].focus = info->comments[cc].cursor = info->comments[cc].cursorstart = info->comments[cc].numClicks = 0;

							ccy += change + 10;
							if (cc < NUM_COMMENTS-1)
								info->comments[cc+1].y = info->comments[cc].y + change + 22;

							if (ccy+comment_scroll < 50 && cc == info->comment_count-1 && commentsDownloadStarted) // disable scrolling until more comments have loaded
							{
								disable_scrolling = 1;
								scroll_velocity = 0.0f;
							}
							if (ccy+comment_scroll < 0 && cc == info->comment_count-1 && !commentsDownloadStarted) // reset to top of comments
							{
								comment_scroll = 0;
								comment_scroll_float = 0.0f;
								scroll_velocity = 0.0f;
							}

							//draw the line that separates comments
							if (ccy+52+comment_scroll<YRES+MENUSIZE-56 && ccy+comment_scroll>-2)
							{
								draw_line(vid_buf, 50+(XRES/2)+2, ccy+52+comment_scroll, XRES+BARSIZE-51, ccy+52+comment_scroll, (cc == info->comment_count-1 && (info->comment_count%20))?175:100, 100, 100, XRES+BARSIZE);
							}
						}
					}
					else
					{
						if (!commentNum)
							commentNum = cc;
						break;
					}
					if (cc == info->comment_count-1 && !commentsDownloadStarted && comment_page < NUM_COMMENTS/20 && !(info->comment_count%20))
					{
						std::stringstream uri;
						uri << SCHEME << SERVER << "/Browse/Comments.json?ID=" << save_id << "&Start=" << (comment_page+1)*20 << "&Count=20";
						if (commentsDownload)
							commentsDownload->Cancel();
						commentsDownload = new Request(uri.str());
						commentsDownload->Start();

						comment_page++;
					}
				}

				if (!commentNum)
					commentNum = info->comment_count;
				char pageText[128];
				sprintf(pageText, "Page %i of %i%s", commentNum/20+1, info->comment_count/20+1, (info->comment_count%20)?"":"+");
				drawtext(vid_buf, XRES+BARSIZE-190, YRES+MENUSIZE-43, pageText, 255, 255, 255, 255);

				//memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			//Render the comment box.
			if (info_ready && svf_login && !fake404save)
			{
				fillrect(vid_buf, 51+(XRES/2), ed.y-6, XRES+BARSIZE-100-((XRES/2)+1), ed.h+25, 0, 0, 0, 255);
				drawrect(vid_buf, 51+(XRES/2), ed.y-6, XRES+BARSIZE-100-((XRES/2)+1), ed.h+25, 200, 200, 200, 255);

				if (strlen(ed.str) <= 500)
					drawrect(vid_buf, 55+(XRES/2), ed.y-3, XRES+BARSIZE-108-((XRES/2)+1), ed.h, 255, 255, 255, 200);
				else
					drawrect(vid_buf, 55+(XRES/2), ed.y-3, XRES+BARSIZE-108-((XRES/2)+1), ed.h, 255, 150, 150, 200);

				ed.y = YRES+MENUSIZE-71-ui_edit_draw(vid_buf, &ed);

				drawrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 255);
				drawtext(vid_buf, XRES+BARSIZE-90, YRES+MENUSIZE-63, "Submit", 255, 255, 255, 255);

				if (commentLen != strlen(ed.str))
				{
					std::string text = ed.str;
					std::transform(text.begin(), text.end(), text.begin(), ::tolower);
					if (myown && text.find("vote") != text.npos)
					{
						if (!commentWarningShown)
						{
							commentWarning = "Do not ask for votes";
							commentWarningShown = true;
						}
					}
					else if (!myown && (text.find("stolen") != text.npos || text.find("copied") != text.npos))
					{
						if (!commentWarningShown)
						{
							if (rand()%2)
								commentWarning = "Stolen? Report the save instead";
							else
								commentWarning = "Please report stolen saves";
							commentWarningShown = true;
						}
					}
					else if (CheckSwearing(text))
					{
						if (!commentWarningShown)
						{
							if (rand()%2)
								commentWarning = "Please do not swear";
							else
								commentWarning = "Bad language may be deleted";
							commentWarningShown = true;
						}
					}
					else if (commentWarningShown)
					{
						commentWarning = "";
						commentWarningShown = false;
					}
					commentLen = strlen(ed.str);
				}
				if (commentWarningShown && commentWarning.length())
				{
					drawtext(vid_buf, 56+(XRES/2), YRES+MENUSIZE-63, commentWarning.c_str(), 255, 0, 0, 255);
				}
			}

			//Save ID text and copybox
			cix = textwidth("Save ID: ");
			cix += ctb.width;
			ctb.x = textwidth("Save ID: ")+(XRES+BARSIZE-cix)/2;
			//ctb.x =
			drawtext(vid_buf, (XRES+BARSIZE-cix)/2, YRES+MENUSIZE-15, "Save ID: ", 255, 255, 255, 255);
			ui_copytext_draw(vid_buf, &ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);

			//Open Button
			bc = openable?255:150;
			drawrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 73, YRES+MENUSIZE-63, "Open", 255, 255, 255, bc);
			drawtext(vid_buf, 56, YRES+MENUSIZE-62, "\x81", 255, 255, 255, bc);
			//Fav Button
			bc = svf_login && !fake404save ? 255 : 150;
			drawrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			if(info->myfav && svf_login){
				drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Unfav.", 255, 230, 230, bc);
			} else {
				drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Fav.", 255, 255, 255, bc);
			}
			drawtext(vid_buf, 107, YRES+MENUSIZE-64, "\xCC", 255, 255, 255, bc);
			//Report Button
			bc = (svf_login && info_ready && !fake404save) ? 255 : 150;
			drawrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 168, YRES+MENUSIZE-63, "Report", 255, 255, 255, bc);
			drawtext(vid_buf, 155, YRES+MENUSIZE-64, "\xE4", 255, 255, 255, bc);
			//Delete Button
			bc = authoritah && !fake404save ? 255 : 150;
			drawrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 218, YRES+MENUSIZE-63, "Delete", 255, 255, 255, bc);
			drawtext(vid_buf, 207, YRES+MENUSIZE-63, "\xAA", 255, 255, 255, bc);
			//Open in browser button
			bc = !fake404save ? 255 : 150;
			drawrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 273, YRES+MENUSIZE-63, "Open in Browser", 255, 255, 255, bc);
			drawtext(vid_buf, 257, YRES+MENUSIZE-62, "\x81", 255, 255, 255, bc);

			//Open Button
			if (sdl_key==SDLK_RETURN && openable && strlen(ed.str) == 0) {
				queue_open = 1;
			}
			if (mx > 50 && mx < 50+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && openable && !queue_open) {
				fillrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (!b && bq) {
					//Button Clicked
					queue_open = 1;
				}
			}
			//Fav Button
			if (mx > 100 && mx < 100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && !queue_open && !fake404save) {
				fillrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (!b && bq) {
					//Button Clicked
					if(info->myfav){
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Removing from favourites...");
						execute_unfav(vid_buf, save_id);
						info->myfav = 0;
					} else {
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Adding to favourites...");
						execute_fav(vid_buf, save_id);
						info->myfav = 1;
					}
				}
			}
			//Report Button
			if (mx > 150 && mx < 150+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open && !fake404save) {
				fillrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (!b && bq) {
					//Button Clicked
					if (report_ui(vid_buf, save_id, false)) {
						retval = 0;
						break;
					}
				}
			}
			//Delete Button
			if (mx > 200 && mx < 200+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && (authoritah || myown) && !queue_open && !fake404save) {
				fillrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (!b && bq) {
					//Button Clicked
					if (myown || !info->publish) {
						if (confirm_ui(vid_buf, "Are you sure you wish to delete this?", "You will not be able recover it.", "Delete")) {
							fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
							info_box(vid_buf, "Deleting...");
							if (execute_delete(vid_buf, save_id)) {
								retval = 0;
								break;
							}
						}
					} else {
						if (confirm_ui(vid_buf, "Are you sure?", "This save will be removed from the search index.", "Remove")) {
							fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
							info_box(vid_buf, "Removing...");
							if (execute_delete(vid_buf, save_id)) {
								retval = 0;
								break;
							}
						}
					}
				}
			}
			//Open in browser button
			if (mx > 250 && mx < 250+107 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50  && !queue_open && !fake404save) {
				fillrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, 40);
				if (!b && bq)
				{
					std::stringstream browserLink;
					browserLink << SCHEME << SERVER << "/Browse/View.html?ID=" << save_id;
					Platform::OpenLink(browserLink.str());
				}
			}
			//Submit Button
			if (mx > XRES+BARSIZE-100 && mx < XRES+BARSIZE-100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open && !fake404save) {
				bool commentsDownloadStarted = commentsDownload && commentsDownload->CheckStarted();
				fillrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40+(!commentsDownloadStarted ? 0 : 1)*80);
				if (!b && bq && !commentsDownloadStarted)
				{
					//Button Clicked
					fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
					info_box(vid_buf, "Submitting Comment...");
					if (!execute_submit(vid_buf, save_id, ed.str))
					{
						std::stringstream uri;
						uri << SCHEME << SERVER << "/Browse/Comments.json?ID=" << save_id << "&Start=0&Count=20";
						if (commentsDownload)
							commentsDownload->Cancel();
						commentsDownload = new Request(uri.str());
						commentsDownload->Start();

						for (int i = 0; i < NUM_COMMENTS; i++)
						{
							info->comments[i].str[0] = 0;
							if (info->commentauthors[i]) { free(info->commentauthors[i]); info->commentauthors[i] = NULL; }
							if (info->commentauthorsunformatted[i]) { free(info->commentauthorsunformatted[i]); info->commentauthorsunformatted[i] = NULL; }
							if (info->commentauthorIDs[i]) { free(info->commentauthorIDs[i]); info->commentauthorIDs[i] = NULL; }
							if (info->commenttimestamps[i]) { free(info->commenttimestamps[i]); info->commenttimestamps[i] = NULL; }
						}
						comment_page = 0;
						info->comment_count = 0;
						comment_scroll = 0;
						comment_scroll_float = 0.0f;
						scroll_velocity = 0.0f;
						ed.str[0] = 0;
					}
				}
			}
#ifdef TOUCHUI
			if (b && !bq && mx >= 52+(XRES/2) && mx <= XRES+BARSIZE-50 && my >= 51 && my <= YRES+MENUSIZE-50)
			{
				scrolling = true;
				lastY = my;
			}
			else if (scrolling)
			{
				if (!b || disable_scrolling)
					scrolling = false;
				else
				{
					scroll_velocity += ((float)my-lastY)*.2f;
					lastY = my;
				}
			}
#endif
			if (scroll_velocity)
			{
				comment_scroll_float += scroll_velocity;
				comment_scroll = int(comment_scroll_float);
				
				if (Engine::Ref().IsMomentumScroll())
				{
					scroll_velocity *= scrollDeceleration;
					if (std::abs(scroll_velocity) < .5f)
						scroll_velocity = 0.0f;
				}
				else
					scroll_velocity = 0.0f;

				if (comment_scroll > 0)
				{
					comment_scroll = 0;
					comment_scroll_float = 0.0f;
					scroll_velocity = 0.0f;
				}
			}
			if (sdl_wheel)
			{
				if (!disable_scrolling || sdl_wheel > 0)
				{
					scroll_velocity += (Engine::Ref().IsMomentumScroll() ? scrollSpeedMomentum : scrollSpeed) * sdl_wheel * 2;
					if (comment_scroll > 0)
						comment_scroll = 0;
				}
			}
			if (sdl_key=='[' && !ed.focus)
			{
				comment_scroll += scrollSpeed * 2;
				comment_scroll_float += scrollSpeed * 2;
				if (comment_scroll_float > 0)
				{
					comment_scroll_float = 0;
					comment_scroll = 0;
				}
			}
			if (sdl_key==']' && !ed.focus && !disable_scrolling)
			{
				comment_scroll -= scrollSpeed * 2;
				comment_scroll_float -= scrollSpeed * 2;
			}
#ifndef TOUCHUI
			//If mouse was clicked outside of the window bounds.
			if (!(mx>50 && my>50 && mx<XRES+BARSIZE-50 && my<YRES+MENUSIZE-50) && !(mx >= ctb.x && mx <= ctb.x+ctb.width && my >= ctb.y && my <= ctb.y+ctb.height) && !queue_open && strlen(ed.str) < 200)
			{
				if (b && !bq)
				{
					clickOutOfBounds = true;
				}
				else if (!b && bq && clickOutOfBounds)
				{
					retval = 0;
					break;
				}
			}
			if (!b)
				clickOutOfBounds = false;
#endif
		}

		//Download completion
		downloadTotal = saveTotal+infoTotal;
		downloadDone = saveDone+infoDone;
		if(downloadTotal>downloadDone)
		{
			clearrect(vid_buf, 52, (YRES/2)+38, (XRES)/2-1, 13);
			fillrect(vid_buf, 51, (YRES/2)+38, (int)((((float)XRES-2)/2.0f)*((float)downloadDone/(float)downloadTotal)), 12, 255, 200, 0, 255);
			if(((float)downloadDone/(float)downloadTotal)>0.5f)
				drawtext(vid_buf, 51+(((XRES/2)-textwidth("Downloading"))/2), (YRES/2)+40, "Downloading", 0, 0, 0, 255);
			else
				drawtext(vid_buf, 51+(((XRES/2)-textwidth("Downloading"))/2), (YRES/2)+40, "Downloading", 255, 255, 255, 255);
		}

		//User opened the save, wait until we've got all the data first...
		if (queue_open || instant_open)
		{
			if (info_ready && data_ready && save)
			{
				SnapshotHistory::TakeSnapshot(globalSim);
				// Do Open!
				try
				{
					globalSim->LoadSave(0, 0, save, 1);

					svf_open = 1;
					svf_own = svf_login && !strcmp(info->author, svf_user);
					svf_publish = info->publish && svf_login && !strcmp(info->author, svf_user);

					if (!fake404save)
						strcpy(svf_id, save_id);
					else
						strcpy(svf_id, "2157797");
					svf_version = save_date ? save_date : "";
					strcpy(svf_name, info->name);
					strcpy(svf_description, info->description);
					strncpy(svf_author, info->author, 63);
					if (info->tags)
					{
						strncpy(svf_tags, info->tags, 255);
						svf_tags[255] = 0;
					} else {
						svf_tags[0] = 0;
					}
					svf_myvote = info->myvote;
					svf_filename[0] = 0;
					svf_fileopen = 0;
					retval = 1;

					authors = save->authors;
					if (!authors.size())
					{
						DefaultSaveInfo();
						authors["published"] = info->publish;
						authors["date"] = info->date; // this is WRONG but we don't get any better info until I use the new api
					}
					else
					{
						if (authors["id"] == 0 || authors["id"] == -1)
							authors["id"] = atoi(svf_id);
					}

					the_game->SetReloadPoint(save);
					delete save;
					break;
				}
				catch (ParseException & e)
				{
					queue_open = 0;

					clear_save_info();
					error_ui(vid_buf, 0, std::string("An error occurred when parsing the save: ") + e.what());
					if (instant_open)
						break;
				}
				delete save;
			}
			else
			{
				fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 190);
				drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);
			}
		}
		if (!info_ready || !data_ready)
		{
			info_box(vid_buf, "Loading");
		}

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (info_ready && svf_login) {
			ui_edit_process(mx, my, b, bq, &ed);
		}
#ifndef TOUCHUI
		if (dofocus)
		{
			ed.focus = 1;
			ed.cursor = ed.cursorstart = strlen(ed.str);
			dofocus = 0;
		}
#endif

		if (sdl_key==SDLK_ESCAPE) {
			retval = 0;
			break;
		}

		if (lasttime<TIMEOUT)
			lasttime++;

		if (strcmp(profileToOpen, ""))
		{
			ProfileViewer *temp = new ProfileViewer(profileToOpen);
			Engine *moreTemp = new Engine();
			moreTemp->ShowWindow(temp);
			//moreTemp->MainLoop();
			MainLoop();
			delete moreTemp;
			profileToOpen = "";
		}
	}
	//Prevent those mouse clicks being passed down.
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	finish:
	//Let the download threads finish downloads and delete the data
	if (saveDataDownload)
		saveDataDownload->Cancel();
	if (saveInfoDownload)
		saveInfoDownload->Cancel();
	if (thumbnailDownload)
		thumbnailDownload->Cancel();
	if (commentsDownload)
		commentsDownload->Cancel();
	info_parse("", info);
	free(info);
	free(old_vid);
	if (thumb_data) free(thumb_data);
	if (save_pic) free(save_pic);
	if (save_pic_thumb) free(save_pic_thumb);
	return retval;
}

int info_parse(const char *info_data, save_info *info)
{
	int i,j;
	char *p, *q;

	if (info->title) free(info->title);
	if (info->name) free(info->name);
	if (info->author) free(info->author);
	if (info->date) free(info->date);
	if (info->description) free(info->description);
	if (info->tags) free(info->tags);
	for (i=0;i<NUM_COMMENTS;i++)
	{
		if (info->commentauthors[i]) free(info->commentauthors[i]);
		if (info->commentauthorsunformatted[i]) free(info->commentauthorsunformatted[i]);
		if (info->commentauthorIDs[i]) free(info->commentauthorIDs[i]);
		if (info->commenttimestamps[i]) free(info->commenttimestamps[i]);
	}
	memset(info, 0, sizeof(save_info));
	for (i = 0; i < NUM_COMMENTS; i++)
	{
		ui_label_init(&info->comments[i], 61+(XRES/2), 0, XRES+BARSIZE-107-(XRES/2), 0);
	}

	if (!info_data || !*info_data)
		return 0;

	i = 0;
	j = 0;
	do_open = 0;
	while (1)
	{
		if (!*info_data)
			break;
		p = (char*)strchr(info_data, '\n');
		if (!p)
			p = (char*)(info_data + strlen(info_data));
		else
			*(p++) = 0;

		if (!strncmp(info_data, "TITLE ", 6))
		{
			info->title = mystrdup(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "NAME ", 5))
		{
			info->name = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "AUTHOR ", 7))
		{
			info->author = mystrdup(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DATE ", 5))
		{
			info->date = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "DESCRIPTION ", 12))
		{
			info->description = mystrdup(info_data+12);
			j++;
		}
		else if (!strncmp(info_data, "VOTEUP ", 7))
		{
			info->voteup = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "VOTEDOWN ", 9))
		{
			info->votedown = atoi(info_data+9);
			j++;
		}
		else if (!strncmp(info_data, "VOTE ", 5))
		{
			info->vote = atoi(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "MYVOTE ", 7))
		{
			info->myvote = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DOWNLOADS ", 10))
		{
			info->downloadcount = atoi(info_data+10);
			j++;
		}
		else if (!strncmp(info_data, "MYFAV ", 6))
		{
			info->myfav = atoi(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "PUBLISH ", 8))
		{
			info->publish = atoi(info_data+8);
			j++;
		}
		else if (!strncmp(info_data, "TAGS ", 5))
		{
			info->tags = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "COMMENT ", 8))
		{
			if (info->comment_count>=NUM_COMMENTS) {
				info_data = p;
				continue;
			} else {
				q = (char*)strchr(info_data+8, ' ');
				*(q++) = 0;
				info->commentauthors[info->comment_count] = mystrdup(info_data+8);
				info->commentauthorsunformatted[info->comment_count] = mystrdup(info_data+8);
				strncpy(info->comments[info->comment_count].str,q, 1023);
				info->comment_count++;
			}
			j++;
		}
		info_data = p;
	}
	if (j>=8) {
		return 1;
	} else {
		return -1;
	}
}

int search_results(char *str, int votes)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	for (i=0; i<GRID_X*GRID_Y; i++)
	{
		if (search_ids[i])
		{
			free(search_ids[i]);
			search_ids[i] = NULL;
		}
		if (search_names[i])
		{
			free(search_names[i]);
			search_names[i] = NULL;
		}
		if (search_dates[i])
		{
			free(search_dates[i]);
			search_dates[i] = NULL;
		}
		if (search_owners[i])
		{
			free(search_owners[i]);
			search_owners[i] = NULL;
		}
		if (search_thumbs[i])
		{
			free(search_thumbs[i]);
			search_thumbs[i] = NULL;
			search_thsizes[i] = 0;
		}
	}
	for (j=0; j<TAG_MAX; j++)
		if (tag_names[j])
		{
			free(tag_names[j]);
			tag_names[j] = NULL;
		}
	server_motd[0] = 0;

	if (!str || !*str)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*str)
			break;
		p = strchr(str, '\n');
		if (!p)
			p = str + strlen(str);
		else
			*(p++) = 0;
		if (!strncmp(str, "OPEN ", 5))
		{
			do_open = 1;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+5);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+5, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "HISTORY ", 8))
		{
			char * id_d_temp = NULL;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+8);

			search_dates[i] = mystrdup(sd);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
				
			//Build thumb cache ID and find
			id_d_temp = (char*)malloc(strlen(search_ids[i])+strlen(search_dates[i])+2);
			strcpy(id_d_temp, search_ids[i]);
			strappend(id_d_temp, "_");
			strappend(id_d_temp, search_dates[i]);
			thumb_cache_find(id_d_temp, search_thumbs+i, search_thsizes+i);
			free(id_d_temp);
			
			i++;
		}
		else if (!strncmp(str, "MOTD ", 5))
		{
			strncpy(server_motd, str+5, 511);
		}
		else if (!strncmp(str, "TAG ", 4))
		{
			if (j >= TAG_MAX)
			{
				str = p;
				continue;
			}
			q = strchr(str+4, ' ');
			if (!q)
			{
				str = p;
				continue;
			}
			*(q++) = 0;
			tag_names[j] = mystrdup(str+4);
			tag_votes[j] = atoi(q);
			j++;
		}
		else
		{
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
			i++;
		}
		str = p;
	}
	if (*str)
		i++;
	return i;
}

int execute_tagop(pixel *vid_buf, const char *op, char *tag)
{
	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Tag.api?Op=" + std::string(op), &status, svf_user_id, svf_session_id, {
		{ "ID", svf_id },
		{ "Tag", tag }
	});

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return 1;
	}
	if (result.empty())
	{
		error_ui(vid_buf, 0, "Could not add tag");
		return 1;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return 1;
	}

	if (result.length() > 2)
	{
		strncpy(svf_tags, result.substr(3).c_str(), 255);
		svf_id[15] = 0;
	}

	return 0;
}

int execute_save(pixel *vid_buf, Save *save)
{
	std::map<std::string, std::string> postData = {
		{ "Name", svf_name },
		{ "Description", svf_description },
		{ "Data:save.bin", std::string((char*)save->GetSaveData(), (int)save->GetSaveSize()) },
		{ "Publish", (svf_publish == 1) ? "Public" : "Private" }
	};
	if (svf_id[0])
	{
		postData.insert(std::pair<std::string, std::string>("ID", svf_id));
	}

	if (save->fromNewerVersion && svf_publish)
	{
		error_ui(vid_buf, 0, "Cannot publish save, incompatible with latest release version.");
		return 1;
	}

	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Save.api", &status, svf_user_id, svf_session_id, postData);

	the_game->SetReloadPoint(save);

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return 1;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		if (result.empty())
			result = mystrdup("Could not save - no reply from server");
		error_ui(vid_buf, 0, result);
		return 1;
	}

	if (result.length() > 2)
	{
		strncpy(svf_id, result.substr(3).c_str(), 15);
		svf_id[15] = 0;
	}

	if (!svf_id[0])
	{
		error_ui(vid_buf, 0, "No ID supplied by server");
		return 1;
	}

	thumb_cache_inval(svf_id);

	svf_own = 1;
	return 0;
}

int execute_delete(pixel *vid_buf, char *id)
{
	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Delete.api", &status, svf_user_id, svf_session_id, {
		{ "ID", id }
	});

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return 0;
	}
	if (result.find("INFO: ", 0, 6) != result.npos)
	{
		info_ui(vid_buf, "Info", result.substr(6));
		return 0;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return 0;
	}

	return 1;
}

bool ParseServerReturn(char *result, int status, bool json)
{
	// no server response, return "Malformed Response"
	if (status == 200 && !result)
	{
		status = 603;
	}
	if (status == 302)
		return true;
	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return true;
	}

	if (json)
	{
		std::istringstream datastream(result);
		Json::Value root;

		try
		{
			datastream >> root;
			// assume everything is fine if an empty [] is returned
			if (root.size() == 0)
			{
				return false;
			}
			int status = root.get("Status", 1).asInt();
			if (status != 1)
			{
				std::string err = root.get("Error", "Unspecified Error").asString();
				error_ui(vid_buf, 0, err);
				return true;
			}
		}
		catch (std::exception &e)
		{
			// sometimes the server returns a 200 with the text "Error: 401"
			if (strstr(result, "Error: ") == result)
			{
				status = atoi(result + 7);
				error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
				return true;
			}
			error_ui(vid_buf, 0, "Could not read response");
			return true;
		}
	}
	else
	{
		if (strncmp((const char *)result, "OK", 2))
		{
			error_ui(vid_buf, 0, result);
			return true;
		}
	}
	return false;
}

bool execute_submit(pixel *vid_buf, char *id, char *message)
{
	int status;

	std::stringstream url;
	url <<  SCHEME << SERVER << "/Browse/Comments.json?ID=" << id;
	Request *comment = new Request(url.str());
	comment->AuthHeaders(svf_user_id, svf_session_id);
	comment->AddPostData({
		 { "Comment", message }
	 });
	comment->Start();
	std::string result = comment->Finish(&status);


	bool ret = ParseServerReturn((char*)result.c_str(), status, true);
	return ret;
}

int execute_report(pixel *vid_buf, char *id, char *reason)
{
	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Report.api", &status, svf_user_id, svf_session_id, {
		{ "ID", id },
		{ "Reason", reason }
	});

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return 0;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return 0;
	}

	return 1;
}

int execute_bug(pixel *vid_buf, std::string feedback)
{
	int status;
	std::string result = Request::SimpleAuth(UPDATESCHEME "starcatcher.us/TPT/bagelreport.lua", &status, svf_user, "", {
		{ "bug", feedback }
	 });

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return 0;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return 0;
	}

	return 1;
}

void execute_fav(pixel *vid_buf, char *id)
{
	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Favourite.api", &status, svf_user_id, svf_session_id, {
		{ "ID", id }
	});

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return;
	}
}

void execute_unfav(pixel *vid_buf, char *id)
{
	int status;
	std::string result = Request::SimpleAuth(SCHEME SERVER "/Favourite.api?Action=Remove", &status, svf_user_id, svf_session_id, {
		{ "ID", id }
	});

	if (status != 200)
	{
		error_ui(vid_buf, status, Request::GetStatusCodeDesc(status));
		return;
	}
	if (result.find("OK", 0, 2) == result.npos)
	{
		error_ui(vid_buf, 0, result);
		return;
	}
}

ui_edit box_R;
ui_edit box_G;
ui_edit box_B;
ui_edit box_A;

void init_color_boxes()
{
	ui_edit_init(&box_R, 5, 264, 30, 14);
	strcpy(box_R.str, "255");
	box_R.focus = 0;

	ui_edit_init(&box_G, 40, 264, 30, 14);
	strcpy(box_G.str, "255");
	box_G.focus = 0;

	ui_edit_init(&box_B, 75, 264, 30, 14);
	strcpy(box_B.str, "255");
	box_B.focus = 0;

	ui_edit_init(&box_A, 110, 264, 30, 14);
	strcpy(box_A.str, "255");
	box_A.focus = 0;
}

void decoration_textbox_color(ui_edit* textbox, int *color, int *color2)
{
	*color = atoi((*textbox).str);
	if (*color > 255) *color = 255;
	if (*color < 0) *color = 0;
	*color2 = *color;
	RGB_to_HSV(currR, currG, currB, &currH, &currS, &currV);
}

ARGBColour decocolor = COLARGB(255, 255, 0, 0);
int currA = 255, currR = 255, currG = 0, currB = 0;
int currH = 0, currS = 255, currV = 255;
int on_left = 1, decobox_hidden = 0;
bool clickedHueBox;
bool clickedSaturation;

int deco_disablestuff;
void decoration_editor(pixel *vid_buf, int b, int bq, int mx, int my)
{
	pixel t;
	int i, hh, ss, vv, can_select_color = 1;
	int cr = 255, cg = 0, cb = 0, ca = 255;
	int th = currH, ts = currS, tv = currV;
	int grid_offset_x;
	int window_offset_x;
	int onleft_button_offset_x;
	//char frametext[64];

	if (!deco_disablestuff && b) //If mouse is down, but a color isn't already being picked
		can_select_color = 0;
	if (!bq)
		deco_disablestuff = 0;
	currR = COLR(decocolor), currG = COLG(decocolor), currB = COLB(decocolor), currA = COLA(decocolor);

	/*for (i = 0; i <= parts_lastActiveIndex; i++)
		if (parts[i].type == PT_ANIM)
		{
			parts[i].tmp2 = framenum;
		}*/

	if (!decobox_hidden)
	{
		ui_edit_process(mx, my, b, bq, &box_R);
		ui_edit_process(mx, my, b, bq, &box_G);
		ui_edit_process(mx, my, b, bq, &box_B);
		ui_edit_process(mx, my, b, bq, &box_A);
	}

	if(on_left==1)
	{
		grid_offset_x = 5;
		window_offset_x = 2;
		onleft_button_offset_x = 259;
		box_R.x = 5;
		box_G.x = 40;
		box_B.x = 75;
		box_A.x = 110;
	}
	else
	{
		grid_offset_x = XRES - 274;
		window_offset_x = XRES - 279;
		onleft_button_offset_x = 4;
		box_R.x = XRES - 254 + 5;
		box_G.x = XRES - 254 + 40;
		box_B.x = XRES - 254 + 75;
		box_A.x = XRES - 254 + 110;
	}
	
	//drawrect(vid_buf, -1, -1, XRES+1, YRES+1, 220, 220, 220, 255);
	//drawrect(vid_buf, -1, -1, XRES+2, YRES+2, 70, 70, 70, 255);
	//drawtext(vid_buf, 2, 388, "Welcome to the decoration editor v.3 (by cracker64) \n\nClicking the current color on the window will move it to the other side. Right click is eraser. ", 255, 255, 255, 255);
	//drawtext(vid_buf, 2, 388, "Welcome to the decoration editor v.4 (by cracker64/jacob1)", 255, 255, 255, 255);
	//sprintf(frametext,"Frame %i/%i",framenum+1,maxframes);
	//drawtext(vid_buf, 2, 399, frametext, 255, 255, 255, 255);

	if(!decobox_hidden)
	{
		char hex[32] = "";
		fillrect(vid_buf, window_offset_x, 2, 2+255+4+10+5, 2+255+20, 0, 0, 0, currA);
		drawrect(vid_buf, window_offset_x, 2, 2+255+4+10+5, 2+255+20, 255, 255, 255, 255);//window around whole thing

		drawrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, 255, 255, 255, 255);
		drawrect(vid_buf, window_offset_x + 230, 2 +255+6, 26, 12, 255, 255, 255, 255);
		drawtext(vid_buf, window_offset_x + 232, 2 +255+9, "Clear", 255, 255, 255, 255);
		ui_edit_draw(vid_buf, &box_R);
		ui_edit_draw(vid_buf, &box_G);
		ui_edit_draw(vid_buf, &box_B);
		ui_edit_draw(vid_buf, &box_A);
		sprintf(hex,"0x%.8X",(currA<<24)+(currR<<16)+(currG<<8)+currB);
		if (mx > (on_left?143:502) && my > 264 && mx < (on_left?203:562) && my < 280)
			drawtext(vid_buf,on_left?145:504,265,hex,currR,currG,currB,200);
		else
			drawtext(vid_buf,on_left?145:504,265,hex,currR,currG,currB,255);

		//draw color square
		for(ss=0; ss<=255; ss++)
		{
			int lasth = -1, currh = 0;
			for(hh=0;hh<=359;hh++)
			{
				currh = clamp_flt((float)hh, 0, 359);
				if (currh == lasth)
					continue;
				lasth = currh;
				t = vid_buf[(ss+5)*(XRES+BARSIZE)+(currh+grid_offset_x)];
				cr = 0;
				cg = 0;
				cb = 0;
				HSV_to_RGB(hh,255-ss,currV,&cr,&cg,&cb);
				cr = ((currA*cr + (255-currA)*PIXR(t))>>8);
				cg = ((currA*cg + (255-currA)*PIXG(t))>>8);
				cb = ((currA*cb + (255-currA)*PIXB(t))>>8);
				vid_buf[(ss+5)*(XRES+BARSIZE)+(currh+grid_offset_x)] = PIXRGB(cr, cg, cb);
			}
		}
		//draw brightness bar
		for(vv=0; vv<=255; vv++)
			for( i=0; i<10; i++)
			{
				t = vid_buf[(vv+5)*(XRES+BARSIZE)+(i+grid_offset_x+255+4)];
				cr = 0;
				cg = 0;
				cb = 0;
				HSV_to_RGB(currH,currS,vv,&cr,&cg,&cb);
				cr = ((currA*cr + (255-currA)*PIXR(t))>>8);
				cg = ((currA*cg + (255-currA)*PIXG(t))>>8);
				cb = ((currA*cb + (255-currA)*PIXB(t))>>8);
				vid_buf[(vv+5)*(XRES+BARSIZE)+(i+grid_offset_x+255+4)] = PIXRGB(cr, cg, cb);
			}
		addpixel(vid_buf,grid_offset_x + clamp_flt((float)currH, 0, 359),5-1,255,255,255,255);
		addpixel(vid_buf,grid_offset_x -1,5+(255-currS),255,255,255,255);

		addpixel(vid_buf,grid_offset_x + clamp_flt((float)th, 0, 359),5-1,100,100,100,255);
		addpixel(vid_buf,grid_offset_x -1,5+(255-ts),100,100,100,255);

		addpixel(vid_buf,grid_offset_x + 255 +3,5+tv,100,100,100,255);
		addpixel(vid_buf,grid_offset_x + 255 +3,5 +currV,255,255,255,255);

		fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, currR, currG, currB, currA);
	}

	if(!box_R.focus)//prevent text update if it is being edited
		sprintf(box_R.str,"%d",currR);
	else
	{
		deco_disablestuff = 1;
		decoration_textbox_color(&box_R, &cr, &currR);
	}
	if(!box_G.focus)
		sprintf(box_G.str,"%d",currG);
	else
	{
		deco_disablestuff = 1;
		decoration_textbox_color(&box_G, &cg, &currG);
	}
	if(!box_B.focus)
		sprintf(box_B.str,"%d",currB);
	else
	{
		deco_disablestuff = 1;
		decoration_textbox_color(&box_B, &cb, &currB);
	}
	if(!box_A.focus)
		sprintf(box_A.str,"%d",currA);
	else
	{
		deco_disablestuff = 1;
		decoration_textbox_color(&box_A, &ca, &currA);
	}

	//fillrect(vid_buf, 250, YRES+4, 40, 15, currR, currG, currB, currA);

	drawrect(vid_buf, 294, YRES+1, 15, 14, 255, 255, 255, 255);
	fillrect(vid_buf, 294, YRES+1, 15, 14, currR, currG, currB, currA);
	if (decobox_hidden)
		drawtext(vid_buf, 297, YRES+5, "\xCB", 255, 255, 255, 255);
	else
		drawtext(vid_buf, 298, YRES+4, "\xCA", 255, 255, 255, 255);

	//in the main window
	if (can_select_color && !decobox_hidden && ((mx >= window_offset_x && my >= 2 && mx <= window_offset_x+255+4+10+5 && my <= 2+255+20) || clickedHueBox || clickedSaturation))
	{
		//inside brightness bar
		if (((mx >= grid_offset_x +255+4 && my >= 5 && mx <= grid_offset_x+255+4+10 && my <= 5+255) || clickedHueBox) && !clickedSaturation)
		{
			my = std::min(std::max(5, my), 260);
			tv =  my - 5;
			if (b && !bq)
				clickedHueBox = true;
			if (b && clickedHueBox)
			{
				currV = my - 5;
				HSV_to_RGB(currH,currS,tv,&currR,&currG,&currB);
				deco_disablestuff = 1;
			}
			HSV_to_RGB(currH,currS,tv,&cr,&cg,&cb);
			//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
			fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, cr, cg, cb, ca);
			if(!box_R.focus)
				sprintf(box_R.str,"%d",cr);
			if(!box_G.focus)
				sprintf(box_G.str,"%d",cg);
			if(!box_B.focus)
				sprintf(box_B.str,"%d",cb);
			if(!box_A.focus)
				sprintf(box_A.str,"%d",ca);
		}
		//inside color grid
		if (((mx >= grid_offset_x && my >= 5 && mx <= grid_offset_x+255 && my <= 5+255) || clickedSaturation) && !clickedHueBox)
		{
			mx = std::min(std::max(grid_offset_x, mx), grid_offset_x+255);
			my = std::min(std::max(5, my), 260);
			th = mx - grid_offset_x;
			th = (int)( th*359/255 );
			ts = 255 - (my - 5);
			if (b && !bq)
				clickedSaturation = true;
			if (b && clickedSaturation)
			{
				currH = th;
				currS = ts;
				HSV_to_RGB(th,ts,currV,&currR,&currG,&currB);
				deco_disablestuff = 1;
			}
			HSV_to_RGB(th,ts,currV,&cr,&cg,&cb);
			//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
			fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, cr, cg, cb, ca);
			//sprintf(box_R.def,"%d",cr);
			if(!box_R.focus)
				sprintf(box_R.str,"%d",cr);
			if(!box_G.focus)
				sprintf(box_G.str,"%d",cg);
			if(!box_B.focus)
				sprintf(box_B.str,"%d",cb);
			if(!box_A.focus)
				sprintf(box_A.str,"%d",ca);
		}
		//switch side button
		if(b && !bq && mx >= window_offset_x + onleft_button_offset_x +1 && my >= 2 +255+6 && mx <= window_offset_x + onleft_button_offset_x +13 && my <= 2 +255+5 +13)
		{
			on_left = !on_left;
			deco_disablestuff = 1;
		}
		//clear button
		if (b && !bq && mx >= window_offset_x+230 && my >= 2+255+6 && mx <= window_offset_x + 230+26 && my <= 2+255+5+13)
		{
			ConfirmPrompt *confirm = new ConfirmPrompt([](bool wasConfirmed) {
				if (wasConfirmed)
				{
					for (int i = 0; i < NPART; i++)
						parts[i].dcolour = COLARGB(0, 0, 0, 0);
				}
			}, "Reset Decoration Layer", "Do you really want to erase everything?", "Erase");
			Engine::Ref().ShowWindow(confirm);
		}
		if (b && !bq && mx > (on_left?143:502) && my > 264 && mx < (on_left?203:562) && my < 280)
		{
			char hex[32];
			sprintf(hex,"0x%.8X",(currA<<24)+(currR<<16)+(currG<<8)+currB);
			Engine::Ref().ClipboardPush(hex);
			the_game->SetInfoTip("Copied to clipboard");
		}
		deco_disablestuff = 1;
	}
	else if (mx > XRES || my > YRES)//mouse outside normal drawing area
	{
		//hide/show button
		if (b && !bq && mx >= 294 && mx <= 295+14 && my >= YRES+1 && my<= YRES+15)
		{
			decobox_hidden = !decobox_hidden;
			the_game->HideZoomWindow();
		}
	}
	if (!b)
	{
		clickedHueBox = clickedSaturation = false;
	}
#ifndef NOMOD
	if (sdl_key==SDLK_RIGHT)
		static_cast<ANIM_ElementDataContainer&>(*globalSim->elementData[PT_ANIM]).NewFrame(globalSim, sdl_mod & (KMOD_CTRL|KMOD_GUI));
	else if (sdl_key==SDLK_LEFT)
		static_cast<ANIM_ElementDataContainer&>(*globalSim->elementData[PT_ANIM]).PreviousFrame(globalSim);
	else if (sdl_key==SDLK_DELETE)
		static_cast<ANIM_ElementDataContainer&>(*globalSim->elementData[PT_ANIM]).DeleteFrame(globalSim);
#endif
	else if (sdl_key == SDLK_TAB)
	{
		if (box_R.focus)
		{
			box_R.focus = box_R.cursorstart = 0;
			decoration_textbox_color(&box_R, &cr, &currR);

			box_G.focus = 1;
			box_G.cursorstart = 0;
			box_G.cursor = strlen(box_G.str);
		}
		else if (box_G.focus)
		{
			box_G.focus = box_G.cursorstart = 0;
			decoration_textbox_color(&box_G, &cg, &currG);

			box_B.focus = 1;
			box_B.cursorstart = 0;
			box_B.cursor = strlen(box_B.str);
		}
		else if (box_B.focus)
		{
			box_B.focus = box_B.cursorstart = 0;
			decoration_textbox_color(&box_B, &cb, &currB);

			box_A.focus = 1;
			box_A.cursorstart = 0;
			box_A.cursor = strlen(box_A.str);
		}
		else if (box_A.focus)
		{
			box_A.focus = box_A.cursorstart = 0;
			decoration_textbox_color(&box_A, &ca, &currA);

			box_R.focus = 1;
			box_R.cursorstart = 0;
			box_R.cursor = strlen(box_R.str);
		}
	}
	if (the_game->ZoomWindowShown() || the_game->GetStampState() != PowderToy::NONE)
		decobox_hidden = 1;
	/*if(sdl_key=='b' || sdl_key==SDLK_ESCAPE)
	{
		/ *for (i = 0; i <= parts_lastActiveIndex; i++)
			if (parts[i].type == PT_ANIM)
			{
				parts[i].tmp2 = 0;
			}* /
		decocolor = (currA<<24)|PIXRGB(currR,currG,currB);
	}*/
	/*for (i = 0; i <= parts_lastActiveIndex; i++)
		if (parts[i].type == PT_ANIM)
		{
			parts[i].tmp2 = 0;
		}*/
	decocolor = COLARGB(currA, currR, currG, currB);
}
struct savelist_e;
typedef struct savelist_e savelist_e;
struct savelist_e {
	char *filename;
	char *name;
	pixel *image;
	savelist_e *next;
	savelist_e *prev;
};
savelist_e *get_local_saves(const char *folder, const char *search, int *results_ret)
{
	int results = 0;
	savelist_e *new_savelist = NULL;
	savelist_e *current_item = NULL;

	std::string searchTerm;
	if (search)
		searchTerm = search;
	std::vector<std::string> saves = Platform::DirectorySearch(folder, searchTerm, {".cps", ".stm"});

	std::sort(saves.rbegin(), saves.rend(), [](std::string a, std::string b) {
		std::transform(a.begin(), a.end(), a.begin(), ::tolower);
		std::transform(b.begin(), b.end(), b.begin(), ::tolower);
		return a > b;
	});

	for (std::string save : saves)
	{
		savelist_e *new_item = (savelist_e*)malloc(sizeof(savelist_e));
		new_item->filename = (char*)malloc(strlen(folder) + save.length() + 1);
		sprintf(new_item->filename, "%s%s", folder, save.c_str());
		new_item->name = mystrdup(save.c_str());
		new_item->image = NULL;
		new_item->next = NULL;
		if (new_savelist == NULL)
		{
			new_savelist = new_item;
			new_item->prev = NULL;
		}
		else
		{
			current_item->next = new_item;
			new_item->prev = current_item;
		}
		current_item = new_item;
		results++;
	}

	*results_ret = results;
	return new_savelist;
}

void free_saveslist(savelist_e *saves)
{
	if(!saves)
		return;
	if(saves->next!=NULL)
		free_saveslist(saves->next);
	if(saves->filename!=NULL)
		free(saves->filename);
	if(saves->name!=NULL)
		free(saves->name);
	if(saves->image!=NULL)
		free(saves->image);
}

int DoLocalSave(std::string savename, Save *save, bool force)
{
	// get saveData, may throw an exception
	const unsigned char *saveData = save->GetSaveData();
	unsigned int saveSize = save->GetSaveSize();
#ifdef WIN
	_mkdir(LOCAL_SAVE_DIR);
#else
	mkdir(LOCAL_SAVE_DIR, 0755);
#endif

	// add .cps extension if needed
	if (savename.length() < 4 || savename.substr(savename.length()-4) != ".cps")
		savename += ".cps";

	// Actual file goes into Saves/ folder
	std::stringstream filename;
	filename << LOCAL_SAVE_DIR << PATH_SEP << savename;

	if (!force && Platform::FileExists(filename.str()))
	{
		return -1;
	}

	FILE *f = fopen(filename.str().c_str(), "wb");
	if (f)
	{
		fwrite(saveData, saveSize, 1, f);
		fclose(f);

		strncpy(svf_filename, savename.c_str(), 255);
		svf_fileopen = 1;

		//Allow reloading
		the_game->SetReloadPoint(save);
		return 0;
	}
	else
	{
		return -2;
	}
}

int save_filename_ui(pixel *vid_buf, Save *save)
{
	const unsigned char *saveData = save->GetSaveData();
	unsigned int saveSize = save->GetSaveSize();
	int xsize = 16+(XRES/3);
	int ysize = 64+(YRES/3);
	float ca = 0;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int imgw, imgh;
	char *savefname = NULL;
	char *filename = NULL;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	pixel *save_data_image;
	pixel *saveImg = NULL;//calloc((XRES/3)*(YRES/3), PIXELSIZE);
	ui_edit ed;
	int ret = 1;

	save_data_image = prerender_save((void*)saveData, saveSize, &imgw, &imgh);
	if(save_data_image!=NULL)
	{
		saveImg = resample_img(save_data_image, imgw, imgh, XRES/3, YRES/3);
	}

	ui_edit_init(&ed, x0+11, y0+25, xsize-20, 0);
	strcpy(ed.def, "[filename]");
	ed.nx = 0;
	
	if (svf_fileopen)
	{
		char * dotloc = NULL;
		strncpy(ed.str, svf_filename, 255);
		if ((dotloc = strstr(ed.str, ".")))
		{
			dotloc[0] = 0;
		}
		ed.cursor = ed.cursorstart = strlen(ed.str);
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, (unsigned char*)save_to_disk_image, 0, 0, 0.7f);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-1, y0-1, xsize+3, ysize+3);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Filename:", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, xsize-16, 16, 255, 255, 255, 180);
		if(saveImg!=NULL)
		{
			draw_image(vid_buf, saveImg, x0+8, y0+40, XRES/3, YRES/3, 255);
		}
		drawrect(vid_buf, x0+8, y0+40, XRES/3, YRES/3, 192, 192, 192, 255);
		
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);
		fillrect(vid_buf, x0, y0+ysize-16, xsize, 16, 170, 170, 192, (int)ca);
		drawtext(vid_buf, x0+8, y0+ysize-12, "Save", 255, 255, 255, 255);

		ui_edit_draw(vid_buf, &ed);
		if (strlen(ed.str) || ed.focus)
			drawtext(vid_buf, x0+12+textwidth(ed.str), y0+25, ".cps", 240, 240, 255, 180);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, bq, &ed);
		
		if ((b && !bq && mx > x0 && mx < x0+xsize && my > y0+ysize-16 && my < y0+ysize) || sdl_key == SDLK_RETURN)
		{
			clean_text(ed.str,256);
			if (strlen(ed.str))
			{
				// TODO: this does nothing because we build the save data earlier to render it
				// can be fixed once rendering doesn't require building a save
				save->authors["title"] = ed.str;
				ret = DoLocalSave(ed.str, save);
				if (ret == -1)
				{
					if (confirm_ui(vid_buf, "A save with that name already exists.", ed.str, "Overwrite"))
						ret = DoLocalSave(ed.str, save, true);
				}
				if (ret == -2)
				{
					error_ui(vid_buf, 0, "Unable to write to save file.");
				}
				break;
			}
		}

		if (b && !bq && (mx < x0 || my < y0 || mx > x0+xsize || my > y0+ysize))
			break;
		if (sdl_key == SDLK_ESCAPE)
		{
			break;
		}
	}
	
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	free(save_data_image);
	free(old_vid);
	if (filename)
		free(filename);
	if (savefname)
		free(savefname);
	return ret;
}

void catalogue_ui(pixel * vid_buf)
{
	int xsize = 8+(XRES/CATALOGUE_S+8)*CATALOGUE_X;
	int ysize = 48+(YRES/CATALOGUE_S+20)*CATALOGUE_Y;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int rescount = 0, imageoncycle = 0, currentoffset = 0, thidden = 0, cactive = 0;
#ifdef TOUCHUI
	bool dragging = false;
	bool touchDragged2 = false;
	int initialMouseY = 0;
#endif
	int initialOffset = 0;
	bool touchDragged = false; // when true, ignore clicks on saves
	int listy = 0, listxc;
	int listx = 0, listyc;
	pixel * vid_buf2;
	float scrollvel = 0.0f, offsetf = 0.0f;
	char savetext[128] = "";
	char * last = mystrdup("");
	savelist_e *saves, *cssave, *csave;
	ui_edit ed;
	
	vid_buf2 = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!vid_buf2)
		return;

	ui_edit_init(&ed, x0+11, y0+29, xsize-20, 0);
	strcpy(ed.def, "[search]");
	ed.focus = 0;
	ed.nx = 0;

	saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, NULL, &rescount);
	cssave = csave = saves;
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		sprintf(savetext, "Found %d save%s", rescount, rescount==1?"":"s");
		clearrect(vid_buf, x0-1, y0-1, xsize+3, ysize+3);
		clearrect(vid_buf2, x0-1, y0-1, xsize+3, ysize+3);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Saves", 255, 216, 32, 255);
		drawtext(vid_buf, x0+xsize-8-textwidth(savetext), y0+8, savetext, 255, 216, 32, 255);
		drawrect(vid_buf, x0+8, y0+24, xsize-16, 16, 255, 255, 255, 180);
		if(strcmp(ed.str, last)){
			free(last);
			last = mystrdup(ed.str);
			if(saves!=NULL) free_saveslist(saves);
			saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, last, &rescount);
			cssave = saves;
			scrollvel = 0.0f;
			offsetf = 0.0f;
			thidden = 0;
		}
		//Scrolling
		if (sdl_wheel!=0)
		{
			scrollvel -= (float)sdl_wheel;
			if(scrollvel > 5.0f) scrollvel = 5.0f;
			if(scrollvel < -5.0f) scrollvel = -5.0f;
			sdl_wheel = 0;
		}
		offsetf += scrollvel;
		scrollvel *= 0.97f;
		if (std::abs(scrollvel) < .1f)
			scrollvel = 0.0f;

#ifdef TOUCHUI
		if (dragging)
		{
			if (!b)
			{
				dragging = false;
			}
			else
			{
				offsetf = initialOffset - my;
				if (std::abs(initialMouseY - my) > 10)
					touchDragged = true;
			}
		}
		// Delay a frame before setting this to false
		if (touchDragged2)
			touchDragged = touchDragged2 = false;
		if (touchDragged && !bq)
		{
			touchDragged2 = true;
		}
		if (b && !bq)
		{
			dragging = true;
			initialOffset = my + currentoffset;
			initialMouseY = my;
		}
#endif

		if(offsetf >= (YRES/CATALOGUE_S+20) && rescount)
		{
			if(rescount - thidden > CATALOGUE_X*(CATALOGUE_Y+1))
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->next==NULL)
						break;
					cssave = cssave->next;
				}
				offsetf -= (YRES/CATALOGUE_S+20);
				initialOffset -= (YRES/CATALOGUE_S+20);
				thidden += CATALOGUE_X;
			} else {
				offsetf = (YRES/CATALOGUE_S+20);
				//touchDragged = true;
			}
		}
		if(offsetf > 0.0f && rescount <= CATALOGUE_X*CATALOGUE_Y && rescount)
		{
			offsetf = 0.0f;
		}
		if(offsetf < 0.0f && rescount)
		{
			if(thidden >= CATALOGUE_X)
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->prev==NULL)
						break;
					cssave = cssave->prev;
				}
				offsetf += (YRES/CATALOGUE_S+20);
				initialOffset += (YRES/CATALOGUE_S+20);
				thidden -= CATALOGUE_X;
			} else {
				offsetf = 0.0f;
				//touchDragged = true;
			}
		}
		currentoffset = (int)offsetf;
		csave = cssave;

		//Diplay
		if(csave!=NULL && rescount)
		{
			listx = 0;
			listy = 0;
			while(csave!=NULL)
			{
				listxc = x0+8+listx*(XRES/CATALOGUE_S+8);
				listyc = y0+48-currentoffset+listy*(YRES/CATALOGUE_S+20);
				if(listyc > y0+ysize) //Stop when we get to the bottom of the viewable
					break;
				cactive = 0;
				if (!touchDragged && my > listyc && my < listyc+YRES/CATALOGUE_S+2 && mx > listxc && mx < listxc+XRES/CATALOGUE_S && my > y0+48 && my < y0+ysize)
				{
					if (!b && bq)
					{
						int size;
						char *data = (char*)file_load(csave->filename, &size);
						if (data)
						{
							SnapshotHistory::TakeSnapshot(globalSim);
							Save *localSave = new Save(data, size);
							bool success = false;
							try
							{
								clear_save_info();
								globalSim->LoadSave(0, 0, localSave, 1);
								strncpy(svf_filename, csave->name, 255);
								svf_fileopen = 1;
								authors = localSave->authors;
								the_game->SetReloadPoint(localSave);
								success = true;
							}
							catch (ParseException & e)
							{
								error_ui(vid_buf, 0, std::string("Unable to load save: ") + e.what());
							}
							free(data);
							delete localSave;
							if (success)
								goto openfin;
						} else {
							error_ui(vid_buf, 0, "Unable to read save file");
						}
					}
					cactive = 1;
				}
				//Generate an image
				if(csave->image==NULL && !imageoncycle){ //imageoncycle: Don't read/parse more than one image per cycle, makes it more resposive for slower computers
					int imgwidth, imgheight, size;
					pixel *tmpimage = NULL;
					void *data = NULL;
					data = file_load(csave->filename, &size);
					if(data!=NULL){
						tmpimage = prerender_save(data, size, &imgwidth, &imgheight);
						if(tmpimage!=NULL)
						{
							csave->image = resample_img(tmpimage, imgwidth, imgheight, XRES/CATALOGUE_S, YRES/CATALOGUE_S);
							free(tmpimage);
						} else {
							//Blank image, TODO: this should default to something else
							csave->image = (pixel*)calloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S), PIXELSIZE);
						}
						free(data);
					} else {
						//Blank image, TODO: this should default to something else
						csave->image = (pixel*)calloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S), PIXELSIZE);
					}
					imageoncycle = 1;
				}
				if(csave->image!=NULL)
					draw_image(vid_buf2, csave->image, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255);
				drawrect(vid_buf2, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255, 255, 255, 190);
				if(cactive)
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 255, 255, 255, 255);
				else
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 240, 240, 255, 180);
				if (mx>=listxc+XRES/GRID_S-4 && mx<=listxc+XRES/GRID_S+6 && my>=listyc-6 && my<=listyc+4)
				{
					if (!touchDragged && !b && bq && confirm_ui(vid_buf, "Do you want to delete?", csave->name, "Delete"))
					{
						remove(csave->filename);
						if(saves!=NULL) free_saveslist(saves);
						saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, last, &rescount);
						cssave = saves;
						scrollvel = 0.0f;
						offsetf = 0.0f;
						thidden = 0;
						if (rescount == 0)
							rmdir(LOCAL_SAVE_DIR PATH_SEP);
						break;
					}
					drawtext(vid_buf2, listxc+XRES/GRID_S-3, listyc-4, "\x85", 255, 48, 32, 255);
				}
				else
					drawtext(vid_buf2, listxc+XRES/GRID_S-3, listyc-4, "\x85", 160, 48, 32, 255);
				drawtext(vid_buf2, listxc+XRES/GRID_S-3, listyc-4, "\x86", 255, 255, 255, 255);
				csave = csave->next;
				if(++listx==CATALOGUE_X){
					listx = 0;
					listy++;
				}
			}
			imageoncycle = 0;
		} else {
			drawtext(vid_buf2, x0+(xsize/2)-(textwidth("No saves found")/2), y0+(ysize/2)+20, "No saves found", 255, 255, 255, 180);
		}
		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);
		//Draw the scrollable area onto the main buffer
		{
			pixel *srctemp = vid_buf2, *desttemp = vid_buf;
			int j = 0;
			for (j = y0+42; j < y0+ysize; j++)
			{
				memcpy(desttemp+j*(XRES+BARSIZE)+x0+1, srctemp+j*(XRES+BARSIZE)+x0+1, (xsize-1)*PIXELSIZE);
				//desttemp+=(XRES+BARSIZE);//*PIXELSIZE;
				//srctemp+=(XRES+BARSIZE);//*PIXELSIZE;
			}
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		if (sdl_key==SDLK_RETURN || sdl_key==SDLK_ESCAPE)
			break;
#ifndef TOUCHUI
		if (!b && bq && (mx < x0 || mx >= x0+xsize || my < y0 || my >= y0+ysize))
			break;
#endif
	}
openfin:
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	if(saves)
		free_saveslist(saves);
	free(vid_buf2);
	return;
}

void clear_save_info()
{
	svf_open = 0;
	svf_filename[0] = 0;
	svf_fileopen = 0;
	svf_publish = 0;
	svf_own = 0;
	svf_myvote = 0;
	svf_id[0] = 0;
	svf_version = "";
	svf_name[0] = 0;
	svf_description[0] = 0;
	svf_author[0] = 0;
	svf_tags[0] = 0;
	the_game->SetReloadPoint(NULL);
}
