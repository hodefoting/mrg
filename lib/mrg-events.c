/* mrg - MicroRaptor Gui
 * Copyright (c) 2014 Øyvind Kolås <pippin@hodefoting.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mrg-internal.h"

typedef struct _MrgGrab MrgGrab;

struct _MrgGrab
{
  MrgItem *item;
  int      device_no;
  int      timeout_id;
  int      start_time;
  float    x; // for tap and hold
  float    y;
  MrgType  type;
};

static void grab_free (Mrg *mrg, MrgGrab *grab)
{
  if (grab->timeout_id)
  {
    mrg_remove_idle (mrg, grab->timeout_id);
    grab->timeout_id = 0;
  }
  _mrg_item_unref (grab->item);
  free (grab);
}

static void device_remove_grab (Mrg *mrg, MrgGrab *grab)
{
  mrg_list_remove (&mrg->grabs, grab);
  grab_free (mrg, grab);
}

static MrgGrab *device_add_grab (Mrg *mrg, int device_no, MrgItem *item, MrgType type)
{
  MrgGrab *grab = calloc (1, sizeof (MrgGrab));
  grab->item = item;
  grab->type = type;
  _mrg_item_ref (item);
  grab->device_no = device_no;
  mrg_list_append (&mrg->grabs, grab);
  return grab;
}

MrgList *device_get_grabs (Mrg *mrg, int device_no)
{
  MrgList *ret = NULL;
  MrgList *l;
  for (l = mrg->grabs; l; l = l->next)
  {
    MrgGrab *grab = l->data;
    if (grab->device_no == device_no)
      mrg_list_append (&ret, grab);
  }
  return ret;
}

/* XXX: stopping sibling grabs should be an addtion to stop propagation,
 * this would permit multiple events to co-register, and use that
 * to signal each other,.. or perhaps more coordination is needed
 */
void _mrg_clear_text_closures (Mrg *mrg)
{
  int i;
  for (i = 0; i < mrg->text_listen_count; i ++)
  {
    if (mrg->text_listen_finalize[i])
       mrg->text_listen_finalize[i](
         mrg->text_listen_data1[i],
         mrg->text_listen_data2[i],
         mrg->text_listen_finalize_data[i]);
  }
  mrg->text_listen_count  = 0;
  mrg->text_listen_active = 0;
}

void mrg_clear (Mrg *mrg)
{
  if (mrg->frozen)
    return;
  mrg_list_free (&mrg->items);
  if (mrg->backend->mrg_clear)
    mrg->backend->mrg_clear (mrg);

  mrg_clear_bindings (mrg);
  _mrg_clear_text_closures (mrg);
}

static void restore_path (cairo_t *cr, cairo_path_t *path)
{
  //int i;
  //cairo_path_data_t *data;
  cairo_new_path (cr);
  cairo_append_path (cr, path);
}

/* using bigger primes would be a good idea, this falls apart due to rounding
 * when zoomed in close
 */
static double path_hash (cairo_path_t *path)
{
  int i;
  double ret = 0;
  cairo_path_data_t *data;
  if (!path)
    return 0.99999;
  for (i = 0; i <path->num_data; i += path->data[i].header.length)
  {
    data = &path->data[i];
    switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO:
        ret *= 17;
        ret += data[1].point.x;
        ret *= 113;
        ret += data[1].point.y;
        break;
      case CAIRO_PATH_LINE_TO:
        ret *= 121;
        ret += data[1].point.x;
        ret *= 1021;
        ret += data[1].point.y;
        break;
      case CAIRO_PATH_CURVE_TO:
        ret *= 3111;
        ret += data[1].point.x;
        ret *= 23;
        ret += data[1].point.y;
        ret *= 107;
        ret += data[2].point.x;
        ret *= 739;
        ret += data[2].point.y;
        ret *= 3;
        ret += data[3].point.x;
        ret *= 51;
        ret += data[3].point.y;
        break;
      case CAIRO_PATH_CLOSE_PATH:
        ret *= 51;
        break;
    }
  }
  return ret;
}

