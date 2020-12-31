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
#include <zlib.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psp_global.h"
#include "psp_sdl.h"

  MAGIC_t MAGIC;

  int psp_exit_now = 0;

  char psp_home_dir[256];

void
myCtrlPeekBufferPositive( SceCtrlData* pc, int count )
{
  if (psp_exit_now) psp_sdl_exit(0);
  sceCtrlPeekBufferPositive( pc, count );
}

int
psp_global_init()
{
  memset(&MAGIC, sizeof(MAGIC_t), 0);

  MAGIC.psp_screenshot_mode = 0;
  MAGIC.psp_cpu_clock = 133;
  MAGIC.pen_color = psp_sdl_rgb(  0x0,  0x0,  0x0 );
  MAGIC.sel_color = psp_sdl_rgb( 0xff, 0xff, 0xff );

  MAGIC.magic_mode = 1;

  psp_magic_new();

  psp_parse_configuration();
 
  scePowerSetClockFrequency(MAGIC.psp_cpu_clock, MAGIC.psp_cpu_clock, MAGIC.psp_cpu_clock/2);

  return 0;
}

int
psp_save_configuration(void)
{
  char  FileName[MAX_PATH];
  FILE* FileDesc;
  int   error = 0;

  strcpy(FileName, "./pspmagic.cfg");

  FileDesc = fopen(FileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n", MAGIC.psp_cpu_clock);
    fprintf(FileDesc, "magic_theme=%d\n", MAGIC.magic_theme);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
psp_parse_configuration(void)
{
  char  FileName[MAX_PATH + 1];
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  strcpy(FileName, "./pspmagic.cfg");

  FileDesc = fopen(FileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock")) MAGIC.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"magic_theme")) MAGIC.magic_theme = Value;
  }

  fclose(FileDesc);

  return 0;
}


