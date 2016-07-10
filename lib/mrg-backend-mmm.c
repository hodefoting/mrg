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

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdlib.h>
#include "mrg-config.h"
#if MRG_MMM

#include "mmm.h"
#include "mrg-internal.h"

static unsigned char *mrg_mmm_get_pixels (Mrg *mrg, int *rowstride)
{
  Mmm *mmm = mrg->backend_data;
  int width, height;

  return mmm_get_buffer_write (mmm, &width, &height, rowstride, NULL);
}

void *mrg_mmm (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mrg->backend_data;
  }
  return NULL;
}

static void mrg_mmm_flush (Mrg *mrg)
{
  Mmm *mmm = mrg->backend_data;

  //MMM_Surface *screen = mrg->backend_data;
  /* XXX: move this safe-guard into flush itself? */
  if (mrg->dirty.x < 0)
  {
    mrg->dirty.width += -mrg->dirty.x;
    mrg->dirty.x = 0;
  }
  if (mrg->dirty.y < 0)
  {
    mrg->dirty.height += -mrg->dirty.y;
    mrg->dirty.y = 0;
  }
  if (mrg->dirty.x + mrg->dirty.width >= mrg->width)
  {
    mrg->dirty.width = mrg->width - mrg->dirty.x - 1;
  }
  if (mrg->dirty.y + mrg->dirty.height >= mrg->height)
  {
    mrg->dirty.height = mrg->height - mrg->dirty.y -1;
  }
  if (mrg->dirty.width < 0)
    mrg->dirty.width = 0;
  if (mrg->dirty.height < 0)
    mrg->dirty.height = 0;

  mmm_write_done (mmm, mrg->dirty.x, mrg->dirty.y, mrg->dirty.width, mrg->dirty.height);
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
}

#if 0
static void mrg_mmm_fullscreen (Mrg *mrg, int fullscreen)
{
  MMM_Surface *screen = mrg->backend_data;
  int width = 640, height = 480;

  if (fullscreen)
  {
    MMM_Rect **modes;
    modes = MMM_ListModes(NULL, MMM_HWSURFACE|MMM_FULLSCREEN);
    if (modes == (MMM_Rect**)0) {
        fprintf(stderr, "No modes available!\n");
        return;
    }
  
    width = modes[0]->w;
    height = modes[0]->h;

    screen = MMM_SetVideoMode(width, height, 32,
                              MMM_SWSURFACE | MMM_FULLSCREEN );
    mrg->backend_data = screen;
  }
  else
  {
    screen = MMM_SetVideoMode(width, height, 32,
                              MMM_SWSURFACE | MMM_RESIZABLE );
    mrg->backend_data = screen;
  }
  mrg->fullscreen = fullscreen;
}
#endif

static void mrg_mmm_consume_events (Mrg *mrg, int block);

static void mrg_mmm_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  while (!_mrg_has_quit (mrg))
  {
    if (_mrg_is_dirty (mrg))
      mrg_ui_update (mrg);

    _mrg_idle_iteration (mrg);

    mrg_mmm_consume_events (mrg, !mrg->idles);
  }
  system ("stty sane");
}

static void mrg_mmm_warp_pointer (Mrg *mrg, float x, float y)
{
  //mmm_warp_cursor (mrg->backend_data, x, y);
}

#if 0
static int timer_cb (Uint32 interval, void *param)
{
  _mrg_idle_iteration (param);
  return param;
}
#endif

static void mrg_mmm_set_title (Mrg *mrg, const char *title)
{
  mmm_set_title (mrg->backend_data, title);
}

static const char *mrg_mmm_get_title (Mrg *mrg)
{
  return mmm_get_title (mrg->backend_data);
}

static void mrg_mmm_set_position  (Mrg *mrg, int x, int y)
{
  mmm_set_x (mrg->backend_data, x);
  mmm_set_y (mrg->backend_data, y);
}

void  mrg_mmm_get_position  (Mrg *mrg, int *x, int *y)
{
  if (x)
    *x = mmm_get_x (mrg->backend_data);
  if (y)
    *y = mmm_get_y (mrg->backend_data);
}

static void *mmm_self = NULL;
static void  mmm_atexit (void)
{
  fprintf (stderr, "teardown time! %p\n", mmm_self);
  mmm_destroy (mmm_self);
}

static Mrg *_mrg_mmm_new (int width, int height);
static Mrg *_mrg_mmm_client_new (int width, int height);


static void mrg_mmm_restart (Mrg *mrg)
{
  const char *mmm_buffer_path = mmm_get_path (mrg_mmm (mrg));
  
  setenv ("MMM_BUFFER", mmm_buffer_path, 1);
}

