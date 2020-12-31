/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <pspctrl.h>
#include <psptypes.h>
#include <png.h>
#include <zlib.h>

#include "psp_sdl.h"
#include "psp_global.h"
#include "psp_kbd.h"
#include "psp_play.h"
#include "psp_magic.h"
#include "psp_menu.h"
#include "psp_irda.h"

  typedef struct point_t {
    int x;
    int y;
  } point_t;

  static int board_dirty = 1;

  static int cursor_curr_id = 0;
  static point_t cursor_prev[2];

  static point_t board_point[2];
  static int     point_mode = 0;

  extern SDL_Surface *back_surface;

void
psp_magic_clear()
{
  cursor_curr_id = 0;
  point_mode = 0;
  board_dirty = 1;

  cursor_prev[0].x = MAGIC_TOP_X;
  cursor_prev[0].y = MAGIC_TOP_Y;
  cursor_prev[1].x = MAGIC_TOP_X;
  cursor_prev[1].y = MAGIC_TOP_Y;

  MAGIC.magic_x = MAGIC_WIDTH  / 2;
  MAGIC.magic_y = MAGIC_HEIGHT / 2;

  board_point[0].x = board_point[1].x = MAGIC.magic_x;
  board_point[0].y = board_point[1].y = MAGIC.magic_y;

  MAGIC.magic_mode = 1;
  MAGIC.magic_step = 4;

  memset(MAGIC.magic_board, 0, sizeof(MAGIC.magic_board));
}

void
psp_magic_cursor_on()
{
  int x;
  int y;

  MAGIC.del_color = psp_sdl_get_back3_color( PSP_SDL_SCREEN_WIDTH / 2, PSP_SDL_SCREEN_HEIGHT / 2 );

  x = cursor_prev[cursor_curr_id].x;
  y = cursor_prev[cursor_curr_id].y;
  u16* vram_buffer= psp_sdl_get_vram_addr(x, y);
  *vram_buffer = MAGIC.del_color;

  x = MAGIC.magic_x + MAGIC_TOP_X;
  y = MAGIC.magic_y + MAGIC_TOP_Y;
  vram_buffer  = psp_sdl_get_vram_addr(x, y);
  *vram_buffer = MAGIC.sel_color;

  cursor_prev[cursor_curr_id].x = x;
  cursor_prev[cursor_curr_id].y = y;
  cursor_curr_id = ! cursor_curr_id;
}

void
psp_magic_display_board()
{
  int x;
  int y;
  ushort *vram_buffer  = (ushort *)back_surface->pixels;
  char   *magic_buffer = &MAGIC.magic_board[0][0];

  MAGIC.del_color = psp_sdl_get_back3_color( PSP_SDL_SCREEN_WIDTH / 2, PSP_SDL_SCREEN_HEIGHT / 2 );

  vram_buffer  += MAGIC_TOP_X + (PSP_LINE_SIZE * MAGIC_TOP_Y);
  for (y = 0; y < MAGIC_HEIGHT; y++) {
    for (x = 0; x < MAGIC_WIDTH; x++) {
      if (*magic_buffer) {
        if (*magic_buffer == 1) *vram_buffer = MAGIC.pen_color;
        else {
          *vram_buffer = MAGIC.del_color;
          *magic_buffer = 0;
        }
      }
      magic_buffer++;
      vram_buffer++;
    }
    vram_buffer  += PSP_LINE_SIZE - MAGIC_WIDTH;
  }
}

void
psp_magic_left()
{
  MAGIC.magic_x--;
  if (MAGIC.magic_x < 0) MAGIC.magic_x = 0;
}

void
psp_magic_goto_x(int x)
{
  MAGIC.magic_x = x;
  if (MAGIC.magic_x < 0) MAGIC.magic_x = 0;
  if (MAGIC.magic_x >= MAGIC_WIDTH) MAGIC.magic_x = (MAGIC_WIDTH-1);
}

