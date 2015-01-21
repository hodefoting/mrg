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

#include "mrg-config.h"
#if MRG_GTK
#include <gtk/gtk.h>
#endif

#include "mrg.h" /* we must include after gtk/gtk.h */

#if MRG_GTK
static void render_ui (Mrg *mrg, void *data)
{
  mrg_print (mrg, data);
}

static void render_ui2 (Mrg *mrg, void *data)
{
  mrg_print (mrg, "world");
}
#endif

int main (int argc, char **argv)
{
#if MRG_GTK
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *canvas;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_default_size (GTK_WINDOW(window), 800, 400);

  gtk_container_add (GTK_CONTAINER(window), box);

  canvas = mrg_gtk_new (render_ui, "hei");
  gtk_box_pack_start (GTK_BOX(box), canvas, TRUE, TRUE, 0);

  canvas = mrg_gtk_new (render_ui2, NULL);
  gtk_box_pack_start (GTK_BOX(box), canvas, TRUE, TRUE, 0);

  canvas = mrg_gtk_new (render_ui2, NULL);
  gtk_box_pack_start (GTK_BOX(box), canvas, TRUE, TRUE, 0);

  gtk_widget_show_all(window);
  gtk_widget_grab_focus (canvas);

  g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_main ();
#endif

  return 0;
}
