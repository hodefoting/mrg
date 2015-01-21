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

typedef struct State {
  float edge_left;
  float edge_right;
  float edge_top;
  float edge_bottom;
  char *string;
} State;


static void update_string (Mrg *mrg, char **string_loc, const char *new_string,
                           void *user_data)
{
  /* using user_data; different type of API hookups for updating the currently
   * edited string can be used here
   */
  free (*string_loc);
  *string_loc = strdup (new_string);
}

static int editable_press (MrgEvent *e, void *data1, void *data2)
{
  char **string = data1;
  mrg_queue_draw (e->mrg, NULL);
  mrg_edit_string (e->mrg, string, update_string, NULL);
  return 1;
}

static int drag_edge_left (MrgEvent *e, void *data1, void *data2)
{
  State *state = data1;
  state->edge_left += e->delta_x;
  mrg_queue_draw (e->mrg, NULL);
  return 1;
}

static int drag_edge_right (MrgEvent *e, void *data1, void *data2)
{
  State *state = data1;
  state->edge_right += e->delta_x;
  mrg_queue_draw (e->mrg, NULL);
  return 1;
}

static int drag_edge_top (MrgEvent *e, void *data1, void *data2)
{
  State *state = data1;
  state->edge_top += e->delta_y;
  mrg_queue_draw (e->mrg, NULL);
  return 1;
}

static int drag_edge_bottom (MrgEvent *e, void *data1, void *data2)
{
  State *state = data1;
  state->edge_bottom += e->delta_y;
  mrg_queue_draw (e->mrg, NULL);
  return 1;
}

static void ui (Mrg *mrg, void *data)
{
  State *state = data;
  float em = mrg_em (mrg);
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
#endif

  mrg_set_edge_left   (mrg, state->edge_left);
  mrg_set_edge_right  (mrg, state->edge_right);
  mrg_set_edge_top    (mrg, state->edge_top);
  mrg_set_edge_bottom (mrg, state->edge_bottom);

#if MRG_CAIRO
  cairo_set_source_rgb (cr, 0, 0, 0);
#endif

  mrg_listen (mrg, MRG_DRAG_MOTION,
              state->edge_left - em, 0, 2 * em, mrg_height (mrg),
              drag_edge_left, state, NULL);

  mrg_listen (mrg, MRG_DRAG_MOTION,
              state->edge_right - em, 0, 2 * em, mrg_height (mrg),
              drag_edge_right, state, NULL);

  mrg_listen (mrg, MRG_DRAG_MOTION,
              0, state->edge_top - em, mrg_width(mrg), 2 * em,
              drag_edge_top, state, NULL);

  mrg_listen (mrg, MRG_DRAG_MOTION,
              0, state->edge_bottom - em, mrg_width(mrg), 2 * em,
              drag_edge_bottom, state, NULL);

  {
    mrg_text_listen (mrg, MRG_PRESS, editable_press, &state->string, NULL);
    mrg_print (mrg, state->string);
    mrg_text_listen_done (mrg);
  }

  {
    static char *string = NULL;
    if (!string)
      string = strdup ("foo");

    mrg_set_style (mrg, "font-size:10");
    mrg_text_listen (mrg, MRG_PRESS, editable_press, &string, NULL);

    mrg_set_style (mrg, "color:blue; background:red; ");

    mrg_print (mrg, string);
    mrg_text_listen_done (mrg);

#if MRG_CAIRO
    cairo_new_path (cr);
#endif
  }

#if MRG_CAIRO
  cairo_set_source_rgb (cr, 0, 0, 1);

  cairo_move_to (cr, state->edge_left, 0);
  cairo_line_to (cr, state->edge_left, mrg_height (mrg));
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);

  cairo_move_to (cr, state->edge_right, 0);
  cairo_line_to (cr, state->edge_right, mrg_height (mrg));
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);

  cairo_move_to (cr, 0, state->edge_top);
  cairo_line_to (cr, mrg_width (mrg), state->edge_top);
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);

  cairo_move_to (cr, 0, state->edge_bottom);
  cairo_line_to (cr, mrg_width (mrg), state->edge_bottom);
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);
#endif

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main (int argc, char **argv)
{
  static State static_state = {-1,-1,-1,-1, NULL};
  State *state = &static_state;
  Mrg *mrg = mrg_new (640, 480, NULL);
  float em = mrg_em (mrg);
  state->edge_left = 2 * em;
  state->edge_top = 2 * em;
  state->edge_right = mrg_width (mrg) - 2 * em;
  state->edge_bottom = mrg_height (mrg) - 2 * em;
  state->string = strdup ("fnord\n\nasdf asdf a");
  mrg_set_ui (mrg, ui, state);
  mrg_main (mrg);
  return 0;
}
