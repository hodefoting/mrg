#include "mrg.h"

void drag_pos (MrgEvent *e, void *data1, void *data2) {
  if (e->type == MRG_DRAG_MOTION)
  {
    float *pos = data1;
    pos[0] += e->delta_x;
    pos[1] += e->delta_y;
    mrg_queue_draw (e->mrg, NULL);
  }
  else if (e->type == MRG_ENTER)
  {
    int *active = data2;
    *active = 1;
    mrg_queue_draw (e->mrg, NULL);
  }
  else if (e->type == MRG_LEAVE)
  {
    int *active = data2;
    *active = 0;
    mrg_queue_draw (e->mrg, NULL);
  }
}

void render_ui (Mrg *mrg, void *data) {
  {
    static float pos[2] = {10, 200};
    static int active = 0;

    mrg_set_font_size (mrg, 42);
    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
      mrg_set_style (mrg, "color:red");
    else
      mrg_set_style (mrg, "color:green");

    mrg_print (mrg, "mmm");
    mrg_text_listen_done (mrg);
  }

  {
    static float pos[2] = {100, 100};
    int active = 0;

    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
    mrg_set_style (mrg, "color:red");
    else
    mrg_set_style (mrg, "color:green");

    mrg_print (mrg, "mrg");
    mrg_text_listen_done (mrg);
  }

  {
    static float pos[2] = {300, 300};
    int active = 0;

    mrg_set_edge_left (mrg, pos[0]);
    mrg_set_edge_top (mrg, pos[1]);
    mrg_text_listen (mrg, MRG_DRAG|MRG_ENTER|MRG_LEAVE, drag_pos, pos, &active);
    if (active)
    mrg_set_style (mrg, "color:red");
    else
    mrg_set_style (mrg, "color:green");

    mrg_print (mrg, "nchanterm");
    mrg_text_listen_done (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}

int main () {
  Mrg *mrg = mrg_new (640, 480, NULL);
  mrg_set_ui (mrg, render_ui, NULL);
  mrg_main (mrg);
  return 0;
}
