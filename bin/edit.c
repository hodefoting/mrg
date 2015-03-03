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

#define _DEFAULT_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include "mrg.h"
#include "mrg-string.h"

static int drag_pos (MrgEvent *e, void *data1, void *data2)
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

typedef struct State
{
  UiCb       ui;
  Mrg       *mrg;
  char      *path;
  char      *data;
  long       length;
  int        mode;
  MrgString *compiler_output;

  int        compile_timeout;

  int        started;
} State;

static float pos[2] = {0,0};

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

static void
file_set_contents (const char *path, const char *data, long length)
{
  FILE *fp = fopen (path, "wb");
  if (length == -1)
    length = strlen (data);
  fwrite(data, length, 1, fp);
  fclose (fp);
}

static int compile_cb (MrgEvent *event, void *data1, void *data2)
{
  FILE *fp;
  State *state = data1;

  int max_lines = 4;
  system ("rm -f /tmp/mrg-tmp");

  if (!strstr (state->path, ".c"))
    return 1;

  file_set_contents ("/tmp/live.c", state->data, -1);

  fp = popen ("rm -f /tmp/mrg-tmp ;gcc /tmp/live.c -std=c99  `pkg-config --cflags --libs mrg` -o /tmp/mrg-tmp 2>&1", "r");
  mrg_string_set (state->compiler_output, "");
  if (fp)
  {
    char buf[4096];
    while (fgets(buf, sizeof(buf)-1, fp) && --max_lines > 0)
      mrg_string_append_str (state->compiler_output, buf);
    pclose (fp);
  }
  else
  {
    mrg_string_append_str (state->compiler_output, "popen failed");
  }
  
  return 1;
}

static int save_cb (MrgEvent *event, void *data1, void *data2)
{
  State *state = data1;
  file_set_contents (state->path, state->data, -1);
  fprintf (stderr, "saved\n");
  return 1;
}

#include <unistd.h>

static int run_cb (MrgEvent *event, void *data1, void *data2)
{
  compile_cb (event, data1, data2);
  system ("/tmp/mrg-tmp &");
  usleep (30000);
  return 1;
}

static void update_string (const char *new_string, void *user_data)
{
  char **string_loc = user_data;
  free (*string_loc);
  *string_loc = strdup (new_string);
}

static char *last_str = NULL;

static int background_task (Mrg *mrg, void *data)
{
  State *state = data;

  if (last_str)
  {
    if (!strcmp (last_str, state->data))
    {
      state->compile_timeout = 0;
      mrg_queue_draw (mrg, NULL);
      return 0;
    }
    else
    {
      free (last_str);
      last_str = NULL;
    }
  }

  run_cb (NULL, state, NULL);
  last_str = strdup (state->data);
  mrg_queue_draw (mrg, NULL);
  state->compile_timeout = 0;

  return 0;
}

static int move_y = 0;

#include <sys/time.h>

