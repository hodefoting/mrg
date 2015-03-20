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

  _mrg_clear_bindings (mrg);
  _mrg_clear_text_closures (mrg);
}

static void restore_path (cairo_t *cr, cairo_path_t *path)
{
  int i;
  cairo_path_data_t *data;
  cairo_new_path (cr);
  for (i = 0; i <path->num_data; i += path->data[i].header.length)
  {
    data = &path->data[i];
    switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO:
        cairo_move_to (cr, data[1].point.x, data[1].point.y);
        break;
      case CAIRO_PATH_LINE_TO:
        cairo_line_to (cr, data[1].point.x, data[1].point.y);
        break;
      case CAIRO_PATH_CURVE_TO:
        cairo_curve_to (cr, data[1].point.x, data[1].point.y,
                            data[2].point.x, data[2].point.y,
                            data[3].point.x, data[3].point.y);
        break;
      case CAIRO_PATH_CLOSE_PATH:
        cairo_close_path (cr);
        break;
    }
  }
}


/* using bigger primes would be a good idea, this falls apart due to rounding
 * when zoomed in close
 */
static uint32_t path_hash (cairo_path_t *path)
{
  int i;
  uint32_t ret = 0;
  cairo_path_data_t *data;
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

MrgList *_mrg_detect_all (Mrg *mrg, float x, float y, MrgType type)
{
  MrgList *a;
  MrgList *ret = NULL;

  if (type == MRG_KEY_DOWN ||
      type == MRG_KEY_UP ||
      type == (MRG_KEY_DOWN|MRG_KEY_UP))
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
  MrgList *l = _mrg_detect_all (mrg, x, y, type);
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

      cairo_user_to_device (cr, &tx, &ty);
      if (ty > mrg->height * 2 ||
          tx > mrg->width * 2 ||
          tx < -mrg->width * 2 ||
          ty < -mrg->height * 2)
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

        /* reuse previous rectangles, when they are exactly the same 
         * XXX: adapt to deal with paths as well,,.
         *
         * or stop doing this?
         * */
        if (item->path_hash == item2->path_hash)
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
    event->mrg = mrg;
    event->type = type;
    event->x = x;
    event->y = y;
  }
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
    tx = transformed_event.delta_x;
    ty = transformed_event.delta_y;
  cairo_matrix_transform_distance (&item->inv_matrix, &tx, &ty);
    transformed_event.delta_x = tx;
    transformed_event.delta_y = ty;
  }

  transformed_event.state = mrg->modifier_state;

  for (i = item->cb_count-1; i >= 0; i--)
  {
    if (item->cb[i].types & (type))
    {
      item->cb[i].cb (&transformed_event, item->cb[i].data1, item->cb[i].data2);
      event->stop_propagate = transformed_event.stop_propagate;
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
    {
      return event->stop_propagate;
    }
  }
  return 0;
}

static MrgItem *_mrg_update_item (Mrg *mrg, float x, float y, MrgType type, MrgList **hitlist)
{
  MrgItem *current = NULL;

  MrgList *l = _mrg_detect_all (mrg, x, y, type);
  if (l)
  {
    mrg_list_reverse (&l);
    current = l->data;
  }
  if (hitlist)
    *hitlist = l;
  else
    mrg_list_free (&l);


  if (mrg->prev == NULL || current == NULL || (current->path_hash != mrg->prev->path_hash))
  {
    int focus_radius = 2;
    if (current)
      _mrg_item_ref (current);

    if (mrg->prev)
    {
      {
        MrgRectangle rect = {floor(mrg->prev->x0-focus_radius),
                             floor(mrg->prev->y0-focus_radius),
                             ceil(mrg->prev->x1)-floor(mrg->prev->x0) + focus_radius * 2,
                             ceil(mrg->prev->y1)-floor(mrg->prev->y0) + focus_radius * 2};
        mrg_queue_draw (mrg, &rect);
      }

      _mrg_emit_cb_item (mrg, mrg->prev, NULL, MRG_LEAVE, x, y);
      _mrg_item_unref (mrg->prev);
      mrg->prev = NULL;
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
      mrg->prev = current;
    }
  }
  current = _mrg_detect (mrg, x, y, type);
  return current;
}

int mrg_pointer_press (Mrg *mrg, float x, float y, int device_no)
{
  MrgList *hitlist = NULL;
  MrgItem *mrg_item = _mrg_update_item (mrg, x, y, MRG_PRESS | MRG_DRAG_PRESS, &hitlist);
  mrg->pointer_x = x;
  mrg->pointer_y = y;

  mrg->drag_event.type = MRG_PRESS;
  mrg->drag_event.x = mrg->drag_event.start_x = mrg->drag_event.prev_x = x;
  mrg->drag_event.y = mrg->drag_event.start_y = mrg->drag_event.prev_y = y;
  mrg->drag_event.delta_x = mrg->drag_event.delta_y = 0;
  mrg->drag_event.device_no = device_no;

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

  if (mrg_item && (mrg_item->types & MRG_DRAG))
  {
    mrg->is_press_grabbed = 1;
    _mrg_item_ref (mrg_item);
    if (mrg->grab)
      _mrg_item_unref (mrg->grab);
    mrg->grab = mrg_item;
    mrg->drag_event.type = MRG_DRAG_PRESS;
  }

  mrg_queue_draw (mrg, NULL); /* in case of style change */

  if (mrg_item)
  {
    int ret = _mrg_emit_cb (mrg, hitlist, &mrg->drag_event, mrg->is_press_grabbed?MRG_DRAG_PRESS:MRG_PRESS, x, y);
    mrg_list_free (&hitlist);
    return ret;
  }

  return 0;
}

