/* mrg - MicroRaptor Gui
 * Copyright (c) 2014 Øyvind Kolås <pippin@hodefoting.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg.h"

static float scroll[2] = {0,0};

void pgup_cb (MrgEvent *e, void *data1, void *data2)
{
  float *pos = data1;
  pos[1] += mrg_height (e->mrg) * 0.8;
  mrg_queue_draw (e->mrg, NULL);
}

void pgdn_cb (MrgEvent *e, void *data1, void *data2)
{
  float *pos = data1;
  pos[1] -= mrg_height (e->mrg) * 0.8;
  mrg_queue_draw (e->mrg, NULL);
}

void drag_pos (MrgEvent *e, void *data1, void *data2)
{
  if (e->type == MRG_DRAG_MOTION && e->device_no == 1)
  {
    float *pos = data1;
    pos[1] += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }
}

typedef struct Mr
{
  Mrg  *mrg;
  char *uri;
  int   mode;

  char *output_png;
} Mr;

void
browser_set_uri (Mr *mr, const char *uri)
{
  if (mr->uri)
    free (mr->uri);
  if (uri)
    mr->uri = strdup (uri);
  else
    mr->uri = NULL;

  mrg_queue_draw (mr->mrg, NULL);
}

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

static void toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
}

static void href_cb (MrgEvent *event, void *src, void *data2)
{
  Mr *mr = data2;
  browser_set_uri (mr, src);
  scroll[0] = scroll[1] = 0;
}

static const char *magic_mime (const char *data, int length)
{
  unsigned char jpgsig[4]={0xff, 0xd8, 0xff, 0xe0};
  char tmpbuf[256+1];
  if (length>256)
    length = 256;
  if (!data)
    return "text/plain";
  if (length<=0)
    return "text/plain";
  memcpy (tmpbuf, data, length);
  tmpbuf[length]=0;
  if (!memcmp(tmpbuf, "\211PNG\r\n\032\n", 8))
    return "image/png";
  else if (!memcmp(tmpbuf, jpgsig, 4))
    return "image/jpeg";
  else if (strstr(tmpbuf, "html"))
    return "text/html";
  else if (strstr(tmpbuf, "svg"))
    return "text/svg";
  else if (strstr(tmpbuf, "SVG"))
    return "text/svg";
  else if (strstr(tmpbuf, "TITLE"))
    return "text/html";
  else if (strstr(tmpbuf, "HTML"))
    return "text/html";
  else if (strstr(tmpbuf, "xml"))
    return "text/xml";
  return "text/plain";
}

static void render_ui (Mrg *mrg, void *data)
{
  Mr *mr = data;
  char *contents = NULL;
  long  length;

  cairo_save (mrg_cr (mrg));
  cairo_rectangle (mrg_cr (mrg), 0,0, mrg_width(mrg), mrg_height(mrg));
  mrg_listen (mrg, MRG_DRAG, drag_pos, scroll, NULL);
  cairo_restore (mrg_cr (mrg));

#if MRG_CAIRO
  cairo_save (mrg_cr (mrg));
  cairo_translate (mrg_cr (mrg), scroll[0], scroll[1]);
#endif
  mrg_get_contents (mrg, NULL, mr->uri, &contents, &length);

  if (contents)
  {
    const char *mime_type = magic_mime (contents, length);
    
    if (!strcmp (mime_type, "text/plain"))
    {
      mrg_print (mrg, contents);
    }
    else if (!strcmp (mime_type, "text/html") ||
             !strcmp (mime_type, "text/xml") ||
             !strcmp (mime_type, "text/svg"))
    {
      mrg_stylesheet_clear (mrg);
      mrg_xml_render (mrg, mr->uri, href_cb, mr, NULL, NULL, contents);
    }
    else
    {
      mrg_printf (mrg, "\nUnhandled mimetype\n\n[%s]", mime_type);
    }
    free (contents);
  }

#if MRG_CAIRO
  cairo_restore (mrg_cr (mrg));
#endif

  mrg_add_binding (mrg, "page-down", NULL, NULL, pgdn_cb, scroll);
  mrg_add_binding (mrg, "page-up",   NULL, NULL, pgup_cb, scroll);
  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_add_binding (mrg, "f", NULL, NULL, toggle_fullscreen_cb, NULL);

#if MRG_CAIRO
  if (mr->output_png) {
    cairo_surface_t *surface = cairo_get_target (mrg_cr (mrg));
    cairo_surface_write_to_png (surface, mr->output_png);
  }
#endif
}

int browser_main (int argc, char **argv)
{
  //Mrg *mrg = mrg_new (-1, -1, NULL);
  Mrg *mrg;
  Mr *mr;
  
  mr = calloc (sizeof (Mr), 1);

  if (argv[1] && argv[2] && !strcmp (argv[2], "-o") && argv[3])
  {
    mr->output_png = argv[3];
  }

  if (mr->output_png)
    mrg = mrg_new (240, 320, "mem");
  else
  //mrg = mrg_new (320, 480, NULL);
    mrg = mrg_new (-1, -1, NULL);

  {
    char *tmp = realpath (argv[1]?argv[1]:argv[0], NULL);
    if (tmp)
    {
      char *uri = malloc (strlen (tmp) + 10);
      sprintf (uri, "file://%s", tmp);
      mr->uri = uri;
    }
    else
    {
      if (!argv[1])
        mr->uri = strdup ("mrg:mrg.html");
      else
        mr->uri = strdup(argv[1]);
    }
  }
  mr->mrg = mrg;

  mrg_css_set (mrg, "document { background: #ffff;}");
  mrg_set_ui (mrg, render_ui, mr);

  if (mr->output_png)
  {
    mrg_ui_update (mrg);
    mrg_ui_update (mrg); /* we do it twice, to resolve some html/css measurements */
  }
  else
  {
    mrg_main (mrg);
  }
  mrg_destroy (mrg);

  free (mr->uri);
  free (mr);
  return 0;
}
