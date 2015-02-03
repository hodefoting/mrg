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

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg.h"
#include <dirent.h>
#include "mmm.h"
#include <unistd.h>
#include "mrg-list.h"
#include "host.h"

#if MRG_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#endif

#include "mrg-internal.h" // XXX: eeek

static const char *css =  " document {background-color:#111;} ";

typedef struct _Client Client;
typedef struct _Host   Host;

struct _Client
{
  /******************/

  long  pid;        /* magic hack, -1 means in-process */
  char *filename;
  Mmm  *mmm;
  
  /******************/

  Mrg         *mrg;
  float        int_x;
  float        int_y;

  int  premax_x;
  int  premax_y;
  int  premax_width;
  int  premax_height;
};

const char *client_get_title (Client *client)
{
  if (client->pid == -1)
    return mrg_get_title (client->mrg);
  return mmm_get_title (client->mmm);
}

#define TITLE_BAR_HEIGHT 16

struct _Host
{
  Mrg     *mrg;
  char    *fbdir;
  MrgList *clients;
  Client  *focused;
};

Host *host;

int host_client_has_focus (Host *host)
{
  return host->focused != NULL;
}

#if 0
static int toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
  return 1;
}
#endif

static void window_raise (Host *host, Client *focused)
{
  if (!focused)
    return;

  mrg_list_remove (&host->clients, focused);
  mrg_list_append (&host->clients, focused);
}

static int titlebar_drag (MrgEvent *e, void *client_, void *host_)
{
  Client *client = client_;

  if (e->type == MRG_DRAG_PRESS)
  {
    window_raise (host_, client);
  }
  else if (e->type == MRG_DRAG_MOTION)
  {
    if (client->pid == -1)
    {
      client->int_x += e->delta_x;
      client->int_y += e->delta_y;
    }
    else
    {
      mmm_set_x (client->mmm, mmm_get_x (client->mmm) + e->delta_x);
      mmm_set_y (client->mmm, mmm_get_y (client->mmm) + e->delta_y);
    }
    mrg_queue_draw (e->mrg, NULL);
  }

  return 0;
}

static int resize_drag (MrgEvent *e, void *client_, void *host_)
{
  Client *client = client_;

  if (e->type == MRG_DRAG_PRESS)
  {
    window_raise (host_, client);
  }
  else if (e->type == MRG_DRAG_MOTION)
  {
    int width, height;

    if (client->pid != -1)
    {
      mmm_host_get_size (client->mmm, &width, &height);
      width  += e->delta_x;
      height += e->delta_y;
      if (width > 80 && height > 40)
      {
        mmm_host_set_size (client->mmm, width, height);
        mrg_queue_draw (e->mrg, NULL);
      }
    }
    else
    {
      width = mrg_width (client->mrg);
      height = mrg_height (client->mrg);
      width += e->delta_x;
      height += e->delta_y;
      if (width > 80 && height > 40)
      {
        mrg_set_size (client->mrg, width, height);
        mrg_queue_draw (e->mrg, NULL);
      }
    }
  }

  return 0;
}

static int pos = 10;

int host_fixed_pos = 0;

static void validate_client (Host *host, const char *client_name)
{
  MrgList *l;
  for (l = host->clients; l; l = l->next)
  {
    Client *client = l->data;
    if (client->filename && 
        !strcmp (client->filename, client_name))
    {
      return;
    }
  }

  {
    Client *client = calloc (sizeof (Client), 1);

    char tmp[256];
    sprintf (tmp, "%s/%s", host->fbdir, client_name);
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
      if (mmm_get_x (client->mmm) == 0 &&
          mmm_get_y (client->mmm) == 0)
      {
        if (host_fixed_pos)
        {
          mmm_set_x (client->mmm, mrg_width (host->mrg) - 512 - 3);
          mmm_set_y (client->mmm, 3);
        }
        else
        {
          mmm_set_x (client->mmm, pos);
          mmm_set_y (client->mmm, 30+pos);

          pos += 12;
        }
      }
    }
    mrg_list_append (&host->clients, client);
  }
}

