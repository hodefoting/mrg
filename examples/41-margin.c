#include "mrg.h" /* startline=14 */

void make_big (Mrg *mrg);
static char *xml;

float margin_right = 40;
float margin_left = 40;
float margin_top = 40;

int drag_right_cb (MrgEvent *event, void *data1, void *data2) { 
 margin_right -= event->delta_x;
 mrg_queue_draw (event->mrg, NULL);
 return 0;
}

int drag_left_cb (MrgEvent *event, void *data1, void *data2) { 
 margin_left += event->delta_x;
 mrg_queue_draw (event->mrg, NULL);
 return 0;
}

int drag_top_cb (MrgEvent *event, void *data1, void *data2) { 
 margin_top += event->delta_y;
 mrg_queue_draw (event->mrg, NULL);
 return 0;
}

void ui (Mrg *mrg, void *data) {
  cairo_t *cr = mrg_cr (mrg);
  float x, y;

  make_big (mrg);

  x = mrg_width (mrg) - margin_right;

  mrg_set_edge_left (mrg, margin_left);
  mrg_set_edge_right (mrg, x);
  mrg_set_edge_top (mrg, margin_top);

  cairo_set_source_rgb (cr, 1,0,0);
  cairo_move_to (cr, x, 0);
  cairo_line_to (cr, x, mrg_height (mrg));
  cairo_stroke (cr);

  mrg_listen (mrg, MRG_DRAG,
              x-10, 0,
                20, mrg_height (mrg),
              drag_right_cb, NULL, NULL);

  x = margin_left;

  cairo_move_to (cr, x, 0);
  cairo_line_to (cr, x, mrg_height (mrg));
  cairo_stroke (cr);

  mrg_listen (mrg, MRG_DRAG,
              x-10, 0,
                20, mrg_height (mrg),
              drag_left_cb, NULL, NULL);

  y = margin_top;

  cairo_move_to (cr, 0, y);
  cairo_line_to (cr, mrg_width (mrg), y);
  cairo_stroke (cr);

  mrg_listen (mrg, MRG_DRAG,
              0,y-10,
              mrg_width (mrg),20,
              drag_top_cb, NULL, NULL);

  mrg_printf_xml (mrg, "%s", xml);
}

void main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
}

void make_big (Mrg *mrg) {
  mrg_set_font_size (mrg, 23);
  mrg_set_xy (mrg, 0, mrg_em (mrg));
}

static char *xml =
"<h3>Not far from being a browser</h3><p>Single pass HTML layout is tricky, but the micro raptor parsing engine can make sense of some <em>basic</em> XHTML with CSS. Some of the limitations are:</p>"
"<ul style='margin-top:-1em;'><li>No centering</li>" 
"<li>float right only with width</li>"
"<li>hacks still needed for basic things</li>"
"</ul><pre>int main() {\n"
"  for (int i = 0; i &lt; 23; i++)\n"
"    { ... }\n"
"}</pre>";

/* cp=1000 off=-1600 */

