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
#include <pspsircs.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspmodulemgr.h>
#include <png.h>

#include "psp_sdl.h"
#include "psp_global.h"
#include "psp_irda.h"
//
// States
#define MODE_WAIT_FRAME 0
#define MODE_STORE      1

static int loc_fd = -1;

static int frame_decode (u8 c);
static int frame_analyze ();

// Global variables that handle trimmers values and buttons states
static u16 loc_trimmerA;
static u16 loc_trimmerB;
static u8  loc_buttonsA;
static u8  loc_buttonsB;

#define MODE_WAIT_FRAME 0
#define MODE_STORE      1

#define FRAME_SIZE   5
static u8 mode=MODE_WAIT_FRAME;
static u8 cnt=0;
static u8 frame_buf[6];

static int 
frame_decode (u8 c) 
{
  switch (mode) {
  case MODE_WAIT_FRAME: // Waiting start of frame
  if (c == '#') {
    mode=MODE_STORE;
    cnt=0;
  }
  break;

  case MODE_STORE:
    frame_buf[cnt]=c;
    cnt++;
    if (cnt >= FRAME_SIZE - 1) { // End of frame. frame size without '#' (start tag)
      mode = MODE_WAIT_FRAME;
      u8 f0 = frame_buf[0];
      u8 f2 = frame_buf[2];
      u16 msbJA = (u16) (f0 & 0x03) << 8;
      u16 msbJB = (u16) (f2 & 0x03) << 8;
      loc_trimmerA = msbJA | frame_buf[1];
      loc_trimmerB = msbJB | frame_buf[3];
      if (f0 & 0x04) loc_buttonsA = 1;
      else loc_buttonsA = 0;
      if (f2 & 0x04) loc_buttonsB = 1;
      else loc_buttonsB = 0;

      return 1; // Frame is good
    }
    break;

  default:
    mode = MODE_WAIT_FRAME;
    break;
  }

  return 0; // Frame not completed yet
}


// return true if frame is completed and decoded
static int 
frame_analyze() 
{
  u8 data;
  int len = sceIoRead(loc_fd, &data, 1);
  if (len == 1) {
    return frame_decode (data);
  }
  return 0; // Nothing new
}

int 
psp_irda_get_joy_data( u16 *trimmerA, u16 *trimmerB, u8 *buttonsA, u8 *buttonsB )
{
# ifdef IRDA_JOY
  // check for IrDA input
  int status = frame_analyze ();
  if (status != 1) return 0;

  *trimmerA = loc_trimmerA;
  *buttonsA = loc_buttonsA;
  *trimmerB = loc_trimmerB;
  *buttonsB = loc_buttonsB;
# endif
  return 1;
}

int 
psp_irda_joy_init(void)
{
# ifdef IRDA_JOY
# ifdef PSPFW30X
  /* Load irda PRX for CFW >= 3.80 */
  u32 mod_id = sceKernelLoadModule("flash0:/kd/irda.prx", 0, NULL);
  sceKernelStartModule(mod_id, 0, NULL, NULL, NULL);
# endif
  loc_fd = sceIoOpen("irda0:", PSP_O_RDWR, 0);
# endif //IRDA_JOY
  return 0;
}

