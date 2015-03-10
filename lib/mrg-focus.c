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

/* NOTE: quite a bit of unneeded code can be eliminated in here */

static void mrg_item_center (MrgItem *item, float *cx, float *cy)
{
  double x = (item->x0 + item->x1)/2;
  double y = (item->y0 * 0.2 + item->y1 * 0.8);

  cairo_matrix_t mat = item->inv_matrix;
  cairo_matrix_invert (&mat);
  cairo_matrix_transform_point (&mat, &x, &y);

  if (cx) *cx = x;
  if (cy) *cy = y;
}

static MrgItem *item_find_next (Mrg *mrg, MrgItem *cursor, int x_delta, int y_delta)
{
  MrgList *a;
  float cursor_x, cursor_y;

  MrgItem *best = NULL;
  float   best_distance = 10000000000000000000000.0;

  mrg_item_center (cursor, &cursor_x, &cursor_y);
  for (a = mrg->items; a; a = a->next)
    {
      MrgItem *item = a->data;
      if (item != cursor)
        {
          float cx, cy;
          float distance;
          mrg_item_center (item, &cx, &cy);

          if (x_delta)
          distance = (cx-cursor_x)*(cx-cursor_x) * 5 + (cy-cursor_y)*(cy-cursor_y);
          else
          if (y_delta)
          distance = (cx-cursor_x)*(cx-cursor_x) + 5  * (cy-cursor_y)*(cy-cursor_y);
          else
          distance = (cx-cursor_x)*(cx-cursor_x) + (cy-cursor_y)*(cy-cursor_y);

          distance = sqrt (distance);

          if (distance <= best_distance)
          {
            if ((x_delta > 0) && (cx <= cursor_x))
            { }
            else if ((x_delta < 0) && (cx >= cursor_x))
            { }
            else if ((y_delta > 0) && (cy <= cursor_y))
            { }
            else if ((y_delta < 0) && (cy >= cursor_y))
            { }
            else
            {
              best_distance = distance;
              best = item;
            }
          }
        }
    }
  return best;
}

static void cmd_focus_up (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);
  MrgItem *next = NULL;

  if (current)
  {
    next = item_find_next (mrg, current, 0, -1);
  }
  else
  {
#if 1
    MrgList *a;
    for (a = mrg->items; a; a = a->next)
      {
        MrgItem *item = a->data;
        if (!(item->x0 == 0 && item->y0 == 0))
          next = item;
      }
#endif
  }

  if (next)
  {
    float cx, cy;
    mrg_item_center (next, &cx, &cy);
    mrg_warp_pointer (mrg, cx, cy);
    event->stop_propagate = 1;
  }
}

static void cmd_focus_down (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);
  MrgItem *next = NULL;

  if (current)
  {
    next = item_find_next (mrg, current, 0, 1);
  }
  else
  {
#if 1
    MrgList *a;
    for (a = mrg->items; a; a = a->next)
      {
        MrgItem *item = a->data;
        if (!(item->x0 == 0 && item->y0 == 0))
          next = item;
      }
#endif
  }

  if (next)
  {
    mrg_warp_pointer (mrg, (next->x0 + next->x1)/2, (next->y0 * 0.2 + next->y1 * 0.8) );
    event->stop_propagate = 1;
  }
}


static void cmd_focus_left (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);
  MrgItem *next = NULL;

  if (current)
  {
    next = item_find_next (mrg, current, -1, 0);
  }
  else
  {
#if 0
    MrgList *a;
    for (a = mrg->items; a; a = a->next)
      {
        MrgItem *item = a->data;
        if (!(item->x0 == 0 && item->y0 == 0))
          next = item;
      }
#endif
  }

  if (next)
  {
    mrg_warp_pointer (mrg, (next->x0 + next->x1)/2, (next->y0 * 0.2 + next->y1 * 0.8) );
    event->stop_propagate = 1;
  }
}


static void cmd_focus_right (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);
  MrgItem *next = NULL;

  if (current)
  {
    next = item_find_next (mrg, current, 1, 0);
  }
  else
  {
#if 0
    MrgList *a;
    for (a = mrg->items; a; a = a->next)
      {
        MrgItem *item = a->data;
        if (!(item->x0 == 0 && item->y0 == 0))
          next = item;
      }
#endif
  }

  if (next)
  {
    mrg_warp_pointer (mrg, (next->x0 + next->x1)/2, (next->y0 * 0.2 + next->y1 * 0.8) );
    event->stop_propagate = 1;
  }
}

static void cmd_focus_previous (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);

  if (current)
  {
    {
      MrgList *a;
      MrgItem *prev = NULL;
      for (a = mrg->items; a; a = a->next)
        {
          MrgItem *item = a->data;
          if (item == current)
          {
            if (prev)
            {
              mrg_warp_pointer (mrg, (prev->x0 + prev->x1)/2, (prev->y0 * 0.2 + prev->y1 * 0.8) );
            }
          }
          prev = item;
        }
      }
  }
  else
  {
    {
      MrgList *a;
      MrgItem *found = NULL;
      for (a = mrg->items; a; a = a->next)
        {
          MrgItem *item = a->data;
          if (!(item->x0 == 0 && item->y0 == 0)){
            found = item;
          }
        }
      if (found)
        mrg_warp_pointer (mrg, (found->x0 + found->x1)/2, (found->y0 * 0.2 + found->y1 * 0.8) );
      }
  }
}

static void cmd_focus_next (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;
  float x = mrg_pointer_x (mrg);
  float y = mrg_pointer_y (mrg);
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);

  if (current)
  {
    {
      MrgList *a;
      int found = 0;
      for (a = mrg->items; a; a = a->next)
        {
          MrgItem *item = a->data;
          if (found)
          {
            mrg_warp_pointer (mrg, (item->x0 + item->x1)/2, (item->y0 * 0.2 + item->y1 * 0.8) );
            return;
          }
          if (item == current)
            found = 1;
        }
        mrg_warp_pointer (mrg, mrg_width(mrg)/2, mrg_height(mrg)-1);
        return;
      }
  }
  else
  {
    {
      MrgList *a;
      for (a = mrg->items; a; a = a->next)
        {
          MrgItem *item = a->data;
          if (!(item->x0 == 0 && item->y0 == 0)){
            mrg_warp_pointer (mrg, (item->x0 + item->x1)/2, (item->y0 * 0.2 + item->y1 * 0.8) );
            return;
          }
        }
      mrg_warp_pointer (mrg, 0, 0);
      return;
    }

  }

  return;
}


static void cmd_select (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;

  mrg_pointer_press   (mrg, mrg_pointer_x (mrg), mrg_pointer_y (mrg), 0);
  mrg_pointer_release (mrg, mrg_pointer_x (mrg), mrg_pointer_y (mrg), 0);
}

void mrg_focus_bindings (Mrg *mrg)
{
  mrg_add_binding (mrg, "up",      NULL, "focus up",    cmd_focus_up,      NULL);
  mrg_add_binding (mrg, "down",    NULL, "focus down",  cmd_focus_down,    NULL);
  mrg_add_binding (mrg, "left",    NULL, "focus left",  cmd_focus_left,    NULL);
  mrg_add_binding (mrg, "right",   NULL, "focus right", cmd_focus_right,   NULL);
  mrg_add_binding (mrg, "tab",     NULL, "focus next",  cmd_focus_next,    NULL);
  mrg_add_binding (mrg, "shift-tab",     NULL, "focus previous",  cmd_focus_previous,    NULL);
  mrg_add_binding (mrg, "return",  NULL, "Select (same as left mouse button)", cmd_select,      NULL);
}