void add_client_mrg (Host        *host,
                     Mrg         *mrg,
                     float        x,
                     float        y)
{
  Client *client = calloc (sizeof (Client), 1);
  client->mmm = NULL;
  client->pid = -1;
  client->int_x = x;
  client->int_y = y;
  client->mrg = mrg;
  pos += 12;
  mrg_list_append (&host->clients, client);
}

void add_int_client (Host        *host,
                     UiRenderFun  ui,
                     void        *ui_data,
                     float        x,
                     float        y,
                     int          width,
                     int          height)
{
  Mrg *mrg = mrg_new (width, height, "mem");
  mrg_set_ui (mrg, ui, ui_data);
  return add_client_mrg (host, mrg, x, y);
}

#include <sys/stat.h>
#include <errno.h>

static int pid_is_alive (long pid)
{
  char path[256];
  struct stat sts;
  if (pid == -1)
    return 1; /* pid of -1 belongs to host */

  sprintf (path, "/proc/%li", pid);
  if (stat(path, &sts) == -1 && errno == ENOENT) {
    return 0;
  }
  return 1;
}

static void host_monitor_dir (Host *host)
{
  MrgList *l;
again:
  for (l = host->clients; l; l = l->next)
  {
    Client *client = l->data;
    if (!pid_is_alive (client->pid))
    {
      char tmp[256];
      sprintf (tmp, "%s/%s", host->fbdir, client->filename);
      if (client->mmm)
      {
        mmm_destroy (client->mmm);
        client->mmm = NULL;
      }

      unlink (tmp);
      mrg_queue_draw (host->mrg, NULL);

      free (client);
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

static int mrg_client_press (MrgEvent *event, void *client_, void *host_)
{
  Host *host = host_;
  Client *client = client_;
  Mmm *mmm = client->mmm;

  char buf[256];
  sprintf (buf, "mouse-press %f %f", event->x, event->y);
  if (host->focused)
  {
    window_raise (host, host->focused);
  }
  mmm_add_event (mmm, buf);
  return 0;
}

static int mrg_client_motion (MrgEvent *event, void *client_, void *host_)
{
  Client *client = client_;
  Mmm    *mmm    = client->mmm;

  char buf[256];
  if (event->mrg->pointer_down[1])
    sprintf (buf, "mouse-drag %f %f", event->x, event->y);
  else
    sprintf (buf, "mouse-motion %f %f", event->x, event->y);
  mmm_add_event (mmm, buf);
  return 0;
}

static int mrg_client_release (MrgEvent *event, void *client_, void *host_)
{
  Client *client = client_;
  Mmm *mmm = client->mmm;
  char buf[256];
  sprintf (buf, "mouse-release %f %f", event->x, event->y);
  mmm_add_event (mmm, buf);
  return 0;
}

static int show_cal = 0;

static int time_pressed(MrgEvent *event, void *mmm, void *data2)
{
  show_cal = !show_cal;
  mrg_queue_draw (event->mrg, NULL);
  return 0;
}

void terminal_main(int argc, char **argv);

static int applications_pressed (MrgEvent *event, void *mmm, void *data2)
{
  system("mrg-terminal &");
  return 0;
}

int host_key_down_cb (MrgEvent *event, void *host_, void *data2)
{
  Host *host = host_;
  if (host->focused)
  {
    fprintf (stderr, "Aaa\n");
    mmm_add_event (host->focused->mmm, event->key_name);
  }
  else
  {
    fprintf (stderr, "Bbb\n");
  }
  return 0;
}

#include <sys/types.h>
#include <signal.h>

static int kill_client (MrgEvent *event, void *client_, void *data2)
{
  Client *client = client_;
  if (client->pid != -1)
    kill (client->pid, 9);
  return 0;
}

static int maximize_client (MrgEvent *event, void *client_, void *host_)
{
  Client *client = client_;
  Host *host = host_;

  window_raise (host, client);

  if (client->pid == -1)
    return 0;

  if (mrg_width (event->mrg) == mmm_get_width (client->mmm) &&
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
        mrg_width (event->mrg),
        mrg_height (event->mrg) - TITLE_BAR_HEIGHT);
    mmm_set_x (client->mmm, 0);
    mmm_set_y (client->mmm, TITLE_BAR_HEIGHT);
  }

  return 0;
}

#include <stdio.h>
#include <time.h>

static void start_client (Mrg *mrg, int x, int y, int width, int height, int focused)
{
  char buf[1024];                                        // icky -2s to circumwent css box-model
  sprintf (buf, "left:%ipx;top:%ipx;width:%ipx;height:%ipx;", x-1, y-1-TITLE_BAR_HEIGHT, width, height + TITLE_BAR_HEIGHT);

  // XXX: use start_with_stylef this is what its there for 
  mrg_start_with_style (mrg, focused?"client.focused":"client", NULL, buf);
}
static void start_title (Mrg *mrg, int x, int y, int width, int height)
{
  char buf[1024];                                        // icky -2s to circumwent css box-model
  sprintf (buf, "left:%ipx;top:%ipx;width:%ipx;height:%ipx;", x+2, y-1, width-1, height);
  mrg_start_with_style (mrg, "title", NULL, buf);
}

void render_client (Host *host, Client *client, float ptr_x, float ptr_y)
{
  Mrg *mrg = host->mrg;
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  cairo_surface_t *surface;
#endif
  int width, height, rowstride;

  if (client->pid == getpid ())
    return;

  int cwidth, cheight;
  int skip_draw = 0;

  const unsigned char *pixels;
  
  int x, y;
  int count = 0;

  pixels = NULL;

  if (client->pid == -1)
  {
    x = client->int_x;
    y = client->int_y;
    width = cwidth = mrg_width (client->mrg);
    height = cheight = mrg_height (client->mrg);
  }
  else
  {
    x = mmm_get_x (client->mmm);
    y = mmm_get_y (client->mmm);
    mmm_host_get_size (client->mmm, &cwidth, &cheight);
  }

  if (x > mrg->dirty.x + mrg->dirty.width ||
      y > mrg->dirty.y + mrg->dirty.height ||
      x + cwidth < mrg->dirty.x ||
      y + cheight < mrg->dirty.y)
  {
    //    return; /* bailing earlier is better */
    skip_draw = 1;
  }
  else
  {
    if (client->pid == -1)
    {
      if (!host_fixed_pos)
      start_client (mrg, x, y, width, height, host->focused == client);
      mrg_render_to_mrg (client->mrg, mrg, x, y);
    }
    else
    {
      do
      {
        pixels = mmm_get_buffer_read (client->mmm, &width, &height, &rowstride);
        if (!pixels)
          usleep (5000);
        count ++;
      } while (pixels == NULL && count < 10);

      if (ptr_x >= x && ptr_x < x + width &&
          ptr_y >= y - TITLE_BAR_HEIGHT && ptr_y < y + height)
      {
        host->focused = client;
      }

      if (!host_fixed_pos)
      start_client (mrg, x, y, width, height, host->focused == client);

      if (pixels)
      {
#if MRG_CAIRO
        surface = cairo_image_surface_create_for_data ((void*)pixels, CAIRO_FORMAT_ARGB32, width, height, rowstride);
        cairo_save (cr);
        cairo_translate (cr, x, y);

        if (host_fixed_pos)
        {
          cairo_rectangle (cr, 0, 0, width, height);
          cairo_set_source_rgb (cr, 0,0,0);
          cairo_set_line_width (cr, 2.0);
          cairo_stroke (cr);
        }

        cairo_set_source_surface (cr, surface, 0, 0);
        if (host->focused == client || !host_fixed_pos)
          cairo_paint (cr);
        else
          cairo_paint_with_alpha (cr, 0.5);
        cairo_surface_destroy (surface);
#endif
        mmm_read_done (client->mmm);

        mrg_listen (mrg, MRG_PRESS, 0, 0, width, height,
                         mrg_client_press, client, host);
        mrg_listen (mrg, MRG_MOTION, 0, 0, width, height,
                         mrg_client_motion, client, NULL);
        mrg_listen (mrg, MRG_RELEASE, 0, 0, width, height,
                         mrg_client_release, client, NULL);

#if MRG_CAIRO
        cairo_restore (cr);
#endif
      }
      else
      {
        // not drawing anything.
      }
    }
  }

      if (!host_fixed_pos)
      {
  start_title (mrg, x - 3, y - TITLE_BAR_HEIGHT, width, TITLE_BAR_HEIGHT);
  mrg_listen (mrg, MRG_DRAG_MOTION, x, y - TITLE_BAR_HEIGHT, width, TITLE_BAR_HEIGHT,
      titlebar_drag, client, host);

  mrg_start (mrg, "close", NULL);
  mrg_text_listen (mrg, MRG_PRESS, kill_client, client, NULL);
  mrg_print (mrg, "X");
  mrg_text_listen_done (mrg);
  mrg_end (mrg);

  mrg_start (mrg, "max", NULL);
  mrg_text_listen (mrg, MRG_PRESS, maximize_client, client, host);
  mrg_print (mrg, "   ");
  mrg_text_listen_done (mrg);
  mrg_end (mrg);

  if (client->pid == -1)
    mrg_print (mrg, "-=[ internal ]=-");
  else
    mrg_print (mrg, mmm_get_title (client->mmm));

  if (!skip_draw)
  {
    mrg_end (mrg);
  }
  mrg_end (mrg);
      }

  if (cwidth)
  {
    /* resize box/handle */
#if MRG_CAIRO
    cairo_set_source_rgba (cr, 1,1,1, 0.5);
    cairo_rectangle (cr, x + cwidth - TITLE_BAR_HEIGHT, y + cheight - TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT);
#endif
    mrg_listen (mrg, MRG_DRAG_MOTION, x + cwidth - TITLE_BAR_HEIGHT, y + cheight - TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT,
        resize_drag, client, host);
#if MRG_CAIRO
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 0,0,0, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);
#endif
  }
}

static void draw_calendar (Mrg *mrg)
{
  FILE *pipe = popen("cal", "r");
  char buf[256];
  int bold = 0;
  int i;
  mrg_start (mrg, "calendar-wrap", NULL);
  mrg_start (mrg, "calendar", NULL);
  while (fgets(buf, sizeof(buf), pipe) != 0)
    for (i = 0; buf[i]; i++)
      switch (buf[i])
      {
        case '_': case '\b': bold = 1; break;
        default:
          if (bold)
            mrg_start_with_style (mrg, "span", NULL, "font-weight: bold; color: blue; ");
          mrg_printf (mrg, "%c", buf[i]);
          if (bold)
          {
            mrg_end (mrg);
            bold = 0;
          }
      }
  pclose(pipe);
  mrg_end (mrg);
  mrg_end (mrg);
}

void host_render (Mrg *mrg, Host *host)
{
  MrgList *l;
  float ptr_x = mrg_pointer_x (mrg);
  float ptr_y = mrg_pointer_y (mrg);

  host_monitor_dir (host);
  host->focused = NULL;

  if (host_fixed_pos)
  {
    for (l = host->clients; l; l = l->next)
    {
      Client *client = l->data;
      if (l->next)
      {
        if (client->pid != -1)
        {
          kill (client->pid, 9);
          client->pid = -2;
        }
      }
      else
      {
        render_client (host, client, ptr_x, ptr_y);
      }
    }
  }
  else
  for (l = host->clients; l; l = l->next)
  {
    Client *client = l->data;
    render_client (host, client, ptr_x, ptr_y);
  }
}

static void render_ui (Mrg *mrg, void *data)
{
  Host *host = data;

  mrg_start (mrg, "host", NULL);

  mrg_set_xy (mrg, 0, 0);
  mrg_start (mrg, "header", NULL);
  mrg_start (mrg, "time", NULL);
  {
    time_t secs               = time(0); struct tm *local = localtime(&secs);
    const char *day_names[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    mrg_text_listen (mrg, MRG_PRESS, time_pressed, NULL, NULL);
    mrg_printf(mrg, "%s %i %s %02d:%02d",
        day_names[local->tm_wday],
        local->tm_mday,
        month_names[local->tm_mon],
        local->tm_hour, local->tm_min);
    mrg_text_listen_done (mrg);
  }
  mrg_end (mrg);

  mrg_start (mrg, "applications", NULL);
  mrg_text_listen (mrg, MRG_PRESS, applications_pressed, NULL, NULL);
  mrg_printf(mrg, "$");
  mrg_text_listen_done (mrg);
  mrg_end (mrg);

  mrg_end (mrg);

  host_render (mrg, host);

  if (show_cal)
  {
    draw_calendar (mrg);
  }

  mrg_end (mrg);
  mrg_add_binding (mrg, "F10", NULL, NULL, mrg_quit_cb, NULL);
  mrg_listen (mrg, MRG_KEY_DOWN, 0,0,0,0, host_key_down_cb, host, NULL);
}

static void init_env (Host *host, const char *path)
{
  char buf[512];
  if (host->fbdir)
    return;
  host->fbdir = strdup (path);
  setenv ("MMM_PATH", host->fbdir, 1);
  sprintf (buf, "mkdir %s &> /dev/null", host->fbdir);
  system (buf);
}


void host_destroy (Host *host)
{
  free (host);
}

static int host_idle_check (Mrg *mrg, void *data)
{
  Host *host = data;
  MrgList *l;
  
  for (l = host->clients; l; l = l->next)
  {
    Client *client = l->data;

    int x, y, width, height;
    if (client->pid != -1)
    {
      if (mmm_get_damage (client->mmm, &x, &y, &width, &height))
      {
        if (width)
        {
          MrgRectangle rect = {x + mmm_get_x (client->mmm), y + mmm_get_y (client->mmm), width, height};
          mrg_queue_draw (mrg, &rect);
        }
        else
        {
          MrgRectangle rect = {mmm_get_x (client->mmm), mmm_get_y (client->mmm),
                               mmm_get_width (client->mmm), mmm_get_height (client->mmm)};
          mrg_queue_draw (mrg, &rect);
          fprintf (stderr, "dead code in mrg?\n");
        }
      }

    while (mmm_has_message (client->mmm))
    {
      fprintf (stderr, "%p: %s\n", client->mmm, mmm_get_message (client->mmm));
    }

    }
    else
    {
      // XXX
    }
  }
  return 1;
}

Host *host_new (Mrg *mrg, const char *path)
{
  Host *host = calloc (sizeof (Host), 1);
  init_env (host, path);
  host->mrg = mrg;
  mrg_add_idle (mrg, host_idle_check, host);
  return host;
}

static void tasklist_ui (Mrg *mrg, void *data)
{
  Host *host = data;
  MrgList *l;

  mrg_start (mrg, "tasklist", NULL);

  for (l = host->clients; l; l = l->next)
  {
    Client *client = l->data;
    mrg_printf (mrg, "%s\n", client_get_title (client));
  }

  mrg_end (mrg);
}

int host_main (int argc, char **argv)
{
  Mrg *mrg;

  //if (getenv ("DISPLAY"))
  //  mrg = mrg_new (640, 480, NULL);
  //else
  mrg = mrg_new (-1, -1, NULL);
  host = host_new (mrg, "/tmp/mmm");

  mrg_set_ui (mrg, render_ui, host);

  //system ("mrg-acoustics &");

  mrg_css_set (mrg, css);

  if(0)add_int_client (host, tasklist_ui, host, 40, 40, 300, 300);

  mrg_main (mrg);
  mrg_destroy (mrg);

  host_destroy (host);
  return 0;
}
