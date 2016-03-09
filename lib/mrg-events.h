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


#ifndef MRG_EVENT_H
#define MRG_EVENT_H

#include <stdint.h>

/* XXX: separate events for different buttons? */
/* pinch/zoom gestures, handled similar to "drag-gesture" ? */

enum _MrgType {
  MRG_PRESS          = 1 << 0,
  MRG_MOTION         = 1 << 1,
  MRG_RELEASE        = 1 << 2,
  MRG_ENTER          = 1 << 3,
  MRG_LEAVE          = 1 << 4,
  MRG_TAP            = 1 << 5,
  MRG_TAP_AND_HOLD   = 1 << 6,

  /* NYI: SWIPE, ZOOM ROT_ZOOM, */

  MRG_DRAG_PRESS     = 1 << 7,
  MRG_DRAG_MOTION    = 1 << 8,
  MRG_DRAG_RELEASE   = 1 << 9,
  MRG_KEY_DOWN       = 1 << 10,
  MRG_KEY_UP         = 1 << 11,
  MRG_SCROLL         = 1 << 12,
  MRG_MESSAGE        = 1 << 13, /* client should store state - preparing
                                 * for restart
                                 */

  MRG_POINTER  = (MRG_PRESS | MRG_MOTION | MRG_RELEASE),
  MRG_TAPS     = (MRG_TAP | MRG_TAP_AND_HOLD),
  MRG_CROSSING = (MRG_ENTER | MRG_LEAVE),
  MRG_DRAG     = (MRG_DRAG_PRESS | MRG_DRAG_MOTION | MRG_DRAG_RELEASE),
  MRG_KEY      = (MRG_KEY_DOWN | MRG_KEY_UP),
  MRG_MISC     = (MRG_MESSAGE),
  MRG_ANY      = (MRG_POINTER | MRG_DRAG | MRG_CROSSING | MRG_KEY | MRG_MISC | MRG_TAPS), 
};

#define MRG_CLICK   MRG_PRESS   // SHOULD HAVE MORE LOGIC

typedef enum   _MrgType  MrgType;
typedef struct _MrgEvent MrgEvent;

struct _MrgRectangle {
  int x;
  int y;
  int width;
  int height;
};

typedef enum _MrgModifierState MrgModifierState;

enum _MrgModifierState
{
  MRG_MODIFIER_STATE_SHIFT   = (1<<0),
  MRG_MODIFIER_STATE_CONTROL = (1<<1),
  MRG_MODIFIER_STATE_ALT     = (1<<2),
  MRG_MODIFIER_STATE_BUTTON1 = (1<<3),
  MRG_MODIFIER_STATE_BUTTON2 = (1<<4),
  MRG_MODIFIER_STATE_BUTTON3 = (1<<5)
};

typedef enum _MrgScrollDirection MrgScrollDirection;
enum _MrgScrollDirection
{
  MRG_SCROLL_DIRECTION_UP,
  MRG_SCROLL_DIRECTION_DOWN,
  MRG_SCROLL_DIRECTION_LEFT,
  MRG_SCROLL_DIRECTION_RIGHT
};

struct _MrgEvent {
  MrgType  type;
  Mrg     *mrg;
  uint32_t time;

  MrgModifierState state;

  int      device_no; /* 0 = left mouse button / virtual focus */
                      /* 1 = middle mouse button */
                      /* 2 = right mouse button */
                      /* 3 = first multi-touch .. (NYI) */

  float   device_x; /* untransformed (device) coordinates  */
  float   device_y;

  /* coordinates; and deltas for motion/drag events in user-coordinates: */
  float   x;
  float   y;
  float   start_x; /* start-coordinates (press) event for drag, */
  float   start_y; /*    untransformed coordinates */
  float   prev_x;  /* previous events coordinates */
  float   prev_y;
  float   delta_x; /* x - prev_x, redundant - but often useful */
  float   delta_y; /* y - prev_y, redundant - ..  */

  MrgScrollDirection scroll_direction;

  /* only valid for key-events */
  unsigned int unicode;
  const char *key_name; /* can be "up" "down" "space" "backspace" "a" "b" "ø" etc .. */
                        /* this is also where the message is delivered for
                         * MESSAGE events
                         */
  int stop_propagate; /* */
};

typedef void (*MrgCb) (MrgEvent *event,
                       void     *data,
                       void     *data2);

/* XXX: need full version for lua ffi */
void mrg_add_binding (Mrg *mrg,
                      const char *key,
                      const char *action,
                      const char *label,
                      MrgCb cb,
                      void  *cb_data);


void mrg_add_binding_full (Mrg *mrg,
                      const char *key,
                      const char *action,
                      const char *label,
                      MrgCb cb,
                      void  *cb_data,
        MrgDestroyNotify destroy_notify,
                      void  *destroy_data);



/**
 * mrg_listen:
 *
 * @types: an or'ed list of types of events to listen for in callback.
 * @cb: a callback of type void (mrgevent, data1, data2)
 * @data1: one data pointer
 * @data2: a second data pointer
 *
 * listens for events in the boundingbox of the current path, 
 */
void mrg_listen      (Mrg     *mrg,
                      MrgType  types,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2);

void mrg_event_stop_propagate (MrgEvent *event);

/**
 * mrg_listen_full:
 *
 * @types: an or'ed list of types of events to listen for in callback.
 * @cb: a callback of type void (mrgevent, data1, data2)
 * @data1: one data pointer
 * @data2: a second data pointer
 * @finalize: a finalize / destroy_notify that also gets passed the userdatas
 * @finalize_data: extra data for finalize
 *
 * listens for events in the boundingbox of the current path, 
 */
void mrg_listen_full (Mrg     *mrg,
                      MrgType  types,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
                      void    *finalize_data);

/* these deal with pointer_no 0 only
 */
void  mrg_warp_pointer (Mrg *mrg, float x, float y);
float mrg_pointer_x    (Mrg *mrg);
float mrg_pointer_y    (Mrg *mrg);

int mrg_scrolled (Mrg *mrg, float x, float y, MrgScrollDirection scroll_direction, uint32_t time);
int mrg_pointer_press     (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_pointer_release   (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_pointer_motion    (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_key_press         (Mrg *mrg, unsigned int keyval, const char *string, uint32_t time);
void mrg_incoming_message (Mrg *mrg, const char *message, long time);

#endif
