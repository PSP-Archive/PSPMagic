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

# ifndef _PSP_GLOBAL_H_
# define _PSP_GLOBAL_H_

# define MAX_PATH 512

# define MAGIC_TOP_X     5
# define MAGIC_TOP_Y     5
# define MAGIC_WIDTH   470
# define MAGIC_HEIGHT  262

# define MAX_MAGIC_STEP  10

  typedef struct MAGIC_t {

    char           magic_board[MAGIC_HEIGHT][MAGIC_WIDTH];
    int            magic_x;
    int            magic_y;
    int            magic_step;
    unsigned short pen_color;
    unsigned short del_color;
    unsigned short sel_color;
    unsigned char  magic_mode;

    char edit_filename[MAX_PATH];
    unsigned char  ask_overwrite;

    int      magic_theme;
    int      psp_cpu_clock;
    int      psp_screenshot_mode;
    int      psp_screenshot_id;

  } MAGIC_t;

  extern char psp_home_dir[256];
  extern MAGIC_t MAGIC;
  extern int psp_exit_now;

  extern int psp_global_init();
  extern int psp_parse_configuration(void);
  extern int psp_save_configuration(void);
  extern void myCtrlPeekBufferPositive( SceCtrlData* pc, int count );

# endif
