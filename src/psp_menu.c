/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspiofilemgr.h>

#include "psp_global.h"
#include "psp_sdl.h"
#include "psp_fmgr.h"
#include "psp_menu.h"
#include "psp_battery.h"
#include "psp_menu_help.h"
#include "psp_kbd.h"
#include "psp_danzeff.h"
#include "psp_magic.h"

# define FILENAME_FIELD_WIDTH  36

extern SDL_Surface *back_surface;

# define MENU_FILE         0
# define MENU_CLOCK        1
# define MENU_STEP         2
# define MENU_MODE         3
# define MENU_THEME        4
# define MENU_SCREENSHOT   5
# define MENU_HELP         6
# define MENU_CLEAR        7
# define MENU_NEW          8
# define MENU_LOAD         9
# define MENU_SAVE        10
# define MENU_BACK        11
# define MENU_EXIT        12

# define MAX_MENU_ITEM (MENU_EXIT + 1)

  static menu_item_t menu_list[] =
  { 
    { "File  : " },
    { "Clock : " },
    { "Step  : " },
    { "Mode  : " },
    { "Theme : " },
    { "Image : " },

    { "Help" },

    { "Clear" },
    { "New" },
    { "Load" },
    { "Save" },
    { "Back" },
    { "Exit" },
  };

  static int cur_menu_id = MENU_BACK;

  static int psp_cpu_clock = 133;
  static int magic_step    = 4;
  static int magic_mode    = 0;
  static int magic_theme   = 0;

void
string_fill_with_space(char *buffer, int size)
{
  int length = strlen(buffer);
  int index;

  for (index = length; index < size; index++) {
    buffer[index] = ' ';
  }
  buffer[size] = 0;
}

void
psp_display_screen_menu_battery(void)
{
  char buffer[64];

  int Length;
  int color;

  snprintf(buffer, 50, " [%s] ", psp_get_battery_string());
  Length = strlen(buffer);

  if (psp_is_low_battery()) color = PSP_MENU_RED_COLOR;
  else                      color = PSP_MENU_GREEN_COLOR;

  psp_sdl_back2_print(240 - ((8*Length) / 2), 6, buffer, color);
}

static void 
psp_display_screen_menu(void)
{
  char buffer[512];
  int menu_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  psp_sdl_blit_background();

  x      = 60;
  y      = 55;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;
    else 
    if (menu_id == MENU_EXIT) color = PSP_MENU_RED_COLOR;
    else
    if (menu_id == MENU_HELP) color = PSP_MENU_GREEN_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_CLOCK) {
      sprintf(buffer,"%d", psp_cpu_clock);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_STEP) {
      sprintf(buffer,"%d", magic_step);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_THEME) {
      if (magic_theme == 0) strcpy( buffer, "silver");
      else                  strcpy( buffer, "gold");
      string_fill_with_space(buffer, 7);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_MODE) {
      if (magic_mode == 0) strcpy( buffer, "off");
      else
      if (magic_mode == 1) strcpy( buffer, "draw");
      else
      if (magic_mode == 2) strcpy( buffer, "erase");
      string_fill_with_space(buffer, 6);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_SCREENSHOT) {
      sprintf(buffer,"%d", MAGIC.psp_screenshot_id);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
      y += y_step;
    } else
    if (menu_id == MENU_SAVE) {
      //y += y_step;
    } else
    if (menu_id == MENU_FILE) {
      int length = strlen(MAGIC.edit_filename);
      int first = 0;
      if (length > (FILENAME_FIELD_WIDTH-1)) {
        first = length - (FILENAME_FIELD_WIDTH-1);
      }
      sprintf(buffer, "%s", MAGIC.edit_filename + first);
      if (menu_id == cur_menu_id) strcat(buffer, "_");
      string_fill_with_space(buffer, FILENAME_FIELD_WIDTH+1);
      psp_sdl_back2_print( 120, y, buffer, color);

    } else
    if (menu_id == MENU_HELP) {
      //y += y_step;
    } else
    if (menu_id == MENU_BACK) {
      y += y_step;
    } else
    if (menu_id == MENU_EXIT) {
      y += y_step;
    }
    y += y_step;
  }
}

static void
psp_main_menu_validate(void)
{
  MAGIC.psp_cpu_clock = psp_cpu_clock;
  MAGIC.magic_step    = magic_step;
  MAGIC.magic_mode    = magic_mode;
  MAGIC.magic_theme   = magic_theme;
  scePowerSetClockFrequency(MAGIC.psp_cpu_clock, MAGIC.psp_cpu_clock, MAGIC.psp_cpu_clock/2);
}

void
psp_main_menu_filename_add_char(int a_char)
{
  int length = strlen(MAGIC.edit_filename);
  if (length < (MAX_PATH-2)) {
    MAGIC.edit_filename[length] = a_char;
    MAGIC.edit_filename[length+1] = 0;
  }
}

void
psp_main_menu_filename_clear()
{
  MAGIC.edit_filename[0] = 0;
}

void
psp_main_menu_filename_del()
{
  int length = strlen(MAGIC.edit_filename);
  if (length >= 1) {
    MAGIC.edit_filename[length - 1] = 0;
  }
}

int
psp_main_menu_confirm_overwrite()
{
  SceCtrlData c;

  if (MAGIC.ask_overwrite) {

    struct stat aStat;
    if (stat(MAGIC.edit_filename, &aStat)) {
      MAGIC.ask_overwrite = 0;
      return 1;
    }

    psp_display_screen_menu();
    psp_sdl_back2_print(200, 70, "X to overwrite ? [] to cancel", PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    psp_kbd_wait_no_button();
    do
    {
      sceCtrlReadBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons & PSP_CTRL_CROSS) {
        return 1;
      } else 
      if (c.Buttons & PSP_CTRL_SQUARE) {
        return 0;
      }

    } while (c.Buttons == 0);
    psp_kbd_wait_no_button();
  }
  return 0;
}