void
psp_magic_right()
{
  MAGIC.magic_x++;
  if (MAGIC.magic_x >= MAGIC_WIDTH) MAGIC.magic_x = (MAGIC_WIDTH-1);
}

void
psp_magic_goto_y(int y)
{
  MAGIC.magic_y = y;
  if (MAGIC.magic_y < 0) MAGIC.magic_y = 0;
  if (MAGIC.magic_y >= MAGIC_HEIGHT) MAGIC.magic_y = (MAGIC_HEIGHT-1);
}
void
psp_magic_up()
{
  MAGIC.magic_y--;
  if (MAGIC.magic_y < 0) MAGIC.magic_y = 0;
}

void
psp_magic_down()
{
  MAGIC.magic_y++;
  if (MAGIC.magic_y >= MAGIC_HEIGHT) MAGIC.magic_y = (MAGIC_HEIGHT-1);
}

void
psp_magic_line()
{
  char *videoram = &MAGIC.magic_board[0][0];
  int x1 = board_point[0].x;
  int y1 = board_point[0].y;
  int x2 = board_point[1].x;
  int y2 = board_point[1].y;

  int swap;
  int dx, dy;
  int ax, ay;
  int sy;
  int x, y;
  char *point;
  char *endpoint;
  char c;

  if (MAGIC.magic_mode == 1) c = 1;
  else                       c = 2;

  if (x1 > x2) {
    swap = x1; x1 = x2; x2 = swap;
    swap = y1; y1 = y2; y2 = swap;
  }
  dx = x2 - x1;
  dy = y2 - y1;
  ax = dx << 1;
  ay = abs(dy) << 1;
  sy = (dy >= 0) ? 1 : -1;
  x = x1;
  y = y1;

  point = videoram + x + y * MAGIC_WIDTH;
  endpoint = videoram + x2 + y2 * MAGIC_WIDTH;
  if (ax > ay) {
    int d = ay - (ax >> 1);
    int aa = ay;
    if (sy > 0) {
      while (point != endpoint) {
        *point = c;
        if (d >= 0) {
          point += MAGIC_WIDTH;
          d -= ax;
        }
        point++;
        d += aa;
      }
    }
    else {
      while (point != endpoint) {
        *point = c;
        if (d >= 0) {
          point -= MAGIC_WIDTH;
          d -= ax;
        }
        point++;
        d += aa;
      }
    }
  } else {
    int d = ax - (ay >> 1);
    int aa = ax;
    if (sy > 0) {
      while (point != endpoint) {
        *(point) = c;
        if (d > 0) {
          point++;
          d -= ay;
        }
        point += MAGIC_WIDTH;
        d += aa;
      }
    } else {
      while (point != endpoint) {
        *(point) = c;
        if (d > 0) {
          point++;
          d -= ay;
        }
        point -= MAGIC_WIDTH;
        d += aa;
      }
    }
  }
  *(point) = c;
}

void
psp_magic_point()
{
  int x = MAGIC.magic_x;
  int y = MAGIC.magic_y;
  if (point_mode) {
    board_point[0].x = board_point[1].x;
    board_point[0].y = board_point[1].y;
    board_point[1].x = x;
    board_point[1].y = y;
    psp_magic_line();

  } else {
    point_mode = 1; 
    board_point[0].x = x;
    board_point[0].y = y;
    board_point[1].x = x;
    board_point[1].y = y;
    if (MAGIC.magic_mode == 1) {
      MAGIC.magic_board[y][x] = 1;
    } else {
      MAGIC.magic_board[y][x] = 2;
    }
  }
}

void
psp_magic_inc_step()
{
  MAGIC.magic_step++;
  if (MAGIC.magic_step > MAX_MAGIC_STEP) MAGIC.magic_step = MAX_MAGIC_STEP;
}

