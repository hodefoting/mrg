#if 0
todo: permit clicking path bar
#endif

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include "mrg.h"
#include "mrg-string.h"
#include "host.h"

static int vertical_pan (MrgEvent *e, void *data1, void *data2)
{
  if (e->type == MRG_DRAG_MOTION && e->device_no == 1)
  {
    float *pos = data1;
    pos[1] += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }
  return 0;
}

typedef void (*UiCb) (Mrg *mrg, void *state);
/*
 *  top-level state, should just keep around a main ui context,. with a
 *  different sub-state?
 */

typedef struct _State State;

struct _State
{
  UiCb   ui;   /* XXX: must be first child, due to editor aliasing of struct */
  Mrg   *mrg;
  char  *path;

  char  *cached_path;
  State *sub_state; /* cached */
  Host  *host;
};

State *edit_state_new (const char *path);
void edit_state_destroy (State *state);

static float pos[2] = {0,0}; // XXX: move into state?

static void go_parent (State *state)
{
  char *lastslash = strrchr (state->path, '/');
  if (lastslash)
  {
    if (lastslash == state->path)
      lastslash[1] = '\0';
    else
      lastslash[0] = '\0';
    mrg_queue_draw (state->mrg, NULL);
  }
}

static void go_next (State *state)
{
  char *lastslash = strrchr (state->path, '/');
  system ("killall -9 mrg-tmp 2>&1");
  if (lastslash)
  {
    struct dirent **namelist;
    int n;

    if (lastslash == state->path)
      lastslash[1] = '\0';
    else
      lastslash[0] = '\0';

    n = scandir (state->path, &namelist, NULL, alphasort);
    if (n)
    {
      int i;
      int done = 0;
      for (i = 0; i < n; i ++)
      {
        if (!done && !strcmp (namelist[i]->d_name, lastslash+1) && i + 1 < n)
        {
          char *tmp = malloc (strlen (state->path) + 2 + strlen (namelist[i+1]->d_name));
          sprintf (tmp, "%s/%s", state->path, namelist[i+1]->d_name);
          free (state->path);
          state->path = tmp;
          done = 1;
        }
        free (namelist[i]);
      }
      free (namelist);

      if (!done)
        lastslash[0] = '/';
    }
    mrg_queue_draw (state->mrg, NULL);
  }
}

static void go_prev (State *state)
{
  char *lastslash = strrchr (state->path, '/');
  system ("killall -9 mrg-tmp 2>&1");
  if (lastslash)
  {
    struct dirent **namelist;
    int n;

    if (lastslash == state->path)
      lastslash[1] = '\0';
    else
      lastslash[0] = '\0';

    n = scandir (state->path, &namelist, NULL, alphasort);
    if (n)
    {
      int i;
      int done = 0;
      for (i = 0; i < n; i ++)
      {
        if (!done && i > 0 &&
            (namelist[i]->d_name[0] != '.') &&
            (namelist[i-1]->d_name[0] != '.') &&
            !strcmp (namelist[i]->d_name, lastslash+1))
        {
          char *tmp = malloc (strlen (state->path) + 2 + strlen (namelist[i-1]->d_name));
          sprintf (tmp, "%s/%s", state->path, namelist[i-1]->d_name);
          free (state->path);
          state->path = tmp;
          done = 1;
        }
      }
      for (i = 0; i < n; i ++)
        free (namelist[i]);
      free (namelist);
      if (!done)
        lastslash[0] = '/';
    }
    mrg_queue_draw (state->mrg, NULL);
  }
}

static int edit_cb (MrgEvent *event, void *data1, void *data2)
{
  State *state = data1;

  state->sub_state = edit_state_new (state->path);
  mrg_queue_draw (event->mrg, NULL);

  return 1;
}

static int go_next_cb (MrgEvent *event, void *data1, void *data2)
{
  go_next (data1);
  pos[0] = pos[1] = 0;
  return 1;
}

