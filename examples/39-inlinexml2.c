#include "mrg.h"
void make_big ();

static void ui (Mrg *mrg, void *data) {
  make_big (mrg);
  mrg_printf_xml (mrg,
  "<style>#foo{color:%s;}</style>\n" 
  "<div style='font-size:1.2em;'>\n"
  "four <em>wings</em> <span id='foo'>good</span></div> ", 
    "green");
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

/* cp=191 off=200 */