void
psp_magic_dec_step()
{
  MAGIC.magic_step--;
  if (MAGIC.magic_step < 1) MAGIC.magic_step = 1;
}

# define ANALOG_THRESHOLD 30

void 
psp_magic_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

void
psp_magic_new()
{
  char tmp_filename[512];

  psp_magic_clear();

  MAGIC.ask_overwrite = 1;

  /* Find a new valid filename */
  int index = 0;
  while (index < 1000) {
    if (index) {
      sprintf(MAGIC.edit_filename, "newfile_%03d.mag", index);
    } else {
      strcpy(MAGIC.edit_filename, "newfile.mag");
    }
    strcpy(tmp_filename, "./mag/");
    strcat(tmp_filename, MAGIC.edit_filename);

    FILE* FileDesc = fopen(tmp_filename, "r");
    if (! FileDesc) break;
    fclose(FileDesc);
    index++;
  }
  int len = strlen(MAGIC.edit_filename);
  if (len > 4) MAGIC.edit_filename[len-4] = 0;
}

int
psp_magic_load(char* filename)
{
  psp_magic_clear();
  gzFile* a_file = 0;
  if ((a_file = gzopen(filename, "rb")) == NULL) return 1;

  char* scan = strrchr(filename, '/');
  if (scan) strcpy( MAGIC.edit_filename, scan+1);
  else      strcpy( MAGIC.edit_filename, filename);
  int len = strlen(MAGIC.edit_filename);
  if (len > 4) MAGIC.edit_filename[len-4] = 0;

  if (! gzread(a_file, &MAGIC.magic_x, sizeof(MAGIC.magic_x))) {
    gzclose(a_file);
    return 1;
  }
  if (! gzread(a_file, &MAGIC.magic_y, sizeof(MAGIC.magic_y))) {
    gzclose(a_file);
    return 1;
  }
  if (! gzread(a_file, &MAGIC.magic_board, sizeof(MAGIC.magic_board))) {
    gzclose(a_file);
    return 1;
  }
  gzclose(a_file);
  return 0;
}

int
psp_magic_save(char* filename)
{
  char tmp_filename[512];
  strcpy(tmp_filename, "./mag/");
  strcat(tmp_filename, filename);
  strcat(tmp_filename, ".mag");

  gzFile* a_file = 0;
  if ((a_file = gzopen(tmp_filename, "wb")) == NULL) return 1;

  if (! gzwrite(a_file, &MAGIC.magic_x, sizeof(MAGIC.magic_x))) {
    gzclose(a_file);
    return 1;
  }
  if (! gzwrite(a_file, &MAGIC.magic_y, sizeof(MAGIC.magic_y))) {
    gzclose(a_file);
    return 1;
  }
  if (! gzwrite(a_file, &MAGIC.magic_board, sizeof(MAGIC.magic_board))) {
    gzclose(a_file);
    return 1;
  }
  gzclose(a_file);
  return 0;
}