static int
path_equal (cairo_path_t *path,
            cairo_path_t *path2)
{
  int i;
  cairo_path_data_t *data;
  cairo_path_data_t *data2;

  if (!path || !path2)
    return 0;

  if (path->num_data != path2->num_data)
    return 0;

  for (i = 0; i <path->num_data; i += path->data[i].header.length)
  {
    int count = 0;
    data = &path->data[i];
    data2 = &path2->data[i];
    if (data->header.type != data2->header.type)
      return 0;
    if (data->header.type == CAIRO_PATH_CURVE_TO)
      count = 3;
    switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO: count = 1; break;
      case CAIRO_PATH_LINE_TO: count = 1; break;
      case CAIRO_PATH_CURVE_TO: count = 3; break;
      default: count = 0; break;
    }
    for (int j = 0; j < count; j ++)
      if (data[j].point.x != data2[j].point.x ||
          data[j].point.y != data2[j].point.y)
        return 0;
  }
  return 1;
}

MrgList *_mrg_detect_list (Mrg *mrg, float x, float y, MrgType type)
{
  MrgList *a;
  MrgList *ret = NULL;

  if (type == MRG_KEY_DOWN ||
      type == MRG_KEY_UP ||
      type == MRG_MESSAGE ||
      type == (MRG_KEY_DOWN|MRG_MESSAGE) ||
      type == (MRG_KEY_DOWN|MRG_KEY_UP) ||
      type == (MRG_KEY_DOWN|MRG_KEY_UP|MRG_MESSAGE))
  {
    for (a = mrg->items; a; a = a->next)
    {
      MrgItem *item = a->data;
      if (item->types & type)
      {
        mrg_list_prepend (&ret, item);
        return ret;
      }
    }
    return NULL;
  }

  for (a = mrg->items; a; a = a->next)
  {
    MrgItem *item= a->data;
  
    double u, v;
    u = x;
    v = y;
    cairo_matrix_transform_point (&item->inv_matrix, &u, &v);

    if (u >= item->x0 && v >= item->y0 &&
        u <  item->x1 && v <  item->y1 && 
        item->types & type)
    {
      if (item->path)
      {
        cairo_t *cr = mrg_cr (mrg);
        restore_path (cr, item->path);
        if (cairo_in_fill (cr, u, v))
        {
          cairo_new_path (cr);
          mrg_list_prepend (&ret, item);
        }
        cairo_new_path (cr);
      }
      else
      {
        mrg_list_prepend (&ret, item);
      }
    }
  }
  return ret;
}

MrgItem *_mrg_detect (Mrg *mrg, float x, float y, MrgType type)
{
  MrgList *l = _mrg_detect_list (mrg, x, y, type);
  if (l)
  {
    mrg_list_reverse (&l);
    MrgItem *ret = l->data;
    mrg_list_free (&l);
    return ret;
  }
  return NULL;
}

void _mrg_item_ref (MrgItem *mrgitem)
{
  if (mrgitem->ref_count < 0)
  {
    fprintf (stderr, "EEEEK!\n");
  }
  mrgitem->ref_count++;
}

void _mrg_item_unref (MrgItem *mrgitem)
{
  if (mrgitem->ref_count <= 0)
  {
    fprintf (stderr, "EEEEK!\n");
    return;
  }
  mrgitem->ref_count--;
  if (mrgitem->ref_count <=0)
  {
    {
      int i;
      for (i = 0; i < mrgitem->cb_count; i++)
      {
        if (mrgitem->cb[i].finalize)
          mrgitem->cb[i].finalize (mrgitem->cb[i].data1, mrgitem->cb[i].data2,
                                   mrgitem->cb[i].finalize_data);
      }
    }
    if (mrgitem->path)
    {
      cairo_path_destroy (mrgitem->path);
    }
    free (mrgitem);
  }
}

