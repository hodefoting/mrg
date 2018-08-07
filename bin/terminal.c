/* This library is free software; you can redistribute it and/or
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
#include <mrg.h>
#include <mrg-vt.h>
#include <mrg-string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

float font_size = 16.0;
float line_spacing = 2.0;

static void event_handler (MrgEvent *event, void *data1, void *data2)
{
  MrgVT *vt = data1;
  const char *str = event->string;

  if (str && !strcmp (str, "resize-event"))
  {
    int width = mrg_width (event->mrg);
    int height =  mrg_height (event->mrg);
    int cols = width / 0.6 / font_size - 1;
    int rows = height / line_spacing / 0.6 / font_size - 1;

    mrg_vt_set_term_size (vt, cols, rows);
    mrg_event_stop_propagate (event);
  }
}

static void render_vt (Mrg *mrg, void *mrg_vt_data)
{
  MrgVT *vt = mrg_vt_data;
  if (mrg_vt_is_done (vt))
    mrg_quit (mrg);
  mrg_vt_draw (vt, mrg, 0, 0, font_size, line_spacing);
  cairo_new_path (mrg_cr (mrg));
  mrg_listen (mrg, MRG_KEY_DOWN, event_handler, vt, NULL);
}

static Mrg *mrg_tmp;
void
signal_child (int signum) {
{
  pid_t pid;
  int   status;
  while ((pid = waitpid(-1, &status, WNOHANG)) != -1)
    {
      fprintf (stderr, "death of %d\n", pid);//unregister_child(pid, status);
    }
}
#if 1
  if (mrg_tmp)
    mrg_quit (mrg_tmp);
  exit (0); /* XXX: make it less brutal? */
#endif
  //fprintf (stderr, "child foo\n");
}

int terminal_main (int argc, char **argv)
{
  Mrg *mrg;
  mrg = mrg_new ((DEFAULT_COLS+1) * font_size * 0.60, (DEFAULT_ROWS+1) * font_size * 0.60 * line_spacing, NULL);
  //mrg = mrg_new (-1, -1, NULL);
  mrg_tmp = mrg;

  signal (SIGCHLD, signal_child);
  MrgVT *vt = mrg_vt_new (mrg, argv[1]?argv[1]:mrg_vt_find_shell_command());

//  mrg_vt_set_term_size (vt, DEFAULT_COLS, DEFAULT_ROWS);
  {
     MrgEvent foo; foo.mrg = mrg;foo.string = NULL;
     event_handler (&foo, vt, NULL);
  }
  mrg_set_ui (mrg, render_vt, vt);
  mrg_set_title (mrg,  argv[1]?argv[1]:mrg_vt_find_shell_command());
  mrg_main (mrg);
  mrg_vt_destroy (vt);
  mrg_destroy (mrg);

  return 0;
}