void
psp_magic_main_loop()
{
# ifdef IRDA_JOY
  u16   trimmerA = 0;
  u16   trimmerB = 0;
  u8    buttonsA = 0;
  u8    buttonsB = 0;
  u16   old_trimmerA = 0;
  u16   old_trimmerB = 0;
  u8    old_buttonsA = 0;
  u8    old_buttonsB = 0;
# endif
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;

  int new_Lx;
  int new_Ly;

  old_pad   = 0;
  last_time = 0;

  psp_main_menu();

  board_dirty = 2;

  while (1) {

    if (board_dirty == 2) {
      psp_sdl_blit_board();
      psp_magic_display_board();
      psp_sdl_flip();
      psp_sdl_blit_board();
    }
    if (board_dirty) {
      board_dirty = 0;
      psp_magic_display_board();
      psp_magic_cursor_on();
      psp_sdl_flip();

      myCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;
      last_time = c.TimeStamp;
    }

    if (MAGIC.psp_screenshot_mode) {
      MAGIC.psp_screenshot_mode = 0;

      psp_sdl_save_screenshot();
    }
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef IRDA_JOY
    /* Check IRDA Joy Device */
    if (psp_irda_get_joy_data( &trimmerA, &trimmerB, &buttonsA, &buttonsB )) {

      if (old_trimmerA != trimmerA) {
        old_trimmerA = trimmerA;
        psp_magic_goto_x( trimmerA );
        board_dirty = 1;
      }
      if (old_trimmerB != trimmerB) {
        old_trimmerB = trimmerB;
        psp_magic_goto_y( trimmerB );
        board_dirty = 1;
      }
      if (old_buttonsA != buttonsA ) {
        old_buttonsA = buttonsA;
        if (buttonsA) {
          if (MAGIC.magic_mode == 1) MAGIC.magic_mode = 0;
          else                       MAGIC.magic_mode = 1;
        }
      }
      if (old_buttonsB != buttonsB ) {
        old_buttonsB = buttonsB;
        if (buttonsB) {
          if (MAGIC.magic_mode == 2) MAGIC.magic_mode = 0;
          else                       MAGIC.magic_mode = 2;
        }
      }
      if (MAGIC.magic_mode) {
        psp_magic_point();
        board_dirty = 1;
      } else {
        point_mode = 0;
      }
    }
# endif

    /* Check Analog Device */
    psp_magic_get_analog_direction( c.Lx, c.Ly, &new_Lx, &new_Ly);
    new_pad = c.Buttons & PSP_ALL_BUTTON_MASK ;

    if (new_pad) {
      if ((c.TimeStamp - last_time) > (PSP_MIN_TIME_EVENT >> MAGIC.magic_step)) 
      {
        last_time = c.TimeStamp;
        old_pad = new_pad;
      } else continue;

    } else {
      if ((c.TimeStamp - last_time) > (PSP_MIN_TIME_EVENT >> MAGIC.magic_step)) {
        last_time = c.TimeStamp;
      } else continue;
    }

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if((new_pad & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) {
      psp_main_menu();
      board_dirty = 2;
    } else {

      if (new_pad & PSP_CTRL_CIRCLE) {
        psp_magic_inc_step();
        psp_kbd_wait_no_button();
      }
      if (new_pad & PSP_CTRL_SQUARE) {
        psp_magic_dec_step();
        psp_kbd_wait_no_button();
      }
      if ((new_pad & PSP_CTRL_LEFT) || (new_Lx < 0)) {
        psp_magic_left();
        board_dirty = 1;
      }
      if ((new_pad & PSP_CTRL_RIGHT) || (new_Lx > 0)) {
        psp_magic_right();
        board_dirty = 1;
      }
      if ((new_pad & PSP_CTRL_UP) || (new_Ly < 0)) {
        psp_magic_up();
        board_dirty = 1;
      }
      if ((new_pad & PSP_CTRL_DOWN) || (new_Ly > 0)) {
        psp_magic_down();
        board_dirty = 1;
      }
      if (new_pad & PSP_CTRL_TRIANGLE) {
        if (MAGIC.magic_mode == 2) MAGIC.magic_mode = 0;
        else                       MAGIC.magic_mode = 2;
        psp_kbd_wait_no_button();
      } else
      if (new_pad & PSP_CTRL_CROSS) {
        if (MAGIC.magic_mode) MAGIC.magic_mode = 0;
        else                  MAGIC.magic_mode = 1;
        psp_kbd_wait_no_button();
      }

      if (MAGIC.magic_mode) {
        psp_magic_point();
        board_dirty = 1;
      } else {
        point_mode = 0;
      }
    }
  }
}

int 
SDL_main(int argc,char *argv[])
{
  getcwd( psp_home_dir, sizeof(psp_home_dir));

  psp_sdl_init();
  psp_global_init();

  psp_magic_main_loop();

  psp_sdl_exit(0);

  return 0;
}

