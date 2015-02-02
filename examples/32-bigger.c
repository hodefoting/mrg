#include "mrg.h"

void make_big (Mrg *mrg) {
 mrg_set_font_size (mrg, 42);
 mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}

void ui (Mrg *mrg, void *data) {
 make_big (mrg);
 mrg_printf (mrg, "hello void\n");
}

void main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
}
/* cp=70 off=250*/
