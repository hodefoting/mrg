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

/* audio should be a separate daemon that reads client files */

/* daemons should have their process files in the same directory,
 * permitting all process of the system to be self-introspectable.
 *
 * drag and drop configuration using filesystem as repository.
 *
 * desktop view,. listing contents of a specific folder, client windows
 * rendered on top. 
 *
 * how to determine that something is an mmm app?
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg.h"
#include <dirent.h>
#include "mmm.h"
#include <unistd.h>
#include "mrg-list.h"
#include "mrg-host.h"
#include <sys/types.h>
#include <signal.h>

#if MRG_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#endif

#include "mrg-internal.h" // XXX: eeek

struct _MrgClient
{
  MrgHost *host;

  /******************/
  long   pid;        /* magic hack, -1 means in-process,
                       (isn't mmm vs mrg the same?) */
  char  *filename;
  Mmm   *mmm;
  Mrg   *mrg;

  /******************/

  float  int_x;
  float  int_y;

  int    premax_x;
  int    premax_y;
  int    premax_width;
  int    premax_height;

  int    ref_count;
};

const char *mrg_client_get_title (MrgClient *client)
{
  if (client->mrg)
    return mrg_get_title (client->mrg);
  return mmm_get_title (client->mmm);
}


struct _MrgHost
{
  Mrg        *mrg;
  char       *fbdir;
  MrgList    *clients;
  MrgClient  *focused;

  int default_width;
  int default_height;
};

static void mrg_client_ref (MrgClient *client)
{
  client->ref_count++;
}

static void mrg_client_unref (void *ignored, void *ignored2, MrgClient *client)
{
  client->ref_count--;
  if (client->ref_count < 0)
  {
    char tmp[256];
    if (client->host->focused == client)
      client->host->focused = NULL;
    sprintf (tmp, "%s/%s", client->host->fbdir, client->filename);
    if (client->mmm)
    {
      mmm_destroy (client->mmm);
      client->mmm = NULL;
    }
    unlink (tmp);
    free (client);
  }
}

int  mrg_client_get_stack_order (MrgClient *client)
{
  MrgList *l;
  MrgHost *host;
  int i = 0;
  if (!client)
    return 0;
  host = client->host;
  for (l = host->clients; l; l = l->next) i++;

  for (l = host->clients; l; l = l->next)
  {
    if (l->data == client)
      return i;
    i--;
  }
  return 0;
}

void mrg_client_set_stack_order (MrgClient *client,
                                 int        zpos)
{
  MrgList *new = NULL;
  MrgList *l;
  MrgHost *host;
  int i = 0;
  if (!client)
    return;
  host = client->host;
  
  for (l = host->clients; l; l = l->next) i++;
  
  i++;
  for (l = host->clients; l; l = l->next)
  {
    MrgClient *ic = l->data;

    i--;
    if (ic == client)
    {
      if (i == zpos)
      {
        mrg_list_free (&new);
        fprintf (stderr, "already there!\n");
        return;
      }
    }
    else
    {
      if (i == zpos && client)
      {
        mrg_list_append (&new, client);
        client = NULL;
      }
      mrg_list_append (&new, ic);
    }
  }
  
  if (client == NULL)
  {
    mrg_list_free (&host->clients);
    host->clients = new;
  }
  else
  {
    //fprintf (stderr, "failed\n");
    //mrg_list_free (&new);
    mrg_list_append (&new, client);
    mrg_list_free (&host->clients);
    host->clients = new;
  }
}

void mrg_client_raise_top (MrgClient *client)
{
  mrg_client_set_stack_order (client, 0);
  return;
}

void mrg_client_maximize (MrgClient *client)
{
  MrgHost *host = client->host;
  Mrg *mrg = host->mrg;

  mrg_client_raise_top (client);

  if (client->mrg)
    return;

  if (mrg_width (mrg) == mmm_get_width (client->mmm) &&
      client->premax_width &&
      client->premax_height)
  {
    mmm_host_set_size (client->mmm, client->premax_width, client->premax_height);
    mmm_set_x (client->mmm, client->premax_x);
    mmm_set_y (client->mmm, client->premax_y);
  }
  else
  {
    client->premax_x = mmm_get_x (client->mmm);
    client->premax_y = mmm_get_y (client->mmm);
    client->premax_width = mmm_get_width (client->mmm);
    client->premax_height = mmm_get_height (client->mmm);

    mmm_host_set_size (client->mmm,
        mrg_width (mrg),
        mrg_height (mrg) - TITLE_BAR_HEIGHT);
    mmm_set_x (client->mmm, 0);
    mmm_set_y (client->mmm, TITLE_BAR_HEIGHT);
  }
}

