#include "mrg.h"
#include "mrg-string.h"
void make_big ();

MrgEvent event_copy;

int handle_event_cb (MrgEvent *event,
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
      mrg, MRG_DRAG,
      handle_event_cb,
      NULL, NULL);
/* there is also a _full version, which takes
destroy_notify, and a destroy_notify_data args*/
 mrg_printf (mrg, "[event trap]\n");
 mrg_text_listen_done (mrg);

 mrg_printf (mrg, "type=");
 switch (event_copy.type)
 {
   case MRG_PRESS: mrg_print (mrg, "MRG_PRESS"); break;
   case MRG_MOTION: mrg_print (mrg, "MRG_MOTION"); break;
   case MRG_RELEASE: mrg_print (mrg, "MRG_RELEASE"); break;
   case MRG_ENTER: mrg_print (mrg, "MRG_ENTER"); break;
   case MRG_LEAVE: mrg_print (mrg, "MRG_LEAVE"); break;
   case MRG_KEY_DOWN: mrg_print (mrg, "MRG_KEY_DOWN"); break;
   case MRG_KEY_UP: mrg_print (mrg, "MRG_KEY_UP"); break;
   case MRG_DRAG_PRESS: mrg_print (mrg, "MRG_DRAG_PRESS"); break;
   case MRG_DRAG_MOTION: mrg_print (mrg, "MRG_DRAG_MOTION"); break;
   case MRG_DRAG_RELEASE: mrg_print (mrg, "MRG_DRAG_RELEASE"); break;
}
 mrg_print (mrg, "\n");

 if (event_copy.type == MRG_KEY_DOWN ||
     event_copy.type == MRG_KEY_UP)
 {
   mrg_printf (mrg, "unicode=%if", event_copy.unicode);
   mrg_printf (mrg, "key_name=\"%s\"", event_copy.key_name);
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

void main () {
  Mrg *mrg = mrg_new (512, 384, NULL);
  mrg_set_ui (mrg, ui, NULL);
  mrg_main (mrg);
}

void make_big (Mrg *mrg) {
 mrg_set_font_size (mrg, 34);
 mrg_set_xy (mrg, 0, mrg_em (mrg) * 1.2);
}

 /* cp=561 off=-481 */
