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
#include <pspthreadman.h>

#include <pspwlan.h>
#include <pspkernel.h>
#include <psppower.h>

#include <stdlib.h>
#include <stdio.h>

#include "psp_global.h"
#include "psp_play.h"

void
psp_play_init_all(int reset_only)
{
}

void
psp_play_init()
{
  psp_play_init_all(0);
}

void
psp_play_reset()
{
  psp_play_init_all(1);
}

static int
loc_psp_play_is_legal_move(int current_index)
{
  int index       = 0;
  int last_index  = 0;
  int move_stones = MANCALA.board_array[current_index];

  if (! move_stones) return 0;

  if (MANCALA.to_move == MANCALA_SOUTH) {
    
    for (index = MANCALA_WIDTH; index < MANCALA_NUM_PITS; index++) {
      if (MANCALA.board_array[index] != 0) break;
    }
    /* Adversary is empty, valid moves should put seeds on his side ! */
    if (index == MANCALA_NUM_PITS) {

      last_index = (current_index + move_stones) % MANCALA_NUM_PITS;
      if ((last_index >= MANCALA_WIDTH) && (last_index < MANCALA_NUM_PITS)) return 1;

      return 0;
    }

  } else {

    for (index = 0; index < MANCALA_WIDTH; index++) {
      if (MANCALA.board_array[index] != 0) break;
    }
    /* Adversary is empty, valid moves should put seeds on his side ! */
    if (index == MANCALA_WIDTH) {

      last_index = (current_index + move_stones) % MANCALA_NUM_PITS;
      if ((last_index >= 0) && (last_index < MANCALA_WIDTH)) return 1;

      return 0;
    }
  }

  return 1;
  
}

static int
loc_psp_play_get_current_index(int current_x)
{
  int current_index = 0;

  if (MANCALA.to_move == MANCALA_NORTH) {
    current_index = (MANCALA_NUM_PITS - 1) - current_x;
  } else {
    current_index = current_x;
  }
  return current_index;
}

static int
loc_psp_play_move_awele()
{
  int current_index = 0;
  int move_stones   = 0;
  int last_index    = 0;
  int min_index     = 0;
  int max_index     = 0;
  int number_stones = 0;
  int current_x     = 0;

  if (MANCALA.to_move == MANCALA_NORTH) {
    min_index = 0;
    max_index = MANCALA_WIDTH;
  } else {
    min_index = MANCALA_WIDTH;
    max_index = MANCALA_NUM_PITS;
  }

  current_x     = MANCALA.current_x[MANCALA.to_move];
  current_index = loc_psp_play_get_current_index(current_x);

  if (! loc_psp_play_is_legal_move(current_index)) return 0;

  move_stones = MANCALA.board_array[current_index];

  MANCALA.board_array[current_index] = 0;
  last_index    = (current_index + 1) % MANCALA_NUM_PITS;
  number_stones = move_stones;

  while (number_stones > 0) {
    MANCALA.board_array[last_index]++;
    number_stones--;
    if (number_stones) {
      last_index = (last_index + 1) % MANCALA_NUM_PITS;
      if (last_index == current_index) {
        last_index = (last_index + 1) % MANCALA_NUM_PITS;
      }
    }
    /* Animate */
    psp_mancala_refresh_screen();
    psp_sdl_flip();
    sceKernelDelayThread(PLAY_ANIMATE_TIME);
  }

  /* Compute score */
  while ((last_index >= min_index) && (last_index < max_index)) {

    number_stones = MANCALA.board_array[last_index];

    if ((number_stones == 2) || (number_stones == 3)) {
      MANCALA.score[MANCALA.to_move] += number_stones;
      MANCALA.board_array[last_index] = 0;
      /* animate ! */
      psp_mancala_refresh_screen();
      psp_sdl_flip();
      sceKernelDelayThread(PLAY_ANIMATE_TIME);

    } else break;

    last_index--;
  }

  if (MANCALA.to_move == MANCALA_NORTH) MANCALA.to_move = MANCALA_SOUTH;
  else                                  MANCALA.to_move = MANCALA_NORTH;

  return 1;
}