float mrg_client_get_x (MrgClient *client)
{
  if (client->mrg)
    return  client->int_x;
  if (client->mmm)
    return mmm_get_x (client->mmm);
  return 0.0;
}

float mrg_client_get_y (MrgClient *client)
{
  if (client->mrg)
    return  client->int_y;
  if (client->mmm)
    return mmm_get_y (client->mmm);
  return 0.0;
}

void mrg_client_set_x (MrgClient *client, float x)
{
  if (client->mrg)
    client->int_x = x;
  else if (client->mmm)
    mmm_set_x (client->mmm, x);
}

void mrg_client_set_y (MrgClient *client, float y)
{
  if (client->mrg)
    client->int_y = y;
  else if (client->mmm)
    mmm_set_y (client->mmm, y);
}

void mrg_client_get_size (MrgClient *client, int *width, int *height)
{
  int w, h;

  if (client->pid != -1)
  {
    mmm_host_get_size (client->mmm, &w, &h);
    if (w <= 0)
      mmm_get_size (client->mmm, &w, &h);
  }
  else
  {
    w = mrg_width (client->mrg);
    h = mrg_height (client->mrg);
  }

  if (width)
    *width = w;

  if (height)
    *height = h;
}

void mrg_client_set_size (MrgClient *client, int width, int height)
{
  if (client->pid != -1)
  {
    mmm_host_set_size (client->mmm, width, height);
  }
  else
  {
    mrg_set_size (client->mrg, width, height);
  }
}

static int pos = 10;

static void validate_client (MrgHost *host, const char *client_name)
{
  MrgList *l;
  for (l = host->clients; l; l = l->next)
  {
    MrgClient *client = l->data;
    if (client->filename && 
        !strcmp (client->filename, client_name))
    {
      return;
    }
  }

  {
    MrgClient *client = calloc (sizeof (MrgClient), 1);

    char tmp[256];
    sprintf (tmp, "%s/%s", host->fbdir, client_name);
    client->host = host;
    client->mmm = mmm_host_open (tmp);
    client->pid = mmm_client_pid (client->mmm);

    if (!client->mmm)
    {
      fprintf (stderr, "failed to open client %s\n", tmp);
      return;
    }

    client->filename = strdup (client_name);

    if (client->pid != getpid ())
    {
      if (mmm_get_width (client->mmm) < 0 ||
          mmm_get_height (client->mmm) < 0)
      {
        int width = host->default_width, height = host->default_height;
        if (width <= 0) width = mrg_width (host->mrg);
        if (height <= 0) height = mrg_height (host->mrg);
        mmm_host_set_size (client->mmm, width, height);
      }
      if (mmm_get_x (client->mmm) == 0 &&
          mmm_get_y (client->mmm) == 0)
      {
         mmm_set_x (client->mmm, pos);
         mmm_set_y (client->mmm, 30+pos);

         // pos += 12;
      }
    }
    mrg_list_append (&host->clients, client);
  }
}

void mrg_host_add_client_mrg (MrgHost *host,
                              Mrg     *mrg,
                              float    x,
                              float    y)
{
  MrgClient *client = calloc (sizeof (MrgClient), 1);
  client->mmm = NULL;
  client->pid = -1;
  client->int_x = x;
  client->int_y = y;
  client->mrg = mrg;
  client->host = host;
  pos += 12;
  mrg_list_append (&host->clients, client);
}

static int pid_is_alive (long pid)
{
  if (pid == -1)
    return 1;
  return kill (pid, 0) == 0;
}

void mrg_host_monitor_dir (MrgHost *host)
{
  MrgList *l;
again:
  for (l = host->clients; l; l = l->next)
  {
    MrgClient *client = l->data;
    if (!pid_is_alive (client->pid))
    {

      mrg_client_unref (NULL, NULL, client);
      mrg_queue_draw (host->mrg, NULL);
      mrg_list_remove (&host->clients, client);
      goto again;
    }
  }

  DIR *dir = opendir (host->fbdir);
  struct dirent *ent;
  static int iteration = 0;

  iteration ++;
  
  while ((ent = readdir (dir)))
  {
    if (ent->d_name[0]!='.')
      validate_client (host, ent->d_name);
  }
  closedir (dir);
}

