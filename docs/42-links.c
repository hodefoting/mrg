
#define _XOPEN_SOURCE 500
#include "mrg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void make_big ();

char *color = NULL;

void link_cb (MrgEvent *event,
     void *href, void *link_data) {
  if (color) free (color);
  color = strdup (href);
}

void ui (Mrg *mrg, void *data) {
  make_big (mrg);
  mrg_xml_renderf (mrg, NULL,
               link_cb, NULL,
"<p>Foo Change <span style='color:%s'>color</span></a> to"
" <a href='red'>red</a>, "
"<a href='green'>green</a>, "
"<a href='blue'>blue</a>, "
"<a href='yellow'>yellow</a> or "
"<a href='black'>black</a> ", color);
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

/* cp=337 off=-483 */
