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
#include <stdarg.h>
#include "mrg.h"

#define LINES 13

static char log[LINES][100];

static void add_line (Mrg *mrg, const char *string)
{
  int i;
  for (i = 0; i < LINES-1; i ++)
  {
    memcpy (log[i], log[i+1], sizeof (log[0]));
  }
  strcpy (log[LINES-1], string);
  mrg_queue_draw (mrg, NULL);
}


void add_linef (Mrg *mrg, const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;
  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = malloc(needed);
  va_end (ap);
  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);
  add_line (mrg, buffer);
  free (buffer);
}


static int event_handler (MrgEvent *e, void *data1, void *data2)
{
  Mrg *mrg = e->mrg;
  switch (e->type)
  {
    case MRG_DRAG_MOTION:
      add_linef (mrg, "drag-motion x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_DRAG_PRESS:
      add_linef (mrg, "drag-press x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_DRAG_RELEASE:
      add_linef (mrg, "drag-release x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_MOTION:
      add_linef (mrg, "motion x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_PRESS:
      add_linef (mrg, "press x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_RELEASE:
      add_linef (mrg, "release x:%f y:%f device:%i", e->x, e->y, e->device_no);
      break;
    case MRG_ENTER:
      add_line (mrg, "enter");
      break;
    case MRG_LEAVE:
      add_line (mrg, "leave");
      break;
    case MRG_KEY_DOWN:
      add_linef (mrg, "key down key_name:%s  unicode:%i, ", e->key_name, e->unicode);
      break;
    case MRG_KEY_UP:
      add_linef (mrg, "key up key_name:%s  unicode:%i, ", e->key_name, e->unicode);
      break;
    default:
      add_line (mrg, "other");
      break;
  }
#if 0
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
#endif
  return 0;
}




static void render_ui (Mrg *mrg, void *data)
{
  int i;
  mrg_set_edge_left (mrg, mrg_em (mrg));
  mrg_set_edge_top (mrg, mrg_em (mrg));
  mrg_listen (mrg, MRG_ANY, 0,0, mrg_width(mrg), mrg_height (mrg), 
              event_handler, NULL, NULL);

  mrg_start_with_style (mrg, "div", NULL, "position:fixed;bottom:0;padding:0.5em;");

  for (i = 0; i < LINES; i++)
  {
    if(i)mrg_print (mrg, "\n");
    mrg_print (mrg, log[i]);
  }

  mrg_end (mrg);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (640, 480, NULL);
  mrg_set_ui (mrg, render_ui, NULL);
  mrg_main (mrg);
  return 0;
}