MrgList *mrg_host_clients (MrgHost *host)
{
  return host->clients;
}

static void mrg_client_press (MrgEvent *event, void *client_, void *host_)
{
  MrgHost *host = host_;
  MrgClient *client = client_;
  Mmm *mmm = client->mmm;

  char buf[256];
  sprintf (buf, "mouse-press %f %f %i", event->x, event->y, event->device_no);
  mrg_host_set_focused (host, client);
  //if (host->focused)
  //  mrg_client_raise_top (host->focused);
  mmm_add_event (mmm, buf);
  mrg_event_stop_propagate (event);
}

static void mrg_client_motion (MrgEvent *event, void *client_, void *host_)
{
  MrgClient *client = client_;
  Mmm    *mmm    = client->mmm;

  /* should check if client is still valid */

  char buf[256];
  if (event->mrg->pointer_down[1])
    sprintf (buf, "mouse-drag %f %f %i", event->x, event->y, event->device_no);
  else
    sprintf (buf, "mouse-motion %f %f %i", event->x, event->y, event->device_no);
  mmm_add_event (mmm, buf);
  mrg_event_stop_propagate (event);
}

static void mrg_client_release (MrgEvent *event, void *client_, void *host_)
{
  MrgClient *client = client_;
  Mmm *mmm = client->mmm;
  char buf[256];
  sprintf (buf, "mouse-release %f %f %i", event->x, event->y, event->device_no);
  mmm_add_event (mmm, buf);
  mrg_event_stop_propagate (event);
}

static void host_key_down_cb (MrgEvent *event, void *host_, void *data2)
{
  MrgHost *host = host_;
  if (host->focused && host->focused->mmm)
  {
    mmm_add_event (host->focused->mmm, event->key_name);
    mrg_event_stop_propagate (event); // XXX? needed?
  }
}

void mrg_host_set_focused (MrgHost *host, MrgClient *client)
{
  host->focused = client;
}

MrgClient *mrg_host_get_focused      (MrgHost *host)
{
  return host->focused;
}

void mrg_client_render (MrgClient *client, Mrg *mrg, float x, float y)
{
  cairo_t *cr = mrg_cr (mrg);
  cairo_surface_t     *surface;
  const unsigned char *pixels;
  int count = 0;
  int width, height;

  if (client->mrg)
  {
    mrg_render_to_mrg (client->mrg, mrg, x, y);
    width  = mrg_width (client->mrg);
    height = mrg_height (client->mrg);
  }
  else
  {
    int rowstride;
    do
    {
      pixels = mmm_get_buffer_read (client->mmm, &width, &height, &rowstride);
      if (!pixels)
        usleep (1000);
      count ++;
    } while (pixels == NULL && count < 10);

    if (pixels)
    {
      surface = cairo_image_surface_create_for_data ((void*)pixels, CAIRO_FORMAT_ARGB32, width, height, rowstride);
      cairo_save (cr);
      cairo_translate (cr, x, y);
      cairo_set_source_surface (cr, surface, 0, 0);
      cairo_paint (cr);
      cairo_surface_destroy (surface);
      mmm_read_done (client->mmm);
      
      cairo_new_path (cr);
      cairo_rectangle (cr, 0, 0, width, height);
      mrg_client_ref (client);
      mrg_listen_full (mrg, MRG_PRESS,
                       mrg_client_press, client, client->host,
                       (void*)mrg_client_unref, client);

      mrg_client_ref (client);
      mrg_listen_full (mrg, MRG_MOTION,
                       mrg_client_motion, client, NULL,
                       (void*)mrg_client_unref, client);
      
      mrg_client_ref (client);
      mrg_listen_full (mrg, MRG_RELEASE, 
                       mrg_client_release, client, NULL,
                       (void*)mrg_client_unref, client);
      
      cairo_restore (cr);
    }
    else
    {
      fprintf (stderr, "didn't get pixels\n");
    }
  }
  cairo_new_path (cr);
}