MrgBackend mrg_backend_mmm = {
  "mmm",
  _mrg_mmm_new,
  mrg_mmm_main,
  mrg_mmm_get_pixels, /* mrg_get_pixels */
  NULL, /* mrg_cr, */
  mrg_mmm_flush,
  NULL, /* mrg_prepare */
  NULL, /* mrg_clear */
  NULL, /* mrg_queue_draw, */
  NULL, /* mrg_destroy */
  mrg_mmm_warp_pointer,
  NULL, //mrg_mmm_fullscreen,
  mrg_mmm_set_position,
  mrg_mmm_get_position,
  mrg_mmm_set_title,
  mrg_mmm_get_title,
  mrg_mmm_restart
};

MrgBackend mrg_backend_mmm_client = {
  "mmm-client",
  _mrg_mmm_client_new,
  mrg_mmm_main,
  mrg_mmm_get_pixels, /* mrg_get_pixels */
  NULL, /* mrg_cr, */
  mrg_mmm_flush,
  NULL, /* mrg_prepare */
  NULL, /* mrg_clear */
  NULL, /* mrg_queue_draw, */
  NULL, /* mrg_destroy */
  mrg_mmm_warp_pointer,
  NULL, //mrg_mmm_fullscreen,
  mrg_mmm_set_position,
  mrg_mmm_get_position,
  mrg_mmm_set_title,
  mrg_mmm_get_title,
  NULL,
};

static Mrg *_mrg_mmm_client_new (int width, int height)
{
  if (!getenv ("MMM_PATH"))
    return NULL;
  return _mrg_mmm_new (width, height);
}

static Mrg *_mrg_mmm_new (int width, int height)
{
  Mrg *mrg;
  Mmm *mmm;
  int fullscreen = 0;

  if (width < 0)
  {
     height = -1;
     fullscreen = 1;
  }

  if (getenv ("MMM_BUFFER"))
  {
    mmm = mmm_client_reopen (getenv ("MMM_BUFFER"));
    mmm_self = mmm;
    atexit (mmm_atexit);
  }
  else
  if (getenv ("MMM_PATH"))
  {
    mmm = mmm_new (width, height, 0, NULL);
    mmm_self = mmm;
    atexit (mmm_atexit);
  }
  else
    mmm = mmm_new (width, height, 0, NULL);
  if (!mmm)
  {
    fprintf (stderr, "unable to init mmm\n");
    return NULL;
  }


  if (fullscreen)
  {
    usleep (50000);
    width = mmm_get_width (mmm);
    height = mmm_get_height (mmm);
  }

  mmm_set_size (mmm, width, height);

  mrg = calloc (sizeof (Mrg), 1);
  mrg->backend = &mrg_backend_mmm;
  mrg->backend_data = mmm;

  _mrg_init (mrg, width, height);
  mrg_set_size (mrg, width, height);
  mrg->do_clip = 1;

  if (fullscreen)
    mrg_set_fullscreen (mrg, fullscreen);

  //mmm_set_fps_limit (mmm, 60);

  return mrg;
}

static void mrg_mmm_consume_events (Mrg *mrg, int block)
{
  Mmm *fb = mrg->backend_data;
  int events = 0;
  while (mmm_has_event (fb))
  {
    const char *event = mmm_get_event (fb);
    char event_type[128]="";
    float x = 0, y = 0;

    sscanf (event, "%s %f %f", event_type, &x, &y);
    if (!strcmp (event_type, "mouse-press"))
    {
      mrg_pointer_press (mrg, x, y, 1, 0);
    }
    else if (!strcmp (event_type, "mouse-drag") ||
             !strcmp (event_type, "mouse-motion"))
    {
      mrg_pointer_motion (mrg, x, y, 1, 0);
    }
    else if (!strcmp (event_type, "mouse-release"))
    {
      mrg_pointer_release (mrg, x, y, 1, 0);
    }
    else if (!strcmp (event_type, "message"))
    {
      mrg_incoming_message (mrg, event + strlen ("message "), 0);
    }
    /* XXX: scroll */
    else
    {
      mrg_key_press (mrg, 0, event, 0);
    }
    events ++;
  }
  if (!events)
  {
    usleep (3000);
  }
  {
    int w, h;
    if (mmm_client_check_size (fb, &w, &h))
      mrg_set_size (mrg, w, h);
  }
}

void mrg_message (Mrg *mrg, const char *message)
{
  if (mrg_mmm (mrg))
    mmm_add_message (mrg_mmm (mrg), message);
}

void mrg_window_set_value (Mrg *mrg, const char *name, const char *value)
{
  if (mrg_mmm (mrg))
    mmm_set_value (mrg_mmm (mrg), name, value);
}

#else

void mrg_message (Mrg *mrg, const char *message)
{
}

void mrg_window_set_value (Mrg *mrg, const char *name, const char *value)
{
}

#endif
