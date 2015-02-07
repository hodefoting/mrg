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

typedef struct Client
{
  int           type;
  UiRenderFun   render;
  void         *render_data;
  Mrg          *mrg;
  float         x;
  float         y;
} Client;

#define MAX_CLIENTS 20

typedef struct Compositor
{
  Mrg    *mrg;
  Client  clients[MAX_CLIENTS];

} Compositor;

static int toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
  return 1;
}

static void render_client (Mrg *mrg, void *data)
{
  mrg_printf (mrg, "client %s", data);
}

static int client_drag (MrgEvent *e, void *client_, void *unused)
{
  Client *client = client_;

  if (e->type == MRG_DRAG_MOTION)
  {
    client->x += e->delta_x;
    client->y += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }

  return 0;
}

static void render_ui (Mrg *mrg, void *data)
{
  Compositor *compositor = data;
  int i;
  cairo_t *cr = mrg_cr (mrg);
  for (i = 0; compositor->clients[i].render; i++)
  {
    Client *client = &compositor->clients[i];
    cairo_rectangle (cr, client->x, client->y, mrg_width (client->mrg), mrg_height (client->mrg));
    cairo_set_line_width (cr, 2.0);
    cairo_set_source_rgb (cr, 1, 0,0);
    cairo_stroke (cr);

    mrg_render_to_mrg (client->mrg, mrg, client->x, client->y);

    cairo_rectangle (cr, client->x, client->y - 20, mrg_width (client->mrg), 20);
    mrg_listen (mrg, MRG_DRAG_MOTION, client_drag, client, NULL);
    cairo_stroke (cr);

  }

  mrg_printf (mrg, "compositor %p", compositor);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_add_binding (mrg, "f", NULL, NULL, toggle_fullscreen_cb, NULL);
}

Compositor *compositor_new (void)
{
  return calloc (sizeof (Compositor), 1);
}

void compositor_destroy (Compositor *compositor)
{
  free (compositor);
}

void add_client (Compositor *compositor,
                 UiRenderFun ui,
                 void       *ui_data,
                 float       x,
                 float       y)
{
  Client *client;
  int i;
  for (i = 0; compositor->clients[i].render; i++);
  client = &compositor->clients[i];

  client->render = ui;
  client->render_data = ui_data;
  client->x = x;
  client->y = y;
  client->mrg = mrg_new (160, 120, "mem");
  mrg_set_ui (client->mrg, client->render, client->render_data);
}

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

static void drag_render_ui (Mrg *mrg, void *data)
{
  {
    static float pos[2] = {10, 10};
    static int active = 0;

    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
    mrg_set_style (mrg, "color:red");
    else
    mrg_set_style (mrg, "color:green");

    mrg_print (mrg, "drag me around");
    mrg_text_listen_done (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}


int main (int argc, char **argv)
{
  Mrg *mrg;
  Compositor *compositor;
  
  compositor = compositor_new ();
  mrg = mrg_new (640, 480, NULL);
  compositor->mrg = mrg;
  mrg_set_ui (mrg, render_ui, compositor);

  add_client (compositor, render_client, "foo", 10, 100);
  add_client (compositor, render_client, "bar", 10, 300);
  add_client (compositor, drag_render_ui, "aaa" , 200, 10);

  add_client (compositor, drag_render_ui, "aaa" , 100, 400);

  mrg_main (mrg);

  compositor_destroy (compositor);
  return 0;
}

#else
int main (int argc, char **argv)
{
  return 0;
}
#endif

