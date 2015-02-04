#include "mrg.h"
void make_big ();

void ui (Mrg *mrg, void *data) {
  make_big (mrg);
  mrg_set_style (mrg, "color:red;");
  mrg_printf (mrg, "\nhello there\n");
}

int main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}

void make_big (Mrg *mrg) {
  mrg_set_font_size (mrg, 42);
  mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}

/* cp=159 off=279 */