static int
loc_psp_play_move_mancala()
{
  int current_index   = 0;
  int move_stones     = 0;
  int last_index      = 0;
  int min_index       = 0;
  int max_index       = 0;
  int mancala_index   = 0;
  int number_stones   = 0;
  int opposite_index  = 0;
  int opposite_stones = 0;
  int current_x       = 0;
  int same_player     = 0;

  if (MANCALA.to_move == MANCALA_NORTH) {
    min_index = MANCALA_WIDTH;
    max_index = MANCALA_NUM_PITS;
    mancala_index = 0;
  } else {
    min_index = 0;
    max_index = MANCALA_WIDTH;
    mancala_index = MANCALA_WIDTH;
  }

  current_x     = MANCALA.current_x[MANCALA.to_move];
  current_index = loc_psp_play_get_current_index(current_x);

  if (! loc_psp_play_is_legal_move(current_index)) return 0;

  move_stones = MANCALA.board_array[current_index];
  MANCALA.board_array[current_index] = 0;

  last_index    = (current_index + 1) % MANCALA_NUM_PITS;
  number_stones = move_stones;

  while (number_stones > 0) {

    if (last_index == mancala_index) {
      MANCALA.score[MANCALA.to_move]++;
      number_stones--;
      if (! number_stones) {
        same_player = 1;
        break;
      }
    }

    MANCALA.board_array[last_index]++;
    number_stones--;
    if (number_stones) {
      last_index = (last_index + 1) % MANCALA_NUM_PITS;
    }
    /* Animate */
    psp_mancala_refresh_screen();
    psp_sdl_flip();
    sceKernelDelayThread(PLAY_ANIMATE_TIME);
  }

  /* Compute score */
  number_stones = MANCALA.board_array[last_index];

  if ((last_index >= min_index) && (last_index < max_index)) {

    if (number_stones == 1) {

      opposite_index = (MANCALA_NUM_PITS - 1) - last_index;
      opposite_stones = MANCALA.board_array[opposite_index];

      if (opposite_stones) {

        MANCALA.score[MANCALA.to_move] += number_stones;
        MANCALA.board_array[last_index] = 0;
        /* animate ! */
        psp_mancala_refresh_screen();
        psp_sdl_flip();
        sceKernelDelayThread(PLAY_ANIMATE_TIME);

        MANCALA.score[MANCALA.to_move] += opposite_stones;
        MANCALA.board_array[opposite_index] = 0;
        /* animate ! */
        psp_mancala_refresh_screen();
        psp_sdl_flip();
        sceKernelDelayThread(PLAY_ANIMATE_TIME);
      }
    }
  }

  if (! same_player) {
    if (MANCALA.to_move == MANCALA_NORTH) MANCALA.to_move = MANCALA_SOUTH;
    else                                  MANCALA.to_move = MANCALA_NORTH;
  }

  return 1;
}

static int
loc_psp_play_move()
{
  int move_ok = 0;

  if (MANCALA.mancala_game_rules == MANCALA_AWARI) {
    move_ok = loc_psp_play_move_awele();
  } else {
    move_ok = loc_psp_play_move_mancala();
  }

  return move_ok;
}

int
psp_play_computer_move()
{
  int ai_move = 0;

  if (psp_play_is_game_over()) return 0;

	sceKernelDelayThread(1500000);

  ai_move = psp_ai_get_best_move();
  MANCALA.current_x[MANCALA.to_move] = ai_move;

  while (! loc_psp_play_move()) {
    ai_move = (ai_move + 1) % MANCALA_WIDTH;
    MANCALA.current_x[MANCALA.to_move] = ai_move;
  }
  return 1;
}

int
psp_play_player_move()
{
  if (psp_play_is_game_over()) return 0;
	if (! loc_psp_play_move())  return 0;

  return 1;
}

int
psp_play_is_game_over()
{
  int index;
  int current_index;

  if (MANCALA.mancala_game_over) return 1;

  /* We check if there is a solution ! */
  for (index  = 0; index < MANCALA_WIDTH; index++) {
    current_index = loc_psp_play_get_current_index( index );
    if (loc_psp_play_is_legal_move(current_index)) break;
  }
  if (index == MANCALA_WIDTH) return 1;

  return 0;
}

void
psp_play_compute_final_score()
{
  int index;
  int seeds_north = 0;
  int seeds_south = 0;

  for (index  = 0; index < MANCALA_WIDTH; index++) {
    seeds_south += MANCALA.board_array[index];
  }
  for (index = MANCALA_WIDTH; index < MANCALA_NUM_PITS; index++) {
    seeds_north += MANCALA.board_array[index];
  }

  if (seeds_north && seeds_south) {
    MANCALA.score[MANCALA_NORTH] += seeds_north;
    MANCALA.score[MANCALA_SOUTH] += seeds_south;
  } else {
    if (seeds_north) {
      MANCALA.score[MANCALA_SOUTH] += seeds_north;
    } else {
      MANCALA.score[MANCALA_NORTH] += seeds_south;
    }
  }
}
