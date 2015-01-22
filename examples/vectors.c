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

int drag_cb (MrgEvent *event, void *data1, void *data2)
{
  if (event->type == MRG_DRAG_MOTION)
  {
    float *pos = data1;
    pos[0] += event->delta_x;
    pos[1] += event->delta_y;
    mrg_queue_draw (event->mrg, NULL);
  }
  return 1;
}

float coord[13][2] = 
{{0,    0.5}, //M
 {0.0,  0.25},  //C
 {0.25, 0.0},
 {0.5,  0.0},
 {0.75, 0.0}, //C
 {1.0,  0.25},  
 {1.0,  0.5},
 {1.0,  0.75}, //C
 {0.75, 1.0},
 {0.5, 1.0},
 {0.25,  1.0}, //C
 {0.0, 0.75},
 {0, 0.5},

};

static void ui (Mrg *mrg, void *data)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
#if 1
  if (mrg_width (mrg) < mrg_height (mrg))
  {
    cairo_translate (cr, 0, (mrg_width (mrg) - mrg_height (mrg))/2);
    cairo_scale (cr, mrg_width (mrg), mrg_width  (mrg));
  }
  else
  {
    cairo_translate (cr, (mrg_width (mrg) - mrg_height (mrg))/2, 0);
    cairo_scale (cr, mrg_height (mrg), mrg_height (mrg));
  }
#endif

  cairo_set_line_width (cr, 0.02);
  cairo_move_to  (cr, coord[0][0], coord[0][1]);
  cairo_curve_to (cr, coord[1][0], coord[1][1],
                      coord[2][0], coord[2][1],
                      coord[3][0], coord[3][1]);
  cairo_curve_to (cr, coord[4][0], coord[4][1],
                      coord[5][0], coord[5][1],
                      coord[6][0], coord[6][1]);
  cairo_curve_to (cr, coord[7][0], coord[7][1],
                      coord[8][0], coord[8][1],
                      coord[9][0], coord[9][1]);
  cairo_curve_to (cr, coord[10][0], coord[10][1],
                      coord[11][0], coord[11][1],
                      coord[12][0], coord[12][1]);
  cairo_set_source_rgb (cr, 0.2, 0.4, 0.8);
  cairo_fill_preserve (cr);

  cairo_set_source_rgb (cr, 1.0, 0,0);
  cairo_stroke (cr);

  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);

  for (int i = 0; i < 13; i ++)
  {
    cairo_arc (cr, coord[i][0], coord[i][1],
               0.025, 0, 2 * 3.1415);
    cairo_fill (cr);

    mrg_listen (mrg, MRG_DRAG, coord[i][0] - .025, coord[i][1] - .025,
                         .05, .05,
                drag_cb, &coord[i][0], NULL);
  }
#endif
  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (320, 240, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}
