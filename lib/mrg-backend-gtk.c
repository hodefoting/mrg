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


#include "mrg-config.h"
#if MRG_GTK
#include <gtk/gtk.h>
#include "mrg-internal.h"

typedef struct MrgGtk {
  GtkWidget *eventbox;
  GtkWidget *drawingarea;
  GtkWidget *window;
} MrgGtk;

static void mrg_gtk_flush (Mrg *mrg)
{
  if (mrg->cr)
  {
    mrg->cr = NULL;
  }
}
Mrg *mrg_gtk_get_mrg (GtkWidget *widget);

static gboolean draw (GtkWidget *widget, cairo_t *cr, void *userdata)
{
  Mrg    *mrg = userdata;
  MrgGtk *mrg_gtk = mrg->backend_data;

  mrg->cr = cr;


/*  if (_mrg_is_dirty (mrg)) */
       /* the gtk backend leaves dirty region handling up to gtk */
    {
      mrg->width = gtk_widget_get_allocated_width (mrg_gtk->eventbox);
      mrg->height = gtk_widget_get_allocated_height (mrg_gtk->eventbox);

      mrg_ui_update (mrg);
    }

  if (_mrg_has_quit (mrg))
    gtk_main_quit ();

  return FALSE;
}

static void mrg_gtk_warp_pointer (Mrg *mrg, float x, float y)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  GdkDisplay *display = gtk_widget_get_display (mrg_gtk->drawingarea);
  GdkDevice  *device;
  gint ox, oy;
  gdk_window_get_origin (gtk_widget_get_window (mrg_gtk->drawingarea), &ox, &oy);
  device = gdk_device_manager_get_client_pointer (gdk_display_get_device_manager (display));
  gdk_device_warp (device, gdk_display_get_default_screen (display), ox + x, oy + y);
}

static void mrg_gtk_main (Mrg *mrg,
                   void (*ui_update)(Mrg *mrg, void *user_data),
                   void *user_data)
{
  /* the gtk mrg backend directly reuses the gtk main loop */
  gtk_main ();
}


static cairo_t *mrg_gtk_cr (Mrg *mrg)
{
  static cairo_t *static_cr = NULL;
  cairo_surface_t *surface;

  if (mrg->cr)
    return mrg->cr;
  if (static_cr)
    return static_cr;

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        1,1);
  static_cr = cairo_create (surface);
  return static_cr;
}

static gboolean button_press_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_press (mrg, event->button.x, event->button.y, event->button.button);
}

static gboolean button_release_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_release (mrg, event->button.x, event->button.y, event->button.button);
}

static gboolean motion_notify_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_motion (mrg, event->motion.x, event->motion.y, 
      (event->motion.state&GDK_BUTTON1_MASK)?0:
      (event->motion.state&GDK_BUTTON2_MASK)?1:
      (event->motion.state&GDK_BUTTON3_MASK)?2:
      (event->motion.state&GDK_BUTTON4_MASK)?3:
      (event->motion.state&GDK_BUTTON5_MASK)?4:0);
}

static gboolean key_press_event (GtkWidget *window, GdkEvent *event, gpointer   mrg)
{
  const gchar *name = NULL;
  switch (event->key.keyval)
  {
    case GDK_KEY_BackSpace: name = "backspace"; break;
    case GDK_KEY_Delete:    name = "delete";    break;
    case GDK_KEY_Insert:    name = "insert";    break;
    case GDK_KEY_space:     name = "space";     break;
    case GDK_KEY_F1:        name = "F1";        break;
    case GDK_KEY_F2:        name = "F2";        break;
    case GDK_KEY_F3:        name = "F3";        break;
    case GDK_KEY_F4:        name = "F4";        break;
    case GDK_KEY_F5:        name = "F5";        break;
    case GDK_KEY_F6:        name = "F6";        break;
    case GDK_KEY_F7:        name = "F7";        break;
    case GDK_KEY_F8:        name = "F8";        break;
    case GDK_KEY_F9:        name = "F9";        break;
    case GDK_KEY_F10:       name = "F10";       break;
    case GDK_KEY_F11:       name = "F11";       break;
    case GDK_KEY_F12:       name = "F12";       break;
    case GDK_KEY_Escape:    name = "escape";    break;
    case GDK_KEY_Tab:       name = "tab";       break;
    case GDK_KEY_Up:        name = "up";        break;
    case GDK_KEY_Down:      name = "down";      break;
    case GDK_KEY_Left:      name = "left";      break;
    case GDK_KEY_Right:     name = "right";     break;
    case GDK_KEY_Return:    name = "return";    break;
    case GDK_KEY_Home:      name = "home";      break;
    case GDK_KEY_End:       name = "end";       break;
    case GDK_KEY_Page_Up:   name = "page-up";   break;
    case GDK_KEY_Page_Down: name = "page-down"; break;
    default: break;
  }

  if (event->key.state & GDK_CONTROL_MASK)
  {
    char buf[128];
    if (name)
    sprintf (buf, "control-%s", name);
    else
    sprintf (buf, "control-%c", event->key.keyval);
    name = g_intern_string (buf);
    if (event->key.state & GDK_SHIFT_MASK)
    {
      char buf[128];
      sprintf (buf, "shift-%s", name);
      name = g_intern_string (buf);
    }
  }
  if (event->key.state & GDK_MOD1_MASK)
  {
    char buf[128];
    if (name)
    sprintf (buf, "alt-%s", name);
    else
    sprintf (buf, "alt-%c", event->key.keyval);
    name = g_intern_string (buf);
    if (event->key.state & GDK_SHIFT_MASK)
    {
      char buf[128];
      sprintf (buf, "shift-%s", name);
      name = g_intern_string (buf);
    }
  }
  if (!name)
    name = event->key.string;

  return mrg_key_press (mrg, gdk_keyval_to_unicode (event->key.keyval), name);
}

