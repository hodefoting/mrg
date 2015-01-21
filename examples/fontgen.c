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

#if MRG_CAIRO

typedef struct Glyph {
  uint32_t unicode;
  int      x_advance;
  int      y_advance;
  int      width;
  int      height;
  int      stride;
  uint8_t *buf;
} Glyph;

typedef struct Font {
  const char *font_family;
  int         font_size;
  Glyph glyphs[512];
} Font;

/*
 *
 */
char *font_names[] = {"monospace", "sans", "bold", "mono bold", NULL};

static void ui (Mrg *mrg, void *data)
{
  int i;
  cairo_t *cr = mrg_cr (mrg);
  for (i = 32; i < 127; i ++)
  {
    char buf[8];
    cairo_set_source_rgb (cr, 0,0,0);
    cairo_paint (cr);
    cairo_move_to (cr, 10, 10);
    cairo_set_source_rgb (cr, 1,1,1);
    sprintf (buf, "%c", i);
    cairo_show_text (cr, buf);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

#endif

int main (int argc, char **argv)
{
#if MRG_CAIRO
  Mrg *mrg = mrg_new (640, 480, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
#endif
  return 0;
}

