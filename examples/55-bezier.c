#include "mrg.h"

int drag_cb (MrgEvent *event, void *data1, void *data2);
float coord[7][2] = {{0.1, 0.5}, {0.2, 0.25}, {0.8, 0.75}, {0.9, 0.5}};

static void ui (Mrg *mrg, void *data) {
  cairo_t *cr = mrg_cr (mrg);
  cairo_translate (cr, (mrg_width (mrg) - mrg_height (mrg))/2, 0);
  cairo_scale (cr, mrg_height (mrg), mrg_height (mrg));
  cairo_set_line_width (cr, 0.02);
  cairo_move_to  (cr, coord[0][0], coord[0][1]);
  cairo_curve_to (cr, coord[1][0], coord[1][1],
                      coord[2][0], coord[2][1],
                      coord[3][0], coord[3][1]);
  cairo_set_source_rgb (cr, 1.0, 0.5, 0);
  cairo_stroke (cr);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);

  for (int i = 0; i < 4; i ++)
  {
    cairo_arc (cr, coord[i][0], coord[i][1],
               0.025, 0, 2 * 3.1415);
    cairo_fill (cr);
    mrg_listen (mrg, MRG_DRAG, coord[i][0] - .025, coord[i][1] - .025,
                     .05, .05, drag_cb, &coord[i][0], NULL);
  }
  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

void main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
}

int drag_cb (MrgEvent *event, void *data1, void *data2) {
  if (event->type == MRG_DRAG_MOTION)
  {
    float *pos = data1;
    pos[0] += event->delta_x;
    pos[1] += event->delta_y;
    mrg_queue_draw (event->mrg, NULL);
  }
  return 1;
}
