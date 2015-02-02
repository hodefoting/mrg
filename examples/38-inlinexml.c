#include "mrg.h"
void make_big ();

const char *css = 
"#foo { color: blue;"
"      font-size:1.2em;}";

void ui (Mrg *mrg, void *data) {
  make_big (mrg);
  mrg_stylesheet_add (mrg, css, NULL, 0, NULL);
  mrg_printf_xml (mrg, "hello <span id='foo'>XML</div>");
}

void main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
}

void make_big (Mrg *mrg) {
  mrg_set_font_size (mrg, 42);
  mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}