static void
psp_main_menu_save()
{
  int error;

  psp_main_menu_validate();
  psp_save_configuration();

  error = psp_magic_save( MAGIC.edit_filename);

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(200, 170, "File saved !", PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(200, 170, "Can't save file !", PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_screenshot(void)
{
  MAGIC.psp_screenshot_mode = 1;
}

int
psp_main_menu_exit(void)
{
  SceCtrlData c;

  psp_display_screen_menu();
  psp_sdl_back2_print(200, 170, "press X to confirm !", PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_CROSS) psp_sdl_exit(0);

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

static void
psp_main_menu_init(void)
{
  psp_cpu_clock = MAGIC.psp_cpu_clock;
  magic_step    = MAGIC.magic_step;
  magic_mode    = MAGIC.magic_mode;
  magic_theme   = MAGIC.magic_theme;
}

#define MAX_CLOCK_VALUES 5
static int clock_values[MAX_CLOCK_VALUES] = { 133, 222, 266, 300, 333 };

static void
psp_main_menu_clock(int step)
{
  int index;
  for (index = 0; index < MAX_CLOCK_VALUES; index++) {
    if (psp_cpu_clock == clock_values[index]) break;
  }
  if (step > 0) {
    index++;
    if (index >= MAX_CLOCK_VALUES) index = 0;
    psp_cpu_clock = clock_values[index];

  } else {
    index--;

    if (index < 0) index = MAX_CLOCK_VALUES - 1;
    psp_cpu_clock = clock_values[index];
  }
}

static void
psp_main_menu_step(int step)
{
  if (step > 0) {
    magic_step++;
    if (magic_step > MAX_MAGIC_STEP) magic_step = MAX_MAGIC_STEP;
  } else {
    magic_step--;
    if (magic_step < 1) magic_step = 1;
  }
}

static void
psp_main_menu_mode(int mode)
{
  if (mode > 0) {
    magic_mode++;
    if (magic_mode > 2) magic_mode = 2;
  } else {
    magic_mode--;
    if (magic_mode < 0) magic_mode = 0;
  }
}

int
psp_main_menu_load(void)
{
  int ret;

  ret = psp_fmgr_menu(PSP_FMGR_FORMAT_MAG);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(200, 170, "File loaded !", PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
    return 1;
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(200, 170, "Can't load file !", PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  return 0;
}

int 
psp_main_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  int         danzeff_mode = 0;
  int         danzeff_key;

  int         step = 0;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_main_menu_init();

  while (! end_menu)
  {
    psp_display_screen_menu();

    if (danzeff_mode) {
      danzeff_moveTo(-75, -65);
      danzeff_render();
    }
    psp_sdl_flip();

    while (1)
    {
      myCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      new_pad = c.Buttons;

      if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
        last_time = c.TimeStamp;
        old_pad = new_pad;
        break;
      }
    }

    if (danzeff_mode) {

      danzeff_key = danzeff_readInput(c);
      if (danzeff_key > DANZEFF_START) {
        if (danzeff_key > ' ') {
          psp_main_menu_filename_add_char(danzeff_key);
        } else
        if (danzeff_key == DANZEFF_RESET) {
          psp_main_menu_filename_clear();
        } else
        if (danzeff_key == DANZEFF_DEL) {
          psp_main_menu_filename_del();
        }

      } else 
      if ((danzeff_key == DANZEFF_START ) || 
          (danzeff_key == DANZEFF_SELECT)) 
      {
        danzeff_mode = 0;
        old_pad = new_pad = 0;

        psp_kbd_wait_no_button();
      }
      continue;
    }

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if(new_pad & PSP_CTRL_START) {
      danzeff_mode = 1;
    } else
    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT)) == (PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT)) {
      /* Ir shell ? */
      sceKernelDelayThread(100);
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      end_menu = 1;
    } else
    if ((new_pad & PSP_CTRL_CROSS ) || 
        (new_pad & PSP_CTRL_CIRCLE) || 
        (new_pad & PSP_CTRL_RIGHT ) ||
        (new_pad & PSP_CTRL_LEFT  )) 
    {
      if (new_pad & PSP_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & PSP_CTRL_RIGHT) step =  1;
      else                          step =  0;

      switch (cur_menu_id ) 
      {
        case MENU_CLOCK        : psp_main_menu_clock( step );
        break;
        case MENU_STEP         : psp_main_menu_step( step );
        break;
        case MENU_THEME        : magic_theme = ! magic_theme;
        break;
        case MENU_MODE         : psp_main_menu_mode( step );
        break;
        case MENU_SCREENSHOT   : psp_main_menu_screenshot();
                                 end_menu = 1;
        break;              

        case MENU_SAVE         : psp_main_menu_save();
        break;               
        case MENU_LOAD         : if (psp_main_menu_load()) {
                                   end_menu = 1;
                                 }
        break;              
        case MENU_NEW          : psp_magic_new();
                                 end_menu = 1;
        break;                 
        case MENU_CLEAR        : psp_magic_clear();
                                 end_menu = 1;
        break;                 
        case MENU_BACK         : end_menu = 1;
        break;                 
        case MENU_EXIT         : psp_main_menu_exit();
        break;                 
        case MENU_HELP         : psp_help_menu();
                                 old_pad = new_pad = 0;
        break;              
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_ITEM-1)) cur_menu_id++;
      else                                 cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if((new_pad & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) {
      /* Back to CPC */
      end_menu = 1;
    }
  }

  if (end_menu > 0) {
    psp_main_menu_validate();
  }
  psp_kbd_wait_no_button();

  return 1;
}