/* for more perfect inegration with cairo - should also support
 * listening for the shape of the current path, that makes the
 * integration even tighter.
 */

void mrg_listen (Mrg     *mrg,
                 MrgType  types,
                 MrgCb    cb,
                 void*    data1,
                 void*    data2)
{
  if (types == MRG_DRAG_MOTION)
    types = MRG_DRAG_MOTION | MRG_DRAG_PRESS;
  return mrg_listen_full (mrg, types, cb, data1, data2, NULL, NULL);
}

void mrg_listen_full (Mrg     *mrg,
                      MrgType  types,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2,
                                         void *finalize_data),
                      void    *finalize_data)
{
  float x, y, width, height;

  if (!mrg->frozen)
  {
    MrgItem *item;
    cairo_t *cr = mrg_cr (mrg);

    /* generate bounding box of what to listen for - from current cairo path */
    if (types & MRG_KEY)
    {
      x = 0;
      y = 0;
      width = 0;
      height = 0;
    }
    else
    {double ex1,ey1,ex2,ey2; 
     cairo_path_extents (cr, &ex1, &ey1, &ex2, &ey2);
     x = ex1;
     y = ey1;
     width = ex2 - ex1;
     height = ey2 - ey1;
    }

    /* early bail for listeners outside screen  */
    {
      double tx = x;
      double ty = y;
      double tw = width;
      double th = height;

      cairo_user_to_device (cr, &tx, &ty);
      cairo_user_to_device_distance (cr, &tw, &th);
      if (ty > mrg->height * 2 ||
          tx > mrg->width * 2 ||
          tx + tw < 0 ||
          ty + th < 0)
      {
        if (finalize)
          finalize (data1, data2, finalize_data);
        return;
      }
    }
    
    item = calloc (sizeof (MrgItem), 1);
    item->x0 = x;
    item->y0 = y;
    item->x1 = x + width;
    item->y1 = y + height;
    item->cb[0].types = types;
    item->cb[0].cb = cb;
    item->cb[0].data1 = data1;
    item->cb[0].data2 = data2;
    item->cb[0].finalize = finalize;
    item->cb[0].finalize_data = finalize_data;
    item->cb_count = 1;
    item->types = types;
    item->path = cairo_copy_path (cr);
    item->path_hash = path_hash (item->path);
    cairo_get_matrix (cr, &item->inv_matrix);
    cairo_matrix_invert (&item->inv_matrix);

    if (mrg->items)
    {
      MrgList *l;
      for (l = mrg->items; l; l = l->next)
      {
        MrgItem *item2 = l->data;

        /* store multiple callbacks for one entry when the paths
         * are exact matches, reducing per event traversal checks at the
         * cost of a little paint-hit (XXX: is this the right tradeoff,
         * perhaps it is better to spend more time during event processing
         * than during paint?)
         */
        if (item->path_hash == item2->path_hash &&
            path_equal (item->path, item2->path))
        {
          /* found an item, copy over cb data  */
          item2->cb[item2->cb_count] = item->cb[0];
          free (item);
          item2->cb_count++;
          item2->types |= types;
          /* increment ref_count? */
          return;
        }
      }
    }
    item->ref_count = 1;
    mrg_list_prepend_full (&mrg->items, item, (void*)_mrg_item_unref, NULL);
  }
}

