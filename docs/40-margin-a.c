#include "mrg.h"
void make_big (Mrg *mrg);

static char *xml =
"<h3>Not far from being a browser</h3><p>Single pass HTML layout is tricky, but the micro raptor parsing engine can make sense of some <em>basic</em> XHTML with CSS. Some of the limitations are:</p>"
"<ul style='margin-top:-1em;'><li>No centering</li>" 
"<li>%s</li>" 
"<li>float right only with width</li>"
"<li>hacks still needed for basic things</li>"
"</ul><pre>int main() {\n"
"  for (int i = 0; i &lt; 23; i++)\n"
"    { ... }\n"
"}</pre>";

void ui (Mrg *mrg, void *data) {
  make_big (mrg);
  mrg_set_edge_left (mrg, 20);
  mrg_set_edge_right (mrg, mrg_width(mrg)-20);
  mrg_printf_xml (mrg, xml, "no z-index");
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

/* cp=580 off=-382 */