void mrg_host_sloppy_focus (MrgHost *host, MrgClient *client, float x, float y)
{
  Mrg *mrg = host->mrg;
  float ptr_x, ptr_y;
  int width, height;

  ptr_x = mrg_pointer_x (mrg);
  ptr_y = mrg_pointer_y (mrg);

  mrg_client_get_size (client, &width, &height);

  if (ptr_x >= x && ptr_x < x + width &&
      ptr_y >= y - TITLE_BAR_HEIGHT && ptr_y < y + height)
  {
    mrg_host_set_focused (host, client);
  }
}

void mrg_client_render_sloppy (MrgClient *client, float x, float y)
{
  MrgHost *host = client->host;
  mrg_host_sloppy_focus (host, client, x, y);
  Mrg *mrg = host->mrg;

  if (client->pid == getpid ())
  {
    fprintf (stderr, "Wtf!\n");
    return;
  }

  mrg_client_render (client, mrg, x, y);
}

void mrg_host_register_events (MrgHost *host)
{
  Mrg *mrg = host->mrg;
  mrg_listen (mrg, MRG_KEY_DOWN, host_key_down_cb, host, NULL);
}

static void init_env (MrgHost *host, const char *path)
{
  char buf[512];
  if (host->fbdir)
    return;
  host->fbdir = strdup (path);
  setenv ("MMM_PATH", host->fbdir, 1);
  sprintf (buf, "mkdir %s &> /dev/null", host->fbdir);
  system (buf);
}

void mrg_host_destroy (MrgHost *host)
{
  free (host);
}

static int host_idle_check (Mrg *mrg, void *data)
{
  MrgHost *host = data;
  MrgList *l;
  
  for (l = host->clients; l; l = l->next)
  {
    MrgClient *client = l->data;

    int x, y, width, height;
    if (client->pid != -1)
    {
      if (client->mmm)
      if (mmm_get_damage (client->mmm, &x, &y, &width, &height))
      {
        /* XXX: this damage calculation is only correct when clients are drawn
         * where expected according to positioning
         */
        MrgRectangle rect = {x + mmm_get_x (client->mmm), y + mmm_get_y (client->mmm), width, height};
        if (width > 0)
          mrg_queue_draw (mrg, &rect);
      }
    }
  }
  return 1;
}

MrgHost *mrg_host_new (Mrg *mrg, const char *path)
{
  MrgHost *host = calloc (sizeof (MrgHost), 1);
  if (!path)
    path = "/tmp/mrg";
  init_env (host, path);
  host->mrg = mrg;

  mrg_add_idle (mrg, host_idle_check, host);
  return host;
}

void mrg_host_get_default_size (MrgHost *host, int *width, int *height)
{
  int w = host->default_width, h = host->default_height;
  if (w <= 0) w = mrg_width (host->mrg);
  if (h <= 0) h = mrg_height (host->mrg);

  if (width)  *width = w;
  if (height) *height = h;
}

void mrg_host_set_default_size (MrgHost *host, int width, int height)
{
  host->default_width  = width;
  host->default_height = height;
}

int mrg_client_get_pid (MrgClient *client)
{
  return client->pid;
}

void mrg_client_kill (MrgClient *client)
{
  if (client->pid > 0)
    {
      kill (client->pid, 9);
      client->pid = -2;
    }
}

const char *
mrg_client_get_message (MrgClient *client)
{
  if (client->mrg)
    return NULL;
  if (client->mmm)
    return mmm_get_message (client->mmm);
  return NULL;
}

int
mrg_client_has_message (MrgClient *client)
{
  if (client->mrg)
    return 0;
  if (client->mmm)
    return mmm_has_message (client->mmm);
  return 0;
}

void
mrg_client_send_message (MrgClient *client, const char *message)
{
  char *tmp = malloc (strlen (message) + strlen ("message  "));
  if (client->mrg)
    return;

  sprintf (tmp, "message %s", message);
  mmm_add_event (client->mmm, tmp);
  free (tmp);
}

const char *mrg_client_get_value (MrgClient *client, const char *name)
{
  if (client->mrg)
    return NULL;
  return mmm_get_value (client->mmm, name);
}

void mrg_client_set_value (MrgClient *client, const char *name, const char *value)
{
  if (client->mrg)
    return;
  mmm_set_value (client->mmm, name, value);
}

