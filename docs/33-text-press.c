#include "mrg.h"
void make_big ();

int value = 11;
void inc_cb (MrgEvent *event,
             void *data1, void *data2) {
  value ++;
  mrg_queue_draw (event->mrg, NULL);
}

void ui (Mrg *mrg, void *data) {
 make_big (mrg);
 mrg_printf (mrg, "\nfoo ");
 mrg_text_listen (mrg, MRG_PRESS,
                  inc_cb, NULL, NULL);
 mrg_printf (mrg, "Hello %i World", value);
 mrg_text_listen_done (mrg);
 mrg_printf (mrg, "bar");
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

/* cp=280 off=-56 */