static int
_mrg_emit_cb_item (Mrg *mrg, MrgItem *item, MrgEvent *event, MrgType type, float x, float y)
{
  static MrgEvent s_event;
  MrgEvent transformed_event;
  int i;

  if (!event)
  {
    event = &s_event;
    event->type = type;
    event->x = x;
    event->y = y;
  }
  event->mrg = mrg;
  transformed_event = *event;
  transformed_event.device_x = event->x;
  transformed_event.device_y = event->y;

  {
    double tx, ty;
    tx = transformed_event.x;
    ty = transformed_event.y;
  cairo_matrix_transform_point (&item->inv_matrix, &tx, &ty);
    transformed_event.x = tx;
    transformed_event.y = ty;

    if ((type & MRG_DRAG_PRESS) ||
        (type & MRG_DRAG_MOTION) ||
        (type & MRG_MOTION))   /* probably a worthwhile check for the performance 
                                  benefit
                                */
    {
      tx = transformed_event.start_x;
      ty = transformed_event.start_y;
    cairo_matrix_transform_point (&item->inv_matrix, &tx, &ty);
      transformed_event.start_x = tx;
      transformed_event.start_y = ty;
    }


    tx = transformed_event.delta_x;
    ty = transformed_event.delta_y;
  cairo_matrix_transform_distance (&item->inv_matrix, &tx, &ty);
    transformed_event.delta_x = tx;
    transformed_event.delta_y = ty;
  }

  transformed_event.state = mrg->modifier_state;
  transformed_event.type = type;

  for (i = item->cb_count-1; i >= 0; i--)
  {
    if (item->cb[i].types & type)
    {
      item->cb[i].cb (&transformed_event, item->cb[i].data1, item->cb[i].data2);
      event->stop_propagate = transformed_event.stop_propagate; /* copy back the response */
      if (event->stop_propagate)
        return event->stop_propagate;
    }
  }
  return 0;
}

static int
_mrg_emit_cb (Mrg *mrg, MrgList *items, MrgEvent *event, MrgType type, float x, float y)
{
  MrgList *l;
  event->stop_propagate = 0;
  for (l = items; l; l = l->next)
  {
    _mrg_emit_cb_item (mrg, l->data, event, type, x, y);
    if (event->stop_propagate)
      return event->stop_propagate;
  }
  return 0;
}

/*
 * update what is the currently hovered item and returns it.. and the list of hits
 * a well.
 *
 */
static MrgItem *_mrg_update_item (Mrg *mrg, int device_no, float x, float y, MrgType type, MrgList **hitlist)
{
  MrgItem *current = NULL;

  MrgList *l = _mrg_detect_list (mrg, x, y, type);
  if (l)
  {
    mrg_list_reverse (&l);
    current = l->data;
  }
  if (hitlist)
    *hitlist = l;
  else
    mrg_list_free (&l);


  if (mrg->prev[device_no] == NULL || current == NULL || (current->path_hash != mrg->prev[device_no]->path_hash))
  {
    int focus_radius = 2;
    if (current)
      _mrg_item_ref (current);

    if (mrg->prev[device_no])
    {
      {
        MrgRectangle rect = {floor(mrg->prev[device_no]->x0-focus_radius),
                             floor(mrg->prev[device_no]->y0-focus_radius),
                             ceil(mrg->prev[device_no]->x1)-floor(mrg->prev[device_no]->x0) + focus_radius * 2,
                             ceil(mrg->prev[device_no]->y1)-floor(mrg->prev[device_no]->y0) + focus_radius * 2};
        mrg_queue_draw (mrg, &rect);
      }

      _mrg_emit_cb_item (mrg, mrg->prev[device_no], NULL, MRG_LEAVE, x, y);
      _mrg_item_unref (mrg->prev[device_no]);
      mrg->prev[device_no] = NULL;
    }
    if (current)
    {
      {
        MrgRectangle rect = {floor(current->x0-focus_radius),
                             floor(current->y0-focus_radius),
                             ceil(current->x1)-floor(current->x0) + focus_radius * 2,
                             ceil(current->y1)-floor(current->y0) + focus_radius * 2};
        mrg_queue_draw (mrg, &rect);
      }
      _mrg_emit_cb_item (mrg, current, NULL, MRG_ENTER, x, y);
      mrg->prev[device_no] = current;
    }
  }
  current = _mrg_detect (mrg, x, y, type);
  return current;
}