static int go_prev_cb (MrgEvent *event, void *data1, void *data2)
{
  go_prev (data1);
  pos[0] = pos[1] = 0;
  return 1;
}

static int go_parent_cb (MrgEvent *event, void *data1, void *data2)
{
  go_parent (data1);
  pos[0] = pos[1] = 0;
  return 1;
}

#if 1
static int toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
  return 1;
}
#endif

static int entry_pressed (MrgEvent *event, void *data1, void *data2)
{
  State *state = data2;
  struct dirent *entry = data1;

  if (!strcmp (entry->d_name, ".."))
  {
    go_parent (state);
    return 1;
  }

  switch (entry->d_type)
  {
    case DT_DIR:
    case DT_REG:
      {
        char *newpath = malloc (strlen (state->path) + strlen (entry->d_name) + 2);
#define PATH_SEP "/"

        if (!strcmp (state->path, PATH_SEP))
          sprintf (newpath, "%s%s", PATH_SEP, entry->d_name);
        else
          sprintf (newpath, "%s%s%s", state->path, PATH_SEP, entry->d_name);
        free (state->path);
        state->path = newpath;
        pos[0] = pos[1] = 0;
        mrg_queue_draw (event->mrg, NULL);
      }
    default:
    break;
  }
  return 0;
}

static void render_dir (Mrg *mrg, void *data)
{
  State *state = data;

  struct dirent **namelist;
  int n = scandir (state->path, &namelist, NULL, alphasort);
  if (!n)
  {
    go_parent (state);
    return;
  }

  mrg_start (mrg, "h2", NULL);
  mrg_print (mrg, state->path);
  mrg_end (mrg);

  if (n < 0)
  {
    perror ("scandir");
  }
  else
  {
    int i;
    mrg_start (mrg, "dir", NULL);
    for (i = 0; i < n; i ++)
    {
      if (strcmp (namelist[i]->d_name, "."))
      {
        struct stat stat_buf;
        char *newpath = malloc (strlen (state->path) + strlen (namelist[i]->d_name) + 2);
#define PATH_SEP "/"

        if (!strcmp (state->path, PATH_SEP))
          sprintf (newpath, "%s%s", PATH_SEP, namelist[i]->d_name);
        else
          sprintf (newpath, "%s%s%s", state->path, PATH_SEP, namelist[i]->d_name);

        lstat (newpath, &stat_buf);
        free (newpath);

        if (S_ISREG(stat_buf.st_mode))
          mrg_start (mrg, "entry.regular", NULL);
        else if (S_ISDIR(stat_buf.st_mode))
          mrg_start (mrg, "entry.dir", NULL);
        else if (S_ISCHR (stat_buf.st_mode))
          mrg_start (mrg, "entry.char", NULL);
        else if (S_ISLNK (stat_buf.st_mode))
          mrg_start (mrg, "entry.link", NULL);
        else
          mrg_start (mrg, "entry.unknown", NULL);

        if (S_ISDIR (stat_buf.st_mode))
        {
          /* mrg_start (mrg, "size.dir", NULL);
             mrg_print (mrg, "[DIR]");
             mrg_end (mrg);
           */
        }
        else
        {
          mrg_start (mrg, "size", NULL);
          if (stat_buf.st_size < 1024)
            mrg_printf (mrg, "%i", (int)stat_buf.st_size);
          else if (stat_buf.st_size < 1023 * 1024)
            mrg_printf (mrg, "%2.1fK", stat_buf.st_size / 1024.0);
          else
            mrg_printf (mrg, "%2.1fM", stat_buf.st_size / 1024.0 / 1024.0);
          mrg_end (mrg);
        }

        mrg_text_listen_full (mrg, MRG_CLICK, entry_pressed, namelist[i], state, (void*)free, NULL);

        mrg_print (mrg, namelist[i]->d_name);

        mrg_text_listen_done (mrg);
        mrg_end (mrg);
      }
      else
      {
        free (namelist[i]);
      }
    }
    mrg_end (mrg);
    free (namelist);
    
    mrg_add_binding (mrg, "escape",    "parent",   NULL, go_parent_cb, state);
    mrg_add_binding (mrg, " ",         "next",     NULL, go_next_cb, state);
    mrg_add_binding (mrg, "control- ", "previous", NULL, go_prev_cb, state);
  }
}

