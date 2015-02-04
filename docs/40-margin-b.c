#include "mrg.h" /* startline=16 */

void make_big (Mrg *mrg);
static char *xml;

float margin_right = 40;

int drag_cb (MrgEvent *event, void *data1, void *data2) { 
 margin_right -= event->delta_x;
 mrg_queue_draw (event->mrg, NULL);
 return 0;
}

void ui (Mrg *mrg, void *data) {
  cairo_t *cr = mrg_cr (mrg);
  float x = mrg_width (mrg) - margin_right;
  make_big (mrg);

  mrg_set_edge_left (mrg, 20);
  mrg_set_edge_right (mrg, x); 
  mrg_printf_xml (mrg, "%s", xml);

  cairo_set_source_rgb (cr, 1,0,0);
  cairo_move_to (cr, x, 0);
  cairo_line_to (cr, x, mrg_height (mrg));
  cairo_stroke (cr);

  mrg_listen (mrg, MRG_DRAG,
              x-10, 0,
                20, mrg_height (mrg),
              drag_cb, NULL, NULL);
}

int main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}

void make_big (Mrg *mrg) {
  mrg_set_font_size (mrg, 23);
  mrg_set_xy (mrg, 0, mrg_em (mrg));
}

static char *xml =
"<h3>License:</h3><p>LGPLv2+, with patch contributions to be licensed under MIT on a per patch basis - permitting possible future more premissive changes to the license.</p>"
"<ul style='margin-top:-1em;'><li>No centering</li>" 
"<li>float right only with width</li>"
"<li>hacks still needed for basic things</li>"
"</ul><pre>int main() {\n"
"  for (int i = 0; i &lt; 23; i++)\n"
"    { ... }\n"
"}</pre>";

/* cp=623 off=-752*/