static int tap_and_hold_fire (Mrg *mrg, void *data)
{
  MrgGrab *grab = data;
  MrgList *list = NULL;
  mrg_list_prepend (&list, grab->item);
  MrgEvent event = {0, };

  event.mrg = mrg;
  event.time = mrg_ms (mrg);

  event.device_x = 
  event.x = mrg->pointer_x[grab->device_no];
  event.device_y = 
  event.y = mrg->pointer_y[grab->device_no];

  // XXX: x and y coordinates
  int ret = _mrg_emit_cb (mrg, list, &event, MRG_TAP_AND_HOLD,
      mrg->pointer_x[grab->device_no], mrg->pointer_y[grab->device_no]);

  mrg_list_free (&list);

  grab->timeout_id = 0;

  return 0;

  return ret;
}

int mrg_pointer_drop (Mrg *mrg, float x, float y, int device_no, uint32_t time,
                      char *string)
{
  MrgList *l;
  MrgList *hitlist = NULL;

  mrg->pointer_x[device_no] = x;
  mrg->pointer_y[device_no] = y;
  if (device_no <= 3)
  {
    mrg->pointer_x[0] = x;
    mrg->pointer_y[0] = y;
  }

  if (device_no < 0) device_no = 0;
  if (device_no >= MRG_MAX_DEVICES) device_no = MRG_MAX_DEVICES-1;
  MrgEvent *event = &mrg->drag_event[device_no];

  if (time == 0)
    time = mrg_ms (mrg);

  event->x = x;
  event->y = y;

  event->delta_x = event->delta_y = 0;

  event->device_no = device_no;
  event->string    = string;
  event->time      = time;
  event->stop_propagate = 0;

  _mrg_update_item (mrg, device_no, x, y, MRG_DROP, &hitlist);

  for (l = hitlist; l; l = l?l->next:NULL)
  {
    MrgItem *mrg_item = l->data;
    _mrg_emit_cb_item (mrg, mrg_item, event, MRG_DROP, x, y);

    if (event->stop_propagate)
      l = NULL;
  }

  mrg_queue_draw (mrg, NULL); /* in case of style change, and more  */
  mrg_list_free (&hitlist);

  return 0;
}


int mrg_pointer_press (Mrg *mrg, float x, float y, int device_no, uint32_t time)
{
  MrgList *hitlist = NULL;
  mrg->pointer_x[device_no] = x;
  mrg->pointer_y[device_no] = y;
  if (device_no <= 3)
  {
    mrg->pointer_x[0] = x;
    mrg->pointer_y[0] = y;
  }

  if (device_no < 0) device_no = 0;
  if (device_no >= MRG_MAX_DEVICES) device_no = MRG_MAX_DEVICES-1;
  MrgEvent *event = &mrg->drag_event[device_no];

  if (time == 0)
    time = mrg_ms (mrg);

  event->x = event->start_x = event->prev_x = x;
  event->y = event->start_y = event->prev_y = y;

  event->delta_x = event->delta_y = 0;

  event->device_no = device_no;
  event->time      = time;
  event->stop_propagate = 0;

  if (mrg->pointer_down[device_no] == 1)
  {
    fprintf (stderr, "mrg thought device %i was already down\n", device_no);
  }
  /* doing just one of these two should be enough? */
  mrg->pointer_down[device_no] = 1;
  switch (device_no)
  {
    case 1:
      mrg->modifier_state |= MRG_MODIFIER_STATE_BUTTON1;
      break;
    case 2:
      mrg->modifier_state |= MRG_MODIFIER_STATE_BUTTON2;
      break;
    case 3:
      mrg->modifier_state |= MRG_MODIFIER_STATE_BUTTON3;
      break;
    default:
      break;
  }


  MrgGrab *grab = NULL;
  MrgList *l;

  _mrg_update_item (mrg, device_no, x, y, 
      MRG_PRESS | MRG_DRAG_PRESS | MRG_TAP | MRG_TAP_AND_HOLD, &hitlist);

  for (l = hitlist; l; l = l?l->next:NULL)
  {
    MrgItem *mrg_item = l->data;
    if (mrg_item &&
        ((mrg_item->types & MRG_DRAG)||
         (mrg_item->types & MRG_TAP) ||
         (mrg_item->types & MRG_TAP_AND_HOLD)))
    {
      grab = device_add_grab (mrg, device_no, mrg_item, mrg_item->types);
      grab->start_time = time;

      if (mrg_item->types & MRG_TAP_AND_HOLD)
      {
         grab->timeout_id = mrg_add_timeout (mrg, mrg->tap_delay_hold, tap_and_hold_fire, grab);
      }
    }
    _mrg_emit_cb_item (mrg, mrg_item, event, MRG_PRESS, x, y);
    if (!event->stop_propagate)
      _mrg_emit_cb_item (mrg, mrg_item, event, MRG_DRAG_PRESS, x, y);

    if (event->stop_propagate)
      l = NULL;
  }

  mrg_queue_draw (mrg, NULL); /* in case of style change, and more  */
  mrg_list_free (&hitlist);

  return 0;
}


