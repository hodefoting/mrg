/*
 * Copyright (c) 2014 Øyvind Kolås <pippin@hodefoting.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg.h"

char *todo =
". implement a multi-line text editor,. it is good to really be able to browse\n"
"· auto scroll\n"
"· keyboard focus is broken in vectors test, scaling issue!\n"
"· fixing of small cursor bugs\n"
"· even more immediate-style inline callbacks in C?\n"
"· drawing of boxes, perhaps with final bottom on mrg_restore, if a box has been set to be begun with a callback earlier.\n"
"· make backends be dynamically loaded .so's - or is cairo bulky enough that we just link?\n"
" implement texture raster overlay + openlgl/sdl example\n"
"· implement flow from box to box... possible with\n"
"  · callback for right/margin / carriage return\n"
"  · which then can use algorithm, or list of rectangles to use with callback\n"
"  · which might be in conflict with \n"
"  · shift-tab is not recognized keybinding in sdl/gtk\n"
;

static int drag_pos (MrgEvent *e, void *data1, void *data2)
{
  if (e->type == MRG_DRAG_MOTION)
  {
    float *pos = data1;
    pos[0] += e->delta_x;
    pos[1] += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }
  else if (e->type == MRG_ENTER)
  {
    int *active = data2;
    *active = 1;
    mrg_queue_draw (e->mrg, NULL);
  }
  else if (e->type == MRG_LEAVE)
  {
    int *active = data2;
    *active = 0;
    mrg_queue_draw (e->mrg, NULL);
  }
  return 0;
}


static void render_ui (Mrg *mrg, void *data)
{
  {
    static float pos[2] = {10, 200};
    static int active = 0;

    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
    mrg_set_style (mrg, "color:red");
    else
    mrg_set_style (mrg, "color:green");

    mrg_print (mrg, todo);
    mrg_text_listen_done (mrg);
  }

  {
    static float pos[2] = {100, 100};
    int active = 0;

    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
    mrg_set_style (mrg, "color:red");
    else
    mrg_set_style (mrg, "color:green");

    mrg_print (mrg, todo);
    mrg_text_listen_done (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (640, 480, NULL);
  mrg_set_ui (mrg, render_ui, NULL);
  mrg_main (mrg);
  return 0;
}
