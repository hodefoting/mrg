#include "mrg.h"

void ui (Mrg *mrg, void *data) {
 mrg_printf (mrg, "hello world\n");
}

void main () {
 Mrg *mrg = mrg_new (512, 384, NULL);
 mrg_set_ui (mrg, ui, NULL);
 mrg_main (mrg);
}
/* cp=77 off=280*/