void mrg_resized (Mrg *mrg, int width, int height, long time)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_KEY_DOWN);
  MrgEvent event = {0, };

  if (!time)
    time = mrg_ms (mrg);
  
  event.mrg = mrg;
  event.time = time;
  event.string = "resize-event"; /* gets delivered to clients as a key_down event, maybe message shouldbe used instead?
   */

  if (item)
  {
    event.stop_propagate = 0;
    _mrg_emit_cb_item (mrg, item, &event, MRG_KEY_DOWN, 0, 0);
  }
}

int mrg_pointer_release (Mrg *mrg, float x, float y, int device_no, uint32_t time)
{
  if (time == 0)
    time = mrg_ms (mrg);

  if (device_no < 0) device_no = 0;
  if (device_no >= MRG_MAX_DEVICES) device_no = MRG_MAX_DEVICES-1;
  MrgEvent *event = &mrg->drag_event[device_no];

  event->time = time;
  event->x = x;
  event->mrg = mrg;
  event->y = y;
  event->device_no = device_no;
  event->stop_propagate = 0;

  switch (device_no)
  {
    case 1:
      if (mrg->modifier_state & MRG_MODIFIER_STATE_BUTTON1)
        mrg->modifier_state -= MRG_MODIFIER_STATE_BUTTON1;
      break;
    case 2:
      if (mrg->modifier_state & MRG_MODIFIER_STATE_BUTTON2)
        mrg->modifier_state -= MRG_MODIFIER_STATE_BUTTON2;
      break;
    case 3:
      if (mrg->modifier_state & MRG_MODIFIER_STATE_BUTTON3)
        mrg->modifier_state -= MRG_MODIFIER_STATE_BUTTON3;
      break;
    default:
      break;
  }

  mrg_queue_draw (mrg, NULL); /* in case of style change */

  if (mrg->pointer_down[device_no] == 0)
  {
    fprintf (stderr, "device %i already up\n", device_no);
  }
  mrg->pointer_down[device_no] = 0;

  mrg->pointer_x[device_no] = x;
  mrg->pointer_y[device_no] = y;
  if (device_no <= 3)
  {
    mrg->pointer_x[0] = x;
    mrg->pointer_y[0] = y;
  }
  MrgList *hitlist = NULL;
  MrgList *grablist = NULL , *g= NULL;
  MrgGrab *grab;

  _mrg_update_item (mrg, device_no, x, y, MRG_RELEASE | MRG_DRAG_RELEASE, &hitlist);
  grablist = device_get_grabs (mrg, device_no);

  for (g = grablist; g; g = g->next)
  {
    grab = g->data;

    if (!event->stop_propagate)
    {
      if (grab->item->types & MRG_TAP)
      {
        long delay = time - grab->start_time;

        if (delay > mrg->tap_delay_min &&
            delay < mrg->tap_delay_max &&
            sqrt(
              (event->start_x - x) * (event->start_x - x) +
              (event->start_y - y) * (event->start_y - y)) < mrg->tap_hysteresis
            )
        {
          _mrg_emit_cb_item (mrg, grab->item, event, MRG_TAP, x, y);
        }
      }

      if (!event->stop_propagate && grab->item->types & MRG_DRAG_RELEASE)
      {
        _mrg_emit_cb_item (mrg, grab->item, event, MRG_DRAG_RELEASE, x, y);
      }
    }

    device_remove_grab (mrg, grab);
  }

  if (hitlist)
  {
    if (!event->stop_propagate)
      _mrg_emit_cb (mrg, hitlist, event, MRG_RELEASE, x, y);
    mrg_list_free (&hitlist);
  }
  mrg_list_free (&grablist);
  return 0;
}

