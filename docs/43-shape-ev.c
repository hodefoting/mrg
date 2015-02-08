
#define _XOPEN_SOURCE 500
#include "mrg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void make_big ();

char *color = NULL;

int set_color_cb (MrgEvent *event,
     void *newcol, void *link_data) {
  if (color) free (color);
  color = strdup (newcol);
  return 0;
}

int zoom_cb (MrgEvent *event, void *d1, void *d2)
{
 double *zoom = d1;
 *zoom = (event->x-50) / 200.0;
 fprintf (stderr, "%f\n", event->x - 50);
 if (*zoom < 0.01) *zoom = 0.01;
 if (*zoom > 2.0) *zoom = 2.0;
 mrg_queue_draw (event->mrg, NULL);
 return 0;
}

void ui (Mrg *mrg, void *data) {
 static double zoom = 1.0;
 cairo_t *cr = mrg_cr (mrg);  
 make_big (mrg);

 cairo_scale (cr, zoom, zoom);

 mrg_printf_xml (mrg,
  "A <b style='color:%s'>color</b>.", color);

 cairo_arc (cr, 200, 150, 50, 0, M_PI*2);
 cairo_set_source_rgb (cr, 0,1,0);
 mrg_listen (mrg, MRG_PRESS, set_color_cb, "green", NULL);
 cairo_fill (cr);
 
 cairo_arc (cr, 300, 150, 50, 0, M_PI*2);
 cairo_set_source_rgb (cr, 0,0,1);
 mrg_listen (mrg, MRG_PRESS, set_color_cb, "blue", NULL);
 cairo_fill (cr);

 cairo_rectangle (cr, 50, 250-10, 400, 25);
 mrg_listen (mrg, MRG_DRAG, zoom_cb, &zoom, NULL);
 cairo_set_source_rgb (cr, 0, 0.4, 0);
 cairo_fill (cr);

 cairo_set_source_rgb (cr, 0, 0, 0);
 cairo_rectangle (cr, 50, 250, 400, 5);
 cairo_fill (cr);

 cairo_rectangle (cr, 50 + 200*zoom, 250-10, 5, 25);
 cairo_fill (cr);
}

int main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}

void make_big (Mrg *mrg) {
  mrg_set_font_size (mrg, 42);
  mrg_set_xy (mrg, 0, mrg_em (mrg));
}

/* cp=337 off=-483 */
