#include "mrg.h"
#include "mrg-string.h"
#include <string.h>
void make_big ();

MrgEvent event_copy;

void handle_event_cb (MrgEvent *event,
                      void *data1, void *data2) {
  event_copy = *event;
  mrg_queue_draw (event->mrg, NULL);
}
 
void ui (Mrg *mrg, void *data) { 
 make_big (mrg);
 if(0)
   {
     cairo_translate (mrg_cr (mrg),mrg_width(mrg)/2, mrg_height(mrg)/2);
     cairo_rotate (mrg_cr (mrg), -0.3);
     cairo_scale (mrg_cr (mrg), 0.8, 0.8);
     cairo_translate (mrg_cr(mrg), -mrg_width(mrg)/2,-mrg_height(mrg)/2);
   }

 mrg_text_listen (
      mrg, MRG_DRAG | MRG_PRESS | MRG_RELEASE | MRG_CROSSING | MRG_SCROLL | MRG_TAP | MRG_TAP_AND_HOLD,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[many]");
 mrg_text_listen_done (mrg);

 mrg_text_listen (
      mrg, MRG_DRAG,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[drag]");
 mrg_text_listen_done (mrg);
 mrg_text_listen (
      mrg, MRG_PRESS | MRG_RELEASE,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[press]");
 mrg_text_listen_done (mrg);
 mrg_text_listen (
      mrg, MRG_SCROLL,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[scroll]");
 mrg_text_listen_done (mrg);

 mrg_text_listen (
      mrg, MRG_TAP | MRG_TAP_AND_HOLD,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[tap]");
 mrg_text_listen_done (mrg);

 mrg_text_listen (
      mrg, MRG_CROSSING,
      handle_event_cb,
      NULL, NULL);
 mrg_printf (mrg, "[crossing]");
 mrg_text_listen_done (mrg);
 mrg_printf (mrg, "\n");

 mrg_listen (mrg, MRG_KEY_DOWN, handle_event_cb, NULL, NULL);
 mrg_listen (mrg, MRG_KEY_UP, handle_event_cb, NULL, NULL);

 mrg_printf (mrg, "type=");
 switch (event_copy.type)
 {
   case MRG_PRESS: mrg_print (mrg, "MRG_PRESS"); break;
   case MRG_MOTION: mrg_print (mrg, "MRG_MOTION"); break;
   case MRG_RELEASE: mrg_print (mrg, "MRG_RELEASE"); break;
   case MRG_SCROLL: mrg_print (mrg, "MRG_ENTER"); break;
   case MRG_ENTER: mrg_print (mrg, "MRG_ENTER"); break;
   case MRG_LEAVE: mrg_print (mrg, "MRG_LEAVE"); break;
   case MRG_KEY_DOWN: mrg_print (mrg, "MRG_KEY_DOWN"); break;
   case MRG_KEY_UP: mrg_print (mrg, "MRG_KEY_UP"); break;
   case MRG_TAP: mrg_print (mrg, "MRG_TAP"); break;
   case MRG_DRAG_PRESS: mrg_print (mrg, "MRG_DRAG_PRESS"); break;
   case MRG_DRAG_MOTION: mrg_print (mrg, "MRG_DRAG_MOTION"); break;
   case MRG_DRAG_RELEASE: mrg_print (mrg, "MRG_DRAG_RELEASE"); break;
   case MRG_TAP_AND_HOLD: mrg_print (mrg, "MRG_TAP_AND_HOLD"); break;
   default: break;
}
 mrg_print (mrg, "\n");

 if (event_copy.type == MRG_KEY_DOWN ||
     event_copy.type == MRG_KEY_UP)
 {
   mrg_printf (mrg, "unicode=%if", event_copy.unicode);
   mrg_printf (mrg, "key_name=\"%s\"", event_copy.string);
   if (!strcmp (event_copy.string, "control-q"))
     mrg_quit (mrg); //exit(0);
 }
 else
 {
   mrg_printf (mrg, "device_id=%i\n", event_copy.device_no);
   mrg_printf (mrg, "x=%2.1f\n", event_copy.x);
   mrg_printf (mrg, "y=%2.1f\n", event_copy.y);
   mrg_printf (mrg, "start_x=%2.1f\n", event_copy.start_x);
   mrg_printf (mrg, "start_y=%2.1f\n", event_copy.start_y);
   mrg_printf (mrg, "delta_x=%2.1f\n", event_copy.delta_x);
   mrg_printf (mrg, "delta_y=%2.1f\n", event_copy.delta_y);
 }
}

int main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
  return 0;
}

void make_big (Mrg *mrg) {
 mrg_set_font_size (mrg, 34);
 mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}

 /* cp=561 off=-481 */
