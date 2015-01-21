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

#include "mrg.h"

static char *css =
"p                    { color: blue; border: 1px solid green;}\n"
"bindings             { display:block; left: 0px; position: fixed; width: 100%; bottom: 0px; padding: 0.2em; background: red; font-size: 14px;}\n"
"binding              { color: black;}\n"
"binding action:hover { color: yellow;}\n"
"key                  { color: white; border: 1px solid red; width: 2em; height: 2em;}\n"
"action               { color: black; display: inline; background: red;}\n";

void mrg_draw_bindings (Mrg *mrg)
{
  float em = mrg_em (mrg);

  mrg_set_edge_left (mrg, 2 * em);
  mrg_set_edge_right (mrg, mrg_width (mrg) - 2 * em);
  mrg_set_edge_bottom (mrg, mrg_height (mrg) - 2 * em);
  mrg_set_edge_top (mrg, 2 * em);

  mrg_stylesheet_clear (mrg);
  mrg_stylesheet_add (mrg, css, NULL, 1000, NULL);
  mrg_printf_xml (mrg, "<h3>Just some dummy html</h3><p>hello %s</p><p>foo <b>bar</b> baz</p>\n", "this is a little test of rendering ui bits using CSS styling, the bindings display that is part of the core microraptor environment is a nice starting test case.");

  mrg_start (mrg, "bindings", NULL);
  mrg_print (mrg, " ");
  mrg_start (mrg, "binding", NULL);
  mrg_start (mrg, "key", NULL);
  mrg_print (mrg, "^Q");
  mrg_end(mrg);
  mrg_start (mrg, "action", NULL);
  mrg_print (mrg, "Quit");
  mrg_end(mrg);
  mrg_end(mrg);
  mrg_print (mrg, " ");
  mrg_start (mrg, "binding", NULL);
  mrg_start (mrg, "key", NULL);
  mrg_print (mrg, "F");
  mrg_end(mrg);
  mrg_start (mrg, "action", NULL);
  mrg_print (mrg, "fullscreen");
  mrg_end(mrg);
  mrg_end(mrg);
  mrg_print (mrg, " ");


  for (int i = 0; i < 8; i++)
  {
    mrg_start (mrg, "binding", NULL);
    mrg_start (mrg, "key", NULL);
    mrg_printf (mrg, "F%i", i);
    mrg_end(mrg);
    mrg_start (mrg, "action", NULL);
    mrg_print (mrg, "fooo");
    mrg_end(mrg);
    mrg_end(mrg);
  }
  mrg_end(mrg);
}

static void ui (Mrg *mrg, void *data)
{
  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_draw_bindings (mrg);
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (400, 300, NULL);
  mrg_set_ui (mrg, ui, argv[1]?argv[1]:"world");
  mrg_main (mrg);
  return 0;
}