static void gui (Mrg *mrg, void *data)
{
  State *state = data;

  if (move_y < 0)
  {
    pos[1] -= mrg_height (mrg) / 8;
  }
  else if (move_y > 0)
  {
    pos[1] += mrg_height (mrg) / 8;
  }

  state->mrg = mrg;

  if (!state->started)
  {
    if (strstr (state->data, "cp="))
    {
      int cursor_pos = atoi (strstr (state->data, "cp=") + strlen("cp="));
      mrg_set_cursor_pos (mrg, cursor_pos);
    }
    else
    {
      mrg_set_cursor_pos (mrg, 1);
    }
    if (strstr (state->data, "off="))
    {
      int offset = atoi (strstr (state->data, "off=") + strlen("off="));
      pos[1] = offset;
    }
    else
    {
      pos[1] = 0.0;
    }
    state->started = 1;
  }

  cairo_t *cr = mrg_cr (mrg);
  cairo_save (cr);
  cairo_new_path (cr);
  cairo_rectangle (cr, 0, 0, mrg_width (mrg), mrg_height (mrg));
  mrg_listen (mrg, MRG_DRAG, drag_pos, pos, NULL);
  cairo_new_path (cr);
  cairo_restore (cr);

  mrg_start (mrg, "editor", NULL);

  cairo_translate (mrg_cr (mrg), pos[0], pos[1]);
  mrg_set_edge_left (mrg, 10);

  mrg_print (mrg, "\n");

  {
    float x, y;
  
    mrg_print_get_xy (mrg, state->data, mrg_get_cursor_pos (mrg), &x, &y);

    y += pos[1];

    if (y > mrg_height (mrg) - mrg_em (mrg) * 1)
    {
      move_y = -1;
      mrg_queue_draw (mrg, NULL);
    }
    else if (y < mrg_em (mrg) * 2)
    {
      move_y = 1;
      mrg_queue_draw (mrg, NULL);
    }
    else
      move_y = 0;
  }

  mrg_set_style (mrg, "syntax-highlight: C");
  mrg_edit_start (mrg, update_string, &state->data);
  mrg_print (mrg, state->data);
  mrg_edit_end (mrg);

  mrg_end (mrg);

  {
    int cursor_pos = mrg_get_cursor_pos (mrg);
    int line_no = 0;
    int col_no = 0;
    int i;
    char *p;

    for (i = 0, p = state->data; *p && i < cursor_pos; p++, i++)
      if (*p=='\n')
      {
        line_no ++;
        col_no = 0;
      }
      else
      {
        col_no ++;
      }

    mrg_set_xy (mrg, 0, mrg_height (mrg));
    mrg_printf (mrg, "%s line %i col %i (%i:%.2f)", state->path, line_no + 1, col_no + 1, cursor_pos, pos[1]);

  {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    mrg_set_xy (mrg, mrg_width (mrg) - mrg_em (mrg) * 4, mrg_height (mrg));
    mrg_printf (mrg, "%02i:%02i", ((tv.tv_sec/60/60)%24)+1,
                                  ((tv.tv_sec/60)%60));
        
  }
  }

  mrg_add_binding (mrg, "F5", NULL, NULL, run_cb, state);
  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_add_binding (mrg, "control-s", NULL, NULL, save_cb, state);

  if (state->compiler_output->length > 0)
  mrg_printf_xml (mrg, "<div id='compiler_output'>%s</div> ",
                  state->compiler_output->str);

  if (!state->compile_timeout && 0)
  {
    state->compile_timeout = mrg_add_timeout (mrg, 500, 
      background_task, state);
  }

  if (move_y)
    mrg_queue_draw (mrg ,NULL);
}

State *edit_state_new (const char *path)
{
  State *state = calloc (sizeof (State), 1);
  state->path  = strdup (path);
  file_get_contents (state->path, &state->data, &state->length);
  state->ui    = gui;
  state->compiler_output = mrg_string_new ("");

  pos[0] = 0;
  pos[1] = 0;

  run_cb (NULL, state, NULL);

  state->started = 0;

  return state;
}

void edit_state_destroy (State *state)
{
  //mrg_edit_string (state->mrg, NULL, NULL, NULL);
  if (state->path)
    free (state->path);
  if (state->data)
    free (state->data);
  if (state->compile_timeout)
  {
    mrg_remove_idle (state->mrg, state->compile_timeout);
    state->compile_timeout = 0;
  }
  mrg_string_free (state->compiler_output, 1);
  free (state);
}

int edit_main (int argc, char **argv)
{
  State *state;
  Mrg *mrg;
  {
    char *tmp = realpath (argv[1]?argv[1]:argv[0], NULL);
    state = edit_state_new (tmp);
  }
  mrg = mrg_new (480, 640, NULL);
  //mrg = mrg_new (-1, -1, NULL);
  mrg_set_ui (mrg, state->ui, state);
  mrg_main (mrg);
  edit_state_destroy (state);
  return 0;
}