/*  for multi-touch, need a list of active grabs - thus a grab corresponds to
 *  a device id. even during drag-grabs events propagate; to stop that stop
 *  propagation like for other events.
 *
 *
 */

int mrg_pointer_motion (Mrg *mrg, float x, float y, int device_no, uint32_t time)
{
  MrgList *hitlist = NULL;
  MrgList *grablist = NULL, *g;
  MrgGrab *grab;

  if (device_no < 0) device_no = 0;
  if (device_no >= MRG_MAX_DEVICES) device_no = MRG_MAX_DEVICES-1;
  MrgEvent *event = &mrg->drag_event[device_no];

  if (time == 0)
    time = mrg_ms (mrg);

  event->mrg  = mrg;
  event->x    = x;
  event->y    = y;
  event->time = time;
  event->device_no = device_no;
  event->stop_propagate = 0;
  
  mrg->pointer_x[device_no] = x;
  mrg->pointer_y[device_no] = y;

  if (device_no <= 3)
  {
    mrg->pointer_x[0] = x;
    mrg->pointer_y[0] = y;
  }

  /* XXX: too brutal; should use enter/leave events */
  if (getenv ("MRG_FAST") == NULL)
  mrg_queue_draw (mrg, NULL); /* XXX: not really needed for all backends,
                                      needs more tinkering */
  grablist = device_get_grabs (mrg, device_no);
  _mrg_update_item (mrg, device_no, x, y, MRG_MOTION, &hitlist);

  event->delta_x = x - event->prev_x;
  event->delta_y = y - event->prev_y;
  event->prev_x  = x;
  event->prev_y  = y;

  MrgList *remove_grabs = NULL;

  for (g = grablist; g; g = g->next)
  {
    grab = g->data;

    if ((grab->type & MRG_TAP) ||
        (grab->type & MRG_TAP_AND_HOLD))
    {
      if (
          sqrt (
            (event->start_x - x) * (event->start_x - x) +
            (event->start_y - y) * (event->start_y - y)) >
              mrg->tap_hysteresis
         )
      {
        fprintf (stderr, "-");
        mrg_list_prepend (&remove_grabs, grab);
      }
      else
      {
        fprintf (stderr, ":");
      }
    }

    if (grab->type & MRG_DRAG_MOTION)
    {
      _mrg_emit_cb_item (mrg, grab->item, event, MRG_DRAG_MOTION, x, y);
      if (event->stop_propagate)
        break;
    }
  }
  if (remove_grabs)
  {
    for (g = remove_grabs; g; g = g->next)
      device_remove_grab (mrg, g->data);
    mrg_list_free (&remove_grabs);
  }
  if (hitlist)
  {
    if (!event->stop_propagate)
      _mrg_emit_cb (mrg, hitlist, event, MRG_MOTION, x, y);
    mrg_list_free (&hitlist);
  }
  mrg_list_free (&grablist);
  return 0;
}

void mrg_incoming_message (Mrg *mrg, const char *message, long time)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_MESSAGE);
  MrgEvent event = {0, };

  if (!time)
    time = mrg_ms (mrg);

  if (item)
  {
    int i;
    event.mrg = mrg;
    event.type = MRG_MESSAGE;
    event.time = time;
    event.string = message;

    fprintf (stderr, "{%s|\n", message);

      for (i = 0; i < item->cb_count; i++)
      {
        if (item->cb[i].types & (MRG_MESSAGE))
        {
          event.state = mrg->modifier_state;
          item->cb[i].cb (&event, item->cb[i].data1, item->cb[i].data2);
          if (event.stop_propagate)
            return;// event.stop_propagate;
        }
      }
  }
}