static int
file_get_contents (const char  *path,
                   char       **contents,
                   long        *length)
{
  FILE *file;
  long  size;
  long  remaining;
  char *buffer;

  file = fopen (path, "rb");

  if (!file)
    return -1;

  fseek (file, 0, SEEK_END);
  *length = size = remaining = ftell (file);
  rewind (file);
  buffer = malloc(size + 8);

  if (!buffer)
    {
      fclose(file);
      return -1;
    }

  remaining -= fread (buffer, 1, remaining, file);
  if (remaining)
    {
      fclose (file);
      free (buffer);
      return -1;
    }
  fclose (file);
  *contents = buffer;
  buffer[size] = 0;
  return 0;
}

static void ui_png (Mrg *mrg, void *data)
{
  State *state = data;
  mrg_image (mrg, 0, 0, mrg_width (mrg), -1, state->path);
  mrg_add_binding (mrg, "escape",    NULL, NULL, go_parent_cb, state);
  mrg_add_binding (mrg, " ",         NULL, NULL, go_next_cb, state);
  mrg_add_binding (mrg, "control- ", NULL, NULL, go_prev_cb, state);
}


static void ui_xhtml (Mrg *mrg, void *data)
{
  State *state = data;
  char *contents = NULL;
  long  length;

  MrgString *tmp = mrg_string_new ("file:///");
  mrg_string_append_str (tmp, state->path);
  mrg_stylesheet_clear (mrg);
  file_get_contents (state->path, &contents, &length);

  if (contents)
  {
    mrg_xml_render (mrg, tmp->str, contents, NULL, NULL);
    free (contents);
  }
  mrg_string_free (tmp, 1);

  mrg_add_binding (mrg, "escape",    NULL, NULL, go_parent_cb, state);
  mrg_add_binding (mrg, " ",         NULL, NULL, go_next_cb, state);
  mrg_add_binding (mrg, "control- ", NULL, NULL, go_prev_cb, state);
  mrg_add_binding (mrg, "control-e", NULL, NULL, edit_cb, state);
}

static void ui_txt (Mrg *mrg, void *data)
{
  State *state = data;
  char *contents = NULL;
  long  length;

  mrg_start (mrg, "h2", NULL);
    mrg_print (mrg, state->path);
  mrg_end (mrg);
  mrg_print (mrg, "\n");

  file_get_contents (state->path, &contents, &length);

  if (contents)
  {
    mrg_print (mrg, contents);
    free (contents);
  }

  mrg_add_binding (mrg, "escape",    NULL, NULL, go_parent_cb, state);
  mrg_add_binding (mrg, " ",         NULL, NULL, go_next_cb, state);
  mrg_add_binding (mrg, "control- ", NULL, NULL, go_prev_cb, state);
  mrg_add_binding (mrg, "control-e", NULL, NULL, edit_cb, state);
}

typedef struct _ExtHandler ExtHandler;

struct _ExtHandler {
  const char *suffix;
  UiCb        ui;
  State    *(*ui_cons) (const char *path);
};

static ExtHandler ext_handlers[]={
  {".png",  ui_png,   NULL},
  {".html", ui_xhtml, NULL},
  {".svg",  ui_xhtml, NULL},
  {".c",    NULL,     edit_state_new},
  {".txt",  ui_txt,   NULL},
  {NULL,    ui_txt,   NULL},
};

