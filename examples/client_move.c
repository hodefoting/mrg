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

static void drag_pos (MrgEvent *e, void *data1, void *data2)
{
  //static float start_x = 0;
  //static float start_y = 0;
  int start_pos_x = 0;
  int start_pos_y = 0;
  if (e->type == MRG_DRAG_MOTION)
  {
    int x, y;
    mrg_get_position (e->mrg, &start_pos_x, &start_pos_y);
    //x = start_pos_x + 1;
    //fprintf (stderr, "%f\n", e->delta_x);
    //y = start_pos_y;
    x = start_pos_x + (e->delta_x);
    y = start_pos_y + (e->delta_y);

    //e->x -= e->delta_x;
    //e->y -= e->delta_y;
    //start_pos_x = x;
    //start_pos_y = y;
    //start_x = e->device_x;
    //start_y = e->device_y;
    mrg_set_position (e->mrg, x, y);
    mrg_queue_draw (e->mrg, NULL);
  }
}

static void render_ui (Mrg *mrg, void *data)
{
  mrg_print (mrg, "drag this window around");

  cairo_rectangle (mrg_cr (mrg), 0,0,mrg_width(mrg),mrg_height(mrg));
  mrg_listen (mrg, MRG_DRAG, drag_pos, NULL, NULL);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (320, 200, NULL);
  mrg_set_ui (mrg, render_ui, NULL);
  mrg_main (mrg);
  return 0;
}
