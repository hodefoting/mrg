#include "mrg.h"
void make_big ();

const char *css =
".parent{ color:red; }\n"
"#child { color: blue;}";

static void ui (Mrg *mrg, void *data)
{
  mrg_set_font_size (mrg, 42);
  mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
  mrg_stylesheet_add (mrg, css, NULL, 0, NULL);
  mrg_start (mrg, ".parent", NULL);
  mrg_printf (mrg, "hello ");
  mrg_start (mrg, "#child", NULL);
  mrg_printf (mrg, "CSS\n");
  mrg_end (mrg);
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
}

/* cp=64 off=-46 */