static void mrg_gtk_queue_draw (Mrg *mrg, MrgRectangle *rectangle)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  gtk_widget_queue_draw_area (mrg_gtk->drawingarea, rectangle->x, rectangle->y, rectangle->width, rectangle->height);
}

static void mrg_gtk_destroy (Mrg *mrg)
{
  if (mrg->backend_data)
  {
    free (mrg->backend_data);
    mrg->backend_data = NULL;
  }
}

static void mrg_gtk_fullscreen (Mrg *mrg, int fullscreen)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  GtkWidget *window = gtk_widget_get_ancestor (mrg_gtk->drawingarea, GTK_TYPE_WINDOW);
  if (fullscreen)
    gtk_window_fullscreen (GTK_WINDOW (window));
  else
    gtk_window_unfullscreen (GTK_WINDOW (window));
  mrg->fullscreen = fullscreen;
}

static gboolean idle_iteration (void *data)
{
  _mrg_idle_iteration (data);
  return TRUE;
}

static void mrg_gtk_set_title (Mrg *mrg, const char *title)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  if (!mrg_gtk->window)
    return;
  gtk_window_set_title (GTK_WINDOW (mrg_gtk->window), title);;
}

static const char *mrg_gtk_get_title (Mrg *mrg)
{
  MrgGtk *mrg_gtk = mrg->backend_data;

  if (!mrg_gtk->window)
    return NULL;

  return gtk_window_get_title (GTK_WINDOW (mrg_gtk->window));
}

static void mrg_gtk_set_position  (Mrg *mrg, int x, int y)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  if (!mrg_gtk->window)
    return;
  gtk_window_move (GTK_WINDOW (mrg_gtk->window), x, y);
}

static void mrg_gtk_get_position  (Mrg *mrg, int *x, int *y)
{
  MrgGtk *mrg_gtk = mrg->backend_data;
  gint root_x, root_y;
  if (!mrg_gtk->window)
    return;
  gtk_window_get_position (GTK_WINDOW (mrg_gtk->window), &root_x, &root_y);
  if (x)
    *x = root_x;
  if (y)
    *y = root_y;
}

static Mrg *_mrg_gtk_new (int width, int height);

MrgBackend mrg_backend_gtk = {
  "gtk",
  _mrg_gtk_new,
  mrg_gtk_main,
  NULL, /* mrg_get_pixels */
  mrg_gtk_cr,
  mrg_gtk_flush,
  NULL, /* mrg_prepare */
  NULL, /* mrg_clear */
  mrg_gtk_queue_draw,
  mrg_gtk_destroy,
  mrg_gtk_warp_pointer,
  mrg_gtk_fullscreen,
  mrg_gtk_set_position,
  mrg_gtk_get_position,
  mrg_gtk_set_title,
  mrg_gtk_get_title,
};

GtkWidget *mrg_gtk_new (void (*ui_update)(Mrg *mrg, void *user_data),
                        void *user_data)
{
  Mrg *mrg;
  MrgGtk *mrg_gtk = calloc (sizeof (MrgGtk), 1);

  mrg_gtk->eventbox = gtk_event_box_new ();
  mrg_gtk->drawingarea = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER(mrg_gtk->eventbox), mrg_gtk->drawingarea);

  gtk_widget_add_events (mrg_gtk->eventbox, GDK_BUTTON_MOTION_MASK |
                                            GDK_POINTER_MOTION_MASK);

  gtk_widget_set_can_focus (mrg_gtk->eventbox, TRUE);

  mrg = calloc (sizeof (Mrg), 1);
  mrg->backend_data = mrg_gtk;
  mrg->backend = &mrg_backend_gtk;

  _mrg_init (mrg, 10, 10);
  
  mrg_set_ui (mrg, ui_update, user_data);

  g_signal_connect (mrg_gtk->eventbox, "button-press-event",
                    G_CALLBACK (button_press_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "button-release-event",
                    G_CALLBACK (button_release_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "motion-notify-event",
                    G_CALLBACK (motion_notify_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "key-press-event",
                    G_CALLBACK (key_press_event), mrg);

  g_signal_connect (mrg_gtk->drawingarea, "draw", G_CALLBACK (draw), mrg);

  g_timeout_add (30, idle_iteration, mrg);

  g_object_set_data_full (G_OBJECT (mrg_gtk->eventbox), "mrg", mrg, (void*)mrg_destroy);

  return mrg_gtk->eventbox;
}

Mrg *mrg_gtk_get_mrg (GtkWidget *widget)
{
  return g_object_get_data (G_OBJECT (widget), "mrg");
}

static Mrg *_mrg_gtk_new (int width, int height)
{
  GtkWidget *window;
  GtkWidget *canvas;
  Mrg *mrg;
  MrgGtk *mrg_gtk;
  int fullscreen = 0;

  if (!gtk_init_check (NULL, NULL))
    return NULL;

  if (width == -1)
  {
    width = 640;
    height = 480;
    fullscreen = 1;
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW(window), width, height);
  g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  canvas = mrg_gtk_new (NULL, NULL);
  if (!canvas)
    return NULL;
  mrg = mrg_gtk_get_mrg (canvas);
  mrg_gtk = mrg->backend_data;
  mrg_set_size (mrg, width, height);
  gtk_container_add (GTK_CONTAINER(window), canvas);
  gtk_widget_grab_focus (canvas);
  gtk_widget_show_all(window);

  mrg_gtk->window = window;

  if (fullscreen)
    mrg_set_fullscreen (mrg, fullscreen);

  return mrg;
}
#endif