void resolve_renderer (State *state)
{
  struct stat stat_buf;
  
  if (state->cached_path)
  {
    if (!strcmp (state->path, state->cached_path))
      return;
    free (state->cached_path);
  }
  state->cached_path = strdup (state->path);

  lstat (state->path, &stat_buf);

  if (state->sub_state)
  {
    edit_state_destroy (state->sub_state);
    state->sub_state = NULL;
  }
  state->ui        = NULL;

  if (S_ISREG(stat_buf.st_mode))
  {
    int i;
    for (i = 0; i < sizeof(ext_handlers)/sizeof(ext_handlers[0]); i++)
    {
      if (ext_handlers[i].suffix)
      {
        if (strstr (state->path, ext_handlers[i].suffix))
        {
          if (ext_handlers[i].ui)
            state->ui = ext_handlers[i].ui;
          else
          {
            state->sub_state = ext_handlers[i].ui_cons (state->path);
            mrg_queue_draw (state->mrg, NULL);
          }
          return;
        }
      }
      else
      {
        state->ui = ext_handlers[i].ui;
        return;
      }
    }
  }
  else if (S_ISDIR(stat_buf.st_mode))
    state->ui = render_dir;
}

static int eeek (MrgEvent *e, void *data1, void *data2)
{
  system ("/tmp/test &");
  return 1;
}

static int reresolve_cb (MrgEvent *e, void *data1, void *data2)
{
  State *state = data1;
  if (state->sub_state)
  {
    edit_state_destroy (state->sub_state);
    state->sub_state = NULL;
  }
  mrg_queue_draw (e->mrg, NULL);
  return 0;
}

static void gui (Mrg *mrg, void *data)
{
  State *state = data;
  state->mrg = mrg;


  resolve_renderer (state);

  if (!state->sub_state)
    mrg_listen (mrg, MRG_DRAG, 0,0, mrg_width(mrg), mrg_height(mrg), vertical_pan, pos, NULL);

#if MRG_CAIRO
  cairo_save (mrg_cr (mrg));
  cairo_translate (mrg_cr (mrg), pos[0], pos[1]);
#endif


  if (state->sub_state)
  {
    state->sub_state->ui (mrg, state->sub_state);
  
    mrg_add_binding (mrg, "control-e", NULL, NULL, reresolve_cb, state);
  }
  else
  {
    mrg_set_edge_left (mrg, 10);

    if (!state->ui)
      go_parent (state);
    else
      state->ui (mrg, data);
  }

#if MRG_CAIRO
  cairo_restore (mrg_cr (mrg));
#endif

  host_render (mrg, state->host);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_add_binding (mrg, "F2", NULL, NULL, eeek, state);
  mrg_add_binding (mrg, "F11", NULL, NULL, go_prev_cb, state);
  mrg_add_binding (mrg, "F12", NULL, NULL, go_next_cb, state);
  mrg_add_binding (mrg, "F9", NULL, NULL, toggle_fullscreen_cb, state);

  mrg_add_binding (mrg, "control-right", NULL, NULL, go_next_cb, state);
  mrg_add_binding (mrg, "control-left", NULL, NULL, go_prev_cb, state);

}

extern int host_fixed_pos;

int dir_main (int argc, char **argv)
{
  Mrg *mrg;
  
  //if (getenv ("DISPLAY"))
  mrg = mrg_new (1024, 768, NULL);
 // else
 //   mrg = mrg_new (-1, -1, NULL);
  //Mrg *mrg = mrg_new (1024, 768, NULL);
  //Mrg *mrg = mrg_new (800, 600, NULL);

  State *state = calloc (sizeof (State), 1);
  {
    char *tmp = realpath (argv[1]?argv[1]:argv[0], NULL);
    state->path = strdup (tmp);
    state->host = host_new (mrg, "/tmp/foo");
  }
  
  host_fixed_pos = 1;

  mrg_set_ui (mrg, gui, state);
  mrg_main (mrg);

  free (state->path);
  free (state);

  return 0;
}
