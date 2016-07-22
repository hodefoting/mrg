#include <mrg.h>
#include "mrg-host.h"

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>


static const char *css =  " document {background-color:#111;} ";

static void titlebar_drag (MrgEvent *e, void *client_, void *host_)
{
  MrgClient *client = client_;

  if (e->type == MRG_DRAG_PRESS)
  {
    mrg_client_raise_top (client);
  }
  else if (e->type == MRG_DRAG_MOTION)
  {
    mrg_client_set_x (client, mrg_client_get_x (client) + e->delta_x);
    mrg_client_set_y (client, mrg_client_get_y (client) + e->delta_y);
    mrg_queue_draw (e->mrg, NULL);
  }
}

static void resize_drag (MrgEvent *e, void *client_, void *host_)
{
  MrgClient *client = client_;

  if (e->type == MRG_DRAG_PRESS)
  {
    mrg_client_raise_top (client);
  }
  else if (e->type == MRG_DRAG_MOTION)
  {
    int width, height;

    mrg_client_get_size (client, &width, &height);
    width  += e->delta_x;
    height += e->delta_y;
    if (width > 80 && height > 40)
    {
      mrg_client_set_size (client, width, height);
      mrg_queue_draw (e->mrg, NULL);
    }
  }
}

static int show_cal = 0;

static void time_pressed (MrgEvent *event, void *mmm, void *data2)
{
  show_cal = !show_cal;
  mrg_queue_draw (event->mrg, NULL);
}

void terminal_main(int argc, char **argv);

static void launch_terminal (MrgEvent *event, void *mmm, void *data2)
{
  system("mrg-terminal &");
}

static void launch_browser (MrgEvent *event, void *mmm, void *data2)
{
  system("mrg browser mrg:index.html &");
}

static void kill_client (MrgEvent *event, void *client_, void *data2)
{
  MrgClient *client = client_;
  mrg_client_kill (client);
}

static void maximize_client (MrgEvent *event, void *client_, void *host_)
{
  MrgClient *client = client_;
  mrg_client_maximize (client);
}

static void render_client (Mrg *mrg, MrgHost *host, MrgClient *client)
{
  cairo_t *cr = mrg_cr (mrg);
  int width, height;

  if (mrg_client_get_pid (client) == getpid ()) /* avoid recursion */
    return;

  int skip_draw = 0;
  int x, y;

  mrg_client_get_size (client, &width, &height);
  x = mrg_client_get_x (client);
  y = mrg_client_get_y (client);

   // XXX: this is broken - and makes things flicker, should be fixed to actually work
#if 0
  if (!mrg_in_dirty_rect (mrg, x, y, width, height))
  {
    //    return; /* bailing earlier is better */
    skip_draw = 1;
  }
  else
#endif
  {
    mrg_start_with_stylef (mrg, mrg_host_get_focused(host)==client?"client.focused":"client", NULL, 
    "left:%ipx;top:%ipx;width:%ipx;height:%ipx;", x-1, y-1-TITLE_BAR_HEIGHT, width, height + TITLE_BAR_HEIGHT);
    mrg_client_render_sloppy (client, x, y);
  }

  mrg_start_with_stylef (mrg, "title", NULL, "left:%ipx;top:%ipx;width:%ipx;height:%ipx;", x-1, y-1 - TITLE_BAR_HEIGHT, width-1, TITLE_BAR_HEIGHT);

  cairo_save (cr);
  cairo_new_path (cr);
  cairo_rectangle (cr, x, y - TITLE_BAR_HEIGHT, width, TITLE_BAR_HEIGHT);
  cairo_restore (cr);

  mrg_listen (mrg, MRG_DRAG_MOTION, titlebar_drag, client, host);

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

  mrg_print (mrg, mrg_client_get_title (client));

  if (!skip_draw)
  {
    mrg_end (mrg);
  }
  mrg_end (mrg);

  if (width)
  {
    /* resize box/handle */
    cairo_set_source_rgba (cr, 1,1,1, 0.5);
    cairo_new_path (cr);
    cairo_rectangle (cr, x + width - TITLE_BAR_HEIGHT, y + height - TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT);
    mrg_listen (mrg, MRG_DRAG_MOTION, resize_drag, client, host);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 0,0,0, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);
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

void mrg_host_render (Mrg *mrg, MrgHost *host)
{
  mrg_host_monitor_dir (host);

  mrg_host_set_focused (host, NULL);
  for (MrgList *l = mrg_host_clients (host); l; l = l->next)
    render_client (mrg, host, (MrgClient*)l->data);

  mrg_host_register_events (host);
}

static void render_ui (Mrg *mrg, void *data)
{
  MrgHost *host = data;

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
  mrg_text_listen (mrg, MRG_PRESS, launch_terminal, NULL, NULL);
  mrg_printf(mrg, " $  ");
  mrg_text_listen_done (mrg);

  mrg_text_listen (mrg, MRG_PRESS, launch_browser, NULL, NULL);
  mrg_printf(mrg, " @ ");
  mrg_text_listen_done (mrg);

  mrg_end (mrg);

  mrg_end (mrg);

  mrg_host_render (mrg, host);

  if (show_cal)
  {
    draw_calendar (mrg);
  }

  mrg_end (mrg);
  mrg_add_binding (mrg, "F10", NULL, NULL, mrg_quit_cb, NULL);
}

static void tasklist_ui (Mrg *mrg, void *data)
{
  MrgHost *host = data;
  MrgList *l;

  mrg_start (mrg, "tasklist", NULL);

  for (l = mrg_host_clients (host); l; l = l->next)
  {
    MrgClient *client = l->data;
    mrg_printf (mrg, "%s\n", mrg_client_get_title (client));
  }

  mrg_end (mrg);
}

MrgHost *host;

#if 0
static int toggle_fullscreen_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_set_fullscreen (event->mrg, !mrg_is_fullscreen (event->mrg));
  return 1;
}
#endif

int host_main (int argc, char **argv)
{
  Mrg *mrg;

  if (getenv ("DISPLAY"))
    mrg = mrg_new (640, 480, NULL);
  else
    mrg = mrg_new (-1, -1, NULL);
  host = mrg_host_new (mrg, "/tmp/mrg");

  mrg_set_ui (mrg, render_ui, host);

  //system ("mrg-acoustics &");

  mrg_css_set (mrg, css);

  if(0)
  {
    Mrg *mrg = mrg_new (300, 300, "mem");
    mrg_set_ui (mrg, tasklist_ui, host);
    mrg_set_title (mrg, "tasklist");
    mrg_host_add_client_mrg (host, mrg, 40, 40);
  }

  mrg_main (mrg);
  mrg_destroy (mrg);

  mrg_host_destroy (host);
  return 0;
}

