#include "mrg.h"
void make_big ();

static void ui (Mrg *mrg, void *data)
{
  make_big (mrg);
  mrg_printf (mrg, "hello little\n");
  mrg_start_with_style (mrg, "span",
                        NULL, "color:red;");
  mrg_printf (mrg, "world terra\n");
  mrg_end (mrg);
}

int main ()
{
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}

void make_big (Mrg *mrg)
{
  mrg_set_font_size (mrg, 42);
  mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}

/* cp=207 off=211 */