int mrg_scrolled (Mrg *mrg, float x, float y, MrgScrollDirection scroll_direction, uint32_t time)
{
  MrgList *hitlist = NULL;
  MrgList *l;

  int device_no = 0;
  mrg->pointer_x[device_no] = x;
  mrg->pointer_y[device_no] = y;

  MrgEvent *event = &mrg->drag_event[device_no];  /* XXX: might
                                       conflict with other code
                                       create a sibling member
                                       of drag_event?*/
  if (time == 0)
    time = mrg_ms (mrg);

  event->x         = event->start_x = event->prev_x = x;
  event->y         = event->start_y = event->prev_y = y;
  event->delta_x   = event->delta_y = 0;
  event->device_no = device_no;
  event->time      = time;
  event->stop_propagate = 0;
  event->scroll_direction = scroll_direction;

  _mrg_update_item (mrg, device_no, x, y, MRG_SCROLL, &hitlist);

  for (l = hitlist; l; l = l?l->next:NULL)
  {
    MrgItem *mrg_item = l->data;

    _mrg_emit_cb_item (mrg, mrg_item, event, MRG_SCROLL, x, y);

    if (event->stop_propagate)
      l = NULL;
  }

  mrg_queue_draw (mrg, NULL); /* in case of style change, and more  */
  mrg_list_free (&hitlist);
  return 0;
}

int mrg_key_press (Mrg *mrg, unsigned int keyval,
                   const char *string, uint32_t time)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_KEY_DOWN);
  MrgEvent event = {0,};

  if (time == 0)
    time = mrg_ms (mrg);

  if (item)
  {
    int i;
    event.mrg = mrg;
    event.type = MRG_KEY_DOWN;
    event.unicode = keyval; 
    event.string = string;
    event.stop_propagate = 0;
    event.time = time;

    for (i = 0; i < item->cb_count; i++)
    {
      if (item->cb[i].types & (MRG_KEY_DOWN))
      {
        event.state = mrg->modifier_state;
        item->cb[i].cb (&event, item->cb[i].data1, item->cb[i].data2);
        if (event.stop_propagate)
          return event.stop_propagate;
      }
    }
  }
  return 0;
}

void mrg_freeze           (Mrg *mrg)
{
  mrg->frozen ++;
}

void mrg_thaw             (Mrg *mrg)
{
  mrg->frozen --;
}

float mrg_pointer_x (Mrg *mrg)
{
  return mrg->pointer_x[0];
}

float mrg_pointer_y (Mrg *mrg)
{
  return mrg->pointer_y[0];
}

void _mrg_debug_overlays (Mrg *mrg)
{
  MrgList *a;
  cairo_t *cr = mrg_cr (mrg);
  cairo_save (cr);

  cairo_set_line_width (cr, 2);
  cairo_set_source_rgba (cr, 0,0,0.8,0.5);
  for (a = mrg->items; a; a = a->next)
  {
    double current_x = mrg_pointer_x (mrg);
    double current_y = mrg_pointer_y (mrg);
    MrgItem *item = a->data;
    cairo_matrix_t matrix = item->inv_matrix;

    cairo_matrix_transform_point (&matrix, &current_x, &current_y);

    if (current_x >= item->x0 && current_x < item->x1 &&
        current_y >= item->y0 && current_y < item->y1)
    {
      cairo_matrix_invert (&matrix);
      cairo_set_matrix (cr, &matrix);
      restore_path (cr, item->path);
      cairo_stroke (cr);
    }
  }
  cairo_restore (cr);
}

void mrg_event_stop_propagate (MrgEvent *event)
{
  if (event)
    event->stop_propagate = 1;
}
