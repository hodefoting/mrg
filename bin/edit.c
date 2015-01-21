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

static int drag_pos (MrgEvent *e, void *data1, void *data2)
{
  if (e->type == MRG_DRAG_MOTION && e->device_no == 1)
  {
    float *pos = data1;
    //pos[0] += e->delta_x;
    pos[1] += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }
  return 0;
}

typedef struct State
{
  Mrg   *mrg;
  char  *path;
  char  *data;
  long   length;
  int    mode;
} State;

static float pos[2] = {0,0};

#if 0
static int save_cb (MrgEvent *event, void *data1, void *data2)
{
  fprintf (stderr, "save!\n");
  return 1;
}
#endif

static int toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
  return 1;
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

  file = fopen (path,"rb");

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

#include "mrg-string.h"

static void gui (Mrg *mrg, void *data)
{
  State *state = data;

  mrg_listen (mrg, MRG_DRAG, 0,0,mrg_width(mrg), mrg_height(mrg), drag_pos, pos, NULL);

  mrg_start (mrg, "editor", NULL);

#if MRG_CAIRO
  cairo_translate (mrg_cr (mrg), pos[0], pos[1]);
#endif

  mrg_set_edge_left (mrg, 10);

  mrg_print (mrg, state->data);

  mrg_end (mrg);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_add_binding (mrg, "f", NULL, NULL, toggle_fullscreen_cb, NULL);
}

static void update_string (Mrg *mrg, char **string_loc, const char *new_string,
                               void *user_data)
{
  free (*string_loc);
  *string_loc = strdup (new_string);
}


int edit_main (int argc, char **argv)
{
  //Mrg *mrg = mrg_new (-1, -1, NULL);
  State *state = calloc (sizeof (State), 1);
  Mrg *mrg;
  {
    char *tmp = realpath (argv[1]?argv[1]:argv[0], NULL);
    state->path = strdup (tmp);
  }
  file_get_contents (state->path, &state->data, &state->length);
  if (!state->data)
    return -1;

  mrg = mrg_new (480, 640, NULL);

  state->mrg = mrg;

  mrg_edit_string (mrg, &state->data, update_string, NULL);
  mrg_set_ui (mrg, gui, state);
  mrg_main (mrg);

  free (state->path);
  free (state);
  return 0;
}