void mrg_resized (Mrg *mrg, int width, int height)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_KEY_DOWN);
  
  mrg->drag_event.mrg = mrg;
  mrg->drag_event.type = MRG_KEY_DOWN;
  mrg->drag_event.key_name = "resize-event"; /* gets delivered to clients as a key_down event 
   */

  if (item)
  {
    mrg->drag_event.stop_propagate = 0;
    _mrg_emit_cb_item (mrg, item, &mrg->drag_event, MRG_KEY_DOWN, 0, 0);
  }
}

int mrg_pointer_release (Mrg *mrg, float x, float y, int device_no)
{
  MrgItem *mrg_item;
  int was_grabbed = 0;
  mrg->drag_event.type = MRG_RELEASE;
  mrg->drag_event.x = x;
  mrg->drag_event.mrg = mrg;
  mrg->drag_event.y = y;
  mrg->drag_event.device_no = device_no;

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

  mrg->pointer_x = x;
  mrg->pointer_y = y;
  MrgList *hitlist = NULL;

  if (mrg->is_press_grabbed)
  {
    mrg->drag_event.type = MRG_DRAG_RELEASE;
    mrg->is_press_grabbed = 0;
    mrg_item = mrg->grab;
    was_grabbed = 1;
  }
  else
  {
    mrg_item = _mrg_update_item (mrg, x, y, MRG_RELEASE | MRG_DRAG_RELEASE, &hitlist);
  }
  if (mrg_item)
  {
    mrg->drag_event.stop_propagate = 0;
    //int ret = _mrg_emit_cb (mrg, hitlist, &mrg->drag_event, was_grabbed?MRG_DRAG_RELEASE:MRG_RELEASE, x, y);
    int ret = _mrg_emit_cb_item (mrg, mrg_item, &mrg->drag_event, was_grabbed?MRG_DRAG_RELEASE:MRG_RELEASE, x, y);
    mrg_list_free (&hitlist);
    return ret;
  }

  if (was_grabbed)
  {
    _mrg_item_unref (mrg->grab);
    mrg->grab = NULL;
  }
  return 0;
}


/*  for multi-touch, need a list of active grabs - thus a grab corresponds to
 *  a device id. even during drag-grabs events propagate; to stop that stop
 *  propagation like for other events.
 *
 *
 */

int mrg_pointer_motion (Mrg *mrg, float x, float y, int device_no)
{
  MrgItem   *mrg_item;
  MrgList   *hitlist = NULL;

  mrg->drag_event.type = MRG_MOTION;
  mrg->drag_event.mrg = mrg;
  mrg->drag_event.x = x;
  mrg->drag_event.y = y;


  mrg->drag_event.device_no = mrg->pointer_down[1]?1:
                         mrg->pointer_down[2]?2:
                         mrg->pointer_down[3]?3:0;

  mrg->pointer_x = x;
  mrg->pointer_y = y;

  if (mrg->is_press_grabbed)
  {
    mrg->drag_event.type = MRG_DRAG_MOTION;
    mrg_item = mrg->grab;
  }
  else
  {
    mrg_item = _mrg_update_item (mrg, x, y, MRG_MOTION | MRG_DRAG_MOTION, &hitlist);
  }

  /* XXX: too brutal; should use enter/leave events */
  if (getenv ("MRG_FAST") == NULL)
  mrg_queue_draw (mrg, NULL); /* XXX: not really needed for all backends,
                                      needs more tinkering */

  mrg->drag_event.stop_propagate = 0;
  if (mrg_item)
  {
    int i;
    //MrgList *l;

    //for (l = hitlist; l; l = l->next)
    //l = hitlist;// l; l = l->next)
    {
      //MrgItem *item = l->data;
      MrgItem *item = mrg_item;

      for (i = 0; i < item->cb_count; i++)
      {
        if (item->cb[i].types & (MRG_DRAG_MOTION | MRG_MOTION))
        {
          if (  !(item->cb[i].types & MRG_DRAG_MOTION) ||
                  mrg->is_press_grabbed)
          _mrg_emit_cb_item (mrg, item, &mrg->drag_event, mrg->is_press_grabbed?MRG_DRAG_MOTION:MRG_MOTION, x, y);
          if (mrg->drag_event.stop_propagate)
            goto done;
        }
      }
    }
    done:
    mrg->drag_event.delta_x = x - mrg->drag_event.prev_x;
    mrg->drag_event.delta_y = y - mrg->drag_event.prev_y;
    mrg->drag_event.prev_x = x;
    mrg->drag_event.prev_y = y;
    return mrg->drag_event.stop_propagate;
  }
  return 0;
}

int mrg_key_press (Mrg *mrg, unsigned int keyval,
                   const char *string)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_KEY_DOWN);

  /* XXX: shouldn't only be a detect,.. it should iterate through _all_
   * keybindings
   */

  /* XXX: there will also be a bug with more than 8 0,0,0,0 bindings
   * registered
   */
  if (item)
  {
    int i;
    mrg->drag_event.mrg = mrg;
    mrg->drag_event.type = MRG_KEY_DOWN;
    mrg->drag_event.unicode = keyval; 
    mrg->drag_event.key_name = string;
    mrg->drag_event.stop_propagate = 0;


    for (i = 0; i < item->cb_count; i++)
    {
      if (item->cb[i].types & (MRG_KEY_DOWN))
      {
        mrg->drag_event.state = mrg->modifier_state;
        item->cb[i].cb (&mrg->drag_event, item->cb[i].data1, item->cb[i].data2);
        if (mrg->drag_event.stop_propagate)
          return mrg->drag_event.stop_propagate;
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
  return mrg->pointer_x;
}

float mrg_pointer_y (Mrg *mrg)
{
  return mrg->pointer_y;
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
  event->stop_propagate = 1;
}
