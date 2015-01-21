/* mrg-bundle / amalgation of all the sources\n
 * permitting simple .so/.dll free 'static' use  
 *
 * git revision generating amalgamation: 
 *    472121f56584605ae6ffe2fdc5dd1395a1ea635d
 * 
 */
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-config.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */

#ifndef MRG_CONFIG_H
#define MRG_CONFIG_H

 /* tweak these numbers to change which features are built into MRG and not,
  * when used as an amalgamation that is only included; these compile-time
  * flags can be set before including the amalgamation.
  */

#ifndef MRG_GTK
#define MRG_GTK       1
#endif
#ifndef MRG_SDL
#define MRG_SDL       1
#endif
#ifndef MRG_NCT
#define MRG_NCT       1
#endif
#ifndef MRG_CAIRO
#define MRG_CAIRO     1
#endif
#ifndef MRG_MEM
#define MRG_MEM       1
#endif
#ifndef MRG_UFB
#define MRG_UFB       1
#endif

/* if we've got GTK we've always got cairo */
#if MRG_GTK
#undef MRG_CAIRO
#define MRG_CAIRO 1
#endif


#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#ifndef MICRO_RAPTOR_GUI
#define MICRO_RAPTOR_GUI
// bundled include of mrg-config.h"

#if MRG_CAIRO
#include <cairo.h>
#endif

/* Mrg is fed with all press, release and motion events and synthesizes
 * generated callbacks as appropriate.
 *
 */
typedef struct _Mrg          Mrg;
typedef struct _MrgColor     MrgColor;
typedef struct _MrgStyle     MrgStyle;
typedef struct _MrgRectangle MrgRectangle;

typedef void (*UiRenderFun)(Mrg *mrg, void *ui_data);

Mrg  *mrg_new           (int width, int height, const char *backend);
void  mrg_destroy       (Mrg *mrg);

void  mrg_set_size      (Mrg *mrg, int width, int height);
void  mrg_set_position  (Mrg *mrg, int x, int y);
void  mrg_get_position  (Mrg *mrg, int *x, int *y);

void  mrg_set_title     (Mrg *mrg, const char *title);
const char *mrg_get_title (Mrg *mrg);

int   mrg_width         (Mrg *mrg);
int   mrg_height        (Mrg *mrg);




void  mrg_set_ui        (Mrg *mrg, UiRenderFun, void *ui_data);

void  mrg_main          (Mrg *mrg);
void  mrg_quit          (Mrg *mrg);
#if MRG_CAIRO
cairo_t *mrg_cr         (Mrg *mrg);
#endif
void mrg_queue_draw     (Mrg *mrg, MrgRectangle *rectangle);

int  mrg_is_terminal    (Mrg *mrg);
void mrg_set_fullscreen (Mrg *mrg, int fullscreen);
int  mrg_is_fullscreen  (Mrg *mrg);

float mrg_ddpx          (Mrg *mrg);

unsigned char *mrg_get_pixels (Mrg *mrg, int *rowstride);

/*********************************************/

Mrg *mrg_new_mrg (Mrg *parent, int width, int height); // NYI

typedef struct _MrgImage MrgImage;

/* force a ui render, this is included for use with the headless backend.
 */
void  mrg_ui_update (Mrg *mrg);

MrgImage *mrg_query_image (Mrg *mrg, const char *path, 
                           int *width,
                           int *height);

/* built in http / local file URI fetcher, this is the callback interface
 * that needs to be implemented for mrg_xml_render if external resources (css
 * files / png images) are to be retrieved and rendered.
 */
int mrg_get_contents (const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length,
                      void        *ignored_user_data);

void mrg_render_to_mrg (Mrg *mrg, Mrg *mrg2, float x, float y);

int mrg_add_idle (Mrg *mrg, int (*idle_cb)(void *idle_data), void *idle_data);

// bundled include of mrg-events.h"
// bundled include of mrg-text.h"
// bundled include of mrg-style.h"
// bundled include of mrg-util.h"

#ifdef __GTK_H__ /* This is only declared if mrg.h is included after gtk.h */

GtkWidget *mrg_gtk_new (void (*ui_update)(Mrg *mrg, void *user_data),
                        void *user_data);


#endif

#include <stddef.h>

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-events.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

/* XXX: separate events for different buttons? */
/* pinch/zoom gestures, handled similar to "drag-gesture" ? */

enum _MrgType {
  MRG_PRESS          = 1 << 0,
  MRG_PRESS_AND_HOLD = 1 << 1, /* NYI */
  MRG_MOTION         = 1 << 2,
  MRG_RELEASE        = 1 << 3,
  MRG_ENTER          = 1 << 4,
  MRG_LEAVE          = 1 << 5,
  MRG_DRAG_PRESS     = 1 << 6,
  MRG_DRAG_MOTION    = 1 << 7,
  MRG_DRAG_RELEASE   = 1 << 8,
  MRG_KEY_DOWN       = 1 << 9,
  MRG_KEY_UP         = 1 << 10,

  MRG_POINTER  = (MRG_PRESS | MRG_MOTION | MRG_RELEASE),
  MRG_CROSSING = (MRG_ENTER | MRG_LEAVE),
  MRG_DRAG     = (MRG_DRAG_PRESS | MRG_DRAG_MOTION | MRG_DRAG_RELEASE),
  MRG_KEY      = (MRG_KEY_DOWN | MRG_KEY_UP),
  MRG_ANY      = (MRG_POINTER | MRG_DRAG | MRG_CROSSING | MRG_KEY), 
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

struct _MrgEvent {
  MrgType  type;
  Mrg     *mrg;

  int      device_no; /* 0 = left mouse button / virtual focus */
                      /* 1 = middle mouse button */
                      /* 2 = right mouse button */
                      /* 3 = first multi-touch .. (NYI) */

  float   device_x; /* untransformed x/y coordinates  */
  float   device_y;

  /* coordinates; and deltas for motion events in user-coordinates: */
  float   x;
  float   y;
  float   start_x; /* start-coordinates (press) event for drag */
  float   start_y;
  float   prev_x;  /* previous events coordinates */
  float   prev_y;
  float   delta_x; /* x - prev_x */
  float   delta_y; /* y - prev_y */

  /* only valid for key-events */
  unsigned int unicode;
  const char *key_name; /* can be "up" "down" "a" "b" "ø" etc .. */
};

typedef int (*MrgCb) (MrgEvent *event,
                      void     *data,
                      void     *data2);

void mrg_add_binding (Mrg *mrg,
                      const char *key,
                      const char *action,
                      const char *label,
                      MrgCb cb,
                      void  *cb_data);

/**
 * mrg_listen:
 *
 * @types: an or'ed list of types of events to listen for in callback.
 *
 *
 */
void mrg_listen      (Mrg     *mrg,
                      MrgType  types,
                      float   x,
                      float   y,
                      float   width,
                      float   height,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2);

void mrg_listen_full (Mrg     *mrg,
                      MrgType  types,
                      float   x,
                      float   y,
                      float   width,
                      float   height,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
                      void    *finalize_data);

/* these deal with pointer_no 0 only
 */
void mrg_warp_pointer     (Mrg *mrg, float x, float y);
float mrg_pointer_x       (Mrg *mrg);
float mrg_pointer_y       (Mrg *mrg);

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-style.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


#ifndef MRG_STYLE_H__
#define MRG_STYLE_H__

#if MRG_CAIRO
#include <cairo.h>
#else
#define cairo_t void
#endif

struct _MrgColor {
  float red;
  float green;
  float blue;
  float alpha;
};

typedef enum {
  MRG_DISPLAY_INLINE = 0,
  MRG_DISPLAY_BLOCK,
  MRG_DISPLAY_LIST_ITEM,
  MRG_DISPLAY_HIDDEN
} MrgDisplay;

/* matches cairo order */
typedef enum
{
  MRG_FONT_WEIGHT_NORMAL = 0,
  MRG_FONT_WEIGHT_BOLD
} MrgFontWeight;

typedef enum
{
  MRG_FILL_RULE_NONZERO = 0,
  MRG_FILL_RULE_EVEN_ODD
} MrgFillRule;

/* matches cairo order */
typedef enum
{
  MRG_FONT_STYLE_NORMAL = 0,
  MRG_FONT_STYLE_ITALIC,
  MRG_FONT_STYLE_OBLIQUE
} MrgFontStyle;

typedef enum
{
  MRG_BOX_SIZING_CONTENT_BOX = 0,
  MRG_BOX_SIZING_BORDER_BOX
} MrgBoxSizing;

/* matching nchanterm definitions */

typedef enum {
  MRG_REGULAR     = 0,
  MRG_BOLD        = (1 << 0),
  MRG_DIM         = (1 << 1),
  MRG_UNDERLINE   = (1 << 2),
  MRG_REVERSE     = (1 << 3),
  MRG_OVERLINE    = (1 << 4),
  MRG_LINETHROUGH = (1 << 5),
  MRG_BLINK       = (1 << 6)
} MrgTextDecoration;

typedef enum {
  MRG_POSITION_STATIC = 0,
  MRG_POSITION_RELATIVE,
  MRG_POSITION_FIXED,
  MRG_POSITION_ABSOLUTE
} MrgPosition;

typedef enum {
  MRG_OVERFLOW_VISIBLE = 0,
  MRG_OVERFLOW_HIDDEN,
  MRG_OVERFLOW_SCROLL,
  MRG_OVERFLOW_AUTO
} MrgOverflow;

typedef enum {
  MRG_FLOAT_NONE = 0,
  MRG_FLOAT_LEFT,
  MRG_FLOAT_RIGHT,
  MRG_FLOAT_FIXED
} MrgFloat;

typedef enum {
  MRG_CLEAR_NONE = 0,
  MRG_CLEAR_LEFT,
  MRG_CLEAR_RIGHT,
  MRG_CLEAR_BOTH
} MrgClear;

typedef enum {
  MRG_TEXT_ALIGN_LEFT = 0,
  MRG_TEXT_ALIGN_RIGHT,
  MRG_TEXT_ALIGN_JUSTIFY,
  MRG_TEXT_ALIGN_CENTER
} MrgTextAlign;

typedef enum {
  MRG_WHITE_SPACE_NORMAL = 0,
  MRG_WHITE_SPACE_NOWRAP,
  MRG_WHITE_SPACE_PRE,
  MRG_WHITE_SPACE_PRE_LINE,
  MRG_WHITE_SPACE_PRE_WRAP
} MrgWhiteSpace;

typedef enum {
  MRG_VERTICAL_ALIGN_BASELINE = 0,
  MRG_VERTICAL_ALIGN_MIDDLE,
  MRG_VERTICAL_ALIGN_BOTTOM,
  MRG_VERTICAL_ALIGN_TOP,
  MRG_VERTICAL_ALIGN_SUB,
  MRG_VERTICAL_ALIGN_SUPER
} MrgVerticalAlign;

typedef enum {
  MRG_CURSOR_AUTO = 0,
  MRG_CURSOR_ALIAS,
  MRG_CURSOR_ALL_SCROLL,
  MRG_CURSOR_CELL,
  MRG_CURSOR_CONTEXT_MENU,
  MRG_CURSOR_COL_RESIZE,
  MRG_CURSOR_COPY,
  MRG_CURSOR_CROSSHAIR,
  MRG_CURSOR_DEFAULT,
  MRG_CURSOR_E_RESIZE,
  MRG_CURSOR_EW_RESIZE,
  MRG_CURSOR_HELP,
  MRG_CURSOR_MOVE,
  MRG_CURSOR_N_RESIZE,
  MRG_CURSOR_NE_RESIZE,
  MRG_CURSOR_NESW_RESIZE,
  MRG_CURSOR_NS_RESIZE,
  MRG_CURSOR_NW_RESIZE,
  MRG_CURSOR_NO_DROP,
  MRG_CURSOR_NONE,
  MRG_CURSOR_NOT_ALLOWED,
  MRG_CURSOR_POINTER,
  MRG_CURSOR_PROGRESS,
  MRG_CURSOR_ROW_RESIZE,
  MRG_CURSOR_S_RESIZE,
  MRG_CURSOR_SE_RESIZE,
  MRG_CURSOR_SW_RESIZE,
  MRG_CURSOR_TEXT,
  MRG_CURSOR_VERTICAL_TEXT,
  MRG_CURSOR_W_RESIZE,
  MRG_CURSOR_WAIT,
  MRG_CURSOR_ZOOM_IN,
  MRG_CURSOR_ZOOM_OUT
} MrgCursor;

typedef enum {
  MRG_LINE_CAP_BUTT,
  MRG_LINE_CAP_ROUND,
  MRG_LINE_CAP_SQUARE
} MrgLineCap;

typedef enum {
  MRG_LINE_JOIN_MITER,
  MRG_LINE_JOIN_ROUND,
  MRG_LINE_JOIN_BEVEL
} MrgLineJoin;

typedef enum {
  MRG_UNICODE_BIDI_NORMAL = 0,
  MRG_UNICODE_BIDI_EMBED,
  MRG_UNICODE_BIDI_BIDI_OVERRIDE
} MrgUnicodeBidi;

typedef enum {
  MRG_DIRECTION_LTR = 0,
  MRG_DIRECTION_RTL
} MrgDirection;

typedef enum {
  MRG_VISIBILITY_VISIBLE = 0,
  MRG_VISIBILITY_HIDDEN
} MrgVisibility;

typedef enum {
  MRG_LIST_STYLE_OUTSIDE = 0,
  MRG_LIST_STYLE_INSIDE
} MrgListStyle;

/* This style class should be able to grow to contain some color names with
 * semantic meaning.
 */
struct _MrgStyle {
  /* text-related */
  float             font_size;
  char              font_family[128];
  MrgColor          color;
  float             text_indent;
  float             letter_spacing;
  float             word_spacing;

  MrgVisibility     visibility;

  MrgTextDecoration text_decoration;
  float             line_height;
  float             line_width;

  MrgColor          background_color;

  float             stroke_width;
  float             text_stroke_width;
  MrgColor          text_stroke_color;
  float             tab_size;

  /* copying style structs takes quite a bit of the time in the system,
   * packing bits ends up saving a lot
   *
   * XXX: this is a gcc extension...
   */
  MrgFillRule         fill_rule:1;
  MrgFontStyle        font_style:2;
  MrgFontWeight       font_weight:2;
  MrgLineCap          stroke_linecap:2;
  MrgLineJoin         stroke_linejoin:2;
  MrgTextAlign        text_align:2;
  MrgFloat            float_:2;
  MrgClear            clear:2;
  MrgOverflow         overflow:2;
  MrgDisplay          display:2;
  MrgPosition         position:2;
  MrgBoxSizing        box_sizing:2;
  MrgVerticalAlign    vertical_align:3;
  MrgWhiteSpace       white_space:3;
  MrgUnicodeBidi      unicode_bidi:2;
  MrgDirection        direction:1;
  MrgListStyle        list_style:3;
  unsigned int        stroke:1;
  unsigned int        fill:1;
  unsigned int        width_auto:1;
  unsigned int        margin_left_auto:1;
  unsigned int        margin_right_auto:1;
  unsigned int        print_symbols:1;
  MrgColor            stroke_color;

  /* vector shape / box related */
  MrgColor          fill_color;

  MrgColor          border_top_color;
  MrgColor          border_left_color;
  MrgColor          border_right_color;
  MrgColor          border_bottom_color;

  MrgCursor         cursor;

  /* layout related */

  float             top;
  float             left;
  float             right;
  float             bottom;
  float             width;
  float             height;
  float             min_height;
  float             max_height;

  float             border_top_width;
  float             border_left_width;
  float             border_right_width;
  float             border_bottom_width;

  float             padding_top;
  float             padding_left;
  float             padding_right;
  float             padding_bottom;

  float             margin_top;
  float             margin_left;
  float             margin_right;
  float             margin_bottom;

  void             *id_ptr;
};

/**
 * mrg_style:
 * @mrg the mrg-context
 *
 * Returns the currently 
 *
 */
MrgStyle *mrg_style (Mrg *mrg);

void mrg_start     (Mrg *mrg, const char *class_name, void *id_ptr);
void mrg_start_with_style (Mrg        *mrg,
                           const char *style_id,
                           void       *id_ptr,
                           const char *style);
void mrg_end       (Mrg *mrg);

void mrg_stylesheet_clear (Mrg *mrg);
void mrg_stylesheet_add (Mrg *mrg, const char *css, const char *uri,
                         int (*get_request) (const char *referer, const char *uri, char **contents, long *length, void *get_request_data),
                         void *get_request_data,
                         char **error);

void mrg_set_style (Mrg *mrg, const char *style);
void mrg_set_stylef (Mrg *mrg, const char *format, ...);


void  mrg_set_line_height (Mrg *mrg, float line_height);
float  mrg_line_height (Mrg *mrg);


/* XXX: doesnt feel like it belongs here */
void mrg_image (Mrg *mrg, float x0, float y0, float width, float height, const char *path);

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-text.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

void  mrg_set_xy          (Mrg *mrg, float x, float y);
float mrg_x               (Mrg *mrg);
float mrg_y               (Mrg *mrg);
int   mrg_print           (Mrg *mrg, const char *str);
void  mrg_printf          (Mrg *mrg, const char *format, ...);
void  mrg_print_xml       (Mrg *mrg, char *xml);
void  mrg_printf_xml      (Mrg *mrg, const char *format, ...);

/* text layout handling */
float mrg_em              (Mrg *mrg);
void  mrg_set_em          (Mrg *mrg, float em);
float mrg_rem             (Mrg *mrg);
void  mrg_set_rem         (Mrg *mrg, float em);

void  mrg_set_edge_left   (Mrg *mrg, float edge);
void  mrg_set_edge_top    (Mrg *mrg, float edge);
void  mrg_set_edge_right  (Mrg *mrg, float edge);
void  mrg_set_edge_bottom (Mrg *mrg, float edge);
float mrg_edge_left       (Mrg *mrg);
float mrg_edge_top        (Mrg *mrg);
float mrg_edge_right      (Mrg *mrg);
float mrg_edge_bottom     (Mrg *mrg);

void  mrg_set_line_spacing (Mrg *mrg, float line_spacing);
float mrg_line_spacing     (Mrg *mrg); 
int   mrg_print_get_xy     (Mrg *mrg, const char *str, int no, float *x, float *y);
void  mrg_set_wrap         (Mrg *mrg, int do_wrap);
float mrg_draw_string      (Mrg *mrg, MrgStyle *style, 
                            float x, float y,
                            const char *string,
                            int utf8_len);
void mrg_edit_string       (Mrg *mrg, char **string,
                            void (*update_string)(Mrg *mrg,
                            char **string_loc,
                            const char *new_string,
                            void  *user_data),
                            void *user_data);

/* should accept at lest 4-8 stacked cb's */
void  mrg_text_listen (Mrg *mrg, MrgType types,
                       MrgCb cb, void *data1, void *data2);

void  mrg_text_listen_full (Mrg *mrg, MrgType types,
                            MrgCb cb, void *data1, void *data2,
          void (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
          void  *finalize_data);

void  mrg_text_listen_done (Mrg *mrg);


/*  renders within configured edges
 */
void mrg_xml_render (Mrg *mrg,
                     char *uri_base,
                     char *html,
                     int (*link_cb) (MrgEvent *event, void *href, void *link_data),
                     void *link_data,
                     int (*fetch) (const char *referer, const char *uri, char **contents, long *length, void *fetch_data),
                     void *fetch_data);

/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-util.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#ifndef MRG_UTIL_H__
#define MRG_UTIL_H__


int mrg_quit_cb (MrgEvent *event, void *data1, void *data2);

void mrg_cairo_set_source_color (cairo_t *cr, MrgColor *color);

void mrg_color_set (MrgColor *color, float red, float green, float blue, float alpha);

#define EM(value)           (value*mrg_em(mrg))
#define PERCENT_X(value)    (value*mrg_width(mrg))
#define PERCENT_Y(value)    (value*mrg_height(mrg))

#endif
#ifndef MRG_HEADERS_ONLY
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-list.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#ifndef __MRG_LIST__
#define  __MRG_LIST__

#include <stdlib.h>

/* The whole mrg_list implementation is in the header and will be inlined
 * wherever it is used.
 */

typedef struct _MrgList MrgList;
  struct _MrgList {void *data;MrgList *next;
  void (*freefunc)(void *data, void *freefunc_data);
  void *freefunc_data;
}
;

static inline void mrg_list_prepend_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data)
{
  MrgList *new_=calloc (sizeof (MrgList), 1);
  new_->next=*list;
  new_->data=data;
  new_->freefunc=freefunc;
  new_->freefunc_data = freefunc_data;
  *list = new_;
}

static inline int mrg_list_length (MrgList *list)
{
  int length = 0;
  MrgList *l;
  for (l = list; l; l = l->next, length++);
  return length;
}

static inline void mrg_list_prepend (MrgList **list, void *data)
{
  MrgList *new_=calloc (sizeof (MrgList), 1);
  new_->next= *list;
  new_->data=data;
  *list = new_;
}


static inline void mrg_list_append_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data)
{
  MrgList *new_= calloc (sizeof (MrgList), 1);
  new_->data=data;
  new_->freefunc = freefunc;
  new_->freefunc_data = freefunc_data;
  if (*list)
    {
      MrgList *last;
      for (last = *list; last->next; last=last->next);
      last->next = new_;
      return;
    }
  *list = new_;
  return;
}

static inline void mrg_list_append (MrgList **list, void *data)
{
  mrg_list_append_full (list, data, NULL, NULL);
}

static inline void mrg_list_remove (MrgList **list, void *data)
{
  MrgList *iter, *prev = NULL;
  if ((*list)->data == data)
    {
      if ((*list)->freefunc)
        (*list)->freefunc ((*list)->data, (*list)->freefunc_data);
      prev = (void*)(*list)->next;
      free (*list);
      *list = prev;
      return;
    }
  for (iter = *list; iter; iter = iter->next)
    if (iter->data == data)
      {
        if (iter->freefunc)
          iter->freefunc (iter->data, iter->freefunc_data);
        prev->next = iter->next;
        free (iter);
        break;
      }
    else
      prev = iter;
}

static inline void mrg_list_free (MrgList **list)
{
  while (*list)
    mrg_list_remove (list, (*list)->data);
}

static inline MrgList *mrg_list_nth (MrgList *list, int no)
{
  while(no-- && list)
    list = list->next;
  return list;
}

static inline MrgList *mrg_list_find (MrgList *list, void *data)
{
  for (;list;list=list->next)
    if (list->data == data)
      break;
  return list;
}

/* a bubble-sort for now, simplest thing that could be coded up
 * right to make the code continue working
 */
static inline void mrg_list_sort (MrgList **list, 
    int(*compare)(const void *a, const void *b, void *userdata),
    void *userdata)
{
  /* replace this with an insertion sort */
  MrgList *temp = *list;
  MrgList *t;
  MrgList *prev;
again:
  prev = NULL;

  for (t = temp; t; t = t->next)
    {
      if (t->next)
        {
          if (compare (t->data, t->next->data, userdata) > 0)
            {
              /* swap */
              if (prev)
                {
                  MrgList *tnn = t->next->next;
                  prev->next = t->next;
                  prev->next->next = t;
                  t->next = tnn;
                }
              else
                {
                  MrgList *tnn = t->next->next;
                  temp = t->next;
                  temp->next = t;
                  t->next = tnn;
                }
              goto again;
            }
        }
      prev = t;
    }
  *list = temp;
}

static inline void
mrg_list_insert_sorted (MrgList **list, void *data,
                       int(*compare)(const void *a, const void *b, void *userdata),
                       void *userdata)
{
  mrg_list_prepend (list, data);
  mrg_list_sort (list, compare, userdata);
}
#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-internal.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#ifndef _MRG_INTERNAL_H
#define _MRG_INTERNAL_H

// bundled include of mrg.h"
// bundled include of mrg-list.h"
// bundled include of mrg-string.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

typedef struct _MrgStyleNode MrgStyleNode;

#define MRG_STYLE_MAX_CLASSES 16
#define MRG_STYLE_MAX_PSEUDO  8


/* XXX: both style_id's and selectors should be parsed into these..
 * with interned strings.
 */
struct _MrgStyleNode 
{
  int         is_direct_parent; /* for use in selector chains with > */
  const char *id;
  const char *element;
  const char *classes[MRG_STYLE_MAX_CLASSES];
  const char *pseudo[MRG_STYLE_MAX_PSEUDO];
};

typedef struct _MrgHtml      MrgHtml;
typedef struct _MrgHtmlState MrgHtmlState;

/* minimal utility class to keep track of callbacks that have been
 * interspersed with drawing, possibly containing conditional calls.
 */
#define MRG_MAX_STYLE_DEPTH 640
#define MRG_MAX_STATE_DEPTH 128 //XXX: can these be different?
#define MRG_MAX_FLOATS      64
#define MRG_MAX_CBS         64

typedef struct MrgItemCb {
  MrgType types;
  MrgCb   cb;
  void*   data1;
  void*   data2;

  void (*finalize) (void *data1, void *data2, void *finalize_data);
  void  *finalize_data;

} MrgItemCb;

typedef int (*MrgCb) (MrgEvent *event,
                      void     *data,
                      void     *data2);

typedef struct MrgBinding {
  char *nick;
  char *command;
  char *label;
  MrgCb cb;
  void *cb_data;
} MrgBinding;


typedef struct MrgItem {
#if MRG_CAIRO
  cairo_matrix_t  inv_matrix;  /* for event coordinate transforms */
#endif


  /* circles, as well as poly-lines/beziers could be added.. */
  float          x0;
  float          y0;
  float          x1;
  float          y1;

  MrgType   types; /* all cb's ored together */
  MrgItemCb cb[MRG_MAX_CBS];
  int       cb_count;

  int       ref_count;
} MrgItem;

/*
 *   div { float:fixed; float-fixed-x: 0% width: 40%; padding-right: 2em; }
 */


typedef struct MrgState {
  MrgStyle     style;

  float      (*wrap_edge_left)  (Mrg *mrg, void *data);
  float      (*wrap_edge_right) (Mrg *mrg, void *data);
  void        *wrap_edge_data;
  void       (*post_nl)  (Mrg *mrg, void *post_nl_data, int last);
  void        *post_nl_data;

  float        edge_top;
  float        edge_left;
  float        edge_right;
  float        edge_bottom;

  /* text-layouting state */

  int          skip_lines;  /* better with an em offset? */
  int          max_lines;   /* better with max-y in ems? ? */

  char        *style_id;
  MrgStyleNode style_node;

  int          overflowed;
  /* ansi/vt100 approximations of set text fg/bg color  */
  int          fg;
  int          bg;

  int          span_bg_started;
  int          children;
} MrgState;

void _mrg_text_init      (Mrg *mrg);
int  _mrg_is_dirty       (Mrg *mrg);
void _mrg_set_clean      (Mrg *mrg);
void _mrg_debug_overlays (Mrg *mrg);
int  _mrg_has_quit       (Mrg *mrg);
void _mrg_init           (Mrg *mrg, int width, int height);
void _mrg_queue_draw     (Mrg *mrg, MrgRectangle *rectangle);

int mrg_pointer_press    (Mrg *mrg, float x, float y, int device_no);
int mrg_pointer_release  (Mrg *mrg, float x, float y, int device_no);
int mrg_pointer_motion   (Mrg *mrg, float x, float y, int device_no);

int mrg_key_press (Mrg *mrg, unsigned int keyval, const char *string);
void mrg_resized (Mrg *mrg, int width, int height);

void _mrg_item_ref (MrgItem *mrg);
void _mrg_item_unref (MrgItem *mrg);

/**
 * mrg_clear:
 *
 * Reset list of callbacks
 */
void mrg_clear           (Mrg *mrg);

/**
 * mrg_freeze:
 *
 * Stop recording (and clearing) of mrg events, should be followed by a
 * corresponding call to mrg_thaw. These calls can be used recursively.
 */

void mrg_flush           (Mrg *mrg);
void mrg_prepare        (Mrg *mrg);

int mrg_utf8_len (const unsigned char first_byte);
int mrg_unichar_to_utf8 (unsigned int c, unsigned char *outbuf);
int mrg_utf8_strlen (const char *s);
const char *mrg_utf8_skip (const char *string, int utf8_length);

#define CPX 2
int _mrg_bindings_key_down (MrgEvent *event, void *data1, void *data2);
void _mrg_clear_bindings (Mrg *mrg);
MrgItem *_mrg_detect (Mrg *mrg, float x, float y, MrgType type);

float _mrg_dynamic_edge_right (Mrg *mrg);
float _mrg_dynamic_edge_left (Mrg *mrg);

long _mrg_ticks (void);
char *_mrg_stylesheet_collate_style (Mrg *mrg);
void _mrg_set_style_properties (Mrg *mrg, const char *style_properties);

void mrg_css_default (Mrg *mrg);

/*******************************************************/



void _mrg_layout_pre (Mrg *mrg, MrgHtml *ctx);
void _mrg_layout_post (Mrg *mrg, MrgHtml *ctx);

typedef struct MrgFloatData {
  MrgFloat  type;
  float     x;
  float     y;
  float     width;
  float     height;
} MrgFloatData;


typedef struct _MrgGeoCache MrgGeoCache;

struct _MrgGeoCache
{
  void *id_ptr;
  float height;
  float width;
  int   hover;
  int gen;
};

#define MRG_XML_MAX_ATTRIBUTES    32
#define MRG_XML_MAX_ATTRIBUTE_LEN 32
#define MRG_XML_MAX_VALUE_LEN     640

struct _MrgHtmlState
{
  float        original_x;
  float        original_y;
  float        block_start_x;
  float        block_start_y;
  float        ptly;
  float        vmarg;
  MrgFloatData float_data[MRG_MAX_FLOATS];
  int          floats;


};
struct _MrgHtml
{
  Mrg *mrg;
  int foo;
  MrgHtmlState  states[MRG_MAX_STYLE_DEPTH];
  MrgHtmlState *state;
  int state_no;
  MrgList *geo_cache;

  char         attribute[MRG_XML_MAX_ATTRIBUTES][MRG_XML_MAX_ATTRIBUTE_LEN];
  char  value[MRG_XML_MAX_ATTRIBUTES][MRG_XML_MAX_VALUE_LEN];

  int   attributes;
};

#define MRG_MAX_DEVICES 8

struct _Mrg {
  float          rem;

  MrgHtml        html;

  cairo_t       *cr;
  int            width;
  int            height;
  float          ddpx;

  MrgList       *stylesheet;
  void          *css_parse_state;



  MrgList       *items; 
  MrgItem       *prev;
  MrgItem       *grab;
  int            frozen;
  int            fullscreen;
  int            is_press_grabbed;
  int            quit;
  float          pointer_x;
  float          pointer_y;
  unsigned char  pointer_down[MRG_MAX_DEVICES];

  MrgBinding     bindings[640];
  int            n_bindings;

  float          x; /* in px */
  float          y; /* in px */

  MrgRectangle   dirty;

  MrgState      *state;
  MrgList       *geo_cache;
  void          *eeek2;  /* something sometimes goes too deep! */
  void          *eeek1;  /* XXX something sometimes goes too deep in state */
  MrgState       states[MRG_MAX_STATE_DEPTH];
  int            state_no;
  int            in_paint;
  unsigned char *glyphs;  /* for terminal backend */
  unsigned char *styles;  /* ----------"--------- */
  void          *backend;
  int            do_clip;

  MrgEvent drag_event;

  void (*ui_update)(Mrg *mrg, void *user_data);
  void *user_data;

  /* backend callbacks */

  void (*mrg_main) (Mrg *mrg,
                    void (*ui_update)(Mrg *mrg, void *user_data),
                                                void *user_data);
  unsigned char *(*mrg_get_pixels)   (Mrg *mrg, int *rowstride);
  cairo_t       *(*mrg_cr)           (Mrg *mrg);
  void           (*mrg_flush)        (Mrg *mrg);
  void           (*mrg_prepare)      (Mrg *mrg);
  void           (*mrg_clear)        (Mrg *mrg);
  void           (*mrg_queue_draw)   (Mrg *mrg, MrgRectangle *rectangle);
  void           (*mrg_destroy)      (Mrg *mrg);
  void           (*mrg_warp_pointer) (Mrg *mrg, float x, float y);
  void           (*mrg_fullscreen)   (Mrg *mrg, int fullscreen);

  void           (*mrg_set_position) (Mrg *mrg, int x, int y);
  void           (*mrg_get_position) (Mrg *mrg, int *x, int *y);
  void           (*mrg_set_title)    (Mrg *mrg, const char *title);
  const char *   (*mrg_get_title)    (Mrg *mrg);

  char *title;

  /** text editing state follows **/
  char    **edited;
  void    (*update_string) (Mrg *mrg, char **string_log, const char *new_string, void *user_data);
  void     *update_string_user_data;
  int       cursor_pos; 
  float     e_x;
  float     e_y;
  float     e_ws;
  float     e_we;
  float     e_em;
  float     offset_x;
  float     offset_y;
#if MRG_CAIRO
  cairo_scaled_font_t *scaled_font;
#endif

  MrgType      text_listen_types;
  MrgCb        text_listen_cb;
  void        *text_listen_data1;
  void        *text_listen_data2;

  void       (*text_listen_finalize)(void *listen_data, void *listen_data2, void *finalize_data);
  void        *text_listen_finalize_data;

  MrgList     *idles;
};


Mrg *_mrg_mrg_new (int width, int height);
Mrg *_mrg_sdl_new (int width, int height);
Mrg *_mrg_gtk_new (int width, int height);
Mrg *_mrg_terminal_new (int width, int height);

void _mrg_get_ascent_descent (Mrg *mrg, float *ascent, float *descent);

void mrg_freeze           (Mrg *mrg);
void mrg_thaw             (Mrg *mrg);
float
mrg_addstr (Mrg *mrg, float x, float y, const char *string, int utf8_length);

char *_mrg_http_post (const char *ip,
                      const char *hostname,
                      int         port,
                      const char *path,
                      const char *body,
                      int         length,
                      int        *ret_length);
MrgGeoCache *_mrg_get_cache (MrgHtml *ctx, void *id_ptr);

void _mrg_border_left (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_right (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_top (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_bottom (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_top_l (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_bottom_l (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_top_m (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_bottom_m (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_top_r (Mrg *mrg, int x, int y, int width, int height);
void _mrg_border_bottom_r (Mrg *mrg, int x, int y, int width, int height);
float paint_span_bg (Mrg   *mrg, float x, float y,
                     float  width);
float paint_span_bg_final (Mrg   *mrg, float x, float y,
                           float  width);
float mrg_parse_float (Mrg *mrg, const char *str, char **endptr);
void _mrg_init_style (Mrg *mrg);
const char * mrg_intern_string (const char *str);

void _mrg_set_wrap_edge_vfuncs (Mrg *mrg,
    float (*wrap_edge_left)  (Mrg *mrg, void *wrap_edge_data),
    float (*wrap_edge_right) (Mrg *mrg, void *wrap_edge_data),
    void *wrap_edge_data);

void _mrg_set_post_nl (Mrg *mrg,
    void (*post_nl)  (Mrg *mrg, void *post_nl_data, int last),
    void *post_nl_data);
int _mrg_child_no (Mrg *mrg);
int mrg_color_set_from_string (Mrg *mrg, MrgColor *color, const char *string);
/* figure out what a uri means, useful in the href callback (XXX: or
 * should the href callback always use pre-resolved uri's thus making
 * this internal?
 * )*/
char *_mrg_resolve_uri (const char *base_uri, const char *uri);

void _mrg_idle_iteration (Mrg *mrg);

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-gtk.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


// bundled include of mrg-config.h"
#if MRG_GTK
#include <gtk/gtk.h>
// bundled include of mrg-internal.h"

typedef struct MrgGtk {
  GtkWidget *eventbox;
  GtkWidget *drawingarea;
  GtkWidget *window;
} MrgGtk;

static void mrg_gtk_flush (Mrg *mrg)
{
  if (mrg->cr)
  {
    mrg->cr = NULL;
  }
}
Mrg *mrg_gtk_get_mrg (GtkWidget *widget);

static gboolean draw (GtkWidget *widget, cairo_t *cr, void *userdata)
{
  Mrg    *mrg = userdata;
  MrgGtk *mrg_gtk = mrg->backend;

  mrg->cr = cr;


/*  if (_mrg_is_dirty (mrg)) */
       /* the gtk backend leaves dirty region handling up to gtk */
    {
      mrg->width = gtk_widget_get_allocated_width (mrg_gtk->eventbox);
      mrg->height = gtk_widget_get_allocated_height (mrg_gtk->eventbox);

      mrg_ui_update (mrg);
    }

  if (_mrg_has_quit (mrg))
    gtk_main_quit ();

  return FALSE;
}

static void mrg_gtk_warp_pointer (Mrg *mrg, float x, float y)
{
  MrgGtk *mrg_gtk = mrg->backend;
  GdkDisplay *display = gtk_widget_get_display (mrg_gtk->drawingarea);
  GdkDevice  *device;
  gint ox, oy;
  gdk_window_get_origin (gtk_widget_get_window (mrg_gtk->drawingarea), &ox, &oy);
  device = gdk_device_manager_get_client_pointer (gdk_display_get_device_manager (display));
  gdk_device_warp (device, gdk_display_get_default_screen (display), ox + x, oy + y);
}

static void mrg_gtk_main (Mrg *mrg,
                   void (*ui_update)(Mrg *mrg, void *user_data),
                   void *user_data)
{
  /* the gtk mrg backend directly reuses the gtk main loop */
  gtk_main ();
}

static cairo_t *mrg_gtk_cr (Mrg *mrg)
{
  return mrg->cr;
}

static gboolean button_press_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_press (mrg, event->button.x, event->button.y, event->button.button);
}

static gboolean button_release_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_release (mrg, event->button.x, event->button.y, event->button.button);
}

static gboolean motion_notify_event (GtkWidget *widget, GdkEvent *event, gpointer mrg)
{
  return mrg_pointer_motion (mrg, event->motion.x, event->motion.y, 
      (event->motion.state&GDK_BUTTON1_MASK)?0:
      (event->motion.state&GDK_BUTTON2_MASK)?1:
      (event->motion.state&GDK_BUTTON3_MASK)?2:
      (event->motion.state&GDK_BUTTON4_MASK)?3:
      (event->motion.state&GDK_BUTTON5_MASK)?4:0);
}

static gboolean key_press_event (GtkWidget *window, GdkEvent *event, gpointer   mrg)
{
  const gchar *name = NULL;
  switch (event->key.keyval)
  {
    case GDK_KEY_BackSpace: name = "backspace"; break;
    case GDK_KEY_Delete:    name = "delete";    break;
    case GDK_KEY_Insert:    name = "insert";    break;
    case GDK_KEY_F1:        name = "F1";        break;
    case GDK_KEY_F5:        name = "F5";        break;
    case GDK_KEY_F11:       name = "F11";       break;
    case GDK_KEY_Escape:    name = "escape";    break;
    case GDK_KEY_Tab:       name = "tab";       break;
    case GDK_KEY_Up:        name = "up";        break;
    case GDK_KEY_Down:      name = "down";      break;
    case GDK_KEY_Left:      name = "left";      break;
    case GDK_KEY_Right:     name = "right";     break;
    case GDK_KEY_Return:    name = "return";    break;
    case GDK_KEY_Home:      name = "home";      break;
    case GDK_KEY_End:       name = "end";       break;
    case GDK_KEY_Page_Up:   name = "page-up";   break;
    case GDK_KEY_Page_Down: name = "page-down"; break;
    default: break;
  }

  if (event->key.state & GDK_CONTROL_MASK)
  {
    char buf[128];
    if (name)
    sprintf (buf, "control-%s", name);
    else
    sprintf (buf, "control-%c", event->key.keyval);
    name = g_intern_string (buf);
    if (event->key.state & GDK_SHIFT_MASK)
    {
      char buf[128];
      sprintf (buf, "shift-%s", name);
      name = g_intern_string (buf);
    }
  }
  if (event->key.state & GDK_MOD1_MASK)
  {
    char buf[128];
    if (name)
    sprintf (buf, "alt-%s", name);
    else
    sprintf (buf, "alt-%c", event->key.keyval);
    name = g_intern_string (buf);
    if (event->key.state & GDK_SHIFT_MASK)
    {
      char buf[128];
      sprintf (buf, "shift-%s", name);
      name = g_intern_string (buf);
    }
  }
  if (!name)
    name = event->key.string;

  return mrg_key_press (mrg, gdk_keyval_to_unicode (event->key.keyval), name);
}

static void mrg_gtk_queue_draw (Mrg *mrg, MrgRectangle *rectangle)
{
  MrgGtk *mrg_gtk = mrg->backend;
  gtk_widget_queue_draw_area (mrg_gtk->drawingarea, rectangle->x, rectangle->y, rectangle->width, rectangle->height);
}

static void mrg_gtk_destroy (Mrg *mrg)
{
  if (mrg->backend)
  {
    free (mrg->backend);
    mrg->backend = NULL;
  }
}

static void mrg_gtk_fullscreen (Mrg *mrg, int fullscreen)
{
  MrgGtk *mrg_gtk = mrg->backend;
  GtkWidget *window = gtk_widget_get_ancestor (mrg_gtk->drawingarea, GTK_TYPE_WINDOW);
  if (fullscreen)
    gtk_window_fullscreen (GTK_WINDOW (window));
  else
    gtk_window_unfullscreen (GTK_WINDOW (window));
  mrg->fullscreen = fullscreen;
}

static gboolean idle_iteration (void *data)
{
  _mrg_idle_iteration (data);
  return TRUE;
}

static void mrg_gtk_set_title (Mrg *mrg, const char *title)
{
  MrgGtk *mrg_gtk = mrg->backend;
  if (!mrg_gtk->window)
    return;
  gtk_window_set_title (GTK_WINDOW (mrg_gtk->window), title);;
}

static const char *mrg_gtk_get_title (Mrg *mrg)
{
  MrgGtk *mrg_gtk = mrg->backend;

  if (!mrg_gtk->window)
    return NULL;

  return gtk_window_get_title (GTK_WINDOW (mrg_gtk->window));
}


static void mrg_gtk_set_position  (Mrg *mrg, int x, int y)
{
  MrgGtk *mrg_gtk = mrg->backend;
  if (!mrg_gtk->window)
    return;
  gtk_window_move (GTK_WINDOW (mrg_gtk->window), x, y);
}

void  mrg_gtk_get_position  (Mrg *mrg, int *x, int *y)
{
  MrgGtk *mrg_gtk = mrg->backend;
  gint root_x, root_y;
  if (!mrg_gtk->window)
    return;
  gtk_window_get_position (GTK_WINDOW (mrg_gtk->window), &root_x, &root_y);
  if (x)
    *x = root_x;
  if (y)
    *y = root_y;
}

GtkWidget *mrg_gtk_new (void (*ui_update)(Mrg *mrg, void *user_data),
                        void *user_data)
{
  Mrg *mrg;
  MrgGtk *mrg_gtk = calloc (sizeof (MrgGtk), 1);

  mrg_gtk->eventbox = gtk_event_box_new ();
  mrg_gtk->drawingarea = gtk_drawing_area_new ();


  gtk_container_add (GTK_CONTAINER(mrg_gtk->eventbox), mrg_gtk->drawingarea);

  gtk_widget_add_events (mrg_gtk->eventbox, GDK_BUTTON_MOTION_MASK |
                                            GDK_POINTER_MOTION_MASK);

  gtk_widget_set_can_focus (mrg_gtk->eventbox, TRUE);

  mrg = calloc (sizeof (Mrg), 1);
  mrg->backend = mrg_gtk;
  mrg->mrg_destroy = mrg_gtk_destroy;

  mrg->mrg_main = mrg_gtk_main;
  mrg->mrg_cr = mrg_gtk_cr;
  mrg->mrg_flush = mrg_gtk_flush;
  mrg->mrg_queue_draw = mrg_gtk_queue_draw;
  mrg->mrg_warp_pointer = mrg_gtk_warp_pointer;
  mrg->mrg_fullscreen = mrg_gtk_fullscreen;
  mrg->mrg_get_title = mrg_gtk_get_title;
  mrg->mrg_set_title = mrg_gtk_set_title;
  mrg->mrg_get_position = mrg_gtk_get_position;
  mrg->mrg_set_position = mrg_gtk_set_position;

  _mrg_init (mrg, 10, 10);
  
  mrg_set_ui (mrg, ui_update, user_data);

  g_signal_connect (mrg_gtk->eventbox, "button-press-event",
                    G_CALLBACK (button_press_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "button-release-event",
                    G_CALLBACK (button_release_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "motion-notify-event",
                    G_CALLBACK (motion_notify_event), mrg);
  g_signal_connect (mrg_gtk->eventbox, "key-press-event",
                    G_CALLBACK (key_press_event), mrg);

  g_signal_connect (mrg_gtk->drawingarea, "draw", G_CALLBACK (draw), mrg);

  g_timeout_add (30, idle_iteration, mrg);

  g_object_set_data_full (G_OBJECT (mrg_gtk->eventbox), "mrg", mrg, (void*)mrg_destroy);

  return mrg_gtk->eventbox;
}

Mrg *mrg_gtk_get_mrg (GtkWidget *widget)
{
  return g_object_get_data (G_OBJECT (widget), "mrg");
}

Mrg *_mrg_gtk_new (int width, int height)
{
  GtkWidget *window;
  GtkWidget *canvas;
  Mrg *mrg;
  MrgGtk *mrg_gtk;
  int fullscreen = 0;

  if (!gtk_init_check (NULL, NULL))
    return NULL;

  if (width == -1)
  {
    width = 640;
    height = 480;
    fullscreen = 1;
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW(window), width, height);
  g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  canvas = mrg_gtk_new (NULL, NULL);
  if (!canvas)
    return NULL;
  mrg = mrg_gtk_get_mrg (canvas);
  mrg_gtk = mrg->backend;
  mrg_set_size (mrg, width, height);
  gtk_container_add (GTK_CONTAINER(window), canvas);
  gtk_widget_grab_focus (canvas);
  gtk_widget_show_all(window);

  mrg_gtk->window = window;

  if (fullscreen)
    mrg_set_fullscreen (mrg, fullscreen);

  return mrg;
}
#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-mrg.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-sdl.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


// bundled include of mrg-config.h"
#if MRG_SDL

#include <SDL.h>
// bundled include of mrg-internal.h"

static unsigned char *mrg_sdl_get_pixels (Mrg *mrg, int *rowstride)
{
  SDL_Surface *screen = mrg->backend;

  if (rowstride)
    *rowstride = screen->pitch;
  return screen->pixels;
}

static void mrg_sdl_flush (Mrg *mrg)
{
  SDL_Surface *screen = mrg->backend;
  /* XXX: move this safe-guard into flush itself? */
  if (mrg->dirty.x < 0)
  {
    mrg->dirty.width += -mrg->dirty.x;
    mrg->dirty.x = 0;
  }
  if (mrg->dirty.y < 0)
  {
    mrg->dirty.height += -mrg->dirty.y;
    mrg->dirty.y = 0;
  }
  if (mrg->dirty.x + mrg->dirty.width >= mrg->width)
  {
    mrg->dirty.width = mrg->width - mrg->dirty.x - 1;
  }
  if (mrg->dirty.y + mrg->dirty.height >= mrg->height)
  {
    mrg->dirty.height = mrg->height - mrg->dirty.y -1;
  }
  if (mrg->dirty.width < 0)
    mrg->dirty.width = 0;
  if (mrg->dirty.height < 0)
    mrg->dirty.height = 0;

  SDL_UpdateRect (screen,mrg->dirty.x, mrg->dirty.y, mrg->dirty.width, mrg->dirty.height);
#if MRG_CAIRO
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
#endif
}

static void mrg_sdl_fullscreen (Mrg *mrg, int fullscreen)
{
  SDL_Surface *screen = mrg->backend;
  int width = 640, height = 480;

  if (fullscreen)
  {
    SDL_Rect **modes;
    modes = SDL_ListModes(NULL, SDL_HWSURFACE|SDL_FULLSCREEN);
    if (modes == (SDL_Rect**)0) {
        fprintf(stderr, "No modes available!\n");
        return;
    }
  
    width = modes[0]->w;
    height = modes[0]->h;

    screen = SDL_SetVideoMode(width, height, 32,
                              SDL_SWSURFACE | SDL_FULLSCREEN );
    mrg->backend = screen;
  }
  else
  {
    screen = SDL_SetVideoMode(width, height, 32,
                              SDL_SWSURFACE | SDL_RESIZABLE );
    mrg->backend = screen;
  }
  mrg->fullscreen = fullscreen;
}

static void mrg_sdl_consume_events (Mrg *mrg, int block);

static void mrg_sdl_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  while (!_mrg_has_quit (mrg))
  {
    if (_mrg_is_dirty (mrg))
      mrg_ui_update (mrg);

    if (mrg->idles)
      _mrg_idle_iteration (mrg);

    mrg_sdl_consume_events (mrg, !mrg->idles);
  }
}

static void mrg_sdl_warp_pointer (Mrg *mrg, float x, float y)
{
  SDL_WarpMouse (x, y);
}

static Uint32 timer_cb (Uint32 interval, void *param)
{
  _mrg_idle_iteration (param);
  return (Uint32) param;
}

static void mrg_sdl_set_title (Mrg *mrg, const char *title)
{
  SDL_WM_SetCaption(title, title);
}

static const char *mrg_sdl_get_title (Mrg *mrg)
{
  char *title, *icon;
  SDL_WM_GetCaption (&title, &icon);
  return title;
}

Mrg *_mrg_sdl_new (int width, int height)
{
  Mrg *mrg;
  SDL_Surface *screen;
  int fullscreen = 0;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf (stderr, "Unable to initialize SDL: %s", SDL_GetError());
    return NULL;
  }
  atexit (SDL_Quit);

  if (width < 0)
  {
     width = 640;
     height = 480;
     fullscreen = 1;
  }

  screen = SDL_SetVideoMode (width, height, 32, SDL_SWSURFACE | SDL_RESIZABLE);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(1);

  if (!screen)
  {
    fprintf (stderr, "Unable to create display surface: %s", SDL_GetError());
    return NULL;
  }

  mrg = calloc (sizeof (Mrg), 1);

  mrg->mrg_main = mrg_sdl_main;
  mrg->mrg_flush = mrg_sdl_flush;
  mrg->mrg_warp_pointer = mrg_sdl_warp_pointer;
  mrg->mrg_fullscreen = mrg_sdl_fullscreen;
  mrg->mrg_get_pixels = mrg_sdl_get_pixels;
  mrg->mrg_set_title = mrg_sdl_set_title;
  mrg->mrg_get_title = mrg_sdl_get_title;
  mrg->backend = screen;

  _mrg_init (mrg, width, height);
  mrg_set_size (mrg, width, height);
  mrg->do_clip = 1;

  if (fullscreen)
    mrg_set_fullscreen (mrg, fullscreen);

  SDL_AddTimer (30, timer_cb, mrg);

  return mrg;
}

/***/

static void mrg_sdl_consume_events (Mrg *mrg, int block)
{
  SDL_Event event;
  int got_event = 0;
  block = 0;

  if (block)
  {
    SDL_WaitEvent (&event);
    goto got_one;
  }

  while (SDL_PollEvent (&event))
  {
    got_one:
    got_event++;
    switch (event.type)
    {
      case SDL_MOUSEMOTION:
        mrg_pointer_motion (mrg, event.motion.x, event.motion.y,
            event.motion.state&1?0:
            event.motion.state&2?1:
            event.motion.state&4?2:0);
        break;
      case SDL_MOUSEBUTTONDOWN:
        mrg_pointer_press (mrg, event.button.x, event.button.y, event.button.button);
        break;
      case SDL_MOUSEBUTTONUP:
        mrg_pointer_release (mrg, event.button.x, event.button.y, event.button.button);
        break;
      case SDL_KEYDOWN:
        {
          char buf[64] = "";
          char *name = NULL;

          buf[mrg_unichar_to_utf8 (event.key.keysym.unicode, (void*)buf)]=0;
          switch (event.key.keysym.sym)
          {
            case SDLK_F11:       name = "F11";break;
            case SDLK_F5:        name = "F5";break;
            case SDLK_F1:        name = "F1";break;
            case SDLK_ESCAPE:    name = "escape";break;
            case SDLK_DOWN:      name = "down";break;
            case SDLK_LEFT:      name = "left";break;
            case SDLK_UP:        name = "up";break;
            case SDLK_RIGHT:     name = "right";break;
            case SDLK_BACKSPACE: name = "backspace";break;
            case SDLK_TAB:       name = "tab";break;
            case SDLK_DELETE:    name = "delete";break;
            case SDLK_INSERT:    name = "insert";break;
            case SDLK_RETURN:    name = "return";break;
            case SDLK_HOME:      name = "home";break;
            case SDLK_END:       name = "end";break;
            case SDLK_PAGEDOWN:  name = "page-down";   break;
            case SDLK_PAGEUP:    name = "page-up"; break;

            default: 
              if (event.key.keysym.unicode < 32)
              {
                buf[0] = event.key.keysym.unicode;
                buf[1] = 0;
              }
              name = (void*)&buf[0];
          }
          /* we're only using the string form/name anyways,. so should
           * probably drop other forms.. 
           */
          if (event.key.keysym.mod & (KMOD_CTRL))
          {
            char buf2[64] = "";
            sprintf (buf2, "control-%c", event.key.keysym.sym);
            name = buf2;
            if (event.key.keysym.mod & (KMOD_SHIFT))
            {
              char buf2[64] = "";
              sprintf (buf2, "shift-%c", event.key.keysym.sym);
              name = buf2;
            }
          }
          if (event.key.keysym.mod & (KMOD_ALT))
          {
            char buf2[64] = "";
            sprintf (buf2, "alt-%c", event.key.keysym.sym);
            name = buf2;
            if (event.key.keysym.mod & (KMOD_SHIFT))
            {
              char buf2[64] = "";
              sprintf (buf2, "shift-%c", event.key.keysym.sym);
              name = buf2;
            }
          }
          mrg_key_press (mrg, event.key.keysym.unicode, name);
        }
                       
        break;
      case SDL_VIDEORESIZE:
        mrg->backend = SDL_SetVideoMode(event.resize.w, event.resize.h, 32,
                                  SDL_SWSURFACE | SDL_RESIZABLE );
        mrg_set_size (mrg, event.resize.w, event.resize.h);
        break;
      case SDL_QUIT:
        exit (0);
    }
  }

  if (!got_event)
  {
    SDL_Delay (40);
  }
}

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-mem.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-config.h"

#if MRG_MEM
// bundled include of mrg-internal.h"

typedef struct MrgMem {
  unsigned char *pixels;
} MrgMem;

static unsigned char *mrg_mem_get_pixels (Mrg *mrg, int *rowstride)
{
  MrgMem *backend = mrg->backend;

  if (rowstride)
    *rowstride = mrg->width * 4;
  return backend->pixels;
}

static void mrg_mem_flush (Mrg *mrg)
{
#if MRG_CAIRO
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
#endif
}

static void mrg_mem_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  fprintf (stderr, "error: main of mem backend invoked\n");
  return;
}

static void mrg_mem_destroy (Mrg *mrg)
{
  free (mrg->backend);
}

Mrg *_mrg_mem_new (int width, int height)
{
  Mrg *mrg;
  MrgMem *backend = calloc (sizeof (MrgMem), 1);

  mrg = calloc (sizeof (Mrg), 1);
#if MRG_CAIRO
  backend->pixels = calloc (width * height * 4, 1);
#endif

  mrg->mrg_main = mrg_mem_main;
  mrg->mrg_flush = mrg_mem_flush;
  mrg->mrg_get_pixels = mrg_mem_get_pixels;
  mrg->mrg_destroy = mrg_mem_destroy;
  mrg->backend = backend;
  _mrg_init (mrg, width, height);

  return mrg;
}
#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb-evsource.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#ifndef EVSOURCE_H
#define EVSOURCE_H

typedef struct _EvSource EvSource;

/* this is the raw interface emitting somewhat raw events, to
 * be to picked up by the ufbice. For client's this would
 * be hooked up to an oc-event source, that provides the low-level
 * hook up.
 */
struct _EvSource
{
  void   *priv; /* private storage  */

  /* returns non 0 if there is events waiting */
  int   (*has_event) (EvSource *ev_source);

  /* get an event, the returned event should be freed by the caller  */
  char *(*get_event) (EvSource *ev_source);

  /* destroy/unref this instance */
  void  (*destroy)   (EvSource *ev_source);

  /* get the underlying fd, useful for using select on  */
  int   (*get_fd)    (EvSource *ev_source);


  void  (*set_coord) (EvSource *ev_source, double x, double y);
  /* set_coord is needed to warp relative cursors into normalized range,
   * like normal mice/trackpads/nipples - to obey edges and more. 
   */

  /* if this returns non-0 select can be used for non-blocking.. */
};

#define evsource_has_event(es)   (es)->has_event((es))
#define evsource_get_event(es)   (es)->get_event((es))
#define evsource_destroy(es)     do{if((es)->destroy)(es)->destroy((es));}while(0)
#define evsource_set_coord(es,x,y) do{if((es)->set_coord)(es)->set_coord((es),(x),(y));}while(0)
#define evsource_get_fd(es)      ((es)->get_fd?(es)->get_fd((es)):0)

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb-evsource-kb.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <termio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <signal.h>
#include "linux/kd.h"
// bundled include of mrg-internal.h"
// bundled include of ufb-evsource.h"

#ifndef MIN
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)  (((a)>(b))?(a):(b))
#endif

/* kept out of struct to be reachable by atexit */

static struct termios orig_attr;

//static void *ick_stage = NULL;

static void real_evsource_kb_destroy (int sign)
{
  static int done = 0;
  
  if (sign == 0)
    return;

  if (done)
    return;
  done = 1;

  //system("(sleep 1;reset)&");
  /* will be called from atexit without self */
  switch (sign)
  {
    case  -11:break;
    case   SIGSEGV: fprintf (stderr, " SIGSEGV\n");break;
    case   SIGABRT: fprintf (stderr, " SIGABRT\n");break;
    case   SIGBUS: fprintf (stderr, " SIGBUS\n");break;
    case   SIGKILL: fprintf (stderr, " SIGKILL\n");break;
    case   SIGINT: fprintf (stderr, " SIGINT\n");break;
    case   SIGTERM: fprintf (stderr, " SIGTERM\n");break;
    case   SIGQUIT: fprintf (stderr, " SIGQUIT\n");break;
    default: fprintf (stderr, "sign: %i\n", sign);
             fprintf (stderr, "%i %i %i %i %i %i %i\n", SIGSEGV, SIGABRT, SIGBUS, SIGKILL, SIGINT, SIGTERM, SIGQUIT);
  }
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &orig_attr);
  //system("stty sane");
}


static void evsource_kb_destroy (int sign)
{
  real_evsource_kb_destroy (-11);
}

static int evsource_kb_init ()
{
//  ioctl(STDIN_FILENO, KDSKBMODE, K_RAW);
  atexit ((void*) real_evsource_kb_destroy);
  signal (SIGSEGV, (void*) real_evsource_kb_destroy);
  signal (SIGABRT, (void*) real_evsource_kb_destroy);
  signal (SIGBUS,  (void*) real_evsource_kb_destroy);
  signal (SIGKILL, (void*) real_evsource_kb_destroy);
  signal (SIGINT,  (void*) real_evsource_kb_destroy);
  signal (SIGTERM, (void*) real_evsource_kb_destroy);
  signal (SIGQUIT, (void*) real_evsource_kb_destroy);

  //ick_stage = self;

  struct termios raw;
  if (tcgetattr (STDIN_FILENO, &orig_attr) == -1)
    {
      fprintf (stderr, "error initializing keyboard\n");
      return -1;
    }
  raw = orig_attr;

  cfmakeraw (&raw);

  raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */
  if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &raw) < 0)
    return 0; // XXX? return other value?

  return 0;
}

static int evsource_kb_has_event (void)
{
  struct timeval tv;
  int retval;

  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET(STDIN_FILENO, &rfds);
  tv.tv_sec = 0; tv.tv_usec = 0;
  retval = select (STDIN_FILENO+1, &rfds, NULL, NULL, &tv);
  return retval == 1;
}

/* note that a nick can have multiple occurences, the labels
 * should be kept the same for all occurences of a combination.
 *
 * this table is taken from nchanterm.
 */
typedef struct UfbKeyCode {
  char *nick;          /* programmers name for key */
  char *label;         /* utf8 label for key */
  char  sequence[10];  /* terminal sequence */
} UfbKeyCode;
static const UfbKeyCode ufb_keycodes[]={  
  {"up",                  "↑",     "\e[A"},
  {"down",                "↓",     "\e[B"}, 
  {"right",               "→",     "\e[C"}, 
  {"left",                "←",     "\e[D"}, 

  {"shift-up",            "⇧↑",    "\e[1;2A"},
  {"shift-down",          "⇧↓",    "\e[1;2B"},
  {"shift-right",         "⇧→",    "\e[1;2C"},
  {"shift-left",          "⇧←",    "\e[1;2D"},

  {"alt-up",              "^↑",    "\e[1;3A"},
  {"alt-down",            "^↓",    "\e[1;3B"},
  {"alt-right",           "^→",    "\e[1;3C"},
  {"alt-left",            "^←",    "\e[1;3D"},

  {"alt-shift-up",        "alt-s↑", "\e[1;4A"},
  {"alt-shift-down",      "alt-s↓", "\e[1;4B"},
  {"alt-shift-right",     "alt-s→", "\e[1;4C"},
  {"alt-shift-left",      "alt-s←", "\e[1;4D"},

  {"control-up",          "^↑",    "\e[1;5A"},
  {"control-down",        "^↓",    "\e[1;5B"},
  {"control-right",       "^→",    "\e[1;5C"},
  {"control-left",        "^←",    "\e[1;5D"},

  /* putty */
  {"control-up",          "^↑",    "\eOA"},
  {"control-down",        "^↓",    "\eOB"},
  {"control-right",       "^→",    "\eOC"},
  {"control-left",        "^←",    "\eOD"},

  {"control-shift-up",    "^⇧↑",   "\e[1;6A"},
  {"control-shift-down",  "^⇧↓",   "\e[1;6B"},
  {"control-shift-right", "^⇧→",   "\e[1;6C"},
  {"control-shift-left",  "^⇧←",   "\e[1;6D"},

  {"control-up",          "^↑",    "\eOa"},
  {"control-down",        "^↓",    "\eOb"},
  {"control-right",       "^→",    "\eOc"},
  {"control-left",        "^←",    "\eOd"},

  {"shift-up",            "⇧↑",    "\e[a"},
  {"shift-down",          "⇧↓",    "\e[b"},
  {"shift-right",         "⇧→",    "\e[c"},
  {"shift-left",          "⇧←",    "\e[d"},

  {"insert",              "ins",   "\e[2~"},
  {"delete",              "del",   "\e[3~"},
  {"page-up",             "PgUp",  "\e[5~"},
  {"page-down",           "PdDn",  "\e[6~"},
  {"home",                "Home",  "\eOH"},
  {"end",                 "End",   "\eOF"},
  {"home",                "Home",  "\e[H"},
  {"end",                 "End",   "\e[F"},
  {"control-delete",      "^del",  "\e[3;5~"},
  {"shift-delete",        "⇧del",  "\e[3;2~"},
  {"control-shift-delete","^⇧del", "\e[3;6~"},

  {"F1",        "F1",  "\e[11~"},
  {"F2",        "F2",  "\e[12~"},
  {"F3",        "F3",  "\e[13~"},
  {"F4",        "F4",  "\e[14~"},
  {"F1",        "F1",  "\eOP"},
  {"F2",        "F2",  "\eOQ"},
  {"F3",        "F3",  "\eOR"},
  {"F4",        "F4",  "\eOS"},
  {"F5",        "F5",  "\e[15~"},
  {"F6",        "F6",  "\e[16~"},
  {"F7",        "F7",  "\e[17~"},
  {"F8",        "F8",  "\e[18~"},
  {"F9",        "F9",  "\e[19~"},
  {"F9",        "F9",  "\e[20~"},
  {"F10",       "F10", "\e[21~"},
  {"F11",       "F11", "\e[22~"},
  {"F12",       "F12", "\e[23~"},
  {"tab",       "↹",  {9, '\0'}},
  {"shift-tab", "shift+↹",  "\e[Z"},
  {"backspace", "⌫",  {127, '\0'}},
  {" ",         "␣",   " "},
  {"\e",        "␛",  "\e"},
  {"return",    "⏎",  {10,0}},
  {"return",    "⏎",  {13,0}},
  /* this section could be autogenerated by code */
  {"control-a", "^A",  {1,0}},
  {"control-b", "^B",  {2,0}},
  {"control-c", "^C",  {3,0}},
  {"control-d", "^D",  {4,0}},
  {"control-e", "^E",  {5,0}},
  {"control-f", "^F",  {6,0}},
  {"control-g", "^G",  {7,0}},
  {"control-h", "^H",  {8,0}}, /* backspace? */
  {"control-i", "^I",  {9,0}},
  {"control-j", "^J",  {10,0}},
  {"control-k", "^K",  {11,0}},
  {"control-l", "^L",  {12,0}},
  {"control-n", "^N",  {14,0}},
  {"control-o", "^O",  {15,0}},
  {"control-p", "^P",  {16,0}},
  {"control-q", "^Q",  {17,0}},
  {"control-r", "^R",  {18,0}},
  {"control-s", "^S",  {19,0}},
  {"control-t", "^T",  {20,0}},
  {"control-u", "^U",  {21,0}},
  {"control-v", "^V",  {22,0}},
  {"control-w", "^W",  {23,0}},
  {"control-x", "^X",  {24,0}},
  {"control-y", "^Y",  {25,0}},
  {"control-z", "^Z",  {26,0}},
  {"alt-0",     "%0",  "\e0"},
  {"alt-1",     "%1",  "\e1"},
  {"alt-2",     "%2",  "\e2"},
  {"alt-3",     "%3",  "\e3"},
  {"alt-4",     "%4",  "\e4"},
  {"alt-5",     "%5",  "\e5"},
  {"alt-6",     "%6",  "\e6"},
  {"alt-7",     "%7",  "\e7"}, /* backspace? */
  {"alt-8",     "%8",  "\e8"},
  {"alt-9",     "%9",  "\e9"},
  {"alt-+",     "%+",  "\e+"},
  {"alt--",     "%-",  "\e-"},
  {"alt-/",     "%/",  "\e/"},
  {"alt-a",     "%A",  "\ea"},
  {"alt-b",     "%B",  "\eb"},
  {"alt-c",     "%C",  "\ec"},
  {"alt-d",     "%D",  "\ed"},
  {"alt-e",     "%E",  "\ee"},
  {"alt-f",     "%F",  "\ef"},
  {"alt-g",     "%G",  "\eg"},
  {"alt-h",     "%H",  "\eh"}, /* backspace? */
  {"alt-i",     "%I",  "\ei"},
  {"alt-j",     "%J",  "\ej"},
  {"alt-k",     "%K",  "\ek"},
  {"alt-l",     "%L",  "\el"},
  {"alt-n",     "%N",  "\em"},
  {"alt-n",     "%N",  "\en"},
  {"alt-o",     "%O",  "\eo"},
  {"alt-p",     "%P",  "\ep"},
  {"alt-q",     "%Q",  "\eq"},
  {"alt-r",     "%R",  "\er"},
  {"alt-s",     "%S",  "\es"},
  {"alt-t",     "%T",  "\et"},
  {"alt-u",     "%U",  "\eu"},
  {"alt-v",     "%V",  "\ev"},
  {"alt-w",     "%W",  "\ew"},
  {"alt-x",     "%X",  "\ex"},
  {"alt-y",     "%Y",  "\ey"},
  {"alt-z",     "%Z",  "\ez"},
  /* Linux Console  */
  {"home",      "Home", "\e[1~"},
  {"end",       "End",  "\e[4~"},
  {"F1",        "F1",   "\e[[A"},
  {"F2",        "F2",   "\e[[B"},
  {"F3",        "F3",   "\e[[C"},
  {"F4",        "F4",   "\e[[D"},
  {"F5",        "F5",   "\e[[E"},
  {"F6",        "F6",   "\e[[F"},
  {"F7",        "F7",   "\e[[G"},
  {"F8",        "F8",   "\e[[H"},
  {"F9",        "F9",   "\e[[I"},
  {"F10",       "F10",  "\e[[J"},
  {"F11",       "F11",  "\e[[K"},
  {"F12",       "F12",  "\e[[L"}, 
  {NULL, }
};

static int fb_keyboard_match_keycode (const char *buf, int length, const UfbKeyCode **ret)
{
  int i;
  int matches = 0;

  if (!strncmp (buf, "\e[M", MIN(length,3)))
    {
      if (length >= 6)
        return 9001;
      return 2342;
    }
  for (i = 0; ufb_keycodes[i].nick; i++)
    if (!strncmp (buf, ufb_keycodes[i].sequence, length))
      {
        matches ++;
        if (strlen (ufb_keycodes[i].sequence) == length && ret)
          {
            *ret = &ufb_keycodes[i];
            return 1;
          }
      }
  if (matches != 1 && ret)
    *ret = NULL;
  return matches==1?2:matches;
}

static char *evsource_kb_get_event (void)
{
  unsigned char buf[20];
  int length;

  for (length = 0; length < 10; length ++)
    if (read (STDIN_FILENO, &buf[length], 1) != -1)
      {
        const UfbKeyCode *match = NULL;

        /* special case ESC, so that we can use it alone in keybindings */
        if (length == 0 && buf[0] == 27)
          {
            struct timeval tv;
            fd_set rfds;
            FD_ZERO (&rfds);
            FD_SET (STDIN_FILENO, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 1000 * 120;
            if (select (1, &rfds, NULL, NULL, &tv) == 0)
              return strdup ("esc");
          }

        switch (fb_keyboard_match_keycode ((void*)buf, length + 1, &match))
          {
            case 1: /* unique match */
              if (!match)
                return NULL;
              return strdup (match->nick);
              break;
            case 0: /* no matches, bail*/
              { 
                static char ret[256];
                if (length == 0 && mrg_utf8_len (buf[0])>1) /* read a 
                                                             * single unicode
                                                             * utf8 character
                                                             */
                  {
                    read (STDIN_FILENO, &buf[length+1], mrg_utf8_len(buf[0])-1);
                    buf[mrg_utf8_len(buf[0])]=0;
                    strcpy (ret, (void*)buf);
                    return strdup(ret); //XXX: simplify
                  }
                if (length == 0) /* ascii */
                  {
                    buf[1]=0;
                    strcpy (ret, (void*)buf);
                    return strdup(ret);
                  }
                sprintf (ret, "unhandled %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c'",
                    length >=0 ? buf[0] : 0, 
                    length >=0 ? buf[0]>31?buf[0]:'?' : ' ', 
                    length >=1 ? buf[1] : 0, 
                    length >=1 ? buf[1]>31?buf[1]:'?' : ' ', 
                    length >=2 ? buf[2] : 0, 
                    length >=2 ? buf[2]>31?buf[2]:'?' : ' ', 
                    length >=3 ? buf[3] : 0, 
                    length >=3 ? buf[3]>31?buf[3]:'?' : ' ',
                    length >=4 ? buf[4] : 0, 
                    length >=4 ? buf[4]>31?buf[4]:'?' : ' ',
                    length >=5 ? buf[5] : 0, 
                    length >=5 ? buf[5]>31?buf[5]:'?' : ' ',
                    length >=6 ? buf[6] : 0, 
                    length >=6 ? buf[6]>31?buf[6]:'?' : ' '
                    );
                return strdup(ret);
              }
              return NULL;
            default: /* continue */
              break;
          }
      }
    else
      return strdup("key read eek");
  return strdup("fail");
}

static int evsource_kb_get_fd (void)
{
  return STDIN_FILENO;
}

static EvSource ev_src_kb = {
  NULL,
  (void*)evsource_kb_has_event,
  (void*)evsource_kb_get_event,
  (void*)evsource_kb_destroy,
  (void*)evsource_kb_get_fd,
  NULL
}; 

EvSource *evsource_kb_new (void)
{
  if (evsource_kb_init() == 0)
  {
    return &ev_src_kb;
  }
  return NULL;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb-evsource-mice.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// bundled include of ufb-evsource.h"

/* written to work with the zforce ir touchscreen of a kobo glo,
 * probably works with a range of /dev/input/event classs of
 * touchscreens.
 */

typedef struct Mice
{
  int     fd;
  double  x;
  double  y;
  int     prev_state;
} Mice;

static Mice  mice;
static Mice* mrg_mice_this = &mice;

static int ufb_evsource_mice_init ()
{
  unsigned char reset[]={0xff};
  /* need to detect which event */

  mrg_mice_this->prev_state = 0;
  mrg_mice_this->fd = open ("/dev/input/mice", O_RDONLY | O_NONBLOCK);
  if (mrg_mice_this->fd == -1)
  {
    fprintf (stderr, "error opening mice device\n");
//    sleep (1);
    return -1;
  }
    fprintf (stderr, "!\n");
  write (mrg_mice_this->fd, reset, 1);
  return 0;
}

static void mice_destroy ()
{
  if (mrg_mice_this->fd != -1)
    close (mrg_mice_this->fd);
}

static int mice_has_event ()
{
  struct timeval tv;
  int retval;

  if (mrg_mice_this->fd == -1)
    return 0;

  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET(mrg_mice_this->fd, &rfds);
  tv.tv_sec = 0; tv.tv_usec = 0;
  retval = select (mrg_mice_this->fd+1, &rfds, NULL, NULL, &tv);
  if (retval == 1)
    return FD_ISSET (mrg_mice_this->fd, &rfds);
  return 0;
}

static char *mice_get_event ()
{
  const char *ret = "mouse-motion";
  double relx, rely;
  signed char buf[3];
  read (mrg_mice_this->fd, buf, 3);
  relx = buf[1];
  rely = -buf[2];

  mrg_mice_this->x += relx;
  mrg_mice_this->y += rely;

  if ((mrg_mice_this->prev_state & 1) != (buf[0] & 1))
    {
      if (buf[0] & 1)
        {
          ret = "mouse-press";
        }
      else
        {
          ret = "mouse-release";
        }
    }
  else if (buf[0] & 1)
    ret = "mouse-drag";

  mrg_mice_this->prev_state = buf[0];

  {
    char *r = malloc (64);
    sprintf (r, "%s %.0f %.0f", ret, mrg_mice_this->x, mrg_mice_this->y);
    return r;
  }

  return NULL;
}

static int mice_get_fd (EvSource *ev_source)
{
  return mrg_mice_this->fd;
}

static void mice_set_coord (EvSource *ev_source, double x, double y)
{
  mrg_mice_this->x = x;
  mrg_mice_this->y = y;
}

static EvSource ev_src_mice = {
  NULL,
  (void*)mice_has_event,
  (void*)mice_get_event,
  (void*)mice_destroy,
  mice_get_fd,
  mice_set_coord
};

EvSource *evsource_mice_new (void)
{
  if (ufb_evsource_mice_init () == 0)
    {
      mrg_mice_this->x = 0;
      mrg_mice_this->y = 0;
      return &ev_src_mice;
    }
  return NULL;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb-evsource-ts.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// bundled include of ufb-evsource.h"

/* written to work with the zforce ir touchscreen of a kobo glo,
 * probably works with a range of /dev/input/event classs of
 * touchscreens.
 */

typedef struct Ts
{
  int    fd;
  double  x;
  double  y;
  int    down;
  int    rotate;
  int    width;
  int    height;
} Ts;

static Ts ts;
static Ts* this = &ts;

static int read_sys_int (const char *path)
{
  int ret = -1;
  int fd = open (path, O_RDONLY);
  char buf[16]="";
  if (fd>0)
    {
      if (read (fd, buf, 16)>0)
        ret = atoi (buf);
      close (fd);
    }
  return ret;
}


static int read_sys_int2 (const char *path, const char c)
{
  int ret = -1;
  int fd = open (path, O_RDONLY);
  char buf[32]="";
  if (fd>0)
    {
      if (read (fd, buf, 32)>0)
        {
          char *p = strchr (buf, c);
          if (p)
            ret = atoi (p+1);
        }
      close (fd);
    }
  return ret;
}

static int ufb_evsource_ts_init ()
{
  /* need to detect which event */

  this->down = 0;
  this->fd = open ("/dev/input/event1", O_RDONLY | O_NONBLOCK);

  this->rotate = read_sys_int ("/sys/class/graphics/fb0/rotate");
  if (this->rotate < 0)
    this->rotate = 0;
  if (this->rotate)
    {
      this->width = read_sys_int2 ("/sys/class/graphics/fb0/modes", ':');
      this->height = read_sys_int2 ("/sys/class/graphics/fb0/modes", 'x');
    }

  if (this->fd == -1)
  {
    fprintf (stderr, "error opening zforce device\n");
    //sleep (1);
    return -1;
  }
  return 0;
}

static void destroy ()
{
  if (this->fd != -1)
    close (this->fd);
}

static int has_event ()
{
  struct timeval tv;
  int retval;

  if (this->fd == -1)
    return 0;

  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET(this->fd, &rfds);
  tv.tv_sec = 0; tv.tv_usec = 0;
  retval = select (this->fd+1, &rfds, NULL, NULL, &tv);
  if (retval == 1)
    return FD_ISSET (this->fd, &rfds);
  return 0;
}

static char *get_event ()
{
  const char *ret = "mouse-motion";
  struct input_event ev;
  int buttstate = 0;
  int sync = 0;

  while (!sync){
    int rc = read (this->fd, &ev, sizeof (ev));
    if (rc != sizeof (ev))
    {
      //fprintf (stderr, "zforce read fail\n");
      return NULL;
    }
    else
    {
      switch (ev.type)
      {
        case 3:
          switch (ev.code)
            {
              case 0: this->x = ev.value; break;
              case 1: this->y = ev.value; break;
              default: break;
            }
          break;
        case 1:
          switch (ev.code)
          {
            case 0x014a:
              switch (ev.value)
                {
                  case 0: ret = "mouse-release";
                          this->down = 0;
                     break;
                  case 1: ret = "mouse-press";
                          this->down = 1;
                     break;
                }
              buttstate = 1;
              break;
          }
          break;
        case 0: /* sync (end of packet) */
          sync = 1;
          break;
      }
    }
  }

  if (!buttstate && this->down)
    ret = "mouse-drag";

  {
    char *r = malloc (64);
    switch (this->rotate)
    {
      case 1:
        sprintf (r, "%s %.0f %.0f", ret, this->y, this->height-this->x);
        break;
      case 2:
        sprintf (r, "%s %.0f %.0f", ret, this->height-this->x, this->width-this->y);
        break;
      case 3:
        sprintf (r, "%s %.0f %.0f", ret, this->height-this->y, this->x);
        break;
      case 0:
      default:
        sprintf (r, "%s %.0f %.0f", ret, this->x, this->y);
        break;
    }
    return r;
  }

  return NULL;
}

static int get_fd (EvSource *ev_source)
{
  return this->fd;
}

static void set_coord (EvSource *ev_source, double x, double y)
{
  fprintf (stderr, "can't really warp on a touch screen..\n");
}

static EvSource src = {
  NULL,
  (void*)has_event,
  (void*)get_event,
  (void*)destroy,
  get_fd,
  set_coord
};

EvSource *evsource_ts_new (void)
{
  if (ufb_evsource_ts_init () == 0)
    {
      return &src;
    }
  return NULL;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#ifndef UFB_FB_H
#define UFB_FB_H

#include <string.h>
#include <stdint.h>

typedef struct Ufb_ Ufb;

typedef enum {
  UFB_FLAG_DEFAULT = 0,
  UFB_FLAG_BUFFER  = 1 << 0  /* hint that we want an extra double buffering layer */
} UfbFlag;

/* create a new framebuffer client, passing in -1, -1 tries to request
 * a fullscreen window for "tablet" and smaller, and a 400, 300 portait
 * by default for desktops.
 */
Ufb*           ufb_new                  (int width, int height,
                                         UfbFlag flags, void *babl_format);

/* shut down an ufb.
 */
void           ufb_destroy              (Ufb *fb);

/* ufb_get_bytes_per_pixel:
 * @fb: an ufb
 * 
 * Name says it all.
 */
int            ufb_get_bytes_per_pixel  (Ufb *fb);

/* set a title to be used
 */
void           ufb_set_title            (Ufb *fb, const char *title);
const char *   ufb_get_title            (Ufb *fb);

/* ufb_set_size:
 * @fb: an fdev in between reads and writes
 * @width: new width
 * @height: new height
 *
 * Resizes a buffer, the buffer is owned by the client that created it,
 * pass -1, -1 to get auto (maximized) dimensions.
 */
void           ufb_set_size             (Ufb *fb, int width, int height);

/* modify the windows position in compositor/window-manager coordinates
 */
void           ufb_set_x (Ufb *fb, int x);
void           ufb_set_y (Ufb *fb, int y);
int            ufb_get_x (Ufb *fb);
int            ufb_get_y (Ufb *fb);

/* ufb_set_fps_limit:
 * @fb an ufb framebuffer
 * @fps_limit new fps limit.
 *
 * Enables an internal frame-limiter for /dev/fb use - that sleeps for the
 * time remaining for drawing a full frame. If set to 0 this rate limiter
 * is not enabled.
 */
void           ufb_set_fps_limit        (Ufb *fb, int fps_limit);

/* query the dimensions of an ufb, note that these values should not
 * be used as basis for stride/direct pixel updates, use the _get_buffer
 * functions, and the returned buffer and dimensions for that.
 */
void           ufb_get_size             (Ufb *fb, int *width, int *height);
int            ufb_get_width            (Ufb *fb);
int            ufb_get_height           (Ufb *fb);

/* Get the native babl_format of a buffer, requires babl
 * to be compiled in.
 */
const char*    ufb_get_babl_format      (Ufb *fb);

/* ufb_get_buffer_write:
 * @fb      framebuffer-client
 * @width   pointer to integer where width will be stored
 * @height  pointer to integer where height will be stored
 * @stride  pointer to integer where stride will be stored
 *
 * Get a pointer to memory when we've got data to put into it, this should be
 * called at the last possible minute, since some of the returned buffer, the
 * width, the height or stride might be invalidated after the corresponding
 * ufb_write_done() call.
 *
 * Return value: pointer to framebuffer, or return NULL if there is nowhere
 * to write data or an error.
 */
unsigned char *ufb_get_buffer_write (Ufb *fb,
                                     int *width, int *height,
                                     int *stride,
                                     void *babl_format);

/* ufb_write_done:
 * @fb: an ufb framebuffer
 * @damage_x: upper left most pixel damaged
 * @damage_y: upper left most pixel damaged
 * @damage_width: width bound of damaged pixels, or -1 for all
 * @damage_height: height bound of damaged pixels, or -1 for all
 *
 * Reports that writing to the buffer is complete, if possible provide the
 * bounding box of the changed region - as eink devices; as well as
 * compositing window managers want to know this information to do efficient
 * updates. width/height of -1, -1 reports that any pixel in the buffer might
 * have changed.
 */
void           ufb_write_done       (Ufb *fb,
                                     int damage_x, int damage_y,
                                     int damage_width, int damage_height);

/* thread safe event queue:  */
void           ufb_add_event        (Ufb *fb, const char *event);
int            ufb_has_event        (Ufb *fb);
const char    *ufb_get_event        (Ufb *fb);


/****** the following are for use by the compositor implementation *****/

/* check for damage, returns true if there is damage/pending data to draw
 */
int            ufb_get_damage       (Ufb *fb,
                                     int *x, int *y,
                                     int *width, int *height);

/* read only access to the buffer, this is most likely a superfluous call
 * for clients themselves; it is useful for compositors.
 */
const unsigned char *ufb_get_buffer_read (Ufb *fb,
                                          int *width, int *height,
                                          int *stride);
/* this clears accumulated damage.  */
void           ufb_read_done        (Ufb *fb);


/* open up a buffer - as held by a client */
Ufb           *ufb_compositor_open        (const char *path);

/* check if the dimensions have changed */
int            ufb_compositor_check_size  (Ufb *fb, int *width, int *height);

/* warp the _mouse_ cursor to given coordinates; doesn't do much on a
 * touch-screen
 */
void           ufb_warp_cursor (Ufb *fb, int x, int y);

/* a clock source, counting since startup in microseconds.
 */
long           ufb_ticks       (void);

long           ufb_client_pid  (Ufb *fb);

/* pset */

#ifndef PSET_H
#define PSET_H

#include <math.h>

 /* dithered optimized, partial accessible due to inlined factorization
  */

#ifndef  CLAMP
#define CLAMP(num, min, max) {if (num < min) num = min;if (num > max)num=max;}
#endif

extern int eink_is_mono;

static inline int a_dither_trc (int input)
{
  static int inited = 0;
  static int trc[1024];
  if (!inited)
  {
    int i;
    inited = 1;
    for (i = 0; i< 1024; i++)
      trc[i] = round(pow (i / 1023.0, 0.75) * 1023.999);
  }
  if (input < 0) input = 0;
  if (input > 1023) input = 1023;
  return trc[input];
}

static inline int a_dither(int x, int y)
{
  return ((x ^ y * 237) * 181 & 511)/2.00;
  //return ((x+    (y+(y<<2)+(y<<3)+(y<<8)))*3213) & 0xff;
}

static inline void ufb_dither_mono (int x, int y, 
                                    int *red,
                                    int *green,
                                    int *blue)
{
  int dither = a_dither(x,y);
  int value = a_dither_trc (*red + *green * 2.5 + *blue / 2) ;
  value = value <= dither ? 0 : 255;
  if(red)*red = value;
  if (red == green) /* quick bail, when red==green==blue it indicates gray*/
                    /* scale conversion,
                     * XXX: need to special case this for this case.. `*/
    return;
  if(green)*green = value;
  if(blue)*blue = value; 
}

static inline void ufb_dither_rgb (int x, int y, 
                                   int *red,
                                   int *green,
                                   int *blue)
{
  int dither = a_dither(x,y);
  dither = (dither >> 3) - 16;
  *red = a_dither_trc (*red * 4) + dither;
  CLAMP(*red, 0, 255);

  //*red = a_dither(x,y);
  if (red == green) /* quick bail, when red==green==blue it indicates gray*/
                    /* scale conversion */
    return;

  *green = a_dither_trc (*green * 4) + dither;
  CLAMP(*green, 0, 255);
  *blue = a_dither_trc (*blue * 4) + dither;
  CLAMP(*blue, 0, 255);
}

static inline void ufb_dither_generic (int x, int y, 
                                       int *red,
                                       int *green,
                                       int *blue)
{
  if (eink_is_mono)
    ufb_dither_mono (x, y, red, green, blue);
  else
    ufb_dither_rgb (x, y, red, green, blue);
}

#undef CLAMP

/* somewhat optimized pset routines; as well as unwrapped version that
 * permits efficient looping.
 */

#define Yu8_SET(p,r,g,b,a) do{p[0] = ((r)+(g)+(b))/3;}while(0)
#define RGBu565_SET(p,r,g,b,a) do{p[0] = (*((uint16_t*)(p)) = ((r) >> 3) + (((g)>>2) << 5) + ((b>> 3) << (5+6)));}while(0);
#define RGBu8_SET(p,r,g,b,a) do{ p[0] = (r); p[1] = (g); p[2] = (b); }while(0)
#define RGBAu8_SET(p,r,g,b,a) do{ p[0] = (r); p[1] = (g); p[2] = (b); p[3] = (a); }while(0)
  

inline static unsigned char *ufb_pix_pset_nodither  (
    Ufb *fb, unsigned char *pix, int bpp, int x, int y,
    int red, int green, int blue, int alpha)
{
  switch (bpp)
  {
    case 1: /* grayscale framebuffer */
      Yu8_SET(pix,red,green,blue,alpha);
      break;
    case 2: /* 16bit frame buffer, R' u5 G' u6 B' u5 */
      RGBu565_SET(pix,red,green,blue,alpha);
      break;
    case 3: /* 24bit frame buffer, R'G'B' u8 */
      RGBu8_SET(pix,red,green,blue,alpha);
      break;
    case 4: /* 32bit frame buffer, R'G'B'A u8 */
      RGBAu8_SET(pix,red,green,blue,alpha);
      break;
  }
  return pix + bpp;
}

inline static unsigned char *ufb_pix_pset_mono  (
    Ufb *fb, unsigned char *pix, int bpp, int x, int y,
    int red, int green, int blue, int alpha)
{
  int mono = (red+green+blue)/3;
  ufb_dither_mono (x, y, &mono, &mono, &mono);

  switch (bpp)
  {
    case 1: /* grayscale framebuffer */
      Yu8_SET(pix,mono,mono,mono,alpha);
      break;
    case 2: /* 16bit frame buffer, R' u5 G' u6 B' u5 */
      RGBu565_SET(pix,mono,mono,mono,alpha);
      break;
    case 3: /* 24bit frame buffer, R'G'B' u8 */
      RGBu8_SET(pix,mono,mono,mono,alpha);
      break;
    case 4: /* 32bit frame buffer, R'G'B'A u8 */
      RGBAu8_SET(pix,mono,mono,mono,alpha);
      break;
  }
  return pix + bpp;
}

#include <stdio.h>

inline static unsigned char *ufb_pix_pset (
    Ufb *fb, unsigned char *pix, int bpp, int x, int y,
    int red, int green, int blue, int alpha)
{
  //fprintf (stderr, "%i %i %i %i\n", x, y, ufb_get_width (fb), ufb_get_height (fb));
  ufb_dither_rgb (x, y, &red, &green , &blue);

  switch (bpp)
  {
    case 1: /* grayscale framebuffer */
      Yu8_SET(pix,red,green,blue,alpha);
      break;
    case 2: /* 16bit frame buffer, R' u5 G' u6 B' u5 */
      RGBu565_SET(pix,red,green,blue,alpha);
      break;
    case 3: /* 24bit frame buffer, R'G'B' u8 */
      RGBu8_SET(pix,red,green,blue,alpha);
      break;
    case 4: /* 32bit frame buffer, R'G'B'A u8 */
      RGBAu8_SET(pix,red,green,blue,alpha);
      break;
  }
  return pix + bpp;
}

inline static unsigned char *ufb_get_pix (Ufb *fb, unsigned char *buffer, int x, int y)
{
  /* this works, because bpp is the first member of fb, quickly getting this
   * in a generic function like 
  this is worth the hack. */
  int bpp =    ((int*)(fb))[0];
  int stride = ((int*)(fb))[1];
  unsigned char *pix = &buffer[y * stride + x * bpp];
  return pix;
}

/* a sufficiently fast pixel setter
 */
inline static void ufb_pset (Ufb *fb, unsigned char *buffer, int x, int y, int red, int green, int blue, int alpha)
{
  int bpp =    ((int*)(fb))[0];
  unsigned char *pix = ufb_get_pix (fb, buffer, x, y);
  ufb_pix_pset (fb, pix, bpp, x, y, red, green, blue, alpha);
}

inline static void ufb_pset_mono (Ufb *fb, unsigned char *buffer, int x, int y, int red, int green, int blue, int alpha)
{
  int bpp =    ((int*)(fb))[0];
  unsigned char *pix = ufb_get_pix (fb, buffer, x, y);
  ufb_pix_pset_mono (fb, pix, bpp, x, y, red, green, blue, alpha);
}

#endif

/********/


#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌ufb.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* tiny little self-contained ufb handling API
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// bundled include of ufb.h"
// bundled include of ufb-evsource.h"

#define UFB_WAIT_ATTEMPTS 300

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>

#define UFB_MAX_EVENT  1024

typedef enum {
  UFB_INITIALIZING,
  UFB_NEUTRAL,
  UFB_DRAWING,
  UFB_WAIT_FLIP,
  UFB_FLIPPING,
} UfbState;

typedef struct _UfbShm UfbShm;

typedef enum {
  UFB_f32,
  UFB_f32S,
  UFB_s16,
  UFB_s16S
} UfbAudioFormat;

#define UFB_AUDIO_BUFFER_SIZE  8192

struct  _UfbShm {  /*  Legend: C = client write, S = compositor write  */

  int32_t    width;           /* C  width of raster in pixels */
  int32_t    height;          /* C  height of raster in pixels */
  int32_t    stride;          /* C  byte offset between starts of
                                    pixelrows in memory   */
  int32_t    pid;             /* C  */

  UfbState   state;           /* CS used for synchronising flips  */

  double     x;               /* CS it isn't certain that the server  */
  double     y;               /* CS abides by these coordinates       */
  int32_t    desired_width;   /* S used for initiating resizes, */
  int32_t    desired_height;  /* S shm makes this be correct */

  int32_t    damage_x;        /* SC */
  int32_t    damage_y;        /* SC */
  int32_t    damage_width;    /* SC */
  int32_t    damage_height;   /* SC */

  int32_t    pad0;             /* ___ END OF CACHELINE ___ */
  int32_t    padding[16];

  uint8_t    title[512];      /* C  window title  */
  uint8_t   *babl_format[128];/* C  pixel format; a string that can be
                                    passed to babl_format() */

  int16_t    read_event;      /* C  last event_no which has been read    */
  int16_t    queued_event;    /* S  last event_no which has been queued  */
  uint8_t    events[UFB_MAX_EVENT][128];  /* S all events are described as strings; */

  /* PCM bits not yet implemented; but this probably is enough: */
  UfbAudioFormat pcm_format;     /* C */
  int            pcm_rate;       /* C */ 
  int            pcm_pos;                       /* S */
  int            pcm_queued_pos;                /* C */
  uint8_t        pcmbuf[UFB_AUDIO_BUFFER_SIZE]; /* C */
};

static Ufb *ufb_new_shm (int width, int height, void *babl_format);
static Ufb *ufb_new_fb  (int width, int height, void *babl_Format);

struct Ufb_
{

  int       bpp;
  int       stride;
  int       width;
  int       height;
  int       mapped_size;
  void     *format; /* babl format */
  char     *path;
  int       fd;
  UfbShm   *shm;
  int       fps_limit;
  void     *fb_mmap;
  int       is_toplevel;
  int       prev_marker;
  int       is_kobo;
  int       is_nook;
  struct    fb_var_screeninfo vinfo;
  struct    fb_fix_screeninfo finfo;
  EvSource *evsource[4];
  int       evsource_count;
  pthread_t event_thread;
};

static void ufb_remap (Ufb *fb);

int ufb_get_bytes_per_pixel (Ufb *fb)
{
  return fb->bpp;
}

void ufb_warp_cursor (Ufb *fb, int x, int y)
{
  int i;
  for (i = 0; i < fb->evsource_count; i++)
    {
      evsource_set_coord (fb->evsource[i], x, y);
    }
}

static struct timeval start_time;
#define usecs(time) ((time.tv_sec - start_time.tv_sec) * 1000000 + time.tv_usec)

static void ufb_init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

inline static long ticks (void)
{
  struct timeval measure_time;
  ufb_init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}

#undef usecs

long ufb_ticks (void)
{
  return ticks ();
}

int
ufb_wait_neutral (Ufb *fb)
{
  int attempts = UFB_WAIT_ATTEMPTS;

  if (fb->is_toplevel)
    return 0;

  while (fb->shm->state != UFB_NEUTRAL && --attempts)
    usleep (500);

  return (attempts > 0 ? 0 : -1);
} 

static int
ufb_set_state (Ufb *fb, UfbState state)
{
  fb->shm->state = state;
  return 1;
}

unsigned char *
ufb_get_buffer_write (Ufb *fb, int *width, int *height, int *stride,
    void *babl_format)
{
  ufb_wait_neutral (fb);
  ufb_set_state (fb, UFB_DRAWING);
  if (width) *width = fb->width;
  if (height) *height = fb->height;
  if (stride) *stride = fb->stride;

  if (fb->is_toplevel)
    {
      /* this goes wrong when we're very different.. */

      int ox, oy;
      int offset;
      ox = (fb->vinfo.xres - fb->width) / 2;
      oy = (fb->vinfo.yres - fb->height) / 2;
      offset = oy * fb->stride + ox * fb->bpp;
      return (void*)fb->fb_mmap + offset;
    }
  return (void*)&fb->shm[1];
}

int eink_is_mono = 0;

void ufb_eink_mono (Ufb *fb)
{
  eink_is_mono ++;
}

void ufb_eink_mono_end (Ufb *fb)
{
  eink_is_mono --;
}

#if 0
void kobo_eink_update_partial (int fb_fd, int mono, int left, int top, int width, int height);

static void
flip_ink (Ufb *fb, int x, int y, int width, int height)
{
  int ox, oy;

  //fprintf (stderr, "%i %i %i %i\n\r", x,y,width,height);

  if (width == -1)
    {
      ox = 0;
      oy = 0;
      x = 0;
      y = 0;
      width = fb->vinfo.xres;
      height = fb->vinfo.yres;
    }
  else
    {
      ox = (fb->vinfo.xres - fb->width) / 2;
      oy = (fb->vinfo.yres - fb->height) / 2;
    }

  x += ox;
  y += oy;

  if (x < 0)
    {
      width += x;
      x = 0;
    }
  if (y < 0)
    {
      height += y;
      y = 0;
    }
  if (x + width >= fb->vinfo.xres)
    {
      width = fb->vinfo.xres - x;
    }
  if (y + height >= fb->vinfo.yres)
    {
      height = fb->vinfo.yres - y;
    }

  kobo_eink_update_partial (fb->fd, eink_is_mono, x, y, width, height);
}
#endif

void
ufb_write_done (Ufb *fb, int x, int y, int width, int height)
{
  if (width == 0 && height == 0)
    {
      /* nothing written */
      fb->shm->state = UFB_NEUTRAL;
      return;
    }
  fb->shm->state = UFB_WAIT_FLIP;


  if (width < 0)
  {
    fb->shm->damage_x = 0;
    fb->shm->damage_y = 0;
    fb->shm->damage_width = fb->shm->width;
    fb->shm->damage_height = fb->shm->height;
  }
  else
  {
    /* XXX: should combine with existing damage */
    fb->shm->damage_x = x;
    fb->shm->damage_y = y;
    fb->shm->damage_width = width;
    fb->shm->damage_height = height;
  }

  fb->shm->state = UFB_WAIT_FLIP;

  if (fb->is_toplevel)
  {
#if 0
    flip_ink (fb, fb->shm->damage_x, fb->shm->damage_y,
                  fb->shm->damage_width, fb->shm->damage_height);
#endif

    if (fb->fps_limit)
    {
    static long prev_ticks = 0;
    long ticks_now = ticks ();
    int elapsed = ticks_now - prev_ticks;

    int min_delay = 1000000 / fb->fps_limit;
    if (elapsed < min_delay)
      {
#if 0
        fprintf (stdout, "sleeping:            %7i    %2.0f%% load\r", 
                 (min_delay - elapsed),
                 (100.0 * elapsed) / min_delay
                 );
        fflush (stdout);
#endif
        usleep ((min_delay - elapsed) * 0.95);
        prev_ticks = ticks ();
      }
    else
      {
        prev_ticks = ticks_now;
      }
    }
  
    fb->shm->state = UFB_NEUTRAL;
  }
}

int
ufb_wait_neutral_or_wait_flip (Ufb *fb)
{
  int attempts = UFB_WAIT_ATTEMPTS;

  if (fb->is_toplevel)
    return 0;

  while (fb->shm->state != UFB_NEUTRAL &&
         fb->shm->state != UFB_WAIT_FLIP &&
         --attempts)
    usleep (500);

  return   (attempts > 0 ? 0 : -1 );
}

const unsigned char *
ufb_get_buffer_read (Ufb *fb, int *width, int *height, int *stride)
{
  if (ufb_wait_neutral_or_wait_flip (fb))
    return NULL;
  //ufb_check_size (fb, NULL, NULL);
  ufb_set_state (fb, UFB_FLIPPING);
  if (width) *width = fb->width;
  if (height) *height = fb->height;
  if (stride) *stride = fb->stride;
  if (fb->is_toplevel)
    return (void*)fb->fb_mmap;
  return (void*)&fb->shm[1];
}

void
ufb_read_done (Ufb *fb)
{
  fb->shm->damage_x = 0;
  fb->shm->damage_y = 0;
  fb->shm->damage_width = 0;
  fb->shm->damage_height = 0;
  fb->shm->state = UFB_NEUTRAL;
}


Ufb *
ufb_compositor_open (const char *path)
{
  Ufb *fb = calloc (sizeof (Ufb), 1);

  /* with a NULL path - we should try to create the buffer
   * ourselves; and assume the display server will pick it
   * up..
   */

  fb->fd = open (path, O_RDWR);
  if (fb->fd == -1)
    {
      free (fb);
      return NULL;
    }
  
  /* first we map just the UfbShm struct */
  fb->shm = mmap (NULL, sizeof (UfbShm), PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
  fb->mapped_size = sizeof (UfbShm);

  ufb_remap (fb);
  return fb;
}

static void
ufb_remap (Ufb *fb)
{
  /* then we remap with extent of framebuffer also included */
  //fb->is_toplevel = 0;
  if (!fb->is_toplevel) 
    {
      int size = sizeof(UfbShm) + fb->shm->height * fb->shm->stride;
      if (size > fb->mapped_size)
        {
          if (pwrite (fb->fd, "", 1, size+1) == -1)
            fprintf (stderr, "failed stretching\n");
          fsync (fb->fd);
          fb->shm = mremap (fb->shm, fb->mapped_size, size, MREMAP_MAYMOVE);
          fb->mapped_size = size;
          if (fb->shm)
          fprintf (stderr, "%i remapped to %i\n", getpid(), size);
          else
          fprintf (stderr, "eeeek!\n");
        }
    }
  fb->width = fb->shm->width;
  fb->height = fb->shm->height;

  if (fb->is_toplevel)
  {
    /* XXX: sanity check dimensions */
    memset (fb->fb_mmap, 255, fb->mapped_size);
#if 0
    flip_ink (fb, 0, 0, -1, -1);
#endif
  }
  else
    fb->stride = fb->shm->stride;
}

int ufb_get_width  (Ufb *fb)
{
  return fb->width;
}
int ufb_get_height (Ufb *fb)
{
  return fb->height;
}

void
ufb_get_size (Ufb *fb, int *width, int *height)
{
  if (width)
    *width = fb->width;
  if (height)
    *height = fb->height;
}

int
ufb_compositor_check_size (Ufb *fb, int *width, int *height)
{
  int ret = 0;
  if (fb->width != fb->shm->width ||
      fb->height != fb->shm->height)
    {
      ufb_remap (fb);
      ret = 1;
    }
  if (width || height)
    ufb_get_size (fb, width, height);
  return ret;
}


void ufb_set_size (Ufb *fb, int width, int height)
{
  if (fb->is_toplevel)
  {
    if (width < 0 && height < 0)
      {
        width = fb->vinfo.xres;
        height = fb->vinfo.yres;
      }
    if (width > fb->vinfo.xres)
      width = fb->vinfo.xres;
    if (height > fb->vinfo.yres)
      height = fb->vinfo.yres;
  }
  else
  {
    while ((fb->shm->state != UFB_NEUTRAL) &&
           (fb->shm->state != UFB_INITIALIZING))
      usleep (500);
    fb->shm->state = UFB_INITIALIZING;
  }

  fb->shm->width = width;
  fb->shm->height = height;
  ufb_remap (fb);
  fb->shm->state = UFB_NEUTRAL;
}


int ufb_has_event (Ufb *fb)
{
  if (fb->shm->read_event != fb->shm->queued_event)
    return 1;
  return 0;
}

const char *ufb_get_event (Ufb *fb)
{
  if (fb->shm->read_event != fb->shm->queued_event)
    {
      fb->shm->read_event++;
      if (fb->shm->read_event >= UFB_MAX_EVENT)
        fb->shm->read_event = 0;
      return (void*)fb->shm->events[fb->shm->read_event];
    }
  return NULL;
}

extern char *__progname; /*XXX: ugly, but portable hack - that works with many libc's */

void _ufb_dither_init (void);

Ufb *ufb_new (int width, int height, UfbFlag flags, void *babl_format)
{
  Ufb *ret = NULL;
  eink_is_mono = 0; /* XXX: should not be needed! */
  const char *env = getenv ("UFB_BUFFER");
  if (env)
  {
    ret = ufb_compositor_open (env); /* pretend a bit that were a compositor when handed a buffer*/
    /* XXX: should be a size request! */
    ufb_set_size (ret, width, height);
    return ret;
  }
  else
  {
    env = getenv ("UFB_PATH");
    if (env)
    {
      ret = ufb_new_shm (width, height, babl_format);
      ufb_set_size (ret, width, height);
    }
  }

  if (!ret)
  { 
    ret = ufb_new_fb (width, height, babl_format);
    ufb_set_size (ret, width, height);
    ret->is_kobo = 1;
    if (!ret)
      fprintf (stderr, "ufb failed to open linux framebuffer, when.. \n");
  }

  if (ret)
    {
      ufb_set_title (ret, __progname);
    }

  if (!ret)
    {
      fprintf (stderr, "unable to get framebuffer\n");
    }

  return ret;
}

void ufb_set_fps_limit (Ufb *ufb, int fps_limit)
{
  ufb->fps_limit = fps_limit;
}


EvSource *evsource_ts_new (void);
EvSource *evsource_kb_new (void);
EvSource *evsource_mice_new (void);

static int event_thread (Ufb *fb)
{
  /* XXX: use fd's and select on them */
  while (1)
  {
    int i;
    int had_event = 0;
    for (i = 0; i < fb->evsource_count; i ++)
      {
        while (evsource_has_event (fb->evsource[i]))
          {
            char *event = evsource_get_event (fb->evsource[i]);
            if (event)
              {
                ufb_add_event (fb, event);
                free (event);
                had_event++;
              }
          }
      }
    if (!had_event)
      usleep (400);
  }

  return 0;
}

static int ufb_add_evsource (Ufb *fb, EvSource *source)
{
  if (source)
    {
      fb->evsource[fb->evsource_count++] = source;
      return 0;
    }
  return 1;
}

static Ufb *ufb_new_fb (int width, int height, void *babl_format)
{
  Ufb *fb = calloc (sizeof (Ufb), 1);

  if (getenv ("DISPLAY"))
  {
    free (fb);
    fprintf (stderr, "Abort, trying to initialized framebuffer from under X\n");
    exit(-1);
  }

  fb->evsource_count = 0;
  fb->is_toplevel = 1;
  fb->fps_limit = 0; /* 10 fps seems good for both.. */
  fb->width = width;
  fb->height = height;
  fb->format = babl_format;

  fb->fd = open ("/dev/fb0", O_RDWR);
  if (fb->fd > 0)
    fb->path = strdup ("/dev/fb0");
  else
    {
      fb->fd = open ("/dev/graphics/fb0", O_RDWR);
      if (!fb->fd)
        {
          fb->path = strdup ("/dev/graphics/fb0");
        }
      else
        {
          free (fb);
          return NULL;
        }
    }

   if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->finfo))
     {
       fprintf (stderr, "error getting fbinfo\n");
       close (fb->fd);
       free (fb->path);
       free (fb);
       return NULL;
     }

   if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vinfo))
     {
       fprintf (stderr, "error getting fbinfo\n");
       close (fb->fd);
       free (fb->path);
       free (fb);
       return NULL;
     }

  fb->bpp = fb->vinfo.bits_per_pixel / 8;
  fb->stride = fb->finfo.line_length;
  fb->mapped_size = fb->finfo.smem_len;
  fb->fb_mmap = mmap (NULL, fb->mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
  fb->shm = calloc (sizeof(UfbShm), 1);

  memset (fb->fb_mmap, 255, fb->mapped_size);

  if (fb->width < 0 && fb->height < 0)
    {
      fb->width = fb->vinfo.xres;
      fb->height = fb->vinfo.yres;
    }

  fb->shm->desired_width = fb->width;
  fb->shm->desired_height = fb->height;
  fb->shm->width = fb->width;
  fb->shm->stride = fb->stride;
  fb->shm->height = fb->height;
  fb->shm->state = UFB_NEUTRAL;
  fb->shm->read_event = fb->shm->queued_event = 0;

  ufb_add_evsource (fb, evsource_kb_new ());

  if (ufb_add_evsource (fb, evsource_ts_new ()) != 0)
    ufb_add_evsource (fb, evsource_mice_new ());


  if (fb->evsource_count)
    {
      pthread_create (&fb->event_thread, NULL, (void*)event_thread, fb);
    }

  return fb;
}

static Ufb *ufb_new_shm (int width, int height, void *babl_format)
{
  Ufb *fb = calloc (sizeof (Ufb), 1);
  if (width < 0 && height < 0)
    {
      width = 640;
      height = 480;
    }

  fb->format = babl_format;
  fb->width = width;
  fb->height = height;
  fb->bpp = 4;
  fb->stride = fb->width * fb->bpp;
  {
    char buf[512];
    sprintf (buf, "%s/fb.XXXXXX", getenv("UFB_PATH"));
    fb->path = strdup (buf);
  }
  fb->fd = mkostemp (fb->path, O_RDWR | O_CREAT | O_TRUNC);
  pwrite (fb->fd, "", 1, sizeof (UfbShm) + fb->stride * fb->height);
  fsync (fb->fd);

  {
    char buf[256];
    sprintf (buf, "chmod a+rwx %s", fb->path);
    system (buf);
  }

  fb->mapped_size = fb->stride * fb->height;
  fb->shm = mmap (NULL, fb->mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

  fb->shm->desired_width = fb->width;
  fb->shm->desired_height = fb->height;
  fb->shm->width = fb->width;
  fb->shm->stride = fb->stride;
  fb->shm->height = fb->height;
  fb->shm->state = UFB_NEUTRAL;
  fb->shm->pid = getpid ();

  return fb;
}

void
ufb_destroy (Ufb *fb)
{
  if (fb->is_toplevel)
    {
      munmap (fb->fb_mmap, fb->mapped_size);
      free (fb->shm);
    }
  else
    {
      //unlink (fb->path);
      munmap (fb->shm, fb->mapped_size);
    }
  close (fb->fd);
  free (fb);
}

void ufb_add_event (Ufb *fb, const char *event)
{
  UfbShm *shm = fb->shm;
  int event_no = shm->queued_event + 1;
  if (event_no >= UFB_MAX_EVENT)
    event_no = 0;

  if (event_no == shm->read_event)
    {
      static int once = 0;
      if (!once)
        fprintf (stderr, "oc event queue overflow\n");
      once = 1;
      return;
    }

  strcpy ((void*)shm->events[event_no], event);

  shm->queued_event++;
  if (shm->queued_event >= UFB_MAX_EVENT)
    shm->queued_event = 0;
}

int ufb_get_damage (Ufb *fb, int *x, int *y, int *width, int *height)
{
  return fb->shm->state == UFB_WAIT_FLIP;
}

const char *ufb_get_babl_format (Ufb *fb)
{
  return (char *)fb->shm->babl_format;
}

void
ufb_set_title (Ufb *ufb, const char *title)
{
  strncpy ((void*)ufb->shm->title, title, 512);
}

const char *
ufb_get_title (Ufb *ufb)
{
  return (void*)ufb->shm->title;
}

#if 1
void
no_dither_blit (int x0, int y0, int width,int height, int src_stride, uint8_t *src, uint8_t *dest, int dest_stride)
{
  int x, y;
  for (y = y0; y < y0 + height; y ++)
    for (x = x0; x < x0 + width; x ++)
    {
      dest[dest_stride * y + x] = src[src_stride * (y-y0) + (x0-x)];
      if (x > x0 + width)
      {
        x = x0;
        y++;
      }
    }
}


void
dither_blit (int x0, int y0, int width,int height, int src_stride, uint8_t *src, uint8_t *dest, int dest_stride)
{
  int x, y;
  for (y = y0; y < y0 + height; y ++)
    for (x = x0; x < x0 + width; x ++)
    {
      dest[dest_stride * y + x] =
        src[src_stride * (y-y0) + (x0-x)] <= a_dither(x,y) ? 0 : 255;

      if (x > x0 + width)
      {
        x = x0;
        y++;
      }
    }
}

unsigned char mask [256*256];

void
dither_lut_blit (int x0, int y0, int width,int height, int src_stride, uint8_t *src, uint8_t *dest, int dest_stride)
{
  int x, y;
  for (y = y0; y < y0 + height; y ++)
    for (x = x0; x < x0 + width; x ++)
    {
      dest[dest_stride * y + x] =
        src[src_stride * (y-y0) + (x0-x)] <= mask[(x&0xff)*256 +(y&0xff)] ? 0 : 255;

      if (x > x0 + width)
      {
        x = x0;
        y++;
      }
    }
}


int compute_trc (void)
{
  unsigned char buf[1024*1024];

  int g;
  int t;

  for (t = 0; t < 256; t++)
  {
      long sum = 0;
  for (g = 0; g < 256; g++)
    {
      int x,y;
      sum = 0;
      for (y = 0; y < 1024; y++)
        for (x = 0; x < 1024; x++)
          {
            buf[y*1024+x] = g < a_dither (x,y) ? 0 : 255;
            sum += buf[y*1024+x];
          }
      if (t - (sum / (1024 * 1024.0)) < 0.5)
        break;

    }
    printf ("%i, ", g);// /* %i (%f) */\n", g, t, sum / (1024 * 1024.0));
    //printf ("%i, /* %i (%f) */\n", g, t, sum / (1024 * 1024.0));
  }
  return 0;
}


int do_test (void)
{
  long ticks;
  
  void *src = calloc (1024 * 1024, 1);
  void *dst = calloc (1024 * 1024, 1);

  int i;

  ticks = ufb_ticks ();
  for (i = 0; i < 10000; i ++)
  {
    dither_blit (0,0,1024,1024,1024,src,dst,1024);
  }
  printf ("dither_blit: %f mb/s\n", 1000000 * (10000.0 )/(ufb_ticks () - ticks));

  ticks = ufb_ticks ();
  for (i = 0; i < 10000; i ++)
  {
    no_dither_blit (0,0,1024,1024,1024,src,dst,1024);
  }
  printf ("no_dither_blit: %f mb/s\n", 1000000 * (10000.0 )/(ufb_ticks () - ticks));

  ticks = ufb_ticks ();
  for (i = 0; i < 10000; i ++)
  {
    dither_lut_blit (0,0,1024,1024,1024,src,dst,1024);
  }
  printf ("lut_dither_blit: %f mb/s\n", 1000000 * (10000.0 )/(ufb_ticks () - ticks));

  return 0;
}

void ufb_set_x (Ufb *fb, int x)
{
  if (!fb->is_toplevel)
  {
    fb->shm->x = x;
  }
}
void ufb_set_y (Ufb *fb, int y)
{
  if (!fb->is_toplevel)
  {
    fb->shm->y = y;
  }
}
int  ufb_get_x (Ufb *fb)
{
  if (!fb->is_toplevel)
  {
    return fb->shm->x;
  }
  return 0;
}
int ufb_get_y (Ufb *fb)
{
  if (!fb->is_toplevel)
  {
    return fb->shm->y;
  }
  return 0;
}

long ufb_client_pid (Ufb *fb)
{
  if (!fb->is_toplevel)
  {
    return fb->shm->pid;
  }
  return 0;
}

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-ufb.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


// bundled include of mrg-config.h"
#if MRG_UFB

// bundled include of ufb.h"
// bundled include of mrg-internal.h"

static unsigned char *mrg_ufb_get_pixels (Mrg *mrg, int *rowstride)
{
  Ufb *ufb = mrg->backend;
  int width, height;

  return ufb_get_buffer_write (ufb, &width, &height, rowstride, NULL);
}

static void mrg_ufb_flush (Mrg *mrg)
{
  Ufb *ufb = mrg->backend;

  //UFB_Surface *screen = mrg->backend;
  /* XXX: move this safe-guard into flush itself? */
  if (mrg->dirty.x < 0)
  {
    mrg->dirty.width += -mrg->dirty.x;
    mrg->dirty.x = 0;
  }
  if (mrg->dirty.y < 0)
  {
    mrg->dirty.height += -mrg->dirty.y;
    mrg->dirty.y = 0;
  }
  if (mrg->dirty.x + mrg->dirty.width >= mrg->width)
  {
    mrg->dirty.width = mrg->width - mrg->dirty.x - 1;
  }
  if (mrg->dirty.y + mrg->dirty.height >= mrg->height)
  {
    mrg->dirty.height = mrg->height - mrg->dirty.y -1;
  }
  if (mrg->dirty.width < 0)
    mrg->dirty.width = 0;
  if (mrg->dirty.height < 0)
    mrg->dirty.height = 0;

  ufb_write_done (ufb, mrg->dirty.x, mrg->dirty.y, mrg->dirty.width, mrg->dirty.height);
#if MRG_CAIRO
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
#endif
}

#if 0
static void mrg_ufb_fullscreen (Mrg *mrg, int fullscreen)
{
  UFB_Surface *screen = mrg->backend;
  int width = 640, height = 480;

  if (fullscreen)
  {
    UFB_Rect **modes;
    modes = UFB_ListModes(NULL, UFB_HWSURFACE|UFB_FULLSCREEN);
    if (modes == (UFB_Rect**)0) {
        fprintf(stderr, "No modes available!\n");
        return;
    }
  
    width = modes[0]->w;
    height = modes[0]->h;

    screen = UFB_SetVideoMode(width, height, 32,
                              UFB_SWSURFACE | UFB_FULLSCREEN );
    mrg->backend = screen;
  }
  else
  {
    screen = UFB_SetVideoMode(width, height, 32,
                              UFB_SWSURFACE | UFB_RESIZABLE );
    mrg->backend = screen;
  }
  mrg->fullscreen = fullscreen;
}
#endif

static void mrg_ufb_consume_events (Mrg *mrg, int block);

static void mrg_ufb_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  while (!_mrg_has_quit (mrg))
  {
    if (_mrg_is_dirty (mrg))
      mrg_ui_update (mrg);

    if (mrg->idles)
      _mrg_idle_iteration (mrg);

    mrg_ufb_consume_events (mrg, !mrg->idles);
  }
}

static void mrg_ufb_warp_pointer (Mrg *mrg, float x, float y)
{
  ufb_warp_cursor (mrg->backend, x, y);
}

#if 0
static int timer_cb (Uint32 interval, void *param)
{
  _mrg_idle_iteration (param);
  return param;
}
#endif

static void mrg_ufb_set_title (Mrg *mrg, const char *title)
{
  ufb_set_title (mrg->backend, title);
}

static const char *mrg_ufb_get_title (Mrg *mrg)
{
  return ufb_get_title (mrg->backend);
}

static void mrg_ufb_set_position  (Mrg *mrg, int x, int y)
{
  ufb_set_x (mrg->backend, x);
  ufb_set_y (mrg->backend, y);
}

void  mrg_ufb_get_position  (Mrg *mrg, int *x, int *y)
{
  if (x)
    *x = ufb_get_x (mrg->backend);
  if (y)
    *y = ufb_get_y (mrg->backend);
}

static void *ufb_self = NULL;
static void  ufb_atexit (void)
{
  fprintf (stderr, "teardown time! %p\n", ufb_self);
  ufb_destroy (ufb_self);
}

Mrg *_mrg_ufb_new (int width, int height)
{
  Mrg *mrg;
  Ufb *ufb;
  int fullscreen = 0;

  //if (!getenv ("UFB_PATH"))
  //  return NULL;

  if (width < 0)
  {
     width = 640;
     height = 480;
     fullscreen = 1;
  }

  if (getenv ("UFB_PATH"))
  {
    ufb = ufb_new (width, height, 0, NULL);
    ufb_self = ufb;
    atexit (ufb_atexit);
  }
  else
    ufb = ufb_new (width, height, 1, NULL);
  if (!ufb)
  {
    fprintf (stderr, "unable to init ufb\n");
    return NULL;
  }
  ufb_set_size (ufb, width, height);

  mrg = calloc (sizeof (Mrg), 1);

  mrg->mrg_main = mrg_ufb_main;
  mrg->mrg_flush = mrg_ufb_flush;
  mrg->mrg_warp_pointer = mrg_ufb_warp_pointer;
  //mrg->mrg_fullscreen = mrg_ufb_fullscreen;
  mrg->mrg_get_pixels = mrg_ufb_get_pixels;
  mrg->mrg_set_title = mrg_ufb_set_title;
  mrg->mrg_get_title = mrg_ufb_get_title;
  mrg->mrg_get_position = mrg_ufb_get_position;
  mrg->mrg_set_position = mrg_ufb_set_position;

  mrg->backend = ufb;

  _mrg_init (mrg, width, height);
  mrg_set_size (mrg, width, height);
  mrg->do_clip = 1;

  if (fullscreen)
    mrg_set_fullscreen (mrg, fullscreen);

  return mrg;
}

/***/


static void mrg_ufb_consume_events (Mrg *mrg, int block)
{
  Ufb *fb = mrg->backend;
  int events = 0;
  while (ufb_has_event (fb))
  {
    const char *event = ufb_get_event (fb);
    char event_type[128];
    float x = 0, y = 0;

    sscanf (event, "%s %f %f", event_type, &x, &y);
    if (!strcmp (event_type, "mouse-press"))
    {
      mrg_pointer_press (mrg, x, y, 1);
    }
    else if (!strcmp (event_type, "mouse-drag") ||
             !strcmp (event_type, "mouse-motion"))
    {
      mrg_pointer_motion (mrg, x, y, 1);
    }
    else if (!strcmp (event_type, "mouse-release"))
    {
      mrg_pointer_release (mrg, x, y, 1);
    }
    else
    {
      mrg_key_press (mrg, 0, event);
    }
    events ++;
  }
  if (!events)
  {
    usleep (20000);
  }
}

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌nchanterm.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* nchanterm, a minimal ANSI based utf8 terminal control abstraction.
 *
 * Copyright (c) 2012 Øyvind Kolås <pippin@gimp.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NCHANTERM_H
#define NCHANTERM_H

typedef struct _Nchanterm Nchanterm;

/* create a new terminal instance, automatically sized to initial size
 * of terminal.
 */
Nchanterm  *nct_new      (void);
/* free up a terminal instance */
void        nct_destroy  (Nchanterm *term);
/* set the size of the terminal, you need to do this in resize-callbacks */
void        nct_set_size (Nchanterm *term, int width, int height);
/* query size of terminal */
int         nct_width    (Nchanterm *term);
int         nct_height   (Nchanterm *term);
/* clear terminal contents to ' ', reset attributes and colors */
void        nct_clear    (Nchanterm *term);
/* set a utf8 char from the buffer str at location x, y with current style */
void        nct_set      (Nchanterm *term, int x, int y, const char *str);
/* read back the utf8 string in a character cell (XXX: style readback missing)*/
const char *nct_get      (Nchanterm *term, int x, int y);
/* print a string at given coordinates, only num_chars first chars printed */
int         nct_print    (Nchanterm *term, int x, int y,
                          const char *string, int num_chars);
enum {
  NCT_COLOR_BLACK  = 0,
  NCT_COLOR_RED    = 1,
  NCT_COLOR_GREEN  = 2,
  NCT_COLOR_YELLOW = 3,
  NCT_COLOR_BLUE   = 4,
  NCT_COLOR_MAGENTA= 5,
  NCT_COLOR_CYAN   = 6,
  NCT_COLOR_WHITE  = 7
};
/* set the foreground/background color used */
void nct_fg_color (Nchanterm *term, int color);
void nct_bg_color (Nchanterm *term, int color);

enum {
  NCT_A_NORMAL     = 0,
  NCT_A_BOLD       = 1 << 0,
  NCT_A_DIM        = 1 << 1,
  NCT_A_UNDERLINE  = 1 << 2,
  NCT_A_REVERSE    = 1 << 3,
};

/* set the attribute mode, this is a bitmask combination of the above modes. */
void nct_set_attr            (Nchanterm *term, int attr);
/* get current attribute mode.  */
int  nct_get_attr            (Nchanterm *term);

/* update what is visible to the user */
void nct_flush               (Nchanterm *term);

/* force a full redraw */
void nct_reflush             (Nchanterm *term);

/* get the width and height of the terminal querying the system */
int  nct_sys_terminal_width  (void);
int  nct_sys_terminal_height (void);

/* toggle visibility of the cursor */
void nct_show_cursor         (Nchanterm *term);
void nct_hide_cursor         (Nchanterm *term);

/* set position of cursor */
void nct_set_cursor_pos      (Nchanterm *term, int  x, int  y);

/* get position of cursor */
void nct_get_cursor_pos      (Nchanterm *term, int *x, int *y);

/* input handling
 *   nchanteds input handling reports keyboard, window resizing and mouse
 *   events in a uniform string based API. UTF8 input is passed through
 *   as the strings of the glyphs as the event string.
 *
 *  "size-changed"   - the geometry of the terminal has changed
 *  "mouse-pressed"  - the left mouse button was clicked
 *  "mouse-drag"     - the left mouse button was clicked
 *  "mouse-motion"   - motion events (also after release)
 *  "idle"           - timeout elapsed.
 *  keys: "up" "down" "space" "control-left" "return" "esc" "a" "A"
 *        "control-a" "F1" "control-c" ...
 */

/* get an event as a string from the terminal, if it is a mouse event coords
 * will be returned in x and y if not NULL, block for up to timeout_ms  */
const char *nct_get_event     (Nchanterm *n, int timeout_ms, int *x, int *y);

/* check if there is pending events, with a timeout */
int         nct_has_event (Nchanterm *n, int timeout_ms);

/* get a human readable/suited for ui string representing a keybinding */
const char *nct_key_get_label (Nchanterm *n, const char *nick);

enum {
  NC_MOUSE_NONE  = 0,
  NC_MOUSE_PRESS = 1,  /* "mouse-pressed", "mouse-released" */
  NC_MOUSE_DRAG  = 2,  /* + "mouse-drag"   (motion with pressed button) */
  NC_MOUSE_ALL   = 3   /* + "mouse-motion" (also delivered for release) */
};
/* set which mouse events to report, defaults to report none */
void nct_mouse           (Nchanterm *term, int nct_mouse_state);
#endif

/************************** end of header ***********************/
#ifndef NCHANTERM_HEADER_ONLY

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termio.h>
#include <locale.h>
#include <fcntl.h>

int nct_sys_terminal_width (void)
{
  struct winsize ws; 
  if (ioctl(0,TIOCGWINSZ,&ws)!=0)
    return 80;
  return ws.ws_col;
} 

int nct_sys_terminal_height (void)
{
  struct winsize ws; 
  if (ioctl(0,TIOCGWINSZ,&ws)!=0)
    return 25;
  return ws.ws_row;
}

typedef struct NcCell
{ 
  char  str[8]; /* utf8 representation of char */
  int   attr;   /* attributes */
  int   color;  /* both fg and bg */
} NcCell;

struct _Nchanterm {
  NcCell  *cells_front;
  NcCell  *cells_back;
  int      cells_width;
  int      cells_height;
  int      mode;
  int      color;
  int      mode_set;
  int      color_set;
  int      width;
  int      height;
  int      cursor_x;
  int      cursor_y;
  float    mouse_x;
  float    mouse_y;
  int      mouse_fd;
  int      utf8;
  int      is_st;
};

/* a quite minimal core set of terminal escape sequences are used to do all
 * things nchanterm can do */
#define ANSI_RESET_DEVICE        "\033c"
#define ANSI_YX                  "\033[%d;%dH"
#define ANSI_CURSOR_FORWARD      "\033[%dC"
#define ANSI_CURSOR_FORWARD1     "\033[C"
#define ANSI_STYLE_RESET         "\033[m"
#define ANSI_STYLE_START         "\033["
#define ANSI_STYLE_END           "m"
#define DECTCEM_CURSOR_SHOW      "\033[?25h"
#define DECTCEM_CURSOR_HIDE      "\033[?25l"
#define TERMINAL_MOUSE_OFF       "\033[?1000l"
#define TERMINAL_MOUSE_ON_BASIC  "\033[?1000h"
#define TERMINAL_MOUSE_ON_DRAG   "\033[?1000h\033[?1003h" /* +ON_BASIC for wider */
#define TERMINAL_MOUSE_ON_FULL   "\033[?1000h\033[?1004h" /* compatibility */
#define XTERM_ALTSCREEN_ON       "\033[?47h"
#define XTERM_ALTSCREEN_OFF      "\033[?47l"

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

void nct_show_cursor (Nchanterm UNUSED *t)
{
  printf ("%s", DECTCEM_CURSOR_SHOW);
}
void nct_hide_cursor (Nchanterm UNUSED *t)
{
  printf ("%s", DECTCEM_CURSOR_HIDE);
}

static int nct_utf8_len (const unsigned char first_byte)
{
  if      ((first_byte & 0x80) == 0)
    return 1; /* ASCII */
  else if ((first_byte & 0xE0) == 0xC0)
    return 2;
  else if ((first_byte & 0xF0) == 0xE0)
    return 3;
  else if ((first_byte & 0xF8) == 0xF0)
    return 4;
  return 1;
}

static int nct_utf8_strlen (const char *s)
{
   int count;
   if (!s)
     return 0;
   for (count = 0; *s; s++)
     if ((*s & 0xC0) != 0x80)
       count++;
   return count;
}

void nct_set_size      (Nchanterm *n, int width, int height)
{
  n->width = width;
  n->height = height;
  nct_set_cursor_pos (n, width, height);
}

int nct_width (Nchanterm *n)
{
  return n->width;
} 
int nct_height (Nchanterm *n)
{
  return n->height;
} 

#define NCT_COLOR_PAIR(f,b)   (f * 8 + b)
#define NCT_COLOR_FG(p)       (((int)p)/8)
#define NCT_COLOR_BG(p)       (((int)p)%8)
#define NCHANT_DEFAULT_COLORS (NCT_COLOR_PAIR(NCT_COLOR_WHITE, NCT_COLOR_BLACK))

/* emit minimal(few at least) escape characters to bring terminal from known
 * current mode to desired mode. */
static void nct_ensure_mode (Nchanterm *n)
{
  int first = 1;
  int data = 0;

  if (n->mode_set == n->mode && n->color_set == n->color)
    return;
  n->mode_set  = n->mode;
  n->color_set = n->color;

  printf(ANSI_STYLE_RESET);
#define SEPERATOR  { if (!data) { printf(ANSI_STYLE_START); data = 1;} \
                     if (first) first=0; else printf (";");}
  if (n->mode & NCT_A_BOLD)     { SEPERATOR; printf ("1"); }
  if ((n->mode & NCT_A_DIM) && !n->is_st) { SEPERATOR; printf ("2");}
  if (n->mode & NCT_A_REVERSE)  { SEPERATOR; printf ("7"); }
  if (n->mode & NCT_A_UNDERLINE){ SEPERATOR; printf ("4"); }
  if (n->color != NCHANT_DEFAULT_COLORS)
    { SEPERATOR; 
      if (NCT_COLOR_BG(n->color) == 0)
      printf ("%i", NCT_COLOR_FG(n->color)+30);
      else
      printf ("%i;%i", NCT_COLOR_FG(n->color)+30,
                       NCT_COLOR_BG(n->color)+40); }
  if (data)
    printf(ANSI_STYLE_END);
#undef SEPERATOR
}

void nct_set_attr (Nchanterm *n, int attr)
{
  if (n->mode != attr)
      n->mode = attr;
}

int nct_get_attr (Nchanterm *n)
{
  return n->mode;
}

void nct_fg_color (Nchanterm *n, int ncolor)
{
  n->color -= NCT_COLOR_FG (n->color) * 8;
  n->color += ncolor * 8;
}

void nct_bg_color (Nchanterm *n, int ncolor)
{
  n->color -= NCT_COLOR_BG (n->color);
  n->color += ncolor;
}

int nct_print (Nchanterm *n, int x, int y, const char *string, int utf8_length)
{
  const char *s;
  int len;
  int pos = 0;
  if (!string)
    return 0;
  if (utf8_length < 0)
    utf8_length = nct_utf8_strlen (string);

  for (s = string; pos < utf8_length && *s; s += nct_utf8_len (*s))
    {
      int c;
      len = nct_utf8_len (*s);
      nct_set (n, x++, y, s);
      for (c = 0; c < len; c++)
        if (!s[c])
          return pos;
      pos++;
    }
  return pos;
}

static void nct_cells_clear (Nchanterm *n)
{
  int i;
  for (i = 0; i < n->cells_width * n->cells_height; i ++)
    {
      n->cells_back[i].str[0]=' ';
      n->cells_back[i].str[1]='\0';
      n->cells_back[i].attr = 0;
      n->cells_back[i].color = NCHANT_DEFAULT_COLORS;
    }
}

void nct_clear (Nchanterm *n)
{
  n->color = NCHANT_DEFAULT_COLORS;
  nct_set_attr (n, NCT_A_NORMAL);
  nct_cells_clear (n);
}

static void nct_cells_clear_front (Nchanterm *n)
{
  int i;
  for (i = 0; i < n->cells_width * n->cells_height; i ++)
    n->cells_front[i].str[0]='\2'; 
}

static void nct_cells_ensure (Nchanterm *n)
{
  int w = n->width;
  int h = n->height;
  if (w != n->cells_width || h != n->cells_height)
    {
      if (n->cells_front)
        free (n->cells_front);
      if (n->cells_back)
        free (n->cells_back);
      n->cells_width = w;
      n->cells_height = h;
      n->cells_front = calloc (sizeof (NcCell) * n->cells_width * n->cells_height, 1);
      n->cells_back  = calloc (sizeof (NcCell) * n->cells_width * n->cells_height, 1);
      nct_cells_clear (n);
      nct_cells_clear_front (n);
    }
}

static NcCell *nct_get_cell (Nchanterm *n, int x, int y)
{
  nct_cells_ensure (n);
  if (x < 1) x = 1;
  if (y < 1) y = 1;
  if (x > n->cells_width) x = n->cells_width;
  if (y > n->cells_height) y = n->cells_height;
  return &n->cells_back[(y-1) * n->cells_width + (x-1)];
}

static NcCell *nct_get_front_cell (Nchanterm *n, int x, int y)
{
  nct_cells_ensure (n);
  if (x < 1) x = 1;
  if (y < 1) y = 1;
  if (x > n->cells_width) x = n->cells_width;
  if (y > n->cells_height) y = n->cells_height;
  return &n->cells_front[(y-1) * n->cells_width + (x-1)];
}

static void _nct_set_attr (Nchanterm *n, int x, int y, int attr, int color)
{
  NcCell *cell = nct_get_cell (n, x, y);
  cell->attr   = attr;
  cell->color  = color;
}

void nct_set (Nchanterm *n, int x, int y, const char *str)
{
  int     i, bytes = nct_utf8_len (*str);
  NcCell *cell     = nct_get_cell (n, x, y);

  if (x <= 0 || y <= 0 ||
      x > n->cells_width || y > n->cells_height)
    return;

  for (i = 0; i < bytes; i ++)
    cell->str[i] = str[i];
  cell->str[bytes] = 0;

  if (!n->utf8) /* strip down to ascii if utf8 */
    {
      if (cell->str[0] & 0x80)
          cell->str[0] = '?';
      cell->str[1] = 0;
    }
  _nct_set_attr (n, x, y, n->mode, n->color);
}

const char *nct_get (Nchanterm *n, int x, int y)
{
  NcCell *cell = nct_get_cell (n, x, y);
  return cell->str;
}

void nct_flush (Nchanterm *n)
{
  int x, y;
  int w = n->cells_width;
  int h = n->cells_height;
  static int had_full = -1;
  int cx=-1, cy=-1;
  nct_cells_ensure (n);

  if (had_full <=0)
    {
      had_full = 40; // do a full refresh every now and then.
      nct_cells_clear_front (n);
    }
  else
    {
      had_full--;
    }

  for (y = 1; y <= h; y ++)
    for (x = 1; x <= w; x ++)
      {

        NcCell *back  = nct_get_cell (n, x, y);
        NcCell *front = nct_get_front_cell (n, x, y);

        /* draw cursor, if any */
        NcCell  cursor = {"!", 0, NCT_COLOR_WHITE};
        if (n->mouse_fd != -1)
          if (x == (int)n->mouse_x && y == (int)n->mouse_y)
          {
            strcpy (cursor.str, back->str);
            cursor.color = back->color / 8;
            cursor.color += back->color % 8;
            cursor.attr = back->attr;
            back = &cursor;
          }


        if (memcmp (back, front, sizeof (NcCell)))
          {
            n->color = back->color;
            nct_set_attr (n, back->attr);
            nct_ensure_mode (n);

            if (y != cy)
              printf (ANSI_YX, y, x);
            else if (x > cx)
              {
                if (x - cx == 1)
                  printf (ANSI_CURSOR_FORWARD1);
                else
                  printf (ANSI_CURSOR_FORWARD, (x - cx));
              }
            else 
              printf (ANSI_YX, y, x);

            printf ("%s", back->str);
            cx = x + 1;
            cy = y;
            memcpy (front, back, sizeof (NcCell));
          }
      }
  /* reset to defaults, to be in a better terminal state for any
   * potential ctrl+c or similar
   */
  n->color = NCHANT_DEFAULT_COLORS;
  nct_set_attr (n, NCT_A_NORMAL);
  nct_ensure_mode (n);
  printf (ANSI_YX, n->cursor_y, n->cursor_x);
  fflush (NULL);
}

void nct_reflush (Nchanterm *n)
{
  nct_cells_clear_front (n);
  nct_flush (n);
}

Nchanterm *nct_new  (void)
{
  Nchanterm *term = calloc (sizeof (Nchanterm), 1);
  const char *locale = setlocale (LC_ALL, "");
  const char *term_env  = getenv ("TERM");
  if (!term_env)
    term_env = "";
  if (locale)
    {
      if (strstr (locale, "utf8")  || strstr (locale, "UTF8")  ||
          strstr (locale, "UTF-8") || strstr (locale, "utf-8"))
        term->utf8 = 1;
    }
  else
    term->utf8 = 1; /* assume utf8 capable if we do not get a locale */

  if (strstr (term_env,  "Eterm"))
    term->utf8 = 0;

  /* some special casing to avoid sending unknown commands to st  */
  if (!strcmp (term_env, "st-256color") || !strcmp (term_env, "st"))
    term->is_st = 1;
  nct_set_size (term, nct_sys_terminal_width (), nct_sys_terminal_height ());

  if (strstr (term_env, "linux"))
    term->mouse_fd = open ("/dev/input/mice", O_RDWR | O_NONBLOCK);
  else
    term->mouse_fd = -1;

  if (term->mouse_fd != -1)
    {
      unsigned char reset = 0xff;
      write (term->mouse_fd, &reset, 1); /* send a reset */
    }

  printf (XTERM_ALTSCREEN_ON);
  return term;
}

void nct_destroy (Nchanterm *n)
{
  if (n->mouse_fd != -1)
    close (n->mouse_fd);
  free (n);
}

void nct_set_cursor_pos (Nchanterm *n,  int x, int  y)
{
  n->cursor_x = x;
  n->cursor_y = y;
}

void nct_get_cursor_pos (Nchanterm *n, int *x, int *y)
{
  if (x) *x = n->cursor_x;
  if (y) *y = n->cursor_y;
}
/*************************** input handling *************************/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define DELAY_MS  100  

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

static int  size_changed = 0;       /* XXX: global state */
static int  signal_installed = 0;   /* XXX: global state */

static const char *mouse_modes[]=
{TERMINAL_MOUSE_OFF,
 TERMINAL_MOUSE_ON_BASIC,
 TERMINAL_MOUSE_ON_DRAG,
 TERMINAL_MOUSE_ON_FULL,
 NULL};

/* note that a nick can have multiple occurences, the labels
 * should be kept the same for all occurences of a combination. */
typedef struct NcKeyCode {
  char *nick;          /* programmers name for key (combo) */
  char *label;         /* utf8 label for key */
  char  sequence[10];  /* terminal sequence */
} NcKeyCode;
static const NcKeyCode keycodes[]={  
  {"up",                  "↑",     "\033[A"},
  {"down",                "↓",     "\033[B"},
  {"right",               "→",     "\033[C"},
  {"left",                "←",     "\033[D"},

  {"shift-up",            "⇧↑",    "\033[1;2A"},
  {"shift-down",          "⇧↓",    "\033[1;2B"},
  {"shift-right",         "⇧→",    "\033[1;2C"},
  {"shift-left",          "⇧←",    "\033[1;2D"},

  {"alt-up",              "^↑",    "\033[1;3A"},
  {"alt-down",            "^↓",    "\033[1;3B"},
  {"alt-right",           "^→",    "\033[1;3C"},
  {"alt-left",            "^←",    "\033[1;3D"},

  {"alt-shift-up",        "alt-s↑", "\033[1;4A"},
  {"alt-shift-down",      "alt-s↓", "\033[1;4B"},
  {"alt-shift-right",     "alt-s→", "\033[1;4C"},
  {"alt-shift-left",      "alt-s←", "\033[1;4D"},

  {"control-up",          "^↑",    "\033[1;5A"},
  {"control-down",        "^↓",    "\033[1;5B"},
  {"control-right",       "^→",    "\033[1;5C"},
  {"control-left",        "^←",    "\033[1;5D"},

  /* putty */
  {"control-up",          "^↑",    "\033OA"},
  {"control-down",        "^↓",    "\033OB"},
  {"control-right",       "^→",    "\033OC"},
  {"control-left",        "^←",    "\033OD"},

  {"control-shift-up",    "^⇧↑",   "\033[1;6A"},
  {"control-shift-down",  "^⇧↓",   "\033[1;6B"},
  {"control-shift-right", "^⇧→",   "\033[1;6C"},
  {"control-shift-left",  "^⇧←",   "\033[1;6D"},

  {"control-up",          "^↑",    "\033Oa"},
  {"control-down",        "^↓",    "\033Ob"},
  {"control-right",       "^→",    "\033Oc"},
  {"control-left",        "^←",    "\033Od"},

  {"shift-up",            "⇧↑",    "\033[a"},
  {"shift-down",          "⇧↓",    "\033[b"},
  {"shift-right",         "⇧→",    "\033[c"},
  {"shift-left",          "⇧←",    "\033[d"},

  {"insert",              "ins",   "\033[2~"},
  {"delete",              "del",   "\033[3~"},
  {"page-up",             "PgUp",  "\033[5~"},
  {"page-down",           "PdDn",  "\033[6~"},
  {"home",                "Home",  "\033OH"},
  {"end",                 "End",   "\033OF"},
  {"home",                "Home",  "\033[H"},
  {"end",                 "End",   "\033[F"},
  {"control-delete",      "^del",  "\033[3;5~"},
  {"shift-delete",        "⇧del",  "\033[3;2~"},
  {"control-shift-delete","^⇧del", "\033[3;6~"},

  {"F1",        "F1",  "\033[11~"},
  {"F2",        "F2",  "\033[12~"},
  {"F3",        "F3",  "\033[13~"},
  {"F4",        "F4",  "\033[14~"},
  {"F1",        "F1",  "\033OP"},
  {"F2",        "F2",  "\033OQ"},
  {"F3",        "F3",  "\033OR"},
  {"F4",        "F4",  "\033OS"},
  {"F5",        "F5",  "\033[15~"},
  {"F6",        "F6",  "\033[16~"},
  {"F7",        "F7",  "\033[17~"},
  {"F8",        "F8",  "\033[18~"},
  {"F9",        "F9",  "\033[19~"},
  {"F9",        "F9",  "\033[20~"},
  {"F10",       "F10", "\033[21~"},
  {"F11",       "F11", "\033[22~"},
  {"F12",       "F12", "\033[23~"},
  {"tab",       "↹",  {9, '\0'}},
  {"shift-tab", "shift+↹",  "\033[Z"},
  {"backspace", "⌫",  {127, '\0'}},
  {"space",     "␣",   " "},
  {"esc",        "␛",  "\033"},
  {"return",    "⏎",  {10,0}},
  {"return",    "⏎",  {13,0}},
  /* this section could be autogenerated by code */
  {"control-a", "^A",  {1,0}},
  {"control-b", "^B",  {2,0}},
  {"control-c", "^C",  {3,0}},
  {"control-d", "^D",  {4,0}},
  {"control-e", "^E",  {5,0}},
  {"control-f", "^F",  {6,0}},
  {"control-g", "^G",  {7,0}},
  {"control-h", "^H",  {8,0}}, /* backspace? */
  {"control-i", "^I",  {9,0}},
  {"control-j", "^J",  {10,0}},
  {"control-k", "^K",  {11,0}},
  {"control-l", "^L",  {12,0}},
  {"control-n", "^N",  {14,0}},
  {"control-o", "^O",  {15,0}},
  {"control-p", "^P",  {16,0}},
  {"control-q", "^Q",  {17,0}},
  {"control-r", "^R",  {18,0}},
  {"control-s", "^S",  {19,0}},
  {"control-t", "^T",  {20,0}},
  {"control-u", "^U",  {21,0}},
  {"control-v", "^V",  {22,0}},
  {"control-w", "^W",  {23,0}},
  {"control-x", "^X",  {24,0}},
  {"control-y", "^Y",  {25,0}},
  {"control-z", "^Z",  {26,0}},
  {"alt-0",     "%0",  "\0330"},
  {"alt-1",     "%1",  "\0331"},
  {"alt-2",     "%2",  "\0332"},
  {"alt-3",     "%3",  "\0333"},
  {"alt-4",     "%4",  "\0334"},
  {"alt-5",     "%5",  "\0335"},
  {"alt-6",     "%6",  "\0336"},
  {"alt-7",     "%7",  "\0337"}, /* backspace? */
  {"alt-8",     "%8",  "\0338"},
  {"alt-9",     "%9",  "\0339"},
  {"alt-+",     "%+",  "\033+"},
  {"alt--",     "%-",  "\033-"},
  {"alt-/",     "%/",  "\033/"},
  {"alt-a",     "%A",  "\033a"},
  {"alt-b",     "%B",  "\033b"},
  {"alt-c",     "%C",  "\033c"},
  {"alt-d",     "%D",  "\033d"},
  {"alt-e",     "%E",  "\033e"},
  {"alt-f",     "%F",  "\033f"},
  {"alt-g",     "%G",  "\033g"},
  {"alt-h",     "%H",  "\033h"}, /* backspace? */
  {"alt-i",     "%I",  "\033i"},
  {"alt-j",     "%J",  "\033j"},
  {"alt-k",     "%K",  "\033k"},
  {"alt-l",     "%L",  "\033l"},
  {"alt-n",     "%N",  "\033m"},
  {"alt-n",     "%N",  "\033n"},
  {"alt-o",     "%O",  "\033o"},
  {"alt-p",     "%P",  "\033p"},
  {"alt-q",     "%Q",  "\033q"},
  {"alt-r",     "%R",  "\033r"},
  {"alt-s",     "%S",  "\033s"},
  {"alt-t",     "%T",  "\033t"},
  {"alt-u",     "%U",  "\033u"},
  {"alt-v",     "%V",  "\033v"},
  {"alt-w",     "%W",  "\033w"},
  {"alt-x",     "%X",  "\033x"},
  {"alt-y",     "%Y",  "\033y"},
  {"alt-z",     "%Z",  "\033z"},
  /* Linux Console  */
  {"home",      "Home", "\033[1~"},
  {"end",       "End",  "\033[4~"},
  {"F1",        "F1",   "\033[[A"},
  {"F2",        "F2",   "\033[[B"},
  {"F3",        "F3",   "\033[[C"},
  {"F4",        "F4",   "\033[[D"},
  {"F5",        "F5",   "\033[[E"},
  {"F6",        "F6",   "\033[[F"},
  {"F7",        "F7",   "\033[[G"},
  {"F8",        "F8",   "\033[[H"},
  {"F9",        "F9",   "\033[[I"},
  {"F10",       "F10",  "\033[[J"},
  {"F11",       "F11",  "\033[[K"},
  {"F12",       "F12",  "\033[[L"}, 
  {NULL, }
};

static struct termios orig_attr; /* in order to restore at exit */
static int    nc_is_raw = 0;
static int    atexit_registered = 0;
static int    mouse_mode = NC_MOUSE_NONE;

static void _nc_noraw (void)
{
  if (nc_is_raw && tcsetattr (STDIN_FILENO, TCSAFLUSH, &orig_attr) != -1)
    nc_is_raw = 0;
}

static void
nc_at_exit (void)
{
  printf (TERMINAL_MOUSE_OFF);
  _nc_noraw();
  if (mouse_mode)
    printf (TERMINAL_MOUSE_OFF);
  printf (XTERM_ALTSCREEN_OFF);
}

static int _nc_raw (void)
{
  struct termios raw;
  if (!isatty (STDIN_FILENO))
    return -1;
  if (!atexit_registered)
    {
      atexit (nc_at_exit);
      atexit_registered = 1;
    }
  if (tcgetattr (STDIN_FILENO, &orig_attr) == -1)
    return -1;
  raw = orig_attr;  /* modify the original mode */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */
  if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &raw) < 0)
    return -1;
  nc_is_raw = 1;
  tcdrain(STDIN_FILENO);
  tcflush(STDIN_FILENO, 1);
  return 0;
}

static int match_keycode (const char *buf, int length, const NcKeyCode **ret)
{
  int i;
  int matches = 0;

  if (!strncmp (buf, "\033[M", MIN(length,3)))
    {
      if (length >= 6)
        return 9001;
      return 2342;
    }
  for (i = 0; keycodes[i].nick; i++)
    if (!strncmp (buf, keycodes[i].sequence, length))
      {
        matches ++;
        if ((int)strlen (keycodes[i].sequence) == length && ret)
          {
            *ret = &keycodes[i];
            return 1;
          }
      }
  if (matches != 1 && ret)
    *ret = NULL;
  return matches==1?2:matches;
}

static void nc_resize_term (int UNUSED dummy)
{
  size_changed = 1;
}

int nct_has_event (Nchanterm UNUSED *n, int delay_ms)
{
  struct timeval tv;
  int retval;
  fd_set rfds;

  if (size_changed)
    return 1;
  FD_ZERO (&rfds);
  FD_SET (STDIN_FILENO, &rfds);
  tv.tv_sec = 0; tv.tv_usec = delay_ms * 1000; 
  retval = select (1, &rfds, NULL, NULL, &tv);
  if (size_changed)
    return 1;
  return retval == 1 && retval != -1;
}


static const char *mouse_get_event_int (Nchanterm *n, int *x, int *y)
{
  static int prev_state = 0;
  const char *ret = "mouse-motion";
  float relx, rely;
  signed char buf[3];
  read (n->mouse_fd, buf, 3);
  relx = buf[1];
  rely = -buf[2];

  n->mouse_x += relx * 0.1;
  n->mouse_y += rely * 0.1;

  if (n->mouse_x < 1) n->mouse_x = 1;
  if (n->mouse_y < 1) n->mouse_y = 1;
  if (n->mouse_x >= n->width)  n->mouse_x = n->width;
  if (n->mouse_y >= n->height) n->mouse_y = n->height;

  if (x) *x = n->mouse_x;
  if (y) *y = n->mouse_y;

  if ((prev_state & 1) != (buf[0] & 1))
    {
      if (buf[0] & 1) ret = "mouse-press";
    }
  else if (buf[0] & 1)
    ret = "mouse-drag";

  if ((prev_state & 2) != (buf[0] & 2))
    {
      if (buf[0] & 2) ret = "mouse2-press";
    }
  else if (buf[0] & 2)
    ret = "mouse2-drag";

  if ((prev_state & 4) != (buf[0] & 4))
    {
      if (buf[0] & 4) ret = "mouse1-press";
    }
  else if (buf[0] & 4)
    ret = "mouse1-drag";

  prev_state = buf[0];
  return ret;
}

static const char *mev_type = NULL;
static int         mev_x = 0;
static int         mev_y = 0;
static int         mev_q = 0;

static const char *mouse_get_event (Nchanterm UNUSED *n, int *x, int *y)
{
  if (!mev_q)
    return NULL;
  *x = mev_x;
  *y = mev_y;
  mev_q = 0;
  return mev_type;
}

static int mouse_has_event (Nchanterm *n)
{
  struct timeval tv;
  int retval;

  if (mouse_mode == NC_MOUSE_NONE)
    return 0;

  if (mev_q)
    return 1;

  if (n->mouse_fd == -1)
    return 0;

  {
    fd_set rfds;
    FD_ZERO (&rfds);
    FD_SET(n->mouse_fd, &rfds);
    tv.tv_sec = 0; tv.tv_usec = 0;
    retval = select (n->mouse_fd+1, &rfds, NULL, NULL, &tv);
  }

  if (retval != 0)
    {
      int nx = 0, ny = 0;
      const char *type = mouse_get_event_int (n, &nx, &ny);

      if ((mouse_mode < NC_MOUSE_DRAG && mev_type && !strcmp (mev_type, "drag")) ||
          (mouse_mode < NC_MOUSE_ALL && mev_type && !strcmp (mev_type, "motion")))
        {
          mev_q = 0;
          return mouse_has_event (n);
        }

      if ((mev_type && !strcmp (type, mev_type) && !strcmp (type, "mouse-motion")) ||
         (mev_type && !strcmp (type, mev_type) && !strcmp (type, "mouse1-drag")) ||
         (mev_type && !strcmp (type, mev_type) && !strcmp (type, "mouse2-drag")))
        {
          if (nx == mev_x && ny == mev_y)
          {
            mev_q = 0;
            return mouse_has_event (n);
          }
        }
      mev_x = nx;
      mev_y = ny;
      mev_type = type;
      mev_q = 1;
    }
  return retval != 0;
}

const char *nct_get_event (Nchanterm *n, int timeoutms, int *x, int *y)
{
  unsigned char buf[20];
  int length;


  if (x) *x = -1;
  if (y) *y = -1;

  if (!signal_installed)
    {
      _nc_raw ();
      signal_installed = 1;
      signal (SIGWINCH, nc_resize_term);
    }
  if (mouse_mode)
    printf(mouse_modes[mouse_mode]);

  {
    int elapsed = 0;
    int got_event = 0;

    do {
      if (size_changed)
        {
          size_changed = 0;
          return "size-changed";
        }
      got_event = mouse_has_event (n);
      if (!got_event)
        got_event = nct_has_event (n, MIN(DELAY_MS, timeoutms-elapsed));
      if (size_changed)
        {
          size_changed = 0;
          return "size-changed";
        }
      /* only do this if the client has asked for idle events,
       * and perhaps programmed the ms timer?
       */
      elapsed += MIN(DELAY_MS, timeoutms-elapsed);
      if (!got_event && timeoutms && elapsed >= timeoutms)
        return "idle";
    } while (!got_event);
  }

  if (mouse_has_event (n))
    return mouse_get_event (n, x, y);

  for (length = 0; length < 10; length ++)
    if (read (STDIN_FILENO, &buf[length], 1) != -1)
      {
        const NcKeyCode *match = NULL;

        /* special case ESC, so that we can use it alone in keybindings */
        if (length == 0 && buf[0] == 27)
          {
            struct timeval tv;
            fd_set rfds;
            FD_ZERO (&rfds);
            FD_SET (STDIN_FILENO, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 1000 * DELAY_MS;
            if (select (1, &rfds, NULL, NULL, &tv) == 0)
              return "esc";
          }

        switch (match_keycode ((void*)buf, length + 1, &match))
          {
            case 1: /* unique match */
              if (!match)
                return NULL;
              return match->nick;
              break;
            case 9001: /* mouse event */
              if (x) *x = ((unsigned char)buf[4]-32)*1.0;
              if (y) *y = ((unsigned char)buf[5]-32)*1.0;
              switch (buf[3])
                {
                  case 32: return "mouse-press";
                  case 33: return "mouse1-press";
                  case 34: return "mouse2-press";
                  case 40: return "alt-mouse-press";
                  case 41: return "alt-mouse1-press";
                  case 42: return "alt-mouse2-press";
                  case 48: return "control-mouse-press";
                  case 49: return "control-mouse1-press";
                  case 50: return "control-mouse2-press";
                  case 56: return "alt-control-mouse-press";
                  case 57: return "alt-control-mouse1-press";
                  case 58: return "alt-control-mouse2-press";
                  case 64: return "mouse-drag";
                  case 65: return "mouse1-drag";
                  case 66: return "mouse2-drag";
                  case 71: return "mouse-motion"; /* shift+motion */
                  case 72: return "alt-mouse-drag";
                  case 73: return "alt-mouse1-drag";
                  case 74: return "alt-mouse2-drag";
                  case 75: return "mouse-motion"; /* alt+motion */
                  case 80: return "control-mouse-drag";
                  case 81: return "control-mouse1-drag";
                  case 82: return "control-mouse2-drag";
                  case 83: return "mouse-motion"; /* ctrl+motion */
                  case 91: return "mouse-motion"; /* ctrl+alt+motion */
                  case 95: return "mouse-motion"; /* ctrl+alt+shift+motion */
                  case 96: return "scroll-up";
                  case 97: return "scroll-down";
                  case 100: return "shift-scroll-up";
                  case 101: return "shift-scroll-down";
                  case 104: return "alt-scroll-up";
                  case 105: return "alt-scroll-down";
                  case 112: return "control-scroll-up";
                  case 113: return "control-scroll-down";
                  case 116: return "control-shift-scroll-up";
                  case 117: return "control-shift-scroll-down";
                  case 35: /* (or release) */
                  case 51: /* (or ctrl-release) */
                  case 43: /* (or alt-release) */
                  case 67: return "mouse-motion";
                           /* have a separate mouse-drag ? */
                  default: {
                             static char rbuf[100];
                             sprintf (rbuf, "mouse (unhandled state: %i)", buf[3]);
                             return rbuf;
                           }
                }
            case 0: /* no matches, bail*/
              { 
                static char ret[256];
                if (length == 0 && nct_utf8_len (buf[0])>1) /* single unicode
                                                               char */
                  {
                    read (STDIN_FILENO, &buf[length+1], nct_utf8_len(buf[0])-1);
                    buf[nct_utf8_len(buf[0])]=0;
                    strcpy (ret, (void*)buf);
                    return ret;
                  }
                if (length == 0) /* ascii */
                  {
                    buf[1]=0;
                    strcpy (ret, (void*)buf);
                    return ret;
                  }
                sprintf (ret, "unhandled %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c' %i:'%c'",
                  length>=0? buf[0]: 0, length>=0? buf[0]>31?buf[0]:'?': ' ', 
                  length>=1? buf[1]: 0, length>=1? buf[1]>31?buf[1]:'?': ' ', 
                  length>=2? buf[2]: 0, length>=2? buf[2]>31?buf[2]:'?': ' ', 
                  length>=3? buf[3]: 0, length>=3? buf[3]>31?buf[3]:'?': ' ',
                  length>=4? buf[4]: 0, length>=4? buf[4]>31?buf[4]:'?': ' ',
                  length>=5? buf[5]: 0, length>=5? buf[5]>31?buf[5]:'?': ' ',
                  length>=6? buf[6]: 0, length>=6? buf[6]>31?buf[6]:'?': ' ');
                return ret;
              }
              return NULL;
            default: /* continue */
              break;
          }
      }
    else
      return "key read eek";
  return "fail";
}

const char *nct_key_get_label (Nchanterm UNUSED *n, const char *nick)
{
  int j;
  int found = -1;
  for (j = 0; keycodes[j].nick; j++)
    if (found == -1 && !strcmp (keycodes[j].nick, nick))
      return keycodes[j].label;
  return NULL;
}

void nct_mouse (Nchanterm *term, int mode)
{
  if (term->is_st && mode > 1)
    mode = 1;
  if (mode != mouse_mode)
  {
    printf (mouse_modes[mode]);
    fflush (stdout);
  }
  mouse_mode = mode;
}
#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-backend-terminal.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-config.h"
#if MRG_NCT

// bundled include of mrg-internal.h"
#define NCHANTERM_HEADER_ONLY
// bundled include of nchanterm.c"

#include <math.h> // remove after removing sqrt

static int mrg_black_on_white;

typedef struct MrgNct {
  Nchanterm     *term;
  unsigned char *nct_pixels;
} MrgNct;

static char *qblocks[]={
  " ",//0
  "▘",//1
  "▝",//2
  "▀",//3
  "▖",//4
  "▌",//5
  "▞",//6
  "▛",//7
  "▗",//8
  "▚",//9
  "▐",//10
  "▜",//11
  "▄",//12
  "▙",//13
  "▟",//14
  "█",//15
  NULL};

static char *utf8_gray_scale[]={" ","░","▒","▓","█","█", NULL};
#if 0
static char *horblocks[] = {" ","▏","▎","▍","▌","▋","▊","▉","█", NULL};
static char *verblocks[] = {" ","▁","▂","▃","▄","▅","▆","▇","█", NULL};

/* sorted so that drawing is bit operations */
static char *lscale[]={
//   " ","╵","╷","│","╶","╰","╭","├","╴","╯","╮","┤","─","┴","┬","┼",NULL};
     " ","╵","╷","│","╶","└","┌","├","╴","┘","┐","┤","─","┴","┬","┼",NULL};
#endif

static void set_gray (Nchanterm *n, int x, int y, float value)
{
  int i = value * 5.0;
  if (i > 5)
    i = 5;
  if (i < 0)
    i = 0;
  nct_set (n, x, y, utf8_gray_scale[i]);
}

/* draws with 4 input subpixels per output subpixel, the routine is
 * written to deal with an input that has 2x2 input pixels per
 * output pixel.
 */
static inline void draw_rgb_cell (Nchanterm *n, int x, int y,
                                  float b[4], float g[4], float r[4])
{
  float sum[3] = {0,0,0};
  int i;
  int bestbits = 0;
  for (i=0; i<4; i++)
    {
      sum[0] += r[i]/4;
      sum[1] += g[i]/4;
      sum[2] += b[i]/4;
    }
  {
    /* go through all fg/bg combinations with all color mixtures and
     * compare with our desired average color output
     */
    int fg, bg;
    float mix;

    int best_fg = 7;
    int best_bg = 0;
    float best_mix = 1.0;
    float best_distance = 1.0;
    int use_geom = 0;

#define POW2(a) ((a)*(a))

    for (fg = 0; fg < 8; fg ++)
      for (bg = 0; bg < 8; bg ++)
        for (mix = 0.0; mix <= 1.0; mix+=0.25)
          {
            int frgb[3] = { (fg & 1)!=0,
                            (fg & 2)!=0,
                            (fg & 4)!=0};
            int brgb[3] = { (bg & 1)!=0,
                            (bg & 2)!=0,
                            (bg & 4)!=0};
            float resrgb[3];
            float distance;
            int c;
            for (c = 0; c < 3; c++)
              resrgb[c] = (frgb[c] * mix + brgb[c] * (1.0-mix)) ;

            distance = sqrtf(POW2(resrgb[0] - sum[0])+
                       POW2(resrgb[1] - sum[1])+
                       POW2(resrgb[2] - sum[2]));
            if (distance < best_distance)
              {
                best_distance = distance;
                best_fg = fg;
                best_bg = bg;
                best_mix = mix;
              }
          }
#if 0
    if (best_mix <= 0.0) /* prefer to draw blocks than to not do so */
      {
        int tmp = best_fg;
        best_fg = best_bg;
        best_bg = tmp;
        best_mix = 1.0-best_mix;
      }
#endif

#if 1
  if (best_bg == 7 && best_fg == 7)
  {
    best_fg = 7;
    best_bg = 0;
    best_mix = 1.0;
  }
  if (best_fg == 0 && best_mix >=1.0 )
  {
    best_bg = 0;
    best_fg = 7;
    best_mix = 0.0;
  }
#endif

  {
    int totbits;
    for (totbits = 0; totbits < 15; totbits++)
        {
          float br[4],bg[4],bb[4];
          int i;
          float distance = 0;

          for (i=0;i<4;i++)
            {
              br[i] = ((totbits >> (i))&1) ? ((best_fg & 1) != 0) :
                                             ((best_bg & 1) != 0);
              bg[i] = ((totbits >> (i))&1) ? ((best_fg & 2) != 0) :
                                             ((best_bg & 2) != 0);
              bb[i] = ((totbits >> (i))&1) ? ((best_fg & 4) != 0) :
                                             ((best_bg & 4) != 0);
            }

          for (i=0;i<4;i++)
            distance += sqrt (POW2(br[i] - r[i])+
                              POW2(bg[i] - g[i])+
                              POW2(bb[i] - b[i]));
#define GEOM_FACTOR  0.10
          if (distance * GEOM_FACTOR < best_distance)
            {
              best_distance = distance/4 * GEOM_FACTOR;
              use_geom = 1;
              bestbits = totbits;
            }
        }
  }

  /* XXX:
   * do another pass checking 1/4th filled permutations,
   * these should give better large curves.
   */


  if (mrg_black_on_white)
  {
    if (best_fg == 7) best_fg = 0;
    else if (best_fg == 0) best_fg = 7;
    if (best_bg == 7) best_bg = 0;
    else if (best_bg == 0) best_bg = 7;
  }

  nct_fg_color (n, best_fg);
  nct_bg_color (n, best_bg);

  if (use_geom)
    nct_set (n, x, y, qblocks[bestbits]);
  else
    set_gray (n, x, y, best_mix);
  }
}

/*  draw a 32bit RGBA file,..
 */
static void nct_buf (Nchanterm *n, int x0, int y0, int w, int h,
                     unsigned char *buf, int rw, int rh)
{
  int u, v;

  if (!buf)
    return;

  for (u = 0; u < w; u++)
    for (v = 0; v < h; v++)
      {
        float r[4], g[4], b[4];
        float xo, yo;
        int no = 0;
        for (yo = 0.0; yo <= 0.5; yo += 0.5)
          for (xo = 0.0; xo <= 0.5; xo += 0.5, no++)
            {
              int c = 0;

              /* do a set of samplings to get a crude higher
               * quality box down-sampler?, do this crunching
               * based on a scaling factor.
               */

              float uo = 0.0, vo = 0.0;
              r[no]=g[no]=b[no]=0.0;
              //for (uo = 0.0 ; uo <= 0.5; uo+= 0.1)
              //for (vo = 0.0 ; vo <= 0.5; vo+= 0.1)
              //
              // using nearest neighbour is best for non photos..
              // we do not want the added AA for crisp edges
                  {
                    int x, y;
                    x = ((u+xo+uo) * 1.0 / w) * rw;
                    y = ((v+yo+vo) * 1.0 / h) * rh;
                    if (x<0) x = 0;
                    if (y<0) y = 0;
                    if (x>=rw) x = rw-1;
                    if (y>=rh) y = rh-1;
                      
                    r[no] += buf [(y * rw + x) * 4 + 0] / 255.0;
                    g[no] += buf [(y * rw + x) * 4 + 1] / 255.0;
                    b[no] += buf [(y * rw + x) * 4 + 2] / 255.0;
                    c++;
                  }
              r[no] /= c;
              g[no] /= c;
              b[no] /= c;
            }
        draw_rgb_cell (n, x0+u, y0+v, r, g, b);
      }
}


static unsigned char *mrg_nct_get_pixels (Mrg *mrg, int *rowstride)
{
  MrgNct *backend = mrg->backend;

  if (rowstride)
    *rowstride = mrg->width * 4;
  return backend->nct_pixels;
}

static void mrg_nct_flush (Mrg *mrg)
{
  MrgNct *backend = mrg->backend;

  nct_clear (backend->term);
  nct_buf (backend->term, 1, 1, nct_width(backend->term), nct_height (backend->term),
           backend->nct_pixels, mrg->width, mrg->height);

  {
    int x, y;
    int w = nct_width(backend->term);
    int h = nct_height(backend->term);
    int offset = 0;
    for (y = 0; y < h; y ++)
      for (x = 0; x < w; x ++)
      {
        if (mrg->glyphs[offset] != 0)
        {
          int style = mrg->styles[offset/4];
          int fg = style & 7;
          int bg = (style/8) & 7;
          int attr = style / 64;

          if (mrg_black_on_white)
          {
            if (fg == 7) fg = 0;
            else if (fg == 0) fg = 7;
            if (bg == 7) bg = 0;
            else if (bg == 0) bg = 7;
          }

          {
            if (bg == 7 && !(attr & MRG_REVERSE)) {
              nct_fg_color (backend->term, bg);
              nct_bg_color (backend->term, fg);
              attr |= MRG_REVERSE;
            }
            else
            {
              nct_fg_color (backend->term, fg);
              nct_bg_color (backend->term, bg);
            }
          }

          nct_set_attr (backend->term, attr);
          nct_set (backend->term, x+1, y, (char *)&mrg->glyphs[offset]);
        }
        offset += 4;
      }
  }

  nct_flush (backend->term);

#if MRG_CAIRO
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
#endif
}

#include <stdio.h>
#include <math.h>

static int was_down = 0;

static int mrg_nct_consume_events (Mrg *mrg)
{
  MrgNct *backend = mrg->backend;
  int ix, iy;
  const char *event = NULL;
    {
      float x, y;
      event = nct_get_event (backend->term, 50, &ix, &iy);

      x = floor((ix * 1.0) / nct_width (backend->term) * mrg->width);
      y = floor((iy * 1.0) / nct_height (backend->term) * mrg->height) - 1;

      if (!strcmp (event, "mouse-press"))
      {
        mrg_pointer_press (mrg, x, y, 0);
        was_down = 1;
      } else if (!strcmp (event, "mouse-release"))
      {
        mrg_pointer_release (mrg, x, y, 0);
      } else if (!strcmp (event, "mouse-motion"))
      {
        nct_set_cursor_pos (backend->term, ix, iy);
        nct_flush (backend->term);
        if (was_down)
        {
          mrg_pointer_release (mrg, x, y, 0);
          was_down = 0;
        }
        mrg_pointer_motion (mrg, x, y, 0);
      } else if (!strcmp (event, "mouse-drag"))
      {
        mrg_pointer_motion (mrg, x, y, 0);
      } else if (!strcmp (event, "size-changed"))
      {
        int width = nct_sys_terminal_width ();
        int height = nct_sys_terminal_height ();
        nct_set_size (backend->term, width, height);
        width *= CPX;
        height *= CPX;
        free (mrg->glyphs);
        free (mrg->styles);
        free (backend->nct_pixels);
        backend->nct_pixels = calloc (width * height * 4, 1);
        mrg->glyphs = calloc ((width/CPX) * (height/CPX) * 4, 1);
        mrg->styles = calloc ((width/CPX) * (height/CPX) * 1, 1);
        mrg_set_size (mrg, width, height);
        mrg_queue_draw (mrg, NULL);
      }
      else
      {
        if (!strcmp (event, "esc"))
          mrg_key_press (mrg, 0, "escape");
        else if (!strcmp (event, "space"))
          mrg_key_press (mrg, 0, " ");
        else if (!strcmp (event, "enter"))
          mrg_key_press (mrg, 0, "\n");
        else if (!strcmp (event, "return"))
          mrg_key_press (mrg, 0, "\n");
        else
        mrg_key_press (mrg, 0, event);
      }
    }
    
    if (nct_has_event (backend->term, 25))
      return mrg_nct_consume_events (mrg);
  return 1;
}

static void mrg_nct_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  while (!_mrg_has_quit (mrg))
  {
    if (_mrg_is_dirty (mrg))
      mrg_ui_update (mrg);
    if (!mrg_nct_consume_events (mrg))
      usleep (100);
  }
}

static void mrg_nct_prepare (Mrg *mrg)
{
}

static void mrg_nct_clear (Mrg *mrg)
{
  memset (mrg->glyphs, 0, (mrg->width/CPX) * (mrg->height/CPX) * 4);
  memset (mrg->styles, 0, (mrg->width/CPX) * (mrg->height/CPX));
}

static void mrg_nct_warp_pointer (Mrg *mrg, float x, float y)
{
  MrgNct *backend = mrg->backend;

  mrg->pointer_x = x;
  mrg->pointer_y = y;
  nct_set_cursor_pos (backend->term, x/CPX, y/CPX);
  nct_flush (backend->term);
}

static void mrg_nct_destroy (Mrg *mrg)
{
  free (mrg->backend);
}

Mrg *_mrg_terminal_new (int width, int height)
{
  Mrg *mrg;
  MrgNct *backend = calloc (sizeof (MrgNct), 1);

  backend->term = nct_new ();
  nct_clear (backend->term);
  nct_flush (backend->term);
  nct_mouse (backend->term, NC_MOUSE_DRAG);
        
  width = nct_sys_terminal_width () * CPX;
  height = nct_sys_terminal_height () * CPX;

  setvbuf(stdin, NULL, _IONBF, 0); 

  mrg = calloc (sizeof (Mrg), 1);
#if MRG_CAIRO
  backend->nct_pixels = calloc (width * height * 4, 1);
#endif
  mrg->glyphs = calloc ((width/CPX) * (height/CPX) * 4, 1);
  mrg->styles = calloc ((width/CPX) * (height/CPX) * 1, 1);

  mrg->mrg_main = mrg_nct_main;
  mrg->mrg_flush = mrg_nct_flush;
  mrg->mrg_prepare = mrg_nct_prepare;
  mrg->mrg_clear = mrg_nct_clear;
  mrg->mrg_warp_pointer = mrg_nct_warp_pointer;
  mrg->mrg_get_pixels = mrg_nct_get_pixels;
  mrg->mrg_destroy = mrg_nct_destroy;
  mrg->backend = backend;

  _mrg_init (mrg, width, height);
  mrg->ddpx = 0.25;

  mrg->fullscreen = 1;

  if (getenv ("MRG_BLACK_ON_WHITE"))
    mrg_black_on_white = 1;

  return mrg;
}
#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-binding.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

void mrg_add_binding (Mrg *mrg,
                      const char *key,
                      const char *action,
                      const char *label,
                      MrgCb cb,
                      void  *cb_data)
{
  int i;
  for (i = 0; mrg->bindings[i].nick; i++)
    if (!strcmp (mrg->bindings[i].nick, key))
    {
      /* we just add them, with later ones having priority.. */
    }
  for (i = 0; mrg->bindings[i].nick; i ++);

  mrg->bindings[i].nick = strdup (key);
  strcpy (mrg->bindings[i].nick, key);

  if (action)
    mrg->bindings[i].command = action ? strdup (action) : NULL;
  if (label)
    mrg->bindings[i].label = label ? strdup (label) : NULL;
  mrg->bindings[i].cb = cb;
  mrg->bindings[i].cb_data = cb_data;
}

int _mrg_bindings_key_down (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  int i;
  int handled = 0;
  int max;
  for (i = 0; mrg->bindings[i].nick; i++);
  max = i-1;

  for (i = max; i>=0; i--)
    if (!strcmp (mrg->bindings[i].nick, event->key_name))
    {
      if (mrg->bindings[i].cb)
      {
        if (mrg->bindings[i].cb (event, mrg->bindings[i].cb_data, NULL))
          return 1;
        handled = 1;
      }
    }
  if (!handled)
  for (i = max; i>=0; i--)
    if (!strcmp (mrg->bindings[i].nick, "unhandled"))
    {
      if (mrg->bindings[i].cb)
      {
        if (mrg->bindings[i].cb (event, mrg->bindings[i].cb_data, NULL))
          return 1;
      }
    }
  return 0;
}

void _mrg_clear_bindings (Mrg *mrg)
{
  int i;
  for (i = 0; mrg->bindings[i].nick; i ++)
  {
    free (mrg->bindings[i].nick);
    if (mrg->bindings[i].command)
      free (mrg->bindings[i].command);
    if (mrg->bindings[i].label)
      free (mrg->bindings[i].label);
  }
  memset (&mrg->bindings, 0, sizeof (mrg->bindings));
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-config.h"
// bundled include of mrg-internal.h"

void mrg_quit (Mrg *mrg)
{
  mrg->quit = 1;
  mrg_queue_draw (mrg, NULL); /* to force some backends into action */
}

void _mrg_init (Mrg *mrg, int width, int height)
{
  mrg->state = &mrg->states[0];
  /* XXX: is there a better place to set the default text color to black? */
  mrg->state->style.color.red = 
  mrg->state->style.color.green = 
  mrg->state->style.color.blue = 0;
  mrg->state->style.color.alpha = 1;
  mrg->ddpx = 1;
  if (getenv ("MRG_DDPX"))
  {
    mrg->ddpx = strtod (getenv ("MRG_DDPX"), NULL);
  }
  mrg_set_size (mrg, width, height);
  _mrg_text_init (mrg);

  mrg->html.state = &mrg->html.states[0];
  mrg->html.mrg = mrg;
}

#if MRG_UFB
Mrg *_mrg_ufb_new (int width, int height);
#endif
#if MRG_SDL
Mrg *_mrg_sdl_new (int width, int height);
#endif
#if MRG_NCT
Mrg *_mrg_terminal_new (int width, int height);
#endif
#if MRG_MEM
Mrg *_mrg_mem_new (int width, int height);
#endif
#if MRG_GTK
Mrg *_mrg_gtk_new (int width, int height);
#endif

static Mrg *mrg_new2 (int width, int height, const char *backend)
{
  Mrg *mrg = NULL;
  if (0);
#if MRG_UFB
  else if (!strcmp (backend, "UFB") ||
           !strcmp (backend, "ufb"))
  {
    mrg = _mrg_ufb_new (width, height);
  }
#endif
#if MRG_GTK
  else if (!strcmp (backend, "GTK") ||
           !strcmp (backend, "gtk"))
  {
    mrg = _mrg_gtk_new (width, height);
  }
#endif
#if MRG_SDL
  else if (!strcmp (backend, "SDL") ||
           !strcmp (backend, "sdl"))
  {
    mrg = _mrg_sdl_new (width, height);
  }
#endif
#if MRG_NCT
  else if (!strcmp (backend, "terminal") ||
           !strcmp (backend, "nchanterm") ||
           !strcmp (backend, "nct"))
  {
    mrg = _mrg_terminal_new (width, height);
  }
#endif
#if MRG_MEM
  else if (!strcmp (backend, "mem"))
  {
    mrg = _mrg_mem_new (width, height);
  }
#endif
  else
  {
    fprintf (stderr, "Unrecognized microraptor backend: %s\n", backend);
    fprintf (stderr, " recognized backends:");
#if MRG_GTK
    fprintf (stderr, " gtk");
#endif
#if MRG_SDL
    fprintf (stderr, " sdl");
#endif
#if MRG_NCT
    fprintf (stderr, " terminal");
#endif
    fprintf (stderr, "\n");
    exit (-1);
  }

  if (!mrg)
  {
    fprintf (stderr, "failed to init mrg\n");
  }
  return mrg;
}

void mrg_style_defaults (Mrg *mrg);

Mrg *mrg_new (int width, int height, const char *backend)
{
  Mrg *mrg = NULL;
  if (!backend)
  {
    backend = getenv ("MRG_BACKEND");
  }

  if (backend)
    mrg = mrg_new2 (width, height, backend);
  else
  {
#if MRG_UFB
    if (getenv ("UFB_PATH"))
    {
      if (!mrg) mrg = _mrg_ufb_new (width, height);
    }
#endif
#if MRG_SDL
    if (!mrg) mrg = _mrg_sdl_new (width, height);
#endif
#if MRG_GTK
    if (!mrg) mrg = _mrg_gtk_new (width, height);
#endif
#if MRG_NCT
    if (!mrg) mrg = _mrg_terminal_new (width, height);
#endif
    if (!mrg)
    {
      fprintf (stderr, "Unable to initialize any mrg backend\n");
      exit (-1);
    }
  }

  mrg_style_defaults (mrg);
  return mrg;
}

void mrg_destroy (Mrg *mrg)
{
  if (mrg->mrg_destroy)
    mrg->mrg_destroy (mrg);
  free (mrg);
}

int  mrg_width (Mrg *mrg)
{
  return mrg->width / mrg->ddpx;
}

int  mrg_height (Mrg *mrg)
{
  return mrg->height / mrg->ddpx;
}

void mrg_set_size (Mrg *mrg, int width, int height)
{
  mrg->width = width;
  mrg->height = height;
  mrg_resized (mrg, width, height);
  mrg_queue_draw (mrg, NULL);
}

int  _mrg_has_quit (Mrg *mrg)
{
  return mrg->quit;
}

static void 
_mrg_rectangle_combine_bounds (MrgRectangle       *rect_dest,
                               const MrgRectangle *rect_other)
{
  if (rect_dest->x == 0 &&
      rect_dest->y == 0 &&
      rect_dest->width == 0 &&
      rect_dest->height == 0)
  {
    *rect_dest = *rect_other;
    return;
  }

  if (rect_other->x < rect_dest->x)
  {
    rect_dest->width += (rect_dest->x - rect_other->x);
    rect_dest->x = rect_other->x;
  }
  if (rect_other->y < rect_dest->y)
  {
    rect_dest->height += (rect_dest->y - rect_other->y);
    rect_dest->y = rect_other->y;
  }
  if (rect_other->x + rect_other->width > rect_dest->x + rect_dest->width)
  {
    rect_dest->width = (rect_other->x + rect_other->width) - rect_dest->x;
  }
  if (rect_other->y + rect_other->height > rect_dest->y + rect_dest->height)
  {
    rect_dest->height = (rect_other->y + rect_other->height) - rect_dest->y;
  }
}

void mrg_queue_draw (Mrg *mrg, MrgRectangle *rectangle)
{
  MrgRectangle rect_copy = {0, };
  if (!rectangle)
  {
    rect_copy.x = 0;
    rect_copy.y = 0;
    rect_copy.width = mrg->width;
    rect_copy.height = mrg->height;
    rectangle = &rect_copy;
  }
  else
  {
    rect_copy = *rectangle;
  }

#if 0
  rect_copy.x *= mrg->ddpx;
  rect_copy.y *= mrg->ddpx;
  rect_copy.width *= mrg->ddpx;
  rect_copy.height *= mrg->ddpx;
#endif

  _mrg_rectangle_combine_bounds (&mrg->dirty, &rect_copy);

  if (mrg->mrg_queue_draw)
    mrg->mrg_queue_draw (mrg, &rect_copy);
}

int _mrg_is_dirty (Mrg *mrg)
{
  return mrg->dirty.width != 0;
}

void _mrg_set_clean  (Mrg *mrg)
{
  mrg->dirty.x = 0;
  mrg->dirty.y = 0;
  mrg->dirty.width = 0;
  mrg->dirty.height = 0;
}

void  mrg_set_ui           (Mrg *mrg, void (*ui)(Mrg *mrg, void *ui_data),
                                      void *ui_data)
{
  mrg->ui_update = ui;
  mrg->user_data = ui_data;
}

extern char *__progname; /*XXX: ugly, but portable hack - that works with many libc's */

void mrg_main (Mrg *mrg)
{
  if (!mrg->title)
  {
    mrg->title = __progname;
  }
  mrg_set_title (mrg, mrg->title);
  mrg->mrg_main (mrg, mrg->ui_update, mrg->user_data);
}

cairo_t *mrg_cr (Mrg *mrg)
{
  if (mrg->mrg_cr)
    return mrg->mrg_cr (mrg);
  else
  {
#if MRG_CAIRO
  unsigned char *pixels = NULL;
  cairo_t *cr;
  cairo_surface_t *surface;
  int rowstride = 0;
  int width, height;

  if (mrg->cr)
    return mrg->cr;

  width = mrg->width;
  height = mrg->height;

  pixels = mrg_get_pixels (mrg, &rowstride);
  assert (pixels);
  surface = cairo_image_surface_create_for_data (
      pixels, CAIRO_FORMAT_ARGB32, width, height, rowstride);
  cr = cairo_create (surface);
  cairo_surface_destroy (surface);
  mrg->cr = cr;
  return cr;
#endif
  return NULL;
  }
}

static long  frame_start;
static long  frame_end;


int  _mrg_bindings_key_down (MrgEvent *event, void *data1, void *data2);
void mrg_text_edit_bindings (Mrg *mrg);
void mrg_focus_bindings (Mrg *mrg);


typedef struct IdleCb {
  int (*cb) (void *idle_data);
  void *idle_data;
} IdleCb;


void _mrg_idle_iteration (Mrg *mrg)
{
  MrgList *l;
  MrgList *to_remove = NULL;
  if (!mrg->idles)
  {
    return;
  }
  for (l = mrg->idles; l; l = l->next)
  {
    IdleCb *item = l->data;

    if (!item->cb (item->idle_data))
      mrg_list_prepend (&to_remove, item);
  }
  for (l = to_remove; l; l = l->next)
    mrg_list_remove (&mrg->idles, l->data);
}

void mrg_prepare (Mrg *mrg)
{

  mrg->state->fg = 0; /* XXX: move to terminal? what about reverse-video? */
  mrg->state->bg = 7;

  frame_start = _mrg_ticks ();

  mrg_clear (mrg);
  mrg->in_paint ++;

  mrg_style_defaults (mrg);

  if (mrg->mrg_prepare)
    mrg->mrg_prepare (mrg);

  mrg_start (mrg, "document", NULL);

#if MRG_CAIRO
  {
    cairo_t *cr = mrg_cr (mrg);
    cairo_save (cr);
    cairo_scale (cr, mrg->ddpx, mrg->ddpx);
    /* gtk does it's own clipping/exposure handling  */
    if (mrg->do_clip && 0) // XXX : re-enable!
    {
      cairo_rectangle (cr,
          mrg->dirty.x,
          mrg->dirty.y,
          mrg->dirty.width,
          mrg->dirty.height);
      cairo_clip (cr);
    }
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
  }
#endif

  mrg_listen (mrg, MRG_KEY_DOWN, 0,0,0,0, _mrg_bindings_key_down, NULL, NULL);

  if (mrg->edited)
    mrg_text_edit_bindings (mrg);

#if 0
  else
    mrg_focus_bindings (mrg);
#endif
}

void mrg_flush  (Mrg *mrg)
{
  _mrg_debug_overlays (mrg);
  mrg_end (mrg);

#if MRG_CAIRO
  cairo_restore (mrg_cr (mrg));
#endif

  if (mrg->mrg_flush)
    mrg->mrg_flush (mrg);

  _mrg_set_clean (mrg);
  mrg->in_paint --;
  frame_end = _mrg_ticks ();
  //fprintf (stderr, "(%f)", (frame_end - frame_start) / 1000.0);
}

void mrg_warp_pointer (Mrg *mrg, float x, float y)
{
  if (mrg->mrg_warp_pointer)
    mrg->mrg_warp_pointer (mrg, x, y);
  mrg->pointer_x = x;
  mrg->pointer_y = y;
}

void mrg_set_fullscreen (Mrg *mrg, int fullscreen)
{
  if (mrg->mrg_fullscreen)
    mrg->mrg_fullscreen (mrg, fullscreen);
}

int mrg_is_fullscreen (Mrg *mrg)
{
  return mrg->fullscreen;
}

int  mrg_is_terminal (Mrg *mrg)
{
  return mrg->glyphs != NULL;
}

static void mrg_parse_style_id (Mrg *mrg, const char *style_id,
                                MrgStyleNode *node)
{
  const char *p;
  char temp[128] = "";
  int  temp_l = 0;

  memset (node, 0, sizeof (MrgStyleNode));

  for (p = style_id; ; p++)
  {
    switch (*p)
    {
      case '.':
      case ':':
      case '#':
      case 0:
        if (temp_l)
        {
          switch (temp[0])
          {
            case '.':
              {
                int i = 0;
                for (i = 0; node->classes[i]; i++);
                node->classes[i] = mrg_intern_string (&temp[1]);
              }
              break;
            case ':':
              {
                int i = 0;
                for (i = 0; node->pseudo[i]; i++);
                node->pseudo[i] = mrg_intern_string (&temp[1]);
              }
              break;
            case '#':
              node->id = mrg_intern_string (&temp[1]);
              break;
            default:
              node->element = mrg_intern_string (temp);
              break;
          }
          temp_l = 0;
        }
        if (*p == 0)
          return;
      default:
        temp[temp_l++] = *p;
        temp[temp_l]=0;
    }
  }
}

int _mrg_child_no (Mrg *mrg)
{
  return mrg->states[mrg->state_no-1].children;
}

void mrg_start_with_style (Mrg        *mrg,
                           const char *style_id,
                           void       *id_ptr,
                           const char *style)
{
  mrg->states[mrg->state_no].children++;
  mrg->state_no++;
  mrg->state = &mrg->states[mrg->state_no];
  *mrg->state = mrg->states[mrg->state_no-1];
  mrg->states[mrg->state_no].children = 0;

  mrg->state->style_id = style_id ? strdup (style_id) : NULL;

  mrg_parse_style_id (mrg, 
      mrg->state->style_id,
      &mrg->state->style_node);

  mrg->state->style.display = MRG_DISPLAY_INLINE;
  mrg->state->style.id_ptr = id_ptr;

  _mrg_init_style (mrg);

#if MRG_CAIRO
  if (mrg->in_paint)
    cairo_save (mrg_cr (mrg));
#endif

  {
    char *collated_style = _mrg_stylesheet_collate_style (mrg);
    if (collated_style)
    {
      mrg_set_style (mrg, collated_style);
      free (collated_style);
    }
  }
  if (style)
  {
    mrg_set_style (mrg, style);
  }
  _mrg_layout_pre (mrg, &mrg->html);
}

void mrg_start (Mrg *mrg, const char *style_id, void *id_ptr)
{
  mrg_start_with_style (mrg, style_id, id_ptr, NULL);
}

void mrg_end (Mrg *mrg)
{
  _mrg_layout_post (mrg, &mrg->html);
  if (mrg->state->style_id)
  {
    free (mrg->state->style_id);
    mrg->state->style_id = NULL;
  }
  mrg->state_no--;
  if (mrg->state_no < 0)
    fprintf (stderr, "unbalanced mrg_start/mrg_end, too many ends\n");
  mrg->state = &mrg->states[mrg->state_no];
#if MRG_CAIRO
  if (mrg->in_paint)
    cairo_restore (mrg_cr (mrg));
#endif
}

void mrg_style_defaults (Mrg *mrg);

void mrg_style_defaults (Mrg *mrg)
{
  float em = mrg_is_terminal (mrg) ? CPX : 16;
  mrg_set_em (mrg,  em);
  mrg_set_rem (mrg, em);
  mrg_set_edge_left (mrg, 0);
  mrg_set_edge_right (mrg, mrg_width (mrg));
  mrg_set_edge_bottom (mrg, mrg_height (mrg));
  mrg_set_edge_top (mrg, 0);
  mrg_set_line_height (mrg, mrg_is_terminal (mrg)?1.0:1.2);

  mrg->state->style.stroke_width = 1;
  mrg_color_set_from_string (mrg, &mrg->state->style.stroke_color, "transparent");
  mrg_color_set_from_string (mrg, &mrg->state->style.fill_color, "black");

  mrg_stylesheet_clear (mrg);
  _mrg_init_style (mrg);
}

#define X(name) \
void  mrg_set_##name (Mrg *mrg, float val)\
{\
  mrg->state->name = val;\
}\
float mrg_##name (Mrg *mrg)\
{\
  return mrg->state->name;\
}

X (edge_left)
X (edge_right)
X (edge_bottom)

#undef X

void  mrg_set_edge_top (Mrg *mrg, float val)
{
  mrg->state->edge_top = val;
  mrg_set_xy (mrg, _mrg_dynamic_edge_left (mrg) + mrg_style(mrg)->text_indent
      , mrg->state->edge_top + mrg_em (mrg));
}
float mrg_edge_top (Mrg *mrg)
{
  return mrg->state->edge_top;
}

void _mrg_set_wrap_edge_vfuncs (Mrg *mrg,
    float (*wrap_edge_left)  (Mrg *mrg, void *wrap_edge_data),
    float (*wrap_edge_right) (Mrg *mrg, void *wrap_edge_data),
    void *wrap_edge_data)
{
  mrg->state->wrap_edge_left = wrap_edge_left;
  mrg->state->wrap_edge_right = wrap_edge_right;
  mrg->state->wrap_edge_data = wrap_edge_data;
}

void _mrg_set_post_nl (Mrg *mrg,
    void (*post_nl)  (Mrg *mrg, void *post_nl_data, int last),
    void *post_nl_data
    )
{
  mrg->state->post_nl = post_nl;
  mrg->state->post_nl_data = post_nl_data;
}

unsigned char *mrg_get_pixels (Mrg *mrg, int *rowstride)
{
  if (mrg->mrg_get_pixels)
    return mrg->mrg_get_pixels (mrg, rowstride);
  return NULL;
}

float mrg_ddpx (Mrg *mrg)
{
  return mrg->ddpx;
}

void  mrg_ui_update (Mrg *mrg)
{
  mrg_prepare (mrg);
  if (mrg->ui_update)
    mrg->ui_update (mrg, mrg->user_data);
  mrg_flush (mrg);
}

static int mrg_mrg_press (MrgEvent *event, void *mrg, void *data2)
{
  mrg_pointer_press (mrg, event->x, event->y, event->device_no);
  return 0;
}

static int mrg_mrg_motion (MrgEvent *event, void *mrg, void *data2)
{
  mrg_pointer_motion (mrg, event->x, event->y, event->device_no);
  return 0;
}

static int mrg_mrg_release (MrgEvent *event, void *mrg, void *data2)
{
  mrg_pointer_release (mrg, event->x, event->y, event->device_no);
  return 0;
}

void mrg_render_to_mrg (Mrg *mrg, Mrg *mrg2, float x, float y)
{
  unsigned char *pixels = NULL;
  cairo_surface_t *surface;
  int width, height;
  int rowstride = 0;
  cairo_t *cr = mrg_cr (mrg2);

  if (_mrg_is_dirty (mrg))
    mrg_ui_update (mrg);
  
  pixels = mrg_get_pixels (mrg, &rowstride);
  width = mrg_width (mrg);
  height = mrg_height (mrg);
  surface = cairo_image_surface_create_for_data (pixels, CAIRO_FORMAT_ARGB32, width, height, rowstride);
  cairo_save (cr);
  cairo_translate (cr, x, y);
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);
  cairo_surface_destroy (surface);
  
  mrg_listen (mrg2, MRG_PRESS, 0, 0, mrg_width (mrg), mrg_height (mrg),
              mrg_mrg_press, mrg, NULL);
  mrg_listen (mrg2, MRG_MOTION, 0, 0, mrg_width (mrg), mrg_height (mrg),
              mrg_mrg_motion, mrg, NULL);
  mrg_listen (mrg2, MRG_RELEASE, 0, 0, mrg_width (mrg), mrg_height (mrg),
              mrg_mrg_release, mrg, NULL);
  cairo_restore (cr);
}

int mrg_add_idle (Mrg *mrg, int (*idle_cb)(void *idle_data), void *idle_data)
{
  IdleCb *item = calloc (sizeof (IdleCb), 1);
  item->cb = idle_cb;
  item->idle_data = idle_data;
  mrg_list_append (&mrg->idles, item);
  return 1;
}

void  mrg_set_position  (Mrg *mrg, int x, int y)
{
  if (mrg->mrg_set_position)
  {
    //int ox, oy;
    //int dx, dy;
    //mrg_get_position (mrg, &ox, &oy);
    //dx = x - ox;
    //dy = y - oy;

    mrg->mrg_set_position (mrg, x, y);
  }
}

void  mrg_get_position  (Mrg *mrg, int *x, int *y)
{
  if (mrg->mrg_get_position)
    mrg->mrg_get_position (mrg, x, y);
}

void  mrg_set_title     (Mrg *mrg, const char *title)
{
  if (title != mrg->title)
  {
    if (mrg->title)
      free (mrg->title);
    mrg->title = title?strdup (title):NULL;
  }
  if (mrg->mrg_set_title)
    mrg->mrg_set_title (mrg, title);
}

const char *mrg_get_title (Mrg *mrg)
{
  if (mrg->mrg_get_title)
    return mrg->mrg_get_title (mrg);
  return mrg->title;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-events.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

void mrg_clear (Mrg *mrg)
{
  if (mrg->frozen)
    return;
  mrg_list_free (&mrg->items);
  if (mrg->mrg_clear)
    mrg->mrg_clear (mrg);

  _mrg_clear_bindings (mrg);
}

MrgItem *_mrg_detect (Mrg *mrg, float x, float y, MrgType type)
{
  MrgList *a;

  if (type == MRG_KEY_DOWN ||
      type == MRG_KEY_UP ||
      type == (MRG_KEY_DOWN|MRG_KEY_UP))
  {
    for (a = mrg->items; a; a = a->next)
    {
      MrgItem *item = a->data;
      if (item->types & type)
        return item;
    }
    return NULL;
  }

  for (a = mrg->items; a; a = a->next)
  {
    MrgItem *item= a->data;
  
    double u, v;
    u = x;
    v = y;

#if MRG_CAIRO
    cairo_matrix_transform_point (&item->inv_matrix, &u, &v);
#endif

    if (u >= item->x0 && v >= item->y0 &&
        u <  item->x1 && v <  item->y1 &&
        item->types & type)
      return item;
  }
  return NULL;
}

static int rectangle_equal (MrgItem *a, MrgItem *b)
{
  return a->x0 == b->x0 &&
         a->y0 == b->y0 &&
         a->x1 == b->x1 &&
         a->y1 == b->y1 
#if MRG_CAIRO
         && !memcmp (&a->inv_matrix, &b->inv_matrix, sizeof (a->inv_matrix))
#endif
         ;
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
    free (mrgitem);
  }
}

void mrg_listen (Mrg     *mrg,
                 MrgType  types,
                 float   x,
                 float   y,
                 float   width,
                 float   height,
                 MrgCb    cb,
                 void*    data1,
                 void*    data2)
{
  if (types == MRG_DRAG_MOTION)
    types = MRG_DRAG_MOTION | MRG_DRAG_PRESS;
  return mrg_listen_full (mrg, types, x, y, width, height, cb, data1, data2,
                          NULL, NULL);
}

void mrg_listen_full (Mrg     *mrg,
                      MrgType  types,
                      float   x,
                      float   y,
                      float   width,
                      float   height,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
                      void    *finalize_data)
{
  if (!mrg->frozen)
  {
    MrgItem *item;

    if (y > mrg->height * 2 ||
        x > mrg->width * 2 ||
        x < -mrg->width ||
        y < -mrg->height)
    {
      if (finalize)
        finalize (data1, data2, finalize_data);
      return;
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
#if MRG_CAIRO
    cairo_get_matrix (mrg_cr (mrg), &item->inv_matrix);
    cairo_matrix_invert (&item->inv_matrix);
#endif

    if (mrg->items)
    {
      MrgList *l;
      for (l = mrg->items; l; l = l->next)
      {
        MrgItem *item2 = l->data;
        if (rectangle_equal (item, item2))
        {
          /* found an item, copy over cb data  */
          item2->cb[item2->cb_count] = item->cb[0];
          free (item);
          item2->cb_count++;
          item2->types |= types;
          return;
        }
      }
    }
    item->ref_count = 1;
    mrg_list_prepend_full (&mrg->items, item, (void*)_mrg_item_unref, NULL);
    //mrg_list_append (&mrg->items, item);
  }
}


static int
_mrg_emit_cb (Mrg *mrg, MrgItem *item, MrgEvent *event, MrgType type, float x, float y)
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

#if MRG_CAIRO
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
#endif

  event = &transformed_event;

  for (i = item->cb_count-1; i >= 0; i--)
  {
    if (item->cb[i].types & (type))
    {
      int val = item->cb[i].cb (event, item->cb[i].data1, item->cb[i].data2);
      if (val)
        return val;
    }
  }
  return 0;
}

static MrgItem *_mrg_update_item (Mrg *mrg, float x, float y, MrgType type)
{
  MrgItem *current = _mrg_detect (mrg, x, y, MRG_ANY);

  if (mrg->prev == NULL || current == NULL || !rectangle_equal (current, mrg->prev))
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

      _mrg_emit_cb (mrg, mrg->prev, NULL, MRG_LEAVE, x, y);
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
      _mrg_emit_cb (mrg, current, NULL, MRG_ENTER, x, y);
      mrg->prev = current;
    }
  }
  current = _mrg_detect (mrg, x, y, type);
  return current;
}


int mrg_pointer_press (Mrg *mrg, float x, float y, int device_no)
{
  MrgItem *mrg_item = _mrg_update_item (mrg, x, y, MRG_PRESS | MRG_DRAG_PRESS);
  mrg->pointer_x = x;
  mrg->pointer_y = y;

  mrg->drag_event.type = MRG_PRESS;
  mrg->drag_event.x = mrg->drag_event.start_x = mrg->drag_event.prev_x = x;
  mrg->drag_event.y = mrg->drag_event.start_y = mrg->drag_event.prev_y = y;
  mrg->drag_event.delta_x = mrg->drag_event.delta_y = 0;
  mrg->drag_event.device_no = device_no;

  if (mrg->pointer_down[device_no] == 1)
  {
    fprintf (stderr, "device %i already doen\n", device_no);
  }
  mrg->pointer_down[device_no] = 1;


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
    return _mrg_emit_cb (mrg, mrg_item, &mrg->drag_event, mrg->is_press_grabbed?MRG_DRAG_PRESS:MRG_PRESS, x, y);
  }

  return 0;
}

void mrg_resized (Mrg *mrg, int width, int height)
{
  MrgItem *item = _mrg_detect (mrg, 0, 0, MRG_KEY_DOWN);
  
  mrg->drag_event.mrg = mrg;
  mrg->drag_event.type = MRG_KEY_DOWN;
  mrg->drag_event.key_name = "resize-event";

  if (item)
  {
    _mrg_emit_cb (mrg, item, &mrg->drag_event, MRG_KEY_DOWN, 0, 0);
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

  mrg_queue_draw (mrg, NULL); /* in case of style change */

  if (mrg->pointer_down[device_no] == 0)
  {
    fprintf (stderr, "device %i already up\n", device_no);
  }
  mrg->pointer_down[device_no] = 0;

  mrg->pointer_x = x;
  mrg->pointer_y = y;

  if (mrg->is_press_grabbed)
  {
    mrg->drag_event.type = MRG_DRAG_RELEASE;
    mrg->is_press_grabbed = 0;
    mrg_item = mrg->grab;
    was_grabbed = 1;
  }
  else
  {
    mrg_item = _mrg_update_item (mrg, x, y, MRG_RELEASE | MRG_DRAG_RELEASE);
  }
  if (mrg_item)
  {
    return _mrg_emit_cb (mrg, mrg_item, &mrg->drag_event, was_grabbed?MRG_DRAG_RELEASE:MRG_RELEASE, x, y);
  }

  if (was_grabbed)
  {
    _mrg_item_unref (mrg->grab);
    mrg->grab = NULL;
  }
  return 0;
}

int mrg_pointer_motion (Mrg *mrg, float x, float y, int device_no)
{
  MrgItem   *mrg_item;

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
    mrg_item = _mrg_update_item (mrg, x, y, MRG_MOTION | MRG_DRAG_MOTION);
  }

  /* XXX: too brutal; should use enter/leave events */
  mrg_queue_draw (mrg, NULL);

  if (mrg_item)
  {
    int i;
    for (i = 0; i < mrg_item->cb_count; i++)
    {
      if (mrg_item->cb[i].types & (MRG_DRAG_MOTION | MRG_MOTION))
      {
        if (  !(mrg_item->cb[i].types & MRG_DRAG_MOTION) ||
                mrg->is_press_grabbed)
        if (_mrg_emit_cb (mrg, mrg_item, &mrg->drag_event, mrg->is_press_grabbed?MRG_DRAG_MOTION:MRG_MOTION, x, y))
          goto done;
      }
    }
    done:
    mrg->drag_event.delta_x = x - mrg->drag_event.prev_x;
    mrg->drag_event.delta_y = y - mrg->drag_event.prev_y;
    mrg->drag_event.prev_x = x;
    mrg->drag_event.prev_y = y;
    return 1;
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

    for (i = 0; i < item->cb_count; i++)
    {
      if (item->cb[i].types & (MRG_KEY_DOWN))
      {
        if (item->cb[i].cb (&mrg->drag_event, item->cb[i].data1, item->cb[i].data2))
          return 1;
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
#if MRG_CAIRO
  MrgList *a;
  cairo_t *cr = mrg_cr (mrg);
  cairo_save (cr);

  cairo_set_line_width (cr, 1);
  cairo_set_source_rgba (cr, 1,0.5,0.5,0.5);
  for (a = mrg->items; a; a = a->next)
  {
    double current_x = mrg_pointer_x (mrg);
    double current_y = mrg_pointer_y (mrg);
    MrgItem *item = a->data;
    cairo_matrix_t matrix = item->inv_matrix;

    cairo_matrix_transform_point (&matrix, &current_x, &current_y);

#if 1
    if (current_x >= item->x0 && current_x < item->x1 &&
        current_y >= item->y0 && current_y < item->y1)
#endif
    {
      cairo_matrix_invert (&matrix);
      cairo_set_matrix (cr, &matrix);
      cairo_rectangle (cr, item->x0, item->y0,
                       (item->x1-item->x0),
                       (item->y1-item->y0));
      cairo_stroke (cr);
    }
  }
  cairo_restore (cr);
#endif
}

/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-focus.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

/* NOTE: quite a bit of unneeded code can be eliminated in here */

static void mrg_item_center (MrgItem *item, float *cx, float *cy)
{
  double x = (item->x0 + item->x1)/2;
  double y = (item->y0 * 0.2 + item->y1 * 0.8);

#if MRG_CAIRO
  cairo_matrix_t mat = item->inv_matrix;
  cairo_matrix_invert (&mat);
  cairo_matrix_transform_point (&mat, &x, &y);
#endif

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

static int cmd_focus_up (MrgEvent *event, void *data, void *data2)
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
    return 1;
  }

  return 0;
}


static int cmd_focus_down (MrgEvent *event, void *data, void *data2)
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
    return 1;
  }

  return 0;
}


static int cmd_focus_left (MrgEvent *event, void *data, void *data2)
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
    return 1;
  }

  return 0;
}


static int cmd_focus_right (MrgEvent *event, void *data, void *data2)
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
    return 1;
  }

  return 0;
}

static int cmd_focus_previous (MrgEvent *event, void *data, void *data2)
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
        return 0;
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

  return 0;
}
static int cmd_focus_next (MrgEvent *event, void *data, void *data2)
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
            return 0;
          }
          if (item == current)
            found = 1;
        }
        mrg_warp_pointer (mrg, mrg_width(mrg)/2, mrg_height(mrg)-1);
        return 0;
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
            return 0;
          }
        }
      mrg_warp_pointer (mrg, 0, 0);
      return 0;
    }

  }

  return 0;
}


static int cmd_select (MrgEvent *event, void *data, void *data2)
{
  Mrg *mrg = event->mrg;

  mrg_pointer_press   (mrg, mrg_pointer_x (mrg), mrg_pointer_y (mrg), 0);
  mrg_pointer_release (mrg, mrg_pointer_x (mrg), mrg_pointer_y (mrg), 0);
  return 0;
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
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-image.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

#if MRG_CAIRO

typedef struct _MrgImage MrgImage;

struct _MrgImage
{
  char *path;
  cairo_surface_t *surface;
};

static MrgList *image_cache = NULL;

static void free_image (Mrg *mrg, void *data)
{
  MrgImage *image = data;
  free (image->path);
  cairo_surface_destroy (image->surface);
  free (data);
}
#endif

MrgImage *mrg_query_image (Mrg *mrg, const char *path, 
                           int *width,
                           int *height)
{
  MrgList *l;

  if (!path)
    return NULL;
  for (l = image_cache; l; l = l->next)
  {
    MrgImage *image = l->data;
    if (!strcmp (image->path, path))
    {
      if (width)
        *width = cairo_image_surface_get_width (image->surface);
      if (height)
        *height = cairo_image_surface_get_height (image->surface);
      return image;
    }
  }
  {
    cairo_surface_t *surface = cairo_image_surface_create_from_png (path);
    if (surface)
    {
      MrgImage *image = malloc (sizeof (MrgImage));
      image->surface = surface;
      image->path = strdup (path);
      mrg_list_prepend_full (&image_cache, image, (void*)free_image, NULL);
      return mrg_query_image (mrg, path, width, height);
    }
  }
  return NULL;
}


void mrg_image (Mrg *mrg, float x0, float y0, float width, float height, const char *path)
{
#if MRG_CAIRO
  int orig_width, orig_height;
  MrgImage *image;
  cairo_t *cr = mrg_cr (mrg);
  cairo_surface_t *surface = NULL;

  if (!path)
    return;

  image = mrg_query_image (mrg, path, &orig_width, &orig_height);
  if (!image)
    return;

  surface = image->surface;

  if (width == -1 && height == -1)
  {
    width = orig_width;
    height = orig_height;
  }

  if (width == -1)
    width = orig_width * height / orig_height;
  if (height == -1)
    height = orig_height * width / orig_width;

  cairo_save (cr);
  
  cairo_rectangle (cr, x0, y0, width, height);
  cairo_clip (cr);
  cairo_translate (cr, x0, y0);
  cairo_scale (cr,
      width / orig_width,
      height / orig_height);

  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);
  cairo_restore (cr);
#endif
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-style-properties.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

/* XXX: missing CSS1:
 *
 *   EM { color: rgb(110%, 0%, 0%) }  // clipped to 100% 
 *
 *
 *   :first-letter
 *   :first-list
 *   :link :visited :active
 *
 */

typedef struct ColorDef {
  const char *name;
  float r;
  float g;
  float b;
  float a;
} ColorDef;

static ColorDef colors[]={
  {"black",    0, 0, 0, 1},
  {"red",      1, 0, 0, 1},
  {"green",    0, 1, 0, 1},
  {"yellow",   1, 1, 0, 1},
  {"blue",     0, 0, 1, 1},
  {"fuchsia",  1, 0, 1, 1},
  {"cyan",     0, 1, 1, 1},
  {"white",    1, 1, 1, 1},
  {"silver",   0.75294, 0.75294, 0.75294, 1},
  {"gray",     0.50196, 0.50196, 0.50196, 1},
  {"magenta",  0.50196, 0, 0.50196, 1},
  {"maroon",   0.50196, 0, 0, 1},
  {"purple",   0.50196, 0, 0.50196, 1},
  {"green",    0, 0.50196, 0, 1},
  {"lime",     0, 1, 0, 1},
  {"olive",    0.50196, 0.50196, 0, 1},
  {"navy",     0, 0,      0.50196, 1},
  {"teal",     0, 0.50196, 0.50196, 1},
  {"aqua",     0, 1, 1, 1},
  {"transparent", 0, 0, 0, 0},
  {"none",     0, 0, 0, 0},
};

static int xdigit_value(const char xdigit)
{
  if (xdigit >= '0' && xdigit <= '9')
   return xdigit - '0';
  switch (xdigit)
  {
    case 'A':case 'a': return 10;
    case 'B':case 'b': return 11;
    case 'C':case 'c': return 12;
    case 'D':case 'd': return 13;
    case 'E':case 'e': return 14;
    case 'F':case 'f': return 15;
  }
  return 0;
}

float mrg_parse_float (Mrg *mrg, const char *str, char **endptr)
{
  if (!str)
  {
    if (endptr)
      *endptr = NULL;
    return 0.0;
  }
#if 0
  if (str[0] == '.')
  {
    char *endptr2;
    int digits;
    double val = strtod (&str[1], &endptr2); /* XXX: , vs . problem in some locales */
    if (endptr)
      *endptr = endptr2;
    digits = endptr2 - &str[1];
    while (digits--)
      val /= 10.0;
    fprintf (stderr, "[%s %f]\n", str, val);
    return val;
  }
#endif
  return strtod (str, endptr); /* XXX: , vs . problem in some locales */
}

static int
mrg_color_parse_rgb (MrgColor *color, const char *color_string)
{
  float *dcolor = (void*)color;
  dcolor[3] = 1.0;
  while (*color_string && *color_string != '(')
    color_string++;
  if (*color_string) color_string++;

  {
    int n_floats = 0;
    char *p =    (void*)color_string;
    char *prev = (void *)NULL;
    for (; p && n_floats < 4 && p != prev && *p; )
    {
      float val;
      prev = p;
      val = mrg_parse_float (NULL, p, &p);
      if (p != prev)
      {
        if (n_floats < 3)
          dcolor[n_floats++] = val/255.0;
        else
          dcolor[n_floats++] = val;

        while (*p == ' ' || *p == ',')
        {
          p++;
          prev++;
        }
      }
    }
  }
  return 0;
}

static int
mrg_color_parse_hex (MrgColor *color, const char *color_string)
{
  float *dcolor = (void*)color;
  int string_length = strlen (color_string);
  int i;
  dcolor[3] = 1.0;

  if (string_length == 7 ||  /* #rrggbb   */
      string_length == 9)    /* #rrggbbaa */
    {
      int num_iterations = (string_length - 1) / 2;
  
      for (i = 0; i < num_iterations; ++i)
        {
          if (isxdigit (color_string[2 * i + 1]) &&
              isxdigit (color_string[2 * i + 2]))
            {
              dcolor[i] = (xdigit_value (color_string[2 * i + 1]) << 4 |
                           xdigit_value (color_string[2 * i + 2])) / 255.f;
            }
          else
            {
              return 0;
            }
        }
      /* Successful #rrggbb(aa) parsing! */
      return 1;
    }
  else if (string_length == 4 ||  /* #rgb  */
           string_length == 5)    /* #rgba */
    {
      int num_iterations = string_length - 1;
      for (i = 0; i < num_iterations; ++i)
        {
          if (isxdigit (color_string[i + 1]))
            {
              dcolor[i] = (xdigit_value (color_string[i + 1]) << 4 |
                           xdigit_value (color_string[i + 1])) / 255.f;
            }
          else
            {
              return 0;
            }
        }
      /* Successful #rgb(a) parsing! */
      return 0;
    }
  /* String was of unsupported length. */
  return 1;
}

int mrg_color_set_from_string (Mrg *mrg, MrgColor *color, const char *string)
{
  int i;

  if (!strcmp (string, "currentColor"))
  {
    MrgStyle *style = mrg_style (mrg);
    color->red = style->color.red;
    color->green = style->color.green;
    color->blue = style->color.blue;
    color->alpha = style->color.alpha;
    return 0;
  }

  for (i = (sizeof(colors)/sizeof(colors[0]))-1; i>=0; i--)
  {
    if (!strcmp (colors[i].name, string))
    {
      color->red = colors[i].r;
      color->green = colors[i].g;
      color->blue = colors[i].b;
      color->alpha = colors[i].a;
      return 0;
    }
  }


  if (string[0] == '#')
    mrg_color_parse_hex (color, string);
  else if (string[0] == 'r' &&
      string[1] == 'g' &&
      string[2] == 'b'
      )
    mrg_color_parse_rgb (color, string);
  return 0;
}

static int mrg_color_to_nct (MrgColor *color)
{
  int i;
  float bestdiff = 10000.0;
  int best = 0;

  for (i = 0; i < 8; i++)
  {
    float diff = 
      (color->red - colors[i].r) *
      (color->red - colors[i].r) +
      (color->green - colors[i].g) *
      (color->green - colors[i].g) +
      (color->blue  - colors[i].b) *
      (color->blue  - colors[i].b);
    if (diff < bestdiff)
    {
      best = i;
      bestdiff = diff;
    }
  }
  return best;
}

static inline float mrg_parse_px_x (Mrg *mrg, const char *str, char **endptr)
{
  float result = 0;
  char *end = NULL;
#define PPI   96

  if (!str)
    return 0.0;


  result = mrg_parse_float (mrg, str, &end); /* XXX: , vs . problem in some locales */
  if (endptr)
    *endptr=end;

  //if (end[0]=='%v') /// XXX  % of viewport; regard less of stacking
  if (end[0]=='%')
  {
    result = result / 100.0 * (mrg_edge_right (mrg) - mrg_edge_left (mrg));

    if (endptr)
      *endptr=end + 1;
  }
  else if (end[0]=='r' && end[1]=='e' && end[2]=='m')
  {
    result *= mrg_rem (mrg);
    if (endptr)
      *endptr=end + 3;
  }
  else if (end[0]=='e' && end[1]=='m')
  {
    result *= mrg_em (mrg);
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='x')
  {
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='t')
  {
    result = (result / PPI) * 72;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='c')
  {
    result = (result / PPI) * 72 / 12;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='i' && end[1]=='n')
  {
    result = result / PPI;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='c' && end[1]=='m')
  {
    result = (result / PPI) * 2.54;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='m' && end[1]=='m')
  {
    result = (result / PPI) * 25.4;
    if (endptr)
      *endptr=end + 2;
  }
  return result;
}

static inline float mrg_parse_px_y (Mrg *mrg, const char *str, char **endptr)
{
  float result = 0;
  char *end = NULL;
  if (!str)
    return 0.0;

  result = mrg_parse_float (mrg, str, &end); /* XXX: , vs . problem in some locales */
  if (endptr)
    *endptr=end;

  if (end[0]=='%')
  {
    result = result / 100.0 * (mrg_edge_bottom (mrg) - mrg_edge_top (mrg));
    if (endptr)
      *endptr=end + 1;
  }
  else if (end[0]=='r' && end[1]=='e' && end[2]=='m')
  {
    result *= mrg_rem (mrg);
    if (endptr)
      *endptr=end + 3;
  }
  else if (end[0]=='e' && end[1]=='m')
  {
    result *= mrg_em (mrg);
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='x')
  {
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='t')
  {
    result = (result / PPI) * 72;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='p' && end[1]=='c')
  {
    result = (result / PPI) * 72 / 12;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='i' && end[1]=='n')
  {
    result = result / PPI;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='c' && end[1]=='m')
  {
    result = (result / PPI) * 2.54;
    if (endptr)
      *endptr=end + 2;
  }
  else if (end[0]=='m' && end[1]=='m')
  {
    result = (result / PPI) * 25.4;
    if (endptr)
      *endptr=end + 2;
  }
  return result;
}

static inline int mrg_parse_pxs (Mrg *mrg, const char *str, float *vals)
{
  int n_floats = 0;
  char *p =    (void*)str;
  char *prev = (void *)NULL;

  for (; p && p != prev && *p; )
  {
    float val;
    prev = p;
    val = n_floats%2==1?
      mrg_parse_px_x (mrg, p, &p):mrg_parse_px_y (mrg, p, &p);
    if (p != prev)
    {
      vals[n_floats++] = val;
    }
  }

  return n_floats;
}


static inline void mrg_css_handle_property_pass0 (Mrg *mrg, const char *name,
                                           const char *value)
{
  MrgStyle *s = mrg_style (mrg);
  /* pass0 deals with properties that parsing of many other property
   * definitions rely on */
  if (!strcmp (name, "font-size"))
  {
    float parsed;
    
    if (mrg->state_no)
    {
      mrg->state_no--;
      parsed = mrg_parse_px_y (mrg, value, NULL);
      mrg->state_no++;
    }
    else
    {
      parsed = mrg_parse_px_y (mrg, value, NULL);
    }
    mrg_set_em (mrg, parsed);
  }
  else if (!strcmp (name, "color"))
  {
    mrg_color_set_from_string (mrg, &s->color, value);
    mrg->state->fg = mrg_color_to_nct (&s->color);
  }
}

static void mrg_css_handle_property_pass1 (Mrg *mrg, const char *name,
                                           const char *value)
{
  MrgStyle *s = mrg_style (mrg);

  if (!strcmp (name, "text-indent"))
    s->text_indent = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "letter-spacing"))
    s->letter_spacing = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "word-spacing"))
    s->word_spacing = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "tab-size"))
    s->tab_size = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "stroke-width"))
    s->stroke_width = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "margin"))
    {
      float vals[10];
      int    n_vals;

      n_vals = mrg_parse_pxs (mrg, value, vals);
      switch (n_vals)
      {
        case 1:
          s->margin_top = vals[0];
          s->margin_right  = vals[0];
          s->margin_bottom = vals[0];
          s->margin_left = vals[0];
          break;
        case 2:
          s->margin_top = vals[0];
          s->margin_right  = vals[1];
          s->margin_bottom = vals[0];
          s->margin_left = vals[1];
          break;
        case 3:
          s->margin_top = vals[0];
          s->margin_right  = vals[1];
          s->margin_bottom = vals[2];
          s->margin_left = vals[1];
          break;
        case 4:
          s->margin_top = vals[0];
          s->margin_right  = vals[1];
          s->margin_bottom = vals[2];
          s->margin_left = vals[3];
          break;
      }
    }
  else if (!strcmp (name, "margin-top"))
    s->margin_top = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "margin-bottom"))
    s->margin_bottom = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "margin-left"))
  {
    if (!strcmp (value, "auto"))
    {
      s->margin_left_auto = 1;
    }
    else
    {
      s->margin_left = mrg_parse_px_x (mrg, value, NULL);
      s->margin_left_auto = 0;
    }
  }
  else if (!strcmp (name, "margin-right"))
  {
    if (!strcmp (value, "auto"))
    {
      s->margin_right_auto = 1;
    }
    else
    {
      s->margin_right = mrg_parse_px_x (mrg, value, NULL);
      s->margin_right_auto = 0;
    }
  }

  else if (!strcmp (name, "padding-top"))
    s->padding_top = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "padding-bottom"))
    s->padding_bottom = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "padding-left"))
    s->padding_left = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "padding-right"))
    s->padding_right = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "padding"))
    {
      float vals[10];
      int   n_vals;
      n_vals = mrg_parse_pxs (mrg, value, vals);
      switch (n_vals)
      {
        case 1:
          s->padding_top = vals[0];
          s->padding_right  = vals[0];
          s->padding_bottom = vals[0];
          s->padding_left = vals[0];
          break;
        case 2:
          s->padding_top = vals[0];
          s->padding_right  = vals[1];
          s->padding_bottom = vals[0];
          s->padding_left = vals[1];
          break;
        case 3:
          s->padding_top = vals[0];
          s->padding_right  = vals[1];
          s->padding_bottom = vals[2];
          s->padding_left = vals[1];
          break;
        case 4:
          s->padding_top = vals[0];
          s->padding_right  = vals[1];
          s->padding_bottom = vals[2];
          s->padding_left = vals[3];
          break;
      }
    }
  else if (!strcmp (name, "border-top-width"))
    s->border_top_width = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "border-bottom-width"))
    s->border_bottom_width = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "border-left-width"))
    s->border_left_width = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "border-right-width"))
    s->border_right_width = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "top"))
    s->top = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "height"))
    s->height = mrg_parse_px_y (mrg, value, NULL);

  else if (!strcmp (name, "left"))
    s->left = mrg_parse_px_x (mrg, value, NULL);
  else if (!strcmp (name, "visibility"))
  {
    if (!strcmp (value, "visible"))
      s->visibility = MRG_VISIBILITY_VISIBLE;
    else if (!strcmp (value, "hidden"))
      s->visibility = MRG_VISIBILITY_HIDDEN;
    else
      s->visibility = MRG_VISIBILITY_VISIBLE;
  }
  else if (!strcmp (name, "min-height"))
    s->min_height = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "max-height"))
    s->max_height = mrg_parse_px_y (mrg, value, NULL);
  else if (!strcmp (name, "border-width"))
    {
      s->border_top_width =
      s->border_bottom_width =
      s->border_right_width = 
      s->border_left_width = mrg_parse_px_y (mrg, value, NULL);
    }
  else if (!strcmp (name, "border-color"))
    {
      mrg_color_set_from_string (mrg, &s->border_top_color, value);
      mrg_color_set_from_string (mrg, &s->border_left_color, value);
      mrg_color_set_from_string (mrg, &s->border_right_color, value);
      mrg_color_set_from_string (mrg, &s->border_bottom_color, value);
    }
  else if (!strcmp (name, "border"))
    {
      char word[64];
      int w = 0;
      const char *p;
      for (p = value; ; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\t':
          case '\0':
            if (w)
            {
              if ((word[0] >= '0' && word[0]<='9') || word[0] == '.')
              {
                s->border_top_width = mrg_parse_px_y (mrg, word, NULL);
                s->border_left_width = mrg_parse_px_y (mrg, word, NULL);
                s->border_right_width = mrg_parse_px_y (mrg, word, NULL);
                s->border_bottom_width = mrg_parse_px_y (mrg, word, NULL);
              } else if (!strcmp (word, "solid") ||
                         !strcmp (word, "dotted") ||
                         !strcmp (word, "inset")) {
              } else {
                mrg_color_set_from_string (mrg, &s->border_top_color, word);
                mrg_color_set_from_string (mrg, &s->border_bottom_color, word);
                mrg_color_set_from_string (mrg, &s->border_left_color, word);
                mrg_color_set_from_string (mrg, &s->border_right_color, word);
              }
              word[0]=0;
              w=0;
            }
            break;
          default:
            word[w++]=*p;
            word[w]=0;
            break;
        }
        if (!*p)
          break;
      }
    }
  else if (!strcmp (name, "border-right"))
    {
      char word[64];
      int w = 0;
      const char *p;
      for (p = value; ; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\t':
          case '\0':
            if (w)
            {
              if ((word[0] >= '0' && word[0]<='9') || (word[0] == '.'))
              {
                s->border_right_width = mrg_parse_px_y (mrg, word, NULL);
              } else if (!strcmp (word, "solid") ||
                         !strcmp (word, "dotted") ||
                         !strcmp (word, "inset")) {
              } else {
                mrg_color_set_from_string (mrg, &s->border_right_color, word);
              }
              word[0]=0;
              w=0;
            }
            break;
          default:
            word[w++]=*p;
            word[w]=0;
            break;
        }
        if (!*p)
          break;
      }
    }
  else if (!strcmp (name, "border-top"))
    {
      char word[64];
      int w = 0;
      const char *p;
      for (p = value; ; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\r':
          case '\t':
          case '\0':
            if (w)
            {
              if ((word[0] >= '0' && word[0]<='9') || (word[0] == '.'))
              {
                s->border_top_width = mrg_parse_px_y (mrg, word, NULL);
              } else if (!strcmp (word, "solid") ||
                         !strcmp (word, "dotted") ||
                         !strcmp (word, "inset")) {
              } else {
                mrg_color_set_from_string (mrg, &s->border_top_color, word);
              }
              word[0]=0;
              w=0;
            }
            break;
          default:
            word[w++]=*p;
            word[w]=0;
            break;
        }
        if (!*p)
          break;
      }
    }
  else if (!strcmp (name, "border-left"))
    {
      char word[64];
      int w = 0;
      const char *p;
      for (p = value; ; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\r':
          case '\t':
          case '\0':
            if (w)
            {
              if ((word[0] >= '0' && word[0]<='9') || (word[0] == '.'))
              {
                s->border_left_width = mrg_parse_px_y (mrg, word, NULL);
              } else if (!strcmp (word, "solid") ||
                         !strcmp (word, "dotted") ||
                         !strcmp (word, "inset")) {
              } else {
                mrg_color_set_from_string (mrg, &s->border_left_color, word);
              }
              word[0]=0;
              w=0;
            }
            break;
          default:
            word[w++]=*p;
            word[w]=0;
            break;
        }
        if (!*p)
          break;
      }
    }
  else if (!strcmp (name, "border-bottom"))
    {
      char word[64];
      int w = 0;
      const char *p;
      for (p = value; ; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\r':
          case '\t':
          case '\0':
            if (w)
            {
              if ((word[0] >= '0' && word[0]<='9') || (word[0] == '.'))
              {
                s->border_bottom_width = mrg_parse_px_y (mrg, word, NULL);
              } else if (!strcmp (word, "solid") ||
                         !strcmp (word, "dotted") ||
                         !strcmp (word, "inset")) {
              } else {
                mrg_color_set_from_string (mrg, &s->border_bottom_color, word);
              }
              word[0]=0;
              w=0;
            }
            break;
          default:
            word[w++]=*p;
            word[w]=0;
            break;
        }
        if (!*p)
          break;
      }
    }
  else if (!strcmp (name, "line-height"))
    mrg_set_line_height (mrg, mrg_parse_px_y (mrg, value, NULL));
  else if (!strcmp (name, "line-width"))
  {
    float val =mrg_parse_px_y (mrg, value, NULL);
    s->line_width = val;
#if MRG_CAIRO
    cairo_set_line_width (mrg_cr (mrg), s->line_width);
#endif
  }

  else if (!strcmp (name, "background-color"))
  {
    mrg_color_set_from_string (mrg, &s->background_color, value);
    mrg->state->bg = mrg_color_to_nct (&s->background_color);
  }
  else if (!strcmp (name, "background"))
  {
    mrg_color_set_from_string (mrg, &s->background_color, value);
    mrg->state->bg = mrg_color_to_nct (&s->background_color);
  }
  else if (!strcmp (name, "fill-color") ||
           !strcmp (name, "fill"))
  {
    mrg_color_set_from_string (mrg, &s->fill_color, value);
  }
  else if (!strcmp (name, "stroke-color") ||
           !strcmp (name, "stroke"))
  {
    mrg_color_set_from_string (mrg, &s->stroke_color, value);
  }
  else if (!strcmp (name, "text-stroke-width"))
  {
    s->text_stroke_width = mrg_parse_px_y (mrg, value, NULL);
  }
  else if (!strcmp (name, "text-stroke-color"))
  {
    mrg_color_set_from_string (mrg, &s->text_stroke_color, value);
  }
  else if (!strcmp (name, "text-stroke"))
  {
    char *col = NULL;
    s->text_stroke_width = mrg_parse_px_y (mrg, value, &col);
    if (col)
      mrg_color_set_from_string (mrg, &s->text_stroke_color, col + 1);
  }
  else if (!strcmp (name, "opacity"))
  {
    float dval = mrg_parse_float (mrg, value, NULL);
      if (dval <= 0.5f)
        s->text_decoration |= MRG_DIM;
      s->fill_color.alpha = dval;
      s->border_top_color.alpha = dval;
      s->border_left_color.alpha = dval;
      s->border_right_color.alpha = dval;
      s->border_bottom_color.alpha = dval;
      s->stroke_color.alpha = dval;
      s->color.alpha = dval;
      s->background_color.alpha = dval;
  }
  else  if (!strcmp (name, "font-weight"))
    {
      if (!strcmp (value, "bold") ||
          !strcmp (value, "bolder"))
      {
        s->text_decoration |= MRG_BOLD;
        s->font_weight = MRG_FONT_WEIGHT_BOLD;
      }
      else
      {
        s->text_decoration ^= (s->text_decoration & MRG_BOLD);
        s->font_weight = MRG_FONT_WEIGHT_NORMAL;
      }
#if MRG_CAIRO
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
#endif
    }
  else if (!strcmp (name, "white-space"))
    {
      if (!strcmp (value, "normal"))
        s->white_space = MRG_WHITE_SPACE_NORMAL;
      else if (!strcmp (value, "nowrap"))
        s->white_space = MRG_WHITE_SPACE_NOWRAP;
      else if (!strcmp (value, "pre"))
        s->white_space = MRG_WHITE_SPACE_PRE;
      else if (!strcmp (value, "pre-line"))
        s->white_space = MRG_WHITE_SPACE_PRE_LINE;
      else if (!strcmp (value, "pre-wrap"))
        s->white_space = MRG_WHITE_SPACE_PRE_WRAP;
      else
        s->white_space = MRG_WHITE_SPACE_NORMAL;
    }
  else if (!strcmp (name, "box-sizing"))
    {
      if (!strcmp (value, "border-box"))
      {
        s->box_sizing = MRG_BOX_SIZING_BORDER_BOX;
        s->box_sizing = MRG_BOX_SIZING_CONTENT_BOX;
      }
    }
  else if (!strcmp (name, "float"))
    {
      if (!strcmp (value, "left"))
        s->float_ = MRG_FLOAT_LEFT;
      else if (!strcmp (value, "right"))
        s->float_ = MRG_FLOAT_RIGHT;
      else 
        s->float_ = MRG_FLOAT_NONE;
    }
  else if (!strcmp (name, "overflow"))
    {
      if (!strcmp (value, "visible"))
        s->overflow = MRG_OVERFLOW_VISIBLE;
      else if (!strcmp (value, "hidden"))
        s->overflow = MRG_OVERFLOW_HIDDEN;
      else if (!strcmp (value, "scroll"))
        s->overflow = MRG_OVERFLOW_SCROLL;
      else if (!strcmp (value, "auto"))
        s->overflow = MRG_OVERFLOW_AUTO;
      else 
        s->overflow = MRG_OVERFLOW_VISIBLE;
    }
  else if (!strcmp (name, "clear"))
    {
      if (!strcmp (value, "left"))
        s->clear = MRG_CLEAR_LEFT;
      else if (!strcmp (value, "right"))
        s->clear = MRG_CLEAR_RIGHT;
      else if (!strcmp (value, "both"))
        s->clear = MRG_CLEAR_BOTH;
      else
        s->clear = MRG_CLEAR_NONE;
    }
  else if (!strcmp (name, "font-style"))
    {
      if (!strcmp (value, "italic"))
      {
        s->font_style = MRG_FONT_STYLE_ITALIC;
      }
      else if (!strcmp (value, "oblique"))
      {
        s->font_style = MRG_FONT_STYLE_OBLIQUE;
      }
      else
      {
        s->font_style = MRG_FONT_STYLE_NORMAL;
      }
#if MRG_CAIRO
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
#endif
    }
  else if (!strcmp (name, "font-family"))
    {
      strncpy (s->font_family, value, 63);
      s->font_family[63]=0;
#if MRG_CAIRO
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
#endif
    }
  else if (!strcmp (name, "fill-rule"))
    {
      if (!strcmp (value, "evenodd"))
        s->fill_rule = MRG_FILL_RULE_EVEN_ODD;
      else if (!strcmp (value, "nonzero"))
        s->fill_rule = MRG_FILL_RULE_NONZERO;
      else
        s->fill_rule = MRG_FILL_RULE_EVEN_ODD;

      if (s->fill_rule == MRG_FILL_RULE_EVEN_ODD)
        cairo_set_fill_rule (mrg_cr (mrg), CAIRO_FILL_RULE_EVEN_ODD);
      else
        cairo_set_fill_rule (mrg_cr (mrg), CAIRO_FILL_RULE_WINDING);
    }
  else if (!strcmp (name, "stroke-linejoin"))
    {
      if (!strcmp (value, "miter"))
        s->stroke_linejoin = MRG_LINE_JOIN_MITER;
      else if (!strcmp (value, "round"))
        s->stroke_linejoin = MRG_LINE_JOIN_ROUND;
      else if (!strcmp (value, "bevel"))
        s->stroke_linejoin = MRG_LINE_JOIN_BEVEL;
      else
        s->stroke_linejoin = MRG_LINE_JOIN_MITER;
#if MRG_CAIRO
      cairo_set_line_join (mrg_cr (mrg), s->stroke_linejoin);
#endif
    }
  else if (!strcmp (name, "stroke-linecap"))
    {
      if (!strcmp (value, "butt"))
        s->stroke_linecap = MRG_LINE_CAP_BUTT;
      else if (!strcmp (value, "round"))
        s->stroke_linecap = MRG_LINE_CAP_ROUND;
      else if (!strcmp (value, "square"))
        s->stroke_linecap = MRG_LINE_CAP_SQUARE;
      else
        s->stroke_linecap = MRG_LINE_CAP_BUTT;
#if MRG_CAIRO
      cairo_set_line_cap (mrg_cr (mrg), s->stroke_linecap);
#endif
    }
  else if (!strcmp (name, "font-family"))
    {
      strncpy (s->font_family, value, 63);
      s->font_family[63]=0;

#if MRG_CAIRO
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
#endif
    }
  else if (!strcmp (name, "vertical-align"))
    {
      if (!strcmp (value, "middle"))
        s->vertical_align = MRG_VERTICAL_ALIGN_MIDDLE;
      if (!strcmp (value, "top"))
        s->vertical_align = MRG_VERTICAL_ALIGN_TOP;
      if (!strcmp (value, "sub"))
        s->vertical_align = MRG_VERTICAL_ALIGN_SUB;
      if (!strcmp (value, "super"))
        s->vertical_align = MRG_VERTICAL_ALIGN_SUPER;
      if (!strcmp (value, "bottom"))
        s->vertical_align = MRG_VERTICAL_ALIGN_BOTTOM;
      else
        s->vertical_align = MRG_VERTICAL_ALIGN_BASELINE;
    }
  else if (!strcmp (name, "cursor"))
  {
      if (!strcmp (value, "auto")) s->cursor = MRG_CURSOR_AUTO;
      else if (!strcmp (value, "alias")) s->cursor = MRG_CURSOR_ALIAS;
      else if (!strcmp (value, "all-scroll")) s->cursor = MRG_CURSOR_ALL_SCROLL;
      else if (!strcmp (value, "cell")) s->cursor = MRG_CURSOR_CELL;
      else if (!strcmp (value, "context-menu")) s->cursor = MRG_CURSOR_CONTEXT_MENU;
      else if (!strcmp (value, "col-resize")) s->cursor = MRG_CURSOR_COL_RESIZE;
      else if (!strcmp (value, "copy")) s->cursor = MRG_CURSOR_COPY;
      else if (!strcmp (value, "crosshair")) s->cursor = MRG_CURSOR_CROSSHAIR;
      else if (!strcmp (value, "default")) s->cursor = MRG_CURSOR_DEFAULT;
      else if (!strcmp (value, "e-resize")) s->cursor = MRG_CURSOR_E_RESIZE;
      else if (!strcmp (value, "ew-resize")) s->cursor = MRG_CURSOR_EW_RESIZE;
      else if (!strcmp (value, "help")) s->cursor = MRG_CURSOR_HELP;
      else if (!strcmp (value, "move")) s->cursor = MRG_CURSOR_MOVE;
      else if (!strcmp (value, "n-resize")) s->cursor = MRG_CURSOR_N_RESIZE;
      else if (!strcmp (value, "ne-resize")) s->cursor = MRG_CURSOR_NE_RESIZE;
      else if (!strcmp (value, "nesw-resize")) s->cursor = MRG_CURSOR_NESW_RESIZE;
      else if (!strcmp (value, "ns-resize")) s->cursor = MRG_CURSOR_NS_RESIZE;
      else if (!strcmp (value, "nw-resize")) s->cursor = MRG_CURSOR_NW_RESIZE;
      else if (!strcmp (value, "no-drop")) s->cursor = MRG_CURSOR_NO_DROP;
      else if (!strcmp (value, "none")) s->cursor = MRG_CURSOR_NONE;
      else if (!strcmp (value, "not-allowed")) s->cursor = MRG_CURSOR_NOT_ALLOWED;
      else if (!strcmp (value, "pointer")) s->cursor = MRG_CURSOR_POINTER;
      else if (!strcmp (value, "progress")) s->cursor = MRG_CURSOR_PROGRESS;
      else if (!strcmp (value, "row-resize")) s->cursor = MRG_CURSOR_ROW_RESIZE;
      else if (!strcmp (value, "s-resize")) s->cursor = MRG_CURSOR_S_RESIZE;
      else if (!strcmp (value, "se-resize")) s->cursor = MRG_CURSOR_SE_RESIZE;
      else if (!strcmp (value, "sw-resize")) s->cursor = MRG_CURSOR_SW_RESIZE;
      else if (!strcmp (value, "text")) s->cursor = MRG_CURSOR_TEXT;
      else if (!strcmp (value, "vertical-text")) s->cursor = MRG_CURSOR_VERTICAL_TEXT;
      else if (!strcmp (value, "w-resize")) s->cursor = MRG_CURSOR_W_RESIZE;
      else if (!strcmp (value, "cursor-wait")) s->cursor = MRG_CURSOR_WAIT;
      else if (!strcmp (value, "zoom-in")) s->cursor = MRG_CURSOR_ZOOM_IN;
      else if (!strcmp (value, "zoom-out")) s->cursor = MRG_CURSOR_ZOOM_OUT;
  }
  else if (!strcmp (name, "display"))
    {
      if (!strcmp (value, "hidden"))
        s->display = MRG_DISPLAY_HIDDEN;
      else if (!strcmp (value, "block"))
        s->display = MRG_DISPLAY_BLOCK;
      else if (!strcmp (value, "list-item"))
        s->display = MRG_DISPLAY_LIST_ITEM;
      else if (!strcmp (value, "inline-block"))
        s->display = MRG_DISPLAY_INLINE;
      else
        s->display = MRG_DISPLAY_INLINE;
    }
  else if (!strcmp (name, "position"))
    {
      if (!strcmp (value, "relative"))
        s->position = MRG_POSITION_RELATIVE;
      else if (!strcmp (value, "static"))
        s->position = MRG_POSITION_STATIC;
      else if (!strcmp (value, "absolute"))
        s->position = MRG_POSITION_ABSOLUTE;
      else if (!strcmp (value, "fixed"))
        s->position = MRG_POSITION_FIXED;
      else
        s->position = MRG_POSITION_STATIC;
    }
  else if (!strcmp (name, "direction"))
    {
      if (!strcmp (value, "rtl"))
        s->direction = MRG_DIRECTION_RTL;
      else if (!strcmp (value, "ltr"))
        s->direction = MRG_DIRECTION_LTR;
      else
        s->direction = MRG_DIRECTION_LTR;
    }
  else if (!strcmp (name, "unicode-bidi"))
    {
      if (!strcmp (value, "normal"))
        s->unicode_bidi = MRG_UNICODE_BIDI_NORMAL;
      else if (!strcmp (value, "embed"))
        s->unicode_bidi = MRG_UNICODE_BIDI_EMBED;
      else if (!strcmp (value, "bidi-override"))
        s->unicode_bidi = MRG_UNICODE_BIDI_BIDI_OVERRIDE;
      else
        s->unicode_bidi = MRG_UNICODE_BIDI_NORMAL;
    }
  else if (!strcmp (name, "text-align"))
    {
      if (!strcmp (value, "left"))
        s->text_align = MRG_TEXT_ALIGN_LEFT;
      else if (!strcmp (value, "right"))
        s->text_align = MRG_TEXT_ALIGN_RIGHT;
      else if (!strcmp (value, "justify"))
        s->text_align = MRG_TEXT_ALIGN_JUSTIFY;
      else if (!strcmp (value, "center"))
        s->text_align = MRG_TEXT_ALIGN_CENTER;
      else
        s->text_align = MRG_TEXT_ALIGN_LEFT;
    }
  else if (!strcmp (name, "text-decoration"))
    {
      if (!strcmp (value, "reverse"))
      {
#if 0
        MrgColor temp = s->color;
        s->text_decoration |= MRG_REVERSE;/* XXX: better do the reversing when drawing? */

        s->color = s->background_color;
        s->background_color = temp;
        {
          int t = mrg->state->fg;
          mrg->state->fg = mrg->state->bg;
          mrg->state->bg = t;
        }
#endif
      }
      else if (!strcmp (value, "underline"))
      {
        s->text_decoration|= MRG_UNDERLINE;
      }
      else if (!strcmp (value, "overline"))
      {
        s->text_decoration|= MRG_OVERLINE;
      }
      else if (!strcmp (value, "linethrough"))
      {
        s->text_decoration|= MRG_LINETHROUGH;
      }
      else if (!strcmp (value, "blink"))
      {
        s->text_decoration|= MRG_BLINK;
      }
      else if (!strcmp (value, "none"))
      {
        s->text_decoration ^= (s->text_decoration &
      (MRG_UNDERLINE|MRG_REVERSE|MRG_OVERLINE|MRG_LINETHROUGH|MRG_BLINK));
      }
    }
}

static void mrg_css_handle_property_pass1med (Mrg *mrg, const char *name,
                                              const char *value)
{
  MrgStyle *s = mrg_style (mrg);

  if (!strcmp (name, "width"))
  {
    if (!strcmp (value, "auto"))
    {
      s->width_auto = 1;
      s->width = 42;
    }
    else
    {
      s->width_auto = 0;
      s->width = mrg_parse_px_x (mrg, value, NULL);

      if (s->position == MRG_POSITION_FIXED) // XXX: seems wrong
      {
        //s->width -= s->border_left_width + s->border_right_width;
      }
    }
  }
}

static float deco_width (Mrg *mrg)
{
  MrgStyle *s = mrg_style (mrg);
  return s->padding_left + s->padding_right + s->border_left_width + s->border_right_width;
}

static void mrg_css_handle_property_pass2 (Mrg *mrg, const char *name,
                                           const char *value)
{
  /* this pass contains things that might depend on values
   * generated by the previous pass.
   */
  MrgStyle *s = mrg_style (mrg);


  if (!strcmp (name, "right"))
  {
    float width = s->width;

    s->right = mrg_parse_px_x (mrg, value, NULL);
    if (width == 0)
    {
      MrgGeoCache *geo = _mrg_get_cache (&mrg->html, s->id_ptr);
      if (geo->gen)
        width = geo->width;
      else
      {
        width = 8 * s->font_size;
        mrg_queue_draw (mrg, NULL);
      }
    }
    s->left = (mrg_width(mrg)-s->right) - width - s->border_left_width - s->padding_left - s->padding_right - s->border_right_width - s->margin_right;
  }
  else if (!strcmp (name, "bottom"))
  {
    float height = s->height;

    s->bottom = mrg_parse_px_y (mrg, value, NULL);

    if (height == 0)
    {
      MrgGeoCache *geo = _mrg_get_cache (&mrg->html, s->id_ptr);
      if (geo->gen)
        height = geo->height;
      else
      {
        height = 2 * s->font_size;
        mrg_queue_draw (mrg, NULL);
      }
    }
    s->top = mrg_height(mrg) - s->bottom - height - s->padding_top - s->border_top_width - s->padding_bottom - s->border_bottom_width - s->margin_bottom;
  }
}

#define MAXLEN 4096

enum
{
  MRG_CSS_PROPERTY_PARSER_STATE_NEUTRAL = 0,
  MRG_CSS_PROPERTY_PARSER_STATE_IN_NAME,
  MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_COLON,
  MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_VAL,
  MRG_CSS_PROPERTY_PARSER_STATE_IN_VAL
};

static void css_parse_properties (Mrg *mrg, const char *style,
  void (*handle_property) (Mrg *mrg, const char *name,
                           const char *value))
{
  const char *p;
  char name[MAXLEN] = "";
  char string[MAXLEN] = "";
  int name_l = 0;
  int string_l = 0;
  int state = MRG_CSS_PROPERTY_PARSER_STATE_NEUTRAL;

  if (!style)
    return;

  for (p = style; *p; p++)
  {
    switch (state)
    {
      case MRG_CSS_PROPERTY_PARSER_STATE_NEUTRAL:
        switch (*p)
        {
          case ' ':
          case '\t':
          case ';':
          case '\n':
          case '\r':
            break;
          default:
            name[name_l++]=*p;
            name[name_l]=0;
            state = MRG_CSS_PROPERTY_PARSER_STATE_IN_NAME;
            break;
        }
        break;
      case MRG_CSS_PROPERTY_PARSER_STATE_IN_NAME:
        switch (*p)
        {
          case ':':
            state = MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_VAL;
            break;
          case ' ':
          case '\n':
          case '\r':
          case '\t':
            state = MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_COLON;
            break;
          default:
            name[name_l++]=*p;
            name[name_l]=0;
            break;
        }
        break;
      case MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_COLON:
        switch (*p)
        {
          case ':':
            state = MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_VAL;
            break;
          default:
            break;
        }
        break;
      case MRG_CSS_PROPERTY_PARSER_STATE_EXPECT_VAL:
        switch (*p)
        {
          case ' ':
          case '\n':
          case '\r':
          case '\t':
            break;
          default:
            string[string_l++]=*p;
            string[string_l]=0;
            state = MRG_CSS_PROPERTY_PARSER_STATE_IN_VAL;
            break;
        }
        break;
      case MRG_CSS_PROPERTY_PARSER_STATE_IN_VAL:
        switch (*p)
        {
          case ';':
            handle_property (mrg, name, string);
            state = MRG_CSS_PROPERTY_PARSER_STATE_NEUTRAL;
            name_l = 0;
            name[0] = 0;
            string_l = 0;
            string[0] = 0;
            break;
          default:
            string[string_l++]=*p;
            string[string_l]=0;
            break;
        }
        break;
    }
  }
  if (name[0])
  handle_property (mrg, name, string);
}

void mrg_set_style (Mrg *mrg, const char *style)
{
  MrgStyle *s;

  css_parse_properties (mrg, style, mrg_css_handle_property_pass0);
  css_parse_properties (mrg, style, mrg_css_handle_property_pass1);
  css_parse_properties (mrg, style, mrg_css_handle_property_pass1med);

  s = mrg_style (mrg);

  if (s->position == MRG_POSITION_STATIC &&
      !s->float_)
  {
    if (s->width_auto &&
        (s->margin_right_auto || 
         s->margin_left_auto))
    {
      if (s->margin_left_auto && s->margin_right_auto)
      {
        s->margin_right_auto = 0;
      }
      else if (s->margin_left_auto)
        s->margin_left_auto = 0;
      else
        s->margin_right_auto = 0;
    }

    if ( s->margin_left_auto &&
        !s->width_auto &&
        !s->margin_right_auto)
    {
      s->margin_left =
        (mrg->state->edge_right - mrg->state->edge_left)
        - deco_width (mrg) - s->margin_right - s->width;
    }
    else if ( !s->margin_left_auto &&
              s->width_auto &&
              !s->margin_right_auto)
    {
      s->width =
        (mrg->state->edge_right - mrg->state->edge_left)
        - deco_width (mrg) - s->margin_left - s->margin_right;
    }
    else if ( !s->margin_left_auto &&
              !s->width_auto &&
              s->margin_right_auto)
    {
      s->margin_right=
        (mrg->state->edge_right - mrg->state->edge_left)
        - deco_width (mrg) - s->margin_left - s->width;
    }
    else if ( s->margin_left_auto &&
              !s->width_auto &&
              s->margin_right_auto)
    {
      s->margin_left =
      s->margin_right =
        ((mrg->state->edge_right - mrg->state->edge_left)
        - deco_width (mrg) - s->width)/2;
    }
  }

  css_parse_properties (mrg, style, mrg_css_handle_property_pass2);
}

void _mrg_init_style (Mrg *mrg)
{
  MrgStyle *s = mrg_style (mrg);

  s->text_decoration= 0;

  /* things not set here, are inherited from the parent style context,
   * more properly would be to rig up a fresh context, and copy inherited
   * values over, that would permit specifying inherit on any propery.
   */

  s->display = MRG_DISPLAY_INLINE;
  s->float_ = MRG_FLOAT_NONE;
  s->clear = MRG_CLEAR_NONE;
  s->overflow = MRG_OVERFLOW_VISIBLE;

  //s->stroke_width = 0.2;
#if 0
  s->stroke_color.red = 1;
  s->stroke_color.green = 0;
  s->stroke_color.blue = 1;
  s->stroke_color.alpha = 1;
  s->fill_color.red = 1;
  s->fill_color.green = 1;
  s->fill_color.blue = 0;
  s->fill_color.alpha = 1;
#endif

  /* this shouldn't be inherited? */
  s->background_color.red = 1;
  s->background_color.green = 1;
  s->background_color.blue = 1;
  s->background_color.alpha = 0;
  mrg->state->fg = 0;
  mrg->state->bg = 7;

  s->border_top_color.alpha = 0;
  s->border_left_color.alpha = 0;
  s->border_right_color.alpha = 0;
  s->border_bottom_color.alpha = 0;
  s->border_top_width = 0;
  s->border_left_width = 0;
  s->border_right_width = 0;
  s->border_bottom_width = 0;
  s->margin_top = 0;
  s->margin_left = 0;
  s->margin_right = 0;
  s->margin_bottom = 0;
  s->padding_top = 0;
  s->padding_left = 0;
  s->padding_right = 0;
  s->padding_bottom = 0;

  s->position = MRG_POSITION_STATIC;
  s->top = 0;
  s->left = 0;
  s->height = 0;
  s->width = 0;
  s->width_auto = 1;
}

void _mrg_set_style_properties (Mrg *mrg, const char *style_properties)
{
  _mrg_init_style (mrg);

  if (style_properties)
  {
    mrg_set_style (mrg, style_properties);
  }
}

void
mrg_set_stylef (Mrg *mrg, const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;
  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = malloc(needed);
  va_end (ap);
  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);
  mrg_set_style (mrg, buffer);
  free (buffer);
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-stylesheet.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"

MrgStyle *mrg_style (Mrg *mrg)
{
  return &mrg->state->style;
}

const char * html_css =
"html, address,\n"
"blockquote,\n"
"body, dd, div,\n"
"dl, dt, fieldset, form,\n"
"frame, frameset,\n"
"h1, h2, h3, h4,\n"
"h5, h6, noframes,\n"
"ol, p, ul, center,\n"
"dir, hr, menu, pre   { display: block; unicode-bidi: embed }\n"
"li              { display: list-item }\n"
"head            { display: none }\n"
"table           { display: table }\n"
"tr              { display: table-row }\n"
"thead           { display: table-header-group }\n"
"tbody           { display: table-row-group }\n"
"tfoot           { display: table-footer-group }\n"
"col             { display: table-column }\n"
"colgroup        { display: table-column-group }\n"
"td, th          { display: table-cell }\n"
"caption         { display: table-caption }\n"
"th              { font-weight: bolder; text-align: center }\n"
"caption         { text-align: center }\n"
"body            { margin: 0.5em }\n"
"h1              { font-size: 2em; margin: .67em 0 }\n"
"h2              { font-size: 1.5em; margin: .75em 0 }\n"
"h3              { font-size: 1.17em; margin: .83em 0 }\n"
"h4, p,\n"
"blockquote, ul,\n"
"fieldset, form,\n"
"ol, dl, dir,\n"
"menu            { margin: 1.12em 0 }\n"
"h5              { font-size: .83em; margin: 1.5em 0 }\n"
"h6              { font-size: .75em; margin: 1.67em 0 }\n"
"h1, h2, h3, h4,\n"
"h5, h6, b,\n"
"strong          { font-weight: bolder }\n"
"blockquote      { margin-left: 4em; margin-right: 4em }\n"
"i, cite, em,\n"
"var, address    { font-style: italic }\n"
"pre, tt, code,\n"
"kbd, samp       { font-family: monospace }\n"
"pre             { white-space: pre }\n"
"button, textarea,\n"
"input, select   { display: inline-block }\n"
"big             { font-size: 1.17em }\n"
"small, sub, sup { font-size: .83em }\n"
"sub             { vertical-align: sub }\n"
"sup             { vertical-align: super }\n"
"table           { border-spacing: 2px; }\n"
"thead, tbody,\n"
"tfoot           { vertical-align: middle }\n"
"td, th, tr      { vertical-align: inherit }\n"
"s, strike, del  { text-decoration: line-through }\n"
"hr              { border: 1px inset black }\n"
"ol, ul, dir,\n"
"menu, dd        { padding-left: 2.5em }\n"
"ol              { list-style-type: decimal }\n"
"ol ul, ul ol,\n"
"ul ul, ol ol    { margin-top: 0; margin-bottom: 0 }\n"
"u, ins          { text-decoration: underline }\n"
//"br:before     { content: \"\\A\"; white-space: pre-line }\n"
"center          { text-align: center }\n"
":link, :visited { text-decoration: underline }\n"
":focus          { outline: thin dotted invert }\n"

".cursor { color: white; background: black; } \n"

"br       { display: block; }\n"
"document { color : black; font-weight: normal; }\n"
"a        { color: blue; text-decoration: underline; }\n"
"a:hover  { background: black; color: white; text-decoration: underline; }\n"
"style, script { display: hidden; }\n"

"hr { margin-top:16px;font-size: 1px; }\n"  /* hack that works in one way, but shrinks top margin too much */
;

void mrg_stylesheet_clear (Mrg *mrg)
{
  if (mrg->stylesheet)
    mrg_list_free (&mrg->stylesheet);
  mrg_css_default (mrg);
}

void mrg_css_default (Mrg *mrg)
{
  char *error = NULL;

  mrg_stylesheet_add (mrg, html_css, NULL, NULL, NULL, &error);
  if (error)
  {
    fprintf (stderr, "Mrg css parsing error: %s\n", error);
  }

  mrg_stylesheet_add (mrg,

" bold { font-weight:bold; } \n"
" dim *, dim { opacity:0.5; } \n"
" underline *, underline{ text-decoration:underline; }\n"
" reverse *,selected *, reverse,selected { text-decoration:reverse;}\n"
" unhandled       { color:cyan; }\n"
" binding:key     { background-color:white; color:black; }\n"
" binding:label   { color:cyan; }\n"
      
      ,NULL, NULL, NULL, &error);

  if (error)
  {
    fprintf (stderr, "mrg css parsing error: %s\n", error);
  }
}

#define MRG_MAX_SELECTOR_LENGTH 16

typedef struct StyleEntry {
  char        *selector;
  MrgStyleNode parsed[MRG_MAX_SELECTOR_LENGTH];
  int          sel_len;
  char        *css;
  int          specificity;
} StyleEntry;

static void free_entry (StyleEntry *entry)
{
  free (entry->selector);
  free (entry->css);
  free (entry);
}

static int compute_specificity (const char *selector)
{
  const char *p;
  int n_id = 0;
  int n_class = 0;
  int n_tag = 0;
  int in_word = 0;

  for (p = selector; *p; p++)
  {
    switch (*p)
    {
      case ' ':
        in_word = 0;
        break;
      case '.':
        in_word = 1;
        n_class++;
        break;
      case '#':
        in_word = 1;
        n_id++;
        break;
      default:
        if (!in_word)
        {
          in_word = 1;
          n_tag++;
        }
    }
  }
  return n_id * 1000 + n_class * 100 + n_tag * 10;
}

static void mrg_parse_selector (Mrg *mrg, const char *selector, StyleEntry *entry)
{
  const char *p = selector;
  char section[256];
  int sec_l = 0;

  char type = ' ';

  for (p = selector; ; p++)
  {
    switch (*p)
    {
      case '.': case ':': case '#': case ' ': case 0:
        if (sec_l)
        {
          switch (type)
          {
            case ' ':
              entry->parsed[entry->sel_len].element = mrg_intern_string (section);
              break;
            case '#':
              entry->parsed[entry->sel_len].id = mrg_intern_string (section);
              break;
            case '.':
              {
                int i = 0;
                for (i = 0; entry->parsed[entry->sel_len].classes[i]; i++);
                entry->parsed[entry->sel_len].classes[i] = mrg_intern_string (section);
              }
              break;
            case ':':
              {
                int i = 0;
                for (i = 0; entry->parsed[entry->sel_len].pseudo[i]; i++);
                entry->parsed[entry->sel_len].pseudo[i] = mrg_intern_string (section);
              }
              break;
          }
        if (*p == ' ' || *p == 0)
        entry->sel_len ++;
        }
        if (*p == 0)
        {
          return;
        }
        section[(sec_l=0)] = 0;
        type = *p;
        break;
      default:
        section[sec_l++] = *p;
        section[sec_l] = 0;
        break;
    }
  }
}

static void mrg_stylesheet_add_selector (Mrg *mrg, const char *selector, const char *css)
{
  StyleEntry *entry = calloc (sizeof (StyleEntry), 1);
  entry->selector = strdup (selector);
  entry->css = strdup (css);
  entry->specificity = compute_specificity (selector);
  mrg_parse_selector (mrg, selector, entry);
  mrg_list_prepend_full (&mrg->stylesheet, entry, (void*)free_entry, NULL);
}

#define MAXLEN 4096

#define MAKE_ERROR \
 if (error)\
 {\
   char errbuf[128];\
   sprintf (errbuf, "%i unexpected %c at %i'  %c%c%c", __LINE__, *p, (p-css),\
     p[0], p[1], p[2]);\
   *error = strdup (errbuf);\
 }

#define MAX_RULES 64

typedef struct _MrgCssParseState MrgCssParseState;

struct _MrgCssParseState {
  int   state;
  char  rule[MAX_RULES][MAXLEN];
  int   rule_no ;
  int   rule_l[MAX_RULES];
  char  val[MAXLEN];
  int   val_l;
};

/* XXX: this funciton has no buffer bounds checking */
/* doesnt balance {} () [] and quotes */

enum
{
  NEUTRAL = 0,
  NEUTRAL_COMMENT,
  IN_RULE,
  RULE_COMMENT,
  IN_ARULE,
  ARULE_COMMENT,
  IN_IMPORT,
  IMPORT_COMMENT,
  EXPECT_COLON,
  COLON_COMMENT,
  EXPECT_VAL,
  EXPECT_VAL_COMMENT,
  IN_VAL,
  VAL_COMMENT,
};

static void _mrg_stylesheet_add (MrgCssParseState *ps, Mrg *mrg, const char *css, const char *uri_base,
                         int (*fetch) (const char *referer, const char *uri, char **contents, long *length, void *fetch_data),
                         void *fetch_data,
                         char **error)
{
  const char *p;
  if (!ps)
    ps = mrg->css_parse_state = calloc (sizeof (MrgCssParseState), 1);

  if (!css)
    return;

  for (p = css; *p; p++)
  {
    switch (ps->state)
    {
      case NEUTRAL_COMMENT:
      case RULE_COMMENT:
      case ARULE_COMMENT:
      case IMPORT_COMMENT:
      case VAL_COMMENT:
      case EXPECT_VAL_COMMENT:
      case COLON_COMMENT:      if (p[0] == '*' && p[1] == '/') { p++; ps->state--; } break;
      case NEUTRAL:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = NEUTRAL_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
          case ';':
            break;
          case '{':
          case '}':
            MAKE_ERROR;
            return;
          case '@':
            ps->state = IN_ARULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
          default:
            ps->state = IN_RULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case IN_ARULE:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = ARULE_COMMENT; } break;
          case '{':
            ps->state = IN_VAL; // should be AVAL for media sections...
            break;
          case '\n':
          case '\t':
          case ' ':
            if (!strcmp (ps->rule[0], "import"))
            {
              MAKE_ERROR;
            }
            else
            {
              ps->state = IN_IMPORT;
            }
            break;
          case ';':
          case '}':
            MAKE_ERROR;
            return;
          default:
            ps->state = IN_ARULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case IN_IMPORT:
        switch (*p)
        {
          int no;
          case '/': if (p[1] == '*') { p++; ps->state = IMPORT_COMMENT; } break;
          case ';':
            while (ps->val_l && (
                ps->val[ps->val_l-1] == ' ' ||
                ps->val[ps->val_l-1] == '\n' ||
                ps->val[ps->val_l-1] == '\t'))
              ps->val_l--;
            while (ps->val_l && ps->val[ps->val_l-1] != ')')
              ps->val_l--;
            if (ps->val[ps->val_l-1] == ')')
              ps->val_l--;
            if (ps->val[ps->val_l-1] == '"')
              ps->val_l--;
            ps->val[ps->val_l]=0;

            if(fetch){
              char *contents;
              long length;
              char *uri = ps->val;

              /* really ugly string trimming to get to import uri.. */
              while (uri[0]==' ') uri++;
              if (!memcmp (uri, "url", 3)) uri+=3;
              if (uri[0]=='(') uri++;
              if (uri[0]=='"') uri++;
      
              /* XXX: should parse out the media part, and check if we should
               * really include this file
               */

              fetch (uri_base, uri, &contents, &length, fetch_data);
              if (contents)
              {
                MrgCssParseState child_parser = {0,};
                _mrg_stylesheet_add (&child_parser, mrg, contents, uri, fetch, fetch_data, error);
                free (contents);
              }
            }

            for (no = 0; no < ps->rule_no+1; no ++)
              ps->rule[no][ps->rule_l[no]=0] = 0;
            ps->val_l = 0;
            ps->val[0] = 0;
            ps->rule_no = 0;
            ps->state = NEUTRAL;
            break;
          default:
            ps->state = IN_IMPORT;
            ps->val[ps->val_l++] = *p;
            ps->val[ps->val_l] = 0;
            break;

        }
        break;
      case IN_RULE:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = RULE_COMMENT; } break;
          case '{':
            ps->state = IN_VAL;
            break;
          case '\n':
          case '\t':
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = ' ';
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
          case ',':
            ps->state = NEUTRAL;
            ps->rule_no++;
            break;
          case ';':
          case '}':
            MAKE_ERROR;
            return;
          default:
            ps->state = IN_RULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case EXPECT_COLON:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = COLON_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
            break;
          case ':':
            ps->state = EXPECT_VAL;
            break;
          default:
            MAKE_ERROR;
            return;
        }
        break;
      case EXPECT_VAL:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = EXPECT_VAL_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
          case ';':
            break;
          case '{':
            ps->state = IN_VAL;
            break;
          default:
            MAKE_ERROR;
            return;
        }
        break;
      case IN_VAL:
        switch (*p)
        {
          int no;

          /* XXX: parsing grammar is a bit more complicated, wrt quotes and
           * brackets..
           */

          case '/': if (p[1] == '*') { p++; ps->state = VAL_COMMENT; } break;
          case '}':
            while (ps->val_l && (
                ps->val[ps->val_l-1] == ' ' ||
                ps->val[ps->val_l-1] == '\n' ||
                ps->val[ps->val_l-1] == '\t'))
              ps->val_l--;
            ps->val[ps->val_l]=0;
            for (no = 0; no < ps->rule_no+1; no ++)
            {
              while (ps->rule_l[no] && (
                  ps->rule[no][ps->rule_l[no]-1] == ' ' ||
                  ps->rule[no][ps->rule_l[no]-1] == '\n' ||
                  ps->rule[no][ps->rule_l[no]-1] == '\t'))
                ps->rule_l[no]--;
              ps->rule[no][ps->rule_l[no]]=0;

              mrg_stylesheet_add_selector (mrg, ps->rule[no], ps->val);
              ps->rule[no][ps->rule_l[no]=0] = 0;
            }

            ps->val_l = 0;
            ps->val[0] = 0;
            ps->rule_no = 0;
            ps->state = NEUTRAL;
            break;
          default:
            ps->state = IN_VAL;
            ps->val[ps->val_l++] = *p;
            ps->val[ps->val_l] = 0;
            break;

        }
        break;
    }
  }
}

void mrg_stylesheet_add (Mrg *mrg, const char *css, const char *uri_base,
                         int (*fetch) (const char *referer, const char *uri, char **contents, long *length, void *fetch_data),
                         void *fetch_data,
                         char **error)
{
  MrgCssParseState *ps = mrg->css_parse_state;
  _mrg_stylesheet_add (ps, mrg, css, uri_base, fetch, fetch_data, error);
}

typedef struct StyleMatch
{
  StyleEntry *entry;
  int score;
} StyleMatch;

static int compare_matches (const void *a, const void *b, void *d)
{
  const StyleMatch *ma = b;
  const StyleMatch *mb = a;
  return ma->score - mb->score;
}

static inline int _mrg_nth_match (const char *selector, int child_no)
{
  const char *tmp = selector + 10;
  int a = 0;
  int b = 0;

  if (!strcmp (tmp, "odd)"))
  {
    a = 2; b = 1;
  }
  else if (!strcmp (tmp, "even)"))
  {
    a = 2; b = 0;
  }
  else
  {
    if (strchr (tmp, 'n'))
    {
      a = atoi (tmp);
      b = atoi (strchr (tmp, 'n')+1);
    }
    else
    {
      b = atoi (tmp);
    }
  }

  if (!a)
    return child_no == b;
  else
    if (child_no == b ||
       ((child_no - b > 0) == (a>0)))
      return !((child_no - b) % a);

  return 0;
}

static inline int match_nodes (Mrg *mrg, MrgStyleNode *sel_node, MrgStyleNode *subject)
{
  int j, k;

  if (sel_node->element &&
      sel_node->element != subject->element)
    return 0;

  if (sel_node->id &&
      sel_node->id != subject->id)
    return 0;

  for (j = 0; sel_node->classes[j]; j++)
  {
    int found = 0;
    for (k = 0; subject->classes[k] && !found; k++)
    {
      if (sel_node->classes[j] == subject->classes[k])
        found = 1;
    }
    if (!found)
      return 0;
  }
  for (j = 0; sel_node->pseudo[j]; j++)
  {
    if (!strcmp (sel_node->pseudo[j], "first-child"))
    {
      if (!(_mrg_child_no (mrg) == 1))
        return 0;
    }
    else if (!strncmp (sel_node->pseudo[j], "nth-child(", 10))
    {
      if (!_mrg_nth_match (sel_node->pseudo[j], _mrg_child_no (mrg)))
        return 0;
    }
    else
    {
      int found = 0;

      for (k = 0; subject->pseudo[k] && !found; k++)
      {
        if (sel_node->pseudo[j] == subject->pseudo[k])
          found = 1;
      }
      if (!found)
        return 0;
    }
  }

  return 1;
}

static int mrg_selector_vs_ancestry (Mrg *mrg, StyleEntry *entry, MrgStyleNode **ancestry, int a_depth)
{
  int s = entry->sel_len - 1;

  /* right most part of selector must match */
  if (!match_nodes (mrg, &entry->parsed[s], ancestry[a_depth-1]))
    return 0;

  s--;
  a_depth--;

  if (s < 0)
    return 1;

  while (s >= 0)
  {
    int ai;
    int found_node = 0;

  /* XXX: deal with '>' */
    // if (entry->parsed[s].direct_ancestor) //
    for (ai = a_depth-1; ai >= 0 && !found_node; ai--)
    {
      if (match_nodes (mrg, &entry->parsed[s], ancestry[ai]))
        found_node = 1;
    }
    if (found_node)
    {
      a_depth = ai;
    }
    else
    {
      return 0;
    }
    s--;
  }

  return 1;
}

static int mrg_css_selector_match (Mrg *mrg, StyleEntry *entry, MrgStyleNode **ancestry, int a_depth)
{
  if (entry->selector[0] == '*' &&
      entry->selector[1] == 0)
    return entry->specificity;

  if (a_depth == 0)
    return 0;

  if (mrg_selector_vs_ancestry (mrg, entry, ancestry, a_depth))
    return entry->specificity;

  return 0;
}

static char *_mrg_css_compute_style (Mrg *mrg, MrgStyleNode **ancestry, int a_depth)
{
  MrgList *l;
  MrgList *matches = NULL;
  int totlen = 2;
  char *ret = NULL;

  for (l = mrg->stylesheet; l; l = l->next)
  {
    StyleEntry *entry = l->data;
    int score = 0;
    score = mrg_css_selector_match (mrg, entry, ancestry, a_depth);

    if (score)
    {
      StyleMatch *match = malloc (sizeof (StyleMatch));
      match->score = score;
      match->entry = entry;
      mrg_list_prepend_full (&matches, match, (void*)free, NULL);
      totlen += strlen (entry->css) + 1;
    }
  }

  if (matches)
  {
    MrgList *l;
    char *p;

    p = ret = malloc (totlen);

    mrg_list_sort (&matches, compare_matches, NULL);
    for (l = matches; l; l = l->next)
    {
      StyleMatch *match = l->data;
      StyleEntry *entry = match->entry;
      strcpy (p, entry->css);
      p += strlen (entry->css);
      strcpy (p, ";");
      p ++;
    }
    mrg_list_free (&matches);
  }
  return ret;
}

static int _mrg_get_ancestry (Mrg *mrg, MrgStyleNode **ancestry)
{
  int i, j;
  for (i = 0, j = 0; i <= mrg->state_no; i++)
    if (mrg->states[i].style_id)
    {
      ancestry[j++] = &(mrg->states[i].style_node);
    }
  ancestry[j] = NULL;
  return j;
}

char *_mrg_stylesheet_collate_style (Mrg *mrg)
{
  MrgStyleNode *ancestry[MRG_MAX_STYLE_DEPTH];
  int ancestors = _mrg_get_ancestry (mrg, ancestry);
  char *ret = _mrg_css_compute_style (mrg, ancestry, ancestors);
  return ret;
}

void  mrg_set_line_height (Mrg *mrg, float line_height)
{
  if (mrg_is_terminal (mrg))
    line_height = 1.0;
  mrg_style (mrg)->line_height = line_height;
}

float mrg_line_height (Mrg *mrg)
{
  return mrg_style (mrg)->line_height;
}

float mrg_rem             (Mrg *mrg)
{
  return mrg->rem;
}

void  mrg_set_rem         (Mrg *mrg, float em)
{
  mrg->rem = em;
}

float mrg_em (Mrg *mrg)
{
  return mrg->state->style.font_size;
}

void  mrg_set_em (Mrg *mrg, float em)
{
  if (mrg_is_terminal (mrg)) /* XXX: not _really_ neccesary, and disables huge-title fonts,.."and (small-font vnc)" */
    em = CPX / mrg->ddpx;
  mrg->state->style.font_size = em;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-text.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-internal.h"
/**************/

float _mrg_dynamic_edge_left (Mrg *mrg)
{
  if (mrg->state->wrap_edge_left)
    return mrg->state->wrap_edge_left (mrg, mrg->state->wrap_edge_data);
  return mrg->state->edge_left;
}

float _mrg_dynamic_edge_right (Mrg *mrg)
{
  if (mrg->state->wrap_edge_right)
    return mrg->state->wrap_edge_right (mrg, mrg->state->wrap_edge_data);
  return mrg->state->edge_right;
}

void _mrg_get_ascent_descent (Mrg *mrg, float *ascent, float *descent)
{
#if MRG_CAIRO
  cairo_scaled_font_t *scaled_font = mrg->scaled_font;
  cairo_font_extents_t extents;
  if (mrg_is_terminal (mrg))
  {
    if (ascent) *ascent = 0;
    if (descent) *descent= 0;
    return;
  }

  if (mrg->in_paint)
  {
    cairo_set_font_size (mrg_cr(mrg), mrg_style(mrg)->font_size);
    scaled_font = cairo_get_scaled_font (mrg_cr (mrg));
  }
  cairo_scaled_font_extents (scaled_font, &extents);

  if (ascent)  *ascent  = extents.ascent;
  if (descent) *descent = extents.descent;
#else
  if (ascent)  *ascent  = mrg_style(mrg)->font_size;
  if (descent) *descent = 0.0;
#endif
}

static float measure_word_width (Mrg *mrg, const char *word)
{
#if MRG_CAIRO
  cairo_scaled_font_t *scaled_font = mrg->scaled_font;
  cairo_text_extents_t extents;
  if (mrg_is_terminal (mrg))
    return mrg_utf8_strlen (word) * CPX / mrg->ddpx;
  if (mrg->in_paint)
  {
    cairo_set_font_size (mrg_cr (mrg), mrg_style(mrg)->font_size);
    scaled_font = cairo_get_scaled_font (mrg_cr (mrg));
  }
  cairo_scaled_font_text_extents (scaled_font, word, &extents);
  return extents.x_advance;
#else
  return mrg_utf8_strlen (word) * mrg_style (mrg)->font_size;
#endif
}

static float _mrg_text_shift (Mrg *mrg)
{
  //MrgStyle *style = mrg_style (mrg);
  float ascent, descent;
  _mrg_get_ascent_descent (mrg, &ascent, &descent);
  return (descent * 0.9); // XXX
}

/* x and y in cairo user units ; returns x advance in user units  */
float mrg_draw_string (Mrg *mrg, MrgStyle *style, 
                      float x, float y,
                      const char *string,
                      int utf8_len)
{
  double new_x, old_x;
  char *temp_string = NULL;
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
#endif

  if (utf8_len < 0)
    utf8_len = mrg_utf8_strlen (string);

  if (mrg_utf8_strlen (string) != utf8_len)
  {
    const char *t;
    int i;

    temp_string = strdup (string);
    for (i = 0, t = temp_string ;i < utf8_len && *t; i++)
    {
      t += mrg_utf8_len (*t);
    }
    *(char *)t = 0;
    string = temp_string;
  }

  if (mrg_is_terminal (mrg) && mrg_em (mrg) <= CPX * 4 / mrg->ddpx)
  {
    const char *t;
    int i;

    /* XXX: include transforms */
    int offset;
    double u = x , v = y;
#if MRG_CAIRO
    cairo_matrix_t matrix;
    cairo_get_matrix (mrg_cr (mrg), &matrix);
    cairo_matrix_transform_point (&matrix, &u, &v);
#endif

    //u = floor(u);
    //v = floor(v);
    
    offset = (int)(v/CPX) * ((int)(mrg->width/CPX) * 4) + (int)(u/CPX) * 4;

    old_x = x;
    for (i = 0, t = string; *t; i++)
    {
      if ( v >= 0 && u >= 0 &&
          (int)u/CPX < (int)(mrg->width/CPX) &&
          (int)v/CPX < (int)(mrg->height/CPX))
      {
        int styleno = offset/4;
        memcpy (&mrg->glyphs[offset], t, mrg_utf8_len (*t));
        mrg->styles[styleno] = mrg->state->fg +
                               mrg->state->bg * 8 +
                               (mrg->state->style.text_decoration & 
                                (MRG_BOLD|MRG_DIM|MRG_UNDERLINE|MRG_REVERSE)) * 64;;
      }
      t += mrg_utf8_len (*t);
      offset += 4;
      x += CPX / mrg->ddpx;
    }
    new_x = x;
  }
  else if (mrg->in_paint)
  {
#if MRG_CAIRO
    cairo_set_font_size (cr, style->font_size);

    if (style->text_stroke_width > 0.01)
    {
      mrg_cairo_set_source_color (cr, &style->text_stroke_color);
      cairo_move_to   (cr, x, y - _mrg_text_shift (mrg));
      cairo_text_path (cr, string);
      cairo_set_line_width (cr, style->text_stroke_width);
      cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
      cairo_stroke (cr);
    }

    mrg_cairo_set_source_color (cr, &style->color);
    cairo_move_to   (cr, x, y - _mrg_text_shift (mrg));
    cairo_get_current_point (cr, &old_x, NULL);
    cairo_show_text (cr, string);
    cairo_get_current_point (cr, &new_x, NULL);
#else
    new_x = old_x = 0;
#endif
  }

  if (mrg->text_listen_cb)
  {
    float em = mrg_em (mrg);

    mrg_listen (mrg,
                mrg->text_listen_types,
                old_x, y - em, new_x - old_x + 1, em * mrg->state->style.line_height,
                mrg->text_listen_cb,
                mrg->text_listen_data1,
                mrg->text_listen_data2);
  }

  if (temp_string)
    free (temp_string);

  return new_x - old_x;
}

float mrg_addstr (Mrg *mrg, float x, float y, const char *string, int utf8_length);


float paint_span_bg_final (Mrg   *mrg, float x, float y,
                           float  width)
{
  MrgStyle *style = mrg_style (mrg);
  cairo_t *cr = mrg_cr (mrg);
  if (style->display != MRG_DISPLAY_INLINE)
    return 0.0;

  if (style->background_color.alpha > 0.001)
  {
    cairo_save (cr);
    cairo_rectangle (cr, x,
                         y - mrg_em (mrg) * style->line_height +_mrg_text_shift (mrg)
                         ,
                         width + style->padding_right,
                         mrg_em (mrg) * style->line_height);
    mrg_cairo_set_source_color (cr, &style->background_color);
    cairo_fill (cr);
    cairo_restore (cr);
  }

  _mrg_border_top_r (mrg, x, y - mrg_em (mrg) , width, mrg_em (mrg));
  _mrg_border_bottom_r (mrg, x, y - mrg_em (mrg), width, mrg_em (mrg));
  _mrg_border_right (mrg, x, y - mrg_em (mrg), width, mrg_em (mrg));

  return style->padding_right + style->border_right_width;
}

float paint_span_bg (Mrg   *mrg, float x, float y,
                     float  width)
{
  MrgStyle *style = mrg_style (mrg);
  cairo_t *cr = mrg_cr (mrg);
  float left_pad = 0.0;
  float left_border = 0.0;
  if (style->display != MRG_DISPLAY_INLINE)
    return 0.0;

  if (!mrg->state->span_bg_started)
  {
    left_pad = style->padding_left;
    left_border = style->border_left_width;
    mrg->state->span_bg_started = 1;
  }


  if (style->background_color.alpha > 0.001)
  {
    cairo_save (cr);
    cairo_rectangle (cr, x + left_border,
                         y - mrg_em (mrg) * style->line_height +_mrg_text_shift (mrg)
                         ,
                         width + left_pad,
                         mrg_em (mrg) * style->line_height);
    mrg_cairo_set_source_color (cr, &style->background_color);
    cairo_fill (cr);
    cairo_restore (cr);
  }

  if (left_pad || left_border)
  {
    _mrg_border_left (mrg, x + left_pad + left_border, y - mrg_em (mrg) , width, mrg_em (mrg));
    _mrg_border_top_l (mrg, x + left_pad + left_border, y - mrg_em (mrg) , width , mrg_em (mrg));
    _mrg_border_bottom_l (mrg, x + left_pad + left_border, y - mrg_em (mrg), width , mrg_em (mrg));
  }
  else
  {
    _mrg_border_top_m (mrg, x, y - mrg_em (mrg) , width, mrg_em (mrg));
    _mrg_border_bottom_m (mrg, x, y - mrg_em (mrg), width, mrg_em (mrg));
  }

  return left_pad + left_border;
}


float
mrg_addstr (Mrg *mrg, float x, float y, const char *string, int utf8_length)
{
  float wwidth = measure_word_width (mrg, string);
  float left_pad;
  left_pad = paint_span_bg (mrg, x, y, wwidth);
  mrg_draw_string (mrg, &mrg->state->style, x + left_pad, y, string, utf8_length);
  return wwidth + left_pad;
}

/******** end of core text-drawing primitives **********/

#if 0
void mrg_xy (Mrg *mrg, float x, float y)
{
  mrg->x = x * mrg_em (mrg);
  mrg->y = y * mrg_em (mrg);
}
#endif

void mrg_set_xy (Mrg *mrg, float x, float y)
{
  mrg->x = x;
  mrg->y = y;
  mrg->state->overflowed = 0;
}

float mrg_x (Mrg *mrg)
{
  return mrg->x;
}

float mrg_y (Mrg *mrg)
{
  return mrg->y;
}

void mrg_set_wrap_skip_lines (Mrg *mrg, int skip_lines);
void mrg_set_wrap_max_lines  (Mrg *mrg, int max_lines);

void mrg_set_wrap_skip_lines (Mrg *mrg, int skip_lines)
{
    mrg->state->skip_lines = skip_lines;
}

void mrg_set_wrap_max_lines  (Mrg *mrg, int max_lines)
{
    mrg->state->max_lines = max_lines;
}

//#define SNAP

static void _mrg_nl (Mrg *mrg)
{
  mrg->x = _mrg_dynamic_edge_left(mrg);
  mrg->y += mrg->state->style.line_height * mrg_em (mrg);
#ifdef SNAP
  float em = mrg_em (mrg);  /* XXX: a global body-line spacing 
                               snap is better grid design */
  mrg->x = ceil (mrg->x / em) * em;
  mrg->y = ceil (mrg->y / em) * em;
#endif

  if (mrg->y >= 
      mrg->state->edge_bottom - mrg->state->style.padding_bottom)
  {
    mrg->state->overflowed=1;
  }

  if (mrg->state->post_nl)
    mrg->state->post_nl (mrg, mrg->state->post_nl_data, 0);
}

static void _mrg_spaces (Mrg *mrg, int count)
{
  while (count--)
    {
     if (mrg->state->style.print_symbols)
        mrg->x+=mrg_addstr (mrg, mrg->x, mrg->y, "␣", -1);
     else
     {
        float diff = mrg_addstr (mrg, mrg->x, mrg->y, " ", 1);

        if (mrg_is_terminal (mrg) && mrg_em (mrg) <= CPX * 4 / mrg->ddpx)
        {
        }
        else
        {
          if (mrg->state->style.text_decoration & MRG_REVERSE)
          {
#if MRG_CAIRO
            cairo_t *cr = mrg_cr (mrg);
            cairo_rectangle (cr, mrg->x + diff*0.1, mrg->y + mrg_em(mrg)*0.2, diff*0.8, -mrg_em (mrg)*1.1);
            cairo_set_source_rgb (cr, 1,1,1);
            cairo_fill (cr);
#endif
          }
        }
        mrg->x += diff;
     }
    }
}

#define EMIT_NL() \
    do {wraps++; \
    if (wraps >= max_lines)\
      return wraps;\
    if (skip_lines-- <=0)\
      {\
         if (print) { if (gotspace)\
             _mrg_spaces (mrg, 1);\
         if (cursor_start == pos -1 && cursor_start>0 && (*mrg->edited) == data)\
           {\
             mrg_start (mrg, ".cursor", NULL);\
             _mrg_spaces (mrg, 1);\
             _mrg_nl (mrg);\
             mrg_end (mrg);\
           }\
         else\
           _mrg_nl (mrg);\
         } else _mrg_nl (mrg);\
      }\
    if (skip_lines<=0)\
      mrg_set_xy (mrg, _mrg_dynamic_edge_left(mrg), mrg_y (mrg));}while(0)

#define EMIT_NL2() \
    do {\
    if (skip_lines-- <=0)\
      {\
         if (print) {if (gotspace)\
             _mrg_spaces (mrg, 1);\
         if (cursor_start == *pos -1 && cursor_start>0 && (*mrg->edited) == data)\
           {\
             mrg_start (mrg, ".cursor", NULL);\
             _mrg_spaces (mrg, 1);\
             _mrg_nl (mrg);\
             mrg_end (mrg);\
           }\
         else\
           _mrg_nl (mrg);\
         } else _mrg_nl (mrg);\
      }\
    if (skip_lines<=0)\
      mrg_set_xy (mrg, _mrg_dynamic_edge_left(mrg), mrg_y (mrg));}while(0)



static void mrg_get_edit_state (Mrg *mrg, 
     float *x, float *y, float *s, float *e,
     float *em_size)
{
  if (x) *x = mrg->e_x;
  if (y) *y = mrg->e_y;
  if (s) *s = mrg->e_ws;
  if (e) *e = mrg->e_we;
  if (em_size) *em_size = mrg->e_em;
}


static void emit_word (Mrg *mrg,
                       int  print,
                       const char *data,
                       const char *word,
                       int         max_lines,
                       int         skip_lines,
                       int         cursor_start,
                       float     *retx,
                       float     *rety,
                       int        *pos,
                       int        *wraps,
                       int        *wl,
                       int         c,
                       int         gotspace)
{
    float len = mrg_utf8_strlen (word);
    float wwidth = measure_word_width (mrg, word);

    if (mrg->x + wwidth >= _mrg_dynamic_edge_right (mrg))
    {
      EMIT_NL2();
    }

    if (mrg->x != mrg_edge_left(mrg) && gotspace)
      { 
        if ((skip_lines<=0)) 
          { 
            if (cursor_start == *pos-1 && cursor_start>=0 && mrg->edited && (*mrg->edited) == data)
            { 
              if (print) { 
               mrg_start (mrg, ".cursor", NULL);
               _mrg_spaces (mrg, 1); 
               mrg_end (mrg);
              } else { 
               mrg->x += measure_word_width (mrg, " ");
              }
            }
            else 
              {
                if (print){
                  if (mrg->state->style.print_symbols)
                    {
                      mrg_start (mrg, "dim", NULL);
                      mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, "␣", -1);
                      mrg_end (mrg);
                    }
                  else
                    _mrg_spaces (mrg, 1);
                } else {
                  if (mrg->state->style.print_symbols)
                  {
                    mrg->x += measure_word_width (mrg, "␣");
                  }
                  else
                  {
                    mrg->x += measure_word_width (mrg, " ");
                  }
                }
              } 
          }
      } 
    if ((skip_lines<=0)) {
      if (print){if (cursor_start >= *pos && *pos + len > cursor_start && mrg->edited && (*mrg->edited) == data)
        { 
#if 0  // XXX: there is a bug in mrg_addstr it doesn't respect the length argument 
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, word, cursor_start - *pos);
          mrg_start (mrg, ".cursor", NULL);
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, mrg_utf8_skip (word, cursor_start - *pos), 1);
          mrg_end (mrg);
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, mrg_utf8_skip (word, cursor_start - *pos + 1), len - (cursor_start - *pos) - 1);
#else

          char *dup, *dup2, *dup3;

          dup = strdup (word);
          dup2 = strdup (mrg_utf8_skip (dup, cursor_start - *pos));
          dup3 = strdup (mrg_utf8_skip (dup, cursor_start - *pos + 1));
          *((char*)mrg_utf8_skip (dup,  cursor_start - *pos)) = 0;
          *((char*)mrg_utf8_skip (dup2, 1)) = 0;

          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, dup, -1);
          mrg_start (mrg, ".cursor", NULL);
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, dup2, -1);
          mrg_end (mrg);
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, dup3, -1);

          free (dup);
          free (dup2);
          free (dup3);
#endif
        }
      else
        {
          mrg->x += mrg_addstr (mrg, mrg->x, mrg->y, word, len); 
        }
      } else {
          mrg->x += wwidth;
      }
    }
    *pos += len;
    *wl = 0;
}

static int mrg_print_wrap (Mrg        *mrg,
                           int         print,
                           const char *data, int length,
                           int         max_lines,
                           int         skip_lines,
                           int         cursor_start,
                           float     *retx,
                           float     *rety)
{
  char word[400]="";
  int wl = 0;
  int c;
  int wraps = 0;
  int pos;
  int gotspace = 0;

  if (mrg->state->overflowed)
  {
    return 0;
  }

  pos = 0;

  if (max_lines <= 0)
    max_lines = 4096;
  if (retx)
    *retx = -1;

  if (mrg->edited && *mrg->edited == data && print)
    {
      mrg->e_x = mrg->x;
      mrg->e_y = mrg->y;
      mrg->e_ws = mrg_edge_left(mrg);
      mrg->e_we = mrg_edge_right(mrg);
      mrg->e_em = mrg_em (mrg);

#if MRG_CAIRO
      if (mrg->scaled_font)
        cairo_scaled_font_destroy (mrg->scaled_font);
      cairo_set_font_size (mrg_cr (mrg), mrg_style(mrg)->font_size);
      mrg->scaled_font = cairo_get_scaled_font (mrg_cr (mrg));
      cairo_scaled_font_reference (mrg->scaled_font);
#endif
    }

  for (c = 0 ; c < length && data[c] && ! mrg->state->overflowed; c++)
    switch (data[c])
      {
        case '\n':
          if (wl)
            {
              emit_word (mrg, print, data, word, 
                         max_lines, skip_lines,
                         cursor_start, retx,
                         rety, &pos, &wraps, &wl, c, gotspace);
            }
          pos++;

          if (mrg->state->style.print_symbols && print)
          {
            mrg_start (mrg, "dim", NULL);
            mrg->x+=mrg_addstr (mrg, mrg->x, mrg->y, "¶", -1);\
            mrg_end (mrg);
          }
          EMIT_NL();
          gotspace = 0;
          break;
        case ' ':
          if (wl == 0)
            {
              if (cursor_start == pos-1 && cursor_start>=0 && mrg->edited && (*mrg->edited) == data)
                {
                  if (print)
                  {
                    mrg_start (mrg, ".cursor", NULL);
                    _mrg_spaces (mrg, 1);
                    mrg_end (mrg);
                  }
                  else
                    mrg->x+=mrg_addstr (mrg, mrg->x, mrg->y, " ", -1);
                }
              else
                {
                  if (mrg->state->style.print_symbols)
                    {
                      mrg_start (mrg, "dim", NULL);
                      mrg->x+=mrg_addstr (mrg, mrg->x, mrg->y, "␣", -1);
                      mrg_end (mrg);
                    }
                  else
                    {
                      mrg->x+=mrg_addstr (mrg, mrg->x, mrg->y, " ", -1);
                    }
                }
            }
          else
            {
              emit_word (mrg, print, data, word, 
                         max_lines, skip_lines,
                         cursor_start, retx,
                         rety, &pos, &wraps, &wl, c, gotspace);
            }
          pos++;
          if (retx && *retx < 0 && pos >= cursor_start)
            {
              float tailwidth;
              const char *rest = &word[mrg_utf8_strlen (word) - (pos-cursor_start)];
              if (mrg_is_terminal (mrg))
                tailwidth = (pos-cursor_start -1) * CPX / mrg->ddpx;
              else
                tailwidth = measure_word_width (mrg, rest);
              *retx = mrg->x - tailwidth;
              *rety = mrg->y;
              return pos;
            }
          gotspace = 1;
          break;
        default:
          word[wl++]= data[c];
          word[wl]  = '\0';
          break;
      }
  if (wl) /* orphaned word for last line. */
    {
      emit_word (mrg, print, data, word, 
                 max_lines, skip_lines,
                 cursor_start, retx,
                 rety, &pos, &wraps, &wl, c, gotspace);
    }
   /* cursor at end */
   if (cursor_start == pos && cursor_start>=0 && mrg->edited && (*mrg->edited) == data)
    {
      if (print)
      {
        mrg_start (mrg, ".cursor", NULL);
        _mrg_spaces (mrg, 1);
        mrg_end (mrg);
      }
      else
        mrg->x += measure_word_width (mrg, " ");
    }
  if (retx && *retx < 0 && pos >= cursor_start)
    {
       *retx = mrg->x; 
       *rety = mrg->y;
      return pos;
    }
  return wraps;
}

int mrg_print_get_xy (Mrg *mrg, const char *string, int no, float *x, float *y)
{
  int ret;
  if (!string)
    return 0;

  if (mrg_edge_left(mrg) != mrg_edge_right(mrg))
    {
      float ox, oy;
      ox = mrg->x;
      oy = mrg->y;
      ret = mrg_print_wrap (mrg, 0, string, strlen (string), mrg->state->max_lines,
                             mrg->state->skip_lines, no, x, y);
      mrg->x = ox;
      mrg->y = oy;
      return ret;
    }
  if (y) *y = mrg->y;
  if (x) *x = mrg->x + no; // XXX: only correct for nct

  return 0;
}

#include <math.h>

int mrg_print (Mrg *mrg, const char *string)
{
  float ret;

#ifdef SNAP
  float em = mrg_em (mrg);  /* XXX: a global body-line spacing 
                               snap is better grid design */
  mrg->x = ceil (mrg->x / em) * em;
  mrg->y = ceil (mrg->y / em) * em;
#endif

  if (mrg->state->style.display == MRG_DISPLAY_HIDDEN)
    return 0;

  if (!string)
    return 0;

  if (mrg_edge_left(mrg) != mrg_edge_right(mrg))
   return mrg_print_wrap (mrg, 1, string, strlen (string), mrg->state->max_lines, mrg->state->skip_lines, mrg->cursor_pos, NULL, NULL);

  ret  = mrg_addstr (mrg, mrg->x, mrg->y, string, mrg_utf8_strlen (string));
  mrg->x += ret;
  return ret;
}

void _mrg_text_init (Mrg *mrg)
{
  mrg->state->style.line_height = 1.0;
  mrg->state->style.print_symbols = 0;
}


void  mrg_text_listen_full (Mrg *mrg, MrgType types,
                            MrgCb cb, void *data1, void *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
                      void    *finalize_data)
{
  mrg->text_listen_types = types;
  mrg->text_listen_cb = cb;
  mrg->text_listen_data1 = data1;
  mrg->text_listen_data2 = data2;
  mrg->text_listen_finalize = finalize;
  mrg->text_listen_finalize_data = finalize_data;
}

void  mrg_text_listen (Mrg *mrg, MrgType types,
                       MrgCb cb, void *data1, void *data2)
{
  mrg_text_listen_full (mrg, types, cb, data1, data2, NULL, NULL);
}

void  mrg_text_listen_done (Mrg *mrg)
{
  mrg->text_listen_cb = NULL;
}

static int cmd_home (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  mrg->cursor_pos = 0;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_end (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  mrg->cursor_pos = mrg_utf8_strlen (*mrg->edited);
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_backspace (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  char *new;
  const char *rest = mrg_utf8_skip (*mrg->edited, mrg->cursor_pos);
  const char *mark = mrg_utf8_skip (*mrg->edited, mrg->cursor_pos-1);

  if (mrg->cursor_pos <= 0)
    {
      mrg->cursor_pos = 0;
    }
  else
    {
      new = malloc (strlen (*mrg->edited) + 1);
      memcpy (new, *mrg->edited, ((mark - *mrg->edited)));
      memcpy (new + ((mark - *mrg->edited)), rest, strlen (rest));
      new [strlen (*mrg->edited)-(rest-mark)] = 0;
      mrg->update_string (mrg, mrg->edited, new, mrg->update_string_user_data);
      free (new);
      mrg->cursor_pos--;
    }
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_delete (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  char *new;
  const char *rest = mrg_utf8_skip (*mrg->edited, mrg->cursor_pos+1);
  const char *mark = mrg_utf8_skip (*mrg->edited, mrg->cursor_pos);

  new = malloc (strlen (*mrg->edited) + 1);
  memcpy (new, *mrg->edited, ((mark - *mrg->edited)));
  memcpy (new + ((mark - *mrg->edited)), rest, strlen (rest));
  new [strlen (*mrg->edited)-(rest-mark)] = 0;

  mrg->update_string (mrg, mrg->edited, new, mrg->update_string_user_data);
  free (new);
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_up (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  float e_x, e_y, e_s, e_e, e_em;
  float cx, cy;
  mrg_get_edit_state (mrg, &e_x, &e_y, &e_s, &e_e, &e_em);

  mrg_set_edge_left (mrg, e_s - mrg->state->style.padding_left);
  mrg_set_edge_right (mrg, e_e + mrg->state->style.padding_right);



  mrg_set_xy (mrg, e_x, e_y);
  mrg_print_get_xy (mrg, *mrg->edited, mrg->cursor_pos, &cx, &cy);

  {
    int no;
    for (no = mrg_utf8_strlen (*mrg->edited) - 1; no>=0; no--)
    {
      float x, y;
      mrg_set_xy (mrg, e_x, e_y);
      mrg_print_get_xy (mrg, *mrg->edited, no, &x, &y);
      if (x <= cx && y <= cy - 1)
      {
        mrg->cursor_pos = no;
        break;
      }
    }
  }

  if (mrg->cursor_pos < 0)
    mrg->cursor_pos = 0;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_down (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  float e_x, e_y, e_s, e_e, e_em;
  float cx, cy;
 
  mrg_get_edit_state (mrg, &e_x, &e_y, &e_s, &e_e, &e_em);
  mrg_set_edge_left (mrg, e_s - mrg->state->style.padding_left);
  mrg_set_edge_right (mrg, e_e + mrg->state->style.padding_right);
  mrg_set_xy (mrg, e_x, e_y);
  mrg_print_get_xy (mrg, *mrg->edited, mrg->cursor_pos, &cx, &cy);

  {
    int no;
    for (no = 0; no < mrg_utf8_strlen (*mrg->edited); no++)
    {
      float x, y;
      mrg_set_xy (mrg, e_x, e_y);
      mrg_print_get_xy (mrg, *mrg->edited, no, &x, &y);
      if (x >= cx && y >= cy + 1)
      {
        mrg->cursor_pos = no;
        break;
      }
    }
  }

  if (mrg->cursor_pos >= mrg_utf8_strlen (*mrg->edited))
    mrg->cursor_pos = mrg_utf8_strlen (*mrg->edited) - 1;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_left (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  mrg->cursor_pos--;
  if (mrg->cursor_pos < 0)
    mrg->cursor_pos = 0;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_right (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  mrg->cursor_pos++;
  if (mrg->cursor_pos > mrg_utf8_strlen (*mrg->edited))
    mrg->cursor_pos = mrg_utf8_strlen (*mrg->edited);
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static void add_utf8 (Mrg *mrg, const char *string)
{
  char *new;
  const char *rest;
  rest = mrg_utf8_skip (*mrg->edited, mrg->cursor_pos);

  new = malloc (strlen (*mrg->edited) + strlen (string) + 1);
  memcpy (new, *mrg->edited, (rest-*mrg->edited));
  memcpy (new + (rest-*mrg->edited), string,  strlen (string));
  memcpy (new + (rest-*mrg->edited) + strlen (string),
          rest, strlen (rest));
  new [strlen (string) + strlen (*mrg->edited)] = 0;
  mrg->update_string (mrg, mrg->edited, new, mrg->update_string_user_data);
  free (new);
  mrg_queue_draw (mrg, NULL);
  mrg->cursor_pos++;
}

static int cmd_unhandled (MrgEvent *event, void *data1, void *data2)
{
  if (mrg_utf8_strlen (event->key_name) != 1)
    return 0;

  add_utf8 (event->mrg, event->key_name);
  return 1;
}

#if 0
static int cmd_space (MrgEvent *event, void *data1, void *data2)
{
  if (!mrg_utf8_strlen (event->key_name) == 1)
    return 0;

  add_utf8 (event->mrg, " ");
  return 1;
}
#endif

static int cmd_return (MrgEvent *event, void *data1, void *data2)
{
  if (!mrg_utf8_strlen (event->key_name) == 1)
    return 0;

  add_utf8 (event->mrg, "\n");
  return 1;
}

static int cmd_escape (MrgEvent *event, void *data, void *data2)
{
  mrg_edit_string (event->mrg, NULL, NULL, NULL);
  return 0;
}

void mrg_text_edit_bindings (Mrg *mrg)
{
  mrg_add_binding (mrg, "escape",  NULL, "Stop editing", cmd_escape,      NULL);
  mrg_add_binding (mrg, "return",    NULL, "insert newline", cmd_return,    NULL);
  //mrg_add_binding (mrg, "space",     NULL, "label", cmd_space,     NULL);
  mrg_add_binding (mrg, "home",      NULL, "Go to start of text", cmd_home,      NULL);
  mrg_add_binding (mrg, "end",       NULL, "Go to end of text", cmd_end,       NULL);
  mrg_add_binding (mrg, "left",      NULL, "Move cursor left", cmd_left,      NULL);
  mrg_add_binding (mrg, "right",     NULL, "Move cursor right", cmd_right,     NULL);
  mrg_add_binding (mrg, "up",        NULL, "Move cursor up", cmd_up,        NULL);
  mrg_add_binding (mrg, "down",      NULL, "Move cursor down", cmd_down,      NULL);
  mrg_add_binding (mrg, "backspace", NULL, "Remove character left of cursor", cmd_backspace, NULL);
  mrg_add_binding (mrg, "delete",    NULL, "Remove selected character", cmd_delete,    NULL);
  mrg_add_binding (mrg, "unhandled", NULL, "Insert if key name is one char", cmd_unhandled, NULL);
}

void mrg_edit_string (Mrg *mrg, char **string,
                      void (*update_string)(Mrg *mrg,
                        char **string_loc,
                        const char *new_string,
                        void  *user_data),
                      void *user_data)
{
  mrg->edited = string;
  mrg->update_string = update_string;
  mrg->update_string_user_data = user_data;
  mrg->cursor_pos = mrg_utf8_strlen (*string);
  mrg_queue_draw (mrg, NULL);
}

void
mrg_printf (Mrg *mrg, const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;
  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = malloc(needed);
  va_end (ap);
  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);
  mrg_print (mrg, buffer);
  free (buffer);
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-util.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


#include <sys/time.h>
#include <string.h>
// bundled include of mrg.h"

static struct timeval start_time;

#define usecs(time)    ((time.tv_sec - start_time.tv_sec) * 1000000 + time.     tv_usec)

static void
init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

long
_mrg_ticks (void)
{
  struct timeval measure_time;
  init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}

#undef usecs

int mrg_quit_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_quit (event->mrg);
  return 1;
}

void mrg_cairo_set_source_color (cairo_t *cr, MrgColor *color)
{
#if MRG_CAIRO
  cairo_set_source_rgba (cr, color->red, color->green, color->blue, color->alpha);
#endif
}

void mrg_color_set (MrgColor *color, float red, float green, float blue, float alpha)
{
  color->red = red;
  color->green = green;
  color->blue = blue;
}

/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mr.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-http.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
#ifndef MRG_HTTP_H
#define MRG_HTTP_H

char *_mrg_http_post (const char *ip,
                      const char *hostname,
                      int         port,
                      const char *path,
                      const char *body,
                      int         length,
                      int        *ret_length);
char *_mrg_http (const char *ip,
                 const char *hostname,
                 int         port,
                 const char *path,
                 int        *length);

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-string.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#ifndef MRG_STRING_H
#define mrg_STRING_H

typedef struct _MrgString MrgString;

struct _MrgString
{
  char *str;
  int   length;
  int   allocated_length;
}  __attribute((packed));

MrgString   *mrg_string_new            (const char *initial);
void         mrg_string_free           (MrgString *string, int freealloc);
char        *mrg_string_dissolve       (MrgString *string);
const char  *mrg_string_get            (MrgString *string);
int          mrg_string_get_length     (MrgString *string);
void         mrg_string_set            (MrgString *string, const char *new_string);
void         mrg_string_clear          (MrgString *string);
void         mrg_string_append_str     (MrgString *string, const char *str);
void         mrg_string_append_byte    (MrgString *string, char  val);
void         mrg_string_append_string  (MrgString *string, MrgString *string2);
void         mrg_string_append_unichar (MrgString *string, unsigned int unichar);
void         mrg_string_append_data    (MrgString *string, const char *data, int len);
void         mrg_string_append_printf  (MrgString *string, const char *format, ...);

int          mrg_utf8_len          (const unsigned char first_byte);
int          mrg_utf8_strlen       (const char *s);
const char  *mrg_utf8_skip         (const char *string, int utf8_length);
int          mrg_unichar_to_utf8   (unsigned int ch,
                                    unsigned char*dest);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-xml.h▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* mrg - MicroRaptor Gui
 * Copyright (c) 2002, 2003, Øyvind Kolås <pippin@hodefoting.com>
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

#ifndef XMLTOK_H
#define XMLTOK_H

#include <stdio.h>

#define inbufsize 4096

typedef struct _MrgXml MrgXml;

enum
{
  t_none = 0,
  t_whitespace,
  t_prolog,
  t_dtd,
  t_comment,
  t_word,
  t_tag,
  t_closetag,
  t_closeemptytag,
  t_endtag,
  t_att = 10,
  t_val,
  t_eof,
  t_entity,
  t_error
};

MrgXml *xmltok_new (FILE * file_in);
MrgXml *xmltok_buf_new (char *membuf);
void    xmltok_free (MrgXml *t);
int     xmltok_lineno (MrgXml *t);
int     xmltok_get (MrgXml *t, char **data, int *pos);

#endif /*XMLTOK_H */
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-xml.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
/* mrg - MicroRaptor Gui
 * Copyright (c) 2002, 2003, 2014 Øyvind Kolås <pippin@hodefoting.com>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
// bundled include of mrg-xml.h"
// bundled include of mrg-string.h"

struct _MrgXml
{
  FILE     *file_in;
  int       state;
  MrgString  *curdata;
  MrgString  *curtag;
  int       c;
  int       c_held;

  unsigned char *inbuf;
  int       inbuflen;
  int       inbufpos;

  int       line_no;
};

enum
{
  s_null = 0,
  s_start,
  s_tag,
  s_tagnamestart,
  s_tagname,
  s_tagnamedone,
  s_intag,
  s_attstart,
  s_attname,
  s_attdone,
  s_att,
  s_atteq,
  s_eqquot,
  s_eqvalstart,
  s_eqapos,
  s_eqaposval,
  s_eqaposvaldone,
  s_eqval,
  s_eqvaldone,
  s_eqquotval,
  s_eqquotvaldone,
  s_tagend,
  s_empty,
  s_inempty,
  s_emptyend,
  s_whitespace,
  s_whitespacedone,
  s_entitystart,
  s_entity,
  s_entitydone,
  s_word,
  s_worddone,
  s_tagclose,
  s_tagclosenamestart,
  s_tagclosename,
  s_tagclosedone,
  s_tagexcl,
  s_commentdash1,
  s_commentdash2,
  s_incomment,
  s_commentenddash1,
  s_commentenddash2,
  s_commentdone,
  s_dtd,
  s_prolog,
  s_prologq,
  s_prologdone,
  s_eof,
  s_error
};

char     *c_ws = " \n\r\t";

enum
{
  c_nil = 0,
  c_eat = 1,                    /* request that another char be used for the next state */
  c_store = 2                   /* store the current char in the output buffer */
};

typedef struct
{
  int       state;
  char     *chars;
  unsigned char r_start;
  unsigned char r_end;
  int       next_state;
  int       resetbuf;
  int       charhandling;
  int       return_type;        /* if set return current buf, with type set to the type */
}
state_entry;

#define max_entries 20

static state_entry state_table[s_error][max_entries];

static void
a (int state,
   char *chars,
   unsigned char r_start,
   unsigned char r_end, int charhandling, int next_state)
{
  int       no = 0;

  while (state_table[state][no].state != s_null)
    no++;
  state_table[state][no].state = state;
  state_table[state][no].r_start = r_start;
  if (chars)
    state_table[state][no].chars = strdup (chars);
  state_table[state][no].r_end = r_end;
  state_table[state][no].charhandling = charhandling;
  state_table[state][no].next_state = next_state;
}

static void
r (int state, int return_type, int next_state)
{
  state_table[state][0].state = state;
  state_table[state][0].return_type = return_type;
  state_table[state][0].next_state = next_state;
}

/* *INDENT-OFF* */

static void
init_statetable (void) {
    static int inited=0;
    if(inited)
        return;
    inited=1;
    memset(state_table,0,sizeof(state_table));
    a(s_start,        "<",    0,0,            c_eat,            s_tag);
    a(s_start,        c_ws,    0,0,            c_eat+c_store,    s_whitespace);
    a(s_start,        "&",    0,0,            c_eat,            s_entitystart);
    a(s_start,        NULL,    0,255,            c_eat+c_store,    s_word);

    a(s_tag,        c_ws,    0,0,            c_eat,            s_tag);
    a(s_tag,        "/",    0,0,            c_eat,            s_tagclose);
    a(s_tag,        "!",    0,0,            c_eat,            s_tagexcl);
    a(s_tag,        "?",    0,0,            c_eat,            s_prolog);
    a(s_tag,        NULL,    0,255,            c_eat+c_store,    s_tagnamestart);

    a(s_tagclose,    NULL,    0,255,            c_eat+c_store,    s_tagclosenamestart);
    a(s_tagclosenamestart,    ">",    0,0,    c_eat,            s_tagclosedone);
    a(s_tagclosenamestart,    NULL,    0,255,    c_eat+c_store,    s_tagclosename);
    a(s_tagclosename,    ">",    0,0,        c_eat,            s_tagclosedone);
    a(s_tagclosename,    NULL,    0,255,        c_eat+c_store,    s_tagclosename);
    r(s_tagclosedone,    t_closetag,                            s_start);

    a(s_whitespace,        c_ws,    0,0,        c_eat+c_store,    s_whitespace);
    a(s_whitespace,        NULL,    0,255,        c_nil,            s_whitespacedone);
    r(s_whitespacedone,    t_whitespace,                        s_start);

    a(s_entitystart,";",    0,0,            c_eat,            s_entitydone);
    a(s_entitystart,NULL,    0,255,            c_eat+c_store,    s_entity);
    a(s_entity,        ";",    0,0,            c_eat,            s_entitydone);
    a(s_entity,NULL,        0,255,            c_eat+c_store,    s_entity);
    r(s_entitydone,    t_entity,                                s_start);

    a(s_word,        c_ws,    0,0,            c_nil,            s_worddone);
    a(s_word,        "<&",    0,0,            c_nil,            s_worddone);
    a(s_word,        NULL,    0,255,            c_eat+c_store,    s_word);
    r(s_worddone,    t_word,                                    s_start);

    a(s_tagnamestart,c_ws,    0,0,            c_nil,            s_tagnamedone);
    a(s_tagnamestart,    "/>",    0,0,        c_nil,            s_tagnamedone);
    a(s_tagnamestart,NULL,    0,255,            c_eat+c_store,    s_tagname);
    a(s_tagname,    c_ws,    0,0,            c_nil,            s_tagnamedone);
    a(s_tagname,    "/>",    0,0,            c_nil,            s_tagnamedone);
    a(s_tagname,    NULL,    0,255,            c_eat+c_store,    s_tagname);
    r(s_tagnamedone,    t_tag,                                s_intag);

    a(s_intag,        c_ws,    0,0,            c_eat,            s_intag);
    a(s_intag,        ">",    0,0,            c_eat,            s_tagend);
    a(s_intag,        "/",    0,0,            c_eat,            s_empty);
    a(s_intag,        NULL,    0,255,            c_eat+c_store,    s_attstart);

    a(s_attstart,    c_ws,    0,0,            c_eat,            s_attdone);
    a(s_attstart,    "=/>",    0,0,            c_nil,            s_attdone);
    a(s_attstart,    NULL,    0,255,            c_eat+c_store,    s_attname);
    a(s_attname,    "=/>",    0,0,            c_nil,            s_attdone);
    a(s_attname,    c_ws,    0,0,            c_eat,            s_attdone);
    a(s_attname,    NULL,    0,255,            c_eat+c_store,    s_attname);
    r(s_attdone,    t_att,                                    s_att);
    a(s_att,        c_ws,    0,0,            c_eat,            s_att);
    a(s_att,        "=",    0,0,            c_eat,            s_atteq);
    a(s_att,        NULL,    0,255,            c_eat,            s_intag);
    a(s_atteq,        "'",    0,0,            c_eat,            s_eqapos);
    a(s_atteq,        "\"",    0,0,            c_eat,            s_eqquot);
    a(s_atteq,        c_ws,    0,0,            c_eat,            s_atteq);
    a(s_atteq,        NULL,    0,255,            c_nil,            s_eqval);

    a(s_eqapos,        "'",    0,0,            c_eat,            s_eqaposvaldone);
    a(s_eqapos,        NULL,    0,255,            c_eat+c_store,    s_eqaposval);
    a(s_eqaposval,        "'",    0,0,        c_eat,            s_eqaposvaldone);
    a(s_eqaposval,        NULL,    0,255,        c_eat+c_store,    s_eqaposval);
    r(s_eqaposvaldone,    t_val,                                    s_intag);

    a(s_eqquot,        "\"",    0,0,            c_eat,            s_eqquotvaldone);
    a(s_eqquot,        NULL,    0,255,            c_eat+c_store,    s_eqquotval);
    a(s_eqquotval,        "\"",    0,0,        c_eat,            s_eqquotvaldone);
    a(s_eqquotval,        NULL,    0,255,        c_eat+c_store,    s_eqquotval);
    r(s_eqquotvaldone,    t_val,                                    s_intag);

    a(s_eqval,        c_ws,    0,0,            c_nil,            s_eqvaldone);
    a(s_eqval,        "/>",    0,0,            c_nil,            s_eqvaldone);
    a(s_eqval,        NULL,    0,255,            c_eat+c_store,    s_eqval);

    r(s_eqvaldone,    t_val,                                    s_intag);

    r(s_tagend,        t_endtag,                s_start);

    r(s_empty,              t_endtag,                               s_inempty);
    a(s_inempty,        ">",0,0,                c_eat,            s_emptyend);
    a(s_inempty,        NULL,0,255,                c_eat,            s_inempty);
    r(s_emptyend,    t_closeemptytag,                        s_start);

    a(s_prolog,        "?",0,0,                c_eat,            s_prologq);
    a(s_prolog,        NULL,0,255,                c_eat+c_store,    s_prolog);

    a(s_prologq,    ">",0,0,                c_eat,            s_prologdone);
    a(s_prologq,    NULL,0,255,                c_eat+c_store,    s_prolog);
    r(s_prologdone,    t_prolog,                s_start);

    a(s_tagexcl,    "-",0,0,                c_eat,            s_commentdash1);
    a(s_tagexcl,    "D",0,0,                c_nil,            s_dtd);
    a(s_tagexcl,    NULL,0,255,                c_eat,            s_start);

    a(s_commentdash1,    "-",0,0,                c_eat,            s_commentdash2);
    a(s_commentdash1,    NULL,0,255,                c_eat,            s_error);

    a(s_commentdash2,    "-",0,0,                c_eat,            s_commentenddash1);
    a(s_commentdash2,    NULL,0,255,                c_eat+c_store,    s_incomment);

    a(s_incomment   ,    "-",0,0,                c_eat,            s_commentenddash1);
    a(s_incomment   ,    NULL,0,255,                c_eat+c_store,    s_incomment);

    a(s_commentenddash1,    "-",0,0,            c_eat,            s_commentenddash2);
    a(s_commentenddash1,    NULL,0,255,            c_eat+c_store,    s_incomment);

    a(s_commentenddash2,    ">",0,0,            c_eat,            s_commentdone);
    a(s_commentenddash2,    NULL,0,255,            c_eat+c_store,    s_incomment);

    r(s_commentdone,    t_comment,                s_start);

}

/* *INDENT-ON* */

static int
is_oneof (char c, char *chars)
{
  while (*chars)
    {
      if (c == *chars)
        return 1;
      chars++;
    }
  return 0;
}

static int
nextchar (MrgXml *t)
{
  int       ret;

  if (t->file_in)
    {
      if (t->inbufpos >= t->inbuflen)
        {
          t->inbuflen = fread (t->inbuf, 1, inbufsize, t->file_in);
          t->inbufpos = 0;
          if (!t->inbuflen)
            return -1;
        }

      ret = (int) t->inbuf[t->inbufpos++];

      if (ret == '\n')
        t->line_no++;
    }
  else
    {
      if (t->inbufpos >= t->inbuflen)
        {
          return -1;
        }
      ret = (int) t->inbuf[t->inbufpos++];
      if (ret == '\n')
        t->line_no++;
    }
  return ret;
}

int
xmltok_get (MrgXml *t, char **data, int *pos)
{
  state_entry *s;

  init_statetable ();
  mrg_string_clear (t->curdata);
  while (1)
    {
      if (!t->c_held)
        {
          t->c = nextchar (t);
          if (t->c == -1)
          {
            if (pos)*pos = t->inbufpos;
            return t_eof;
          }
          t->c_held = 1;
        }
      if (t->state == s_dtd)
        {     /* FIXME: should make better code for skipping DTD */

          /*            int angle = 0; */
          int       squote = 0;
          int       dquote = 0;
          int       abracket = 1;

          /*            int sbracket = 0; */

          mrg_string_append_byte (t->curdata, t->c);

          while (abracket)
            {
              switch (t->c = nextchar (t))
                {
                case -1:
                  return t_eof;
                case '<':
                  if ((!squote) && (!dquote))
                    abracket++;
                  mrg_string_append_byte (t->curdata, t->c);
                  break;
                case '>':
                  if ((!squote) && (!dquote))
                    abracket--;
                  if (abracket)
                    mrg_string_append_byte (t->curdata, t->c);
                  break;
                case '"':
                case '\'':
                case '[':
                case ']':
                default:
                  mrg_string_append_byte (t->curdata, t->c);
                  break;
                }
            }
          t->c_held = 0;
          t->state = s_start;

          if (pos)*pos = t->inbufpos;
          return t_dtd;
        }
      s = &state_table[t->state][0];
      while (s->state)
        {
          if (s->return_type != t_none)
            {
              *data = (char *) mrg_string_get (t->curdata);
              t->state = s->next_state;
              if (s->return_type == t_tag)
                mrg_string_set (t->curtag, mrg_string_get (t->curdata));
              if (s->return_type == t_endtag)
                *data = (char *) mrg_string_get (t->curtag);
              if (s->return_type == t_closeemptytag)
                *data = (char *) mrg_string_get (t->curtag);
              if (pos)
                *pos = t->inbufpos;
              return s->return_type;
            }
          if ((s->chars && is_oneof (t->c, s->chars))
              || ((s->r_start + s->r_end)
                  && (t->c >= s->r_start && t->c <= s->r_end)))
            {
              if (s->charhandling & c_store)
                {
                  mrg_string_append_byte (t->curdata, t->c);
                }
              if (s->charhandling & c_eat)
                {
                  t->c_held = 0;
                }
              t->state = s->next_state;
              break;
            }
          s++;
        }
    }
  if (pos)
    *pos = t->inbufpos;
  return t_eof;
}

MrgXml *
xmltok_new (FILE * file_in)
{
  MrgXml *ret;

  ret = calloc (1, sizeof (MrgXml));
  ret->file_in = file_in;
  ret->state = s_start;
  ret->curtag = mrg_string_new ("");
  ret->curdata = mrg_string_new ("");
  ret->inbuf = calloc (1, inbufsize);
  return ret;
}

MrgXml *
xmltok_buf_new (char *membuf)
{
  MrgXml *ret;

  ret = calloc (1, sizeof (MrgXml));
  ret->file_in = NULL;
  ret->state = s_start;
  ret->curtag = mrg_string_new ("");
  ret->curdata = mrg_string_new ("");
  ret->inbuf = (void*)membuf;
  ret->inbuflen = strlen (membuf);
  ret->inbufpos = 0;
  return ret;
}

void
xmltok_free (MrgXml *t)
{
  mrg_string_free (t->curtag, 1);
  mrg_string_free (t->curdata, 1);

  if (t->file_in)
    {
      /*        fclose (t->file_in); */
      free (t->inbuf);
    }
  free (t);
}

char     *empty_tags[] = {
  "img", "IMG", "br", "BR", "hr", "HR", "META", "meta", "link", "LINK",
  NULL
};

char     *endomission_tags[] = {
  "li", "LI", "p", "P", "td", "TD", "tr", "TR", NULL
};

int
xmltok_lineno (MrgXml *t)
{
  return t->line_no;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-uri-fetcher.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// bundled include of mrg-list.h"

static int
file_get_contents (const char  *path,
                   char       **contents,
                   long        *length)
{
  FILE *file;
  long  size;
  long  remaining;
  char *buffer;


  file = fopen (path,"rb");

  if (!file)
    return -1;

  fseek (file, 0, SEEK_END);
  *length = size = remaining = ftell (file);
  rewind (file);
  buffer = malloc(size + 8);

  if (!buffer)
    {
      fclose(file);
      return -1;
    }

  remaining -= fread (buffer, 1, remaining, file);
  if (remaining)
    {
      fclose (file);
      free (buffer);
      return -1;
    }
  fclose (file);
  *contents = buffer;
  buffer[size] = 0;
  return 0;
}

// bundled include of mrg-http.h"

enum
{
  URI_STATE_IN_PROTOCOL = 0,
  URI_STATE_IN_HOST,
  URI_STATE_IN_PORT,
  URI_STATE_E_S1,
  URI_STATE_E_S2,
  URI_STATE_IN_PATH,
  URI_STATE_IN_FRAGMENT,
};

int split_uri (char *uri,
               char **protocol,
               char **host,
               char **port,
               char **path,
               char **fragment)
{
  char *p;

  if (strstr (uri, "//"))
  {
    int mr = URI_STATE_IN_PROTOCOL;

    if (protocol)
      *protocol = uri;

    if (uri[0] == '/' &&
        uri[1] == '/')
    {
      mr = URI_STATE_E_S1;
      *protocol = NULL;
    }

    for (p = uri; *p; p++)
    {
      switch (mr)
      {
        case URI_STATE_IN_PROTOCOL:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_E_S1;
              break;
          }
          break;
        case URI_STATE_E_S1:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_E_S2;
              break;
          }
          break;
        case URI_STATE_E_S2:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_IN_HOST;
              if (host) *host = p+1;
              break;
          }
          break;
        case URI_STATE_IN_HOST:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_IN_PORT;
              if (port) *port = p+1;
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PORT:
          switch (*p)
          {
            default:
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PATH:
          switch (*p)
          {
            default:
              break;
            case '#':
              *p = '\0';
              mr = URI_STATE_IN_FRAGMENT;
              if (fragment) *fragment = p+1;
              break;
          }
          break;
      }
    }
  }
  else
  {

    int mr = URI_STATE_IN_HOST;
    if (protocol)
      *protocol = NULL;

    if (uri[0]=='/')
    {
      if (host)
        *host = NULL;
      if (port)
        *port = NULL;
      *uri = '\0';
      mr = URI_STATE_IN_PATH;
      if (path) *path = uri+1;
    }
    else 
    {
      mr = URI_STATE_IN_PATH;
      if (path) *path = uri;
    }

    for (p = uri; *p; p++)
    {
      switch (mr)
      {
        case URI_STATE_IN_PROTOCOL:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_E_S1;
              break;
          }
          break;
        case URI_STATE_E_S1:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_E_S2;
              break;
          }
          break;
        case URI_STATE_E_S2:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_IN_HOST;
              if (host) *host = p+1;
              break;
          }
          break;
        case URI_STATE_IN_HOST:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_IN_PORT;
              if (port) *port = p+1;
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PORT:
          switch (*p)
          {
            default:
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PATH:
          switch (*p)
          {
            default:
              break;
            case '#':
              *p = '\0';
              mr = URI_STATE_IN_FRAGMENT;
              if (fragment) *fragment = p+1;
              break;
          }
          break;
      }
    }
  }
  if (*protocol && (*protocol)[0] == 0)
    *protocol = NULL;
  return 0;
}

static int
_mr_get_contents (const char  *referer,
                 const char  *uri,
                 char       **contents,
                 long        *length)
{
  char *uri_dup = strdup (uri);
  char *protocol = NULL;
  char *host = NULL;
  char *port = NULL;
  char *path = NULL;
  char *fragment = NULL;
  split_uri (uri_dup, &protocol, &host, &port, &path, &fragment);

  fprintf (stderr, "request: %s %s ", referer, uri);

  //fprintf (stderr, "%s,%s,%i,%s\n",protocol, host, port?atoi(port):80, path);
  if (protocol && !strcmp (protocol, "http"))
  {
    int len;
    char *pathdup = malloc (strlen (path) + 2);
    pathdup[0] = '/';
    strcpy (&pathdup[1], path);
   // fprintf (stderr, "%s %i\n",host, port?atoi(port):80);
    char *cont = _mrg_http (NULL, host, port?atoi(port):80, pathdup, &len);
    *contents = cont;
    *length = len;
    //fprintf (stderr, "%s\n", cont);
    //
    fprintf (stderr, "%i\n", len);
    free (uri_dup);
    return 0;
  }
  else if (protocol && !strcmp (protocol, "file"))
  {
    char *path2 = malloc (strlen (path) + 2);
    int ret;
    sprintf (path2, "/%s", path);
    ret = file_get_contents (path2, contents, length);
    free (path2);
    free (uri_dup);
    fprintf (stderr, "%i\n", (int)*length);
    return ret;
  }
  else
  {
    char *c = NULL;
    long  l = 0;
    int ret;
    free (uri_dup);
    ret = file_get_contents (uri, &c, &l);
    if (contents) *contents = c;
    if (length) *length = l;
    fprintf (stderr, "%li\n", l);
    return ret;
  }

  return -1;
}

typedef struct _CacheEntry {
  char *uri;
  char *contents;
  long  length;
} CacheEntry;

static MrgList *cache = NULL;

char *_mrg_resolve_uri (const char *base_uri, const char *uri)
{
  char *ret;
  char *uri_dup = strdup (uri);

  if (!base_uri)
    return uri_dup;

  char *base_dup = strdup (base_uri);

  char *protocol = NULL;
  char *host = NULL;
  char *port = NULL;
  char *path = NULL;
  char *fragment = NULL;
  char *base_protocol = NULL;
  char *base_host = NULL;
  char *base_port = NULL;
  char *base_path = NULL;
  char *base_fragment = NULL;

  //int retlen = 0;
  int samehost = 0;

  split_uri (uri_dup, &protocol, &host, &port, &path, &fragment);
  split_uri (base_dup, &base_protocol, &base_host, &base_port, &base_path, &base_fragment);

  if (!protocol)
    protocol = base_protocol;
  if (!host)
  {
    host = base_host;
    port = base_port;
    samehost = 1;
  }

  ret = malloc (
      (path?strlen (path):0)
      + (fragment?strlen (fragment):0) +
      (host?strlen (host):0) + 640);
  if (protocol)
  {
    if (uri[0] == '/' && uri[1] != '/')
      sprintf (ret, "%s://%s%s%s%s", protocol, host, port?":":"", port?port:"", uri);
    else if (uri[0] == '.' && uri[1] == '.' && uri[2] == '/' &&
             uri[3] == '.' && uri[4] == '.')
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;

      sprintf (ret, "c%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", &path[6], fragment?"#":"", fragment?fragment:"");
    }
    else if (uri[0] == '.' && uri[1] == '.')
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;

      sprintf (ret, "%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", &path[3], fragment?"#":"", fragment?fragment:"");
    }
    else
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      
      sprintf (ret, "%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", path, fragment?"#":"", fragment?fragment:"");
    }
  }
  else
  {
    if (uri[0] == '/')
      sprintf (ret, "%s", path);
    else
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      sprintf (ret, "/%s%s", base_path, path);
    }
  }

  free (uri_dup);
  free (base_dup);
  return ret;
}

/* caching uri fetcher
 */
int
mrg_get_contents (const char  *referer,
                  const char  *input_uri,
                  char       **contents,
                  long        *length,
                  void        *ignored_user_data)
{
  MrgList *i;

  /* should resolve before mrg_get_contents  */
  char *uri = _mrg_resolve_uri (referer, input_uri);

  for (i = cache; i; i = i->next)
  {
    CacheEntry *entry = i->data;
    if (!strcmp (entry->uri, uri))
    {
      *contents = malloc (entry->length + 1);
      memcpy (*contents, entry->contents, entry->length);
      (*contents)[entry->length]=0;
      free (uri);
      if (length) *length = entry->length;
      if (length)
      {
        return 0;
      }
      else
      {
        return -1;
      }
    }
  }

  {
    CacheEntry *entry = calloc (sizeof (CacheEntry), 1);
    char *c = NULL;
    long  l = 0;

    entry->uri = uri;
    _mr_get_contents (referer, uri, &c, &l);
    if (c){
      entry->contents = c;
      entry->length = l;
    } else
    {
      entry->contents = NULL;
      entry->length = 0;
    }
    mrg_list_prepend (&cache, entry);
  }
  return mrg_get_contents (referer, input_uri, contents, length, ignored_user_data);
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-xml-render.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg.h"
// bundled include of mrg-xml.h"
#include <string.h>
#include <math.h>
// bundled include of mrg-internal.h"

static void
_mrg_draw_background_increment (Mrg *mrg, void *data, int last);

MrgGeoCache *_mrg_get_cache (MrgHtml *ctx, void *id_ptr)
{
  MrgList *l;
  for (l = ctx->geo_cache; l; l = l->next)
  {
    MrgGeoCache *item = l->data;
    if (item->id_ptr == id_ptr)
    {
      item->gen++;
      return item;
    }
  }
  {
    MrgGeoCache *item = calloc (sizeof (MrgGeoCache), 1);
    item->id_ptr = id_ptr;
    mrg_list_prepend_full (&ctx->geo_cache, item, (void*)free, NULL);
    return item;
  }
  return NULL;
}

static float _mrg_dynamic_edge_right2 (Mrg *mrg, MrgHtmlState *state)
{
  float ret = mrg_edge_right (mrg);
  float y = mrg_y (mrg);
  float em = mrg_em (mrg);
  int i;

  if (state->floats)
    for (i = 0; i < state->floats; i++)
    {
      MrgFloatData *f = &state->float_data[i];
      if (f->type == MRG_FLOAT_RIGHT &&
          y >= f->y  &&
          y - em < f->y + f->height &&

          f->x < ret)
          ret = f->x;
    }
  return ret;
}

static float _mrg_dynamic_edge_left2 (Mrg *mrg, MrgHtmlState *state)
{
  float ret = mrg_edge_left (mrg);
  float y = mrg_y (mrg);
  float em = mrg_em (mrg);
  int i;

  if (state->floats)
    for (i = 0; i < state->floats; i++)
    {
      MrgFloatData *f = &state->float_data[i];
      if (f->type == MRG_FLOAT_LEFT &&
          y >= f->y &&
          y - em < f->y + f->height &&
          f->x + f->width > ret)
          ret = f->x + f->width;
    }
  return ret;
}

static float _mrg_parent_dynamic_edge_left (MrgHtml *ctx)
{
  MrgHtmlState *state = ctx->state;
  if (ctx->state_no)
    state = &ctx->states[ctx->state_no-1];
  return _mrg_dynamic_edge_left2 (ctx->mrg, state);
}

static float _mrg_parent_dynamic_edge_right (MrgHtml *ctx)
{
  MrgHtmlState *state = ctx->state;
  if (ctx->state_no)
    state = &ctx->states[ctx->state_no-1];
  return _mrg_dynamic_edge_right2 (ctx->mrg, state);
}

static float wrap_edge_left (Mrg *mrg, void *data)
{
  MrgHtml *ctx = data;
  MrgHtmlState *state = ctx->state;
  return _mrg_dynamic_edge_left2 (mrg, state);
}

static float wrap_edge_right (Mrg *mrg, void *data)
{
  MrgHtml *ctx = data;
  MrgHtmlState *state = ctx->state;
  return _mrg_dynamic_edge_right2 (mrg, state);
}

static void clear_left (MrgHtml *ctx)
{
  Mrg *mrg = ctx->mrg;
  float y = mrg_y (mrg);
  int i;

  if (ctx->state->floats)
  {
    for (i = 0; i < ctx->state->floats; i++)
      {
        MrgFloatData *f = &ctx->state->float_data[i];
        {
          if (f->type == MRG_FLOAT_LEFT)
          {
            if (f->y + f->height > y)
              y = f->y + f->height;
          }
        }
      }
  }
  mrg_set_xy (mrg, mrg_x (mrg), y);
}

static void clear_right (MrgHtml *ctx)
{
  Mrg *mrg = ctx->mrg;
  float y = mrg_y (mrg);
  int i;

  if (ctx->state->floats)
  {
    for (i = 0; i < ctx->state->floats; i++)
      {
        MrgFloatData *f = &ctx->state->float_data[i];
        {
          if (f->type == MRG_FLOAT_RIGHT)
          {
            if (f->y + f->height > y)
              y = f->y + f->height;
          }
        }
      }
  }
  mrg_set_xy (mrg, mrg_x (mrg), y);
}

static void clear_both (MrgHtml *ctx)
{
  Mrg *mrg = ctx->mrg;
#if 0
  clear_left (mrg);
  clear_right (mrg);
#else
  float y = mrg_y (mrg);
  int i;

  if (ctx->state->floats)
  {
    for (i = 0; i < ctx->state->floats; i++)
      {
        MrgFloatData *f = &ctx->state->float_data[i];
        {
          if (f->y + f->height > y)
            y = f->y + f->height;
        }
      }
  }
  y += mrg_em (mrg) * mrg_style(mrg)->line_height;
  mrg_set_xy (mrg, mrg_x (mrg), y);
  //_mrg_draw_background_increment (mrg, &mrg->html, 0);
#endif
}

static void mrg_path_fill_stroke (Mrg *mrg)
{
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);
  if (style->fill_color.alpha > 0.001)
  {
    mrg_cairo_set_source_color (cr, &style->fill_color);
    cairo_fill_preserve (cr);
  }

  if (style->stroke_width > 0.001)
  {
    cairo_set_line_width (cr, style->stroke_width);
    mrg_cairo_set_source_color (cr, &style->stroke_color);
    cairo_stroke (cr);
  }
  cairo_new_path (cr);
}

void _mrg_border_top (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_top_width &&
      style->border_top_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x - style->padding_left - style->border_left_width,
                       y - style->padding_top - style->border_top_width);
    cairo_rel_line_to (cr, width + style->padding_left + style->padding_right + style->border_left_width + style->border_right_width, 0);
    cairo_rel_line_to (cr, -style->border_right_width, style->border_top_width);
    cairo_rel_line_to (cr, - (width + style->padding_right + style->padding_left), 0);

    mrg_cairo_set_source_color (cr, &style->border_top_color);
    cairo_fill (cr);
  }
  cairo_restore (cr);
#endif
}
void _mrg_border_bottom (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_bottom_width &&
      style->border_bottom_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x + width + style->padding_right, y + height + style->padding_bottom);
    cairo_rel_line_to (cr, style->border_right_width, style->border_bottom_width);
    cairo_rel_line_to (cr, - (width + style->padding_left + style->padding_right + style->border_left_width + style->border_right_width), 0);
    cairo_rel_line_to (cr, style->border_left_width, -style->border_bottom_width);

    mrg_cairo_set_source_color (cr, &style->border_bottom_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);
#endif
}

void _mrg_border_top_r (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_top_width &&
      style->border_top_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x,
                       y - style->padding_top - style->border_top_width);
    cairo_rel_line_to (cr, width + style->padding_right + style->border_right_width, 0);
    cairo_rel_line_to (cr, -style->border_right_width, style->border_top_width);
    cairo_rel_line_to (cr, - (width + style->padding_right), 0);

    mrg_cairo_set_source_color (cr, &style->border_top_color);
    cairo_fill (cr);
  }
  cairo_restore (cr);
#endif
}
void _mrg_border_bottom_r (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_bottom_width &&
      style->border_bottom_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x + width + style->padding_right, y + height + style->padding_bottom);
    cairo_rel_line_to (cr, style->border_right_width, style->border_bottom_width);
    cairo_rel_line_to (cr, - (width + style->padding_left + style->padding_right + style->border_left_width + style->border_right_width), 0);
    cairo_rel_line_to (cr, style->border_left_width, -style->border_bottom_width);

    mrg_cairo_set_source_color (cr, &style->border_bottom_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);
#endif
}

void _mrg_border_top_l (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_top_width &&
      style->border_top_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x - style->padding_left - style->border_left_width,
                       y - style->padding_top - style->border_top_width);
    cairo_rel_line_to (cr, width + style->padding_left + style->padding_right + style->border_left_width, 0);
    cairo_rel_line_to (cr, 0, style->border_top_width);
    cairo_rel_line_to (cr, - (width + style->padding_left), 0);

    mrg_cairo_set_source_color (cr, &style->border_top_color);
    cairo_fill (cr);
  }
  cairo_restore (cr);
#endif
}
void _mrg_border_bottom_l (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_bottom_width &&
      style->border_bottom_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x + width, y + height + style->padding_bottom);
    cairo_rel_line_to (cr, 0, style->border_bottom_width);
    cairo_rel_line_to (cr, - (width + style->padding_left + style->border_left_width), 0);
    cairo_rel_line_to (cr, style->border_left_width, -style->border_bottom_width);

    mrg_cairo_set_source_color (cr, &style->border_bottom_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);
#endif
}


void _mrg_border_top_m (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_top_width &&
      style->border_top_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x,
                       y - style->padding_top - style->border_top_width);
    cairo_rel_line_to (cr, width, 0);
    cairo_rel_line_to (cr, 0, style->border_top_width);
    cairo_rel_line_to (cr, -width, 0);

    mrg_cairo_set_source_color (cr, &style->border_top_color);
    cairo_fill (cr);
  }
  cairo_restore (cr);
#endif
}
void _mrg_border_bottom_m (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_bottom_width &&
      style->border_bottom_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x + width, y + height + style->padding_bottom);
    cairo_rel_line_to (cr, 0, style->border_bottom_width);
    cairo_rel_line_to (cr, - width, 0);
    cairo_rel_line_to (cr, 0, -style->border_bottom_width);

    mrg_cairo_set_source_color (cr, &style->border_bottom_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);
#endif
}
void _mrg_border_left (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_left_width &&
      style->border_left_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x - style->padding_left - style->border_left_width,
                       y - style->padding_top - style->border_top_width);
    cairo_rel_line_to (cr, style->border_left_width, style->border_top_width);
    cairo_rel_line_to (cr, 0, height + style->padding_top + style->padding_bottom );
    cairo_rel_line_to (cr, -style->border_left_width, style->border_bottom_width);
    mrg_cairo_set_source_color (cr, &style->border_left_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);

#endif
}
void _mrg_border_right (Mrg *mrg, int x, int y, int width, int height)
{
#if MRG_CAIRO
  cairo_t *cr = mrg_cr (mrg);
  MrgStyle *style = mrg_style (mrg);

  cairo_save (cr);

  if (style->border_right_width &&
      style->border_right_color.alpha > 0.001)
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x + width + style->padding_right, y + height + style->padding_bottom);
    cairo_rel_line_to (cr, style->border_right_width, style->border_bottom_width);
    cairo_rel_line_to (cr, 0, - (height + style->padding_top + style->padding_bottom + style->border_top_width + style->border_bottom_width));
    cairo_rel_line_to (cr, -style->border_right_width, style->border_top_width);

    mrg_cairo_set_source_color (cr, &style->border_right_color);
    cairo_fill (cr);
  }

  cairo_restore (cr);
#endif
}

static void mrg_box (Mrg *mrg, int x, int y, int width, int height)
{
  _mrg_draw_background_increment (mrg, &mrg->html, 1);
  _mrg_border_top (mrg, x, y, width, height);
  _mrg_border_left (mrg, x, y, width, height);
  _mrg_border_right (mrg, x, y, width, height);
  _mrg_border_bottom (mrg, x, y, width, height);
}

static void mrg_box_fill (Mrg *mrg, MrgStyle *style, float x, float y, float width, float height)
{
  cairo_t *cr = mrg_cr (mrg);
  if (style->background_color.alpha <= 0.0001)
    return;

  height = floor (y + height) - floor(y);
  y = floor (y);

  cairo_save (cr);
  {
    cairo_new_path (cr);
    cairo_move_to (cr, x,
                       y);
    cairo_rel_line_to (cr, 0, height );
    cairo_rel_line_to (cr, width, 0);
    cairo_rel_line_to (cr, 0, -(height ));

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    mrg_cairo_set_source_color (cr, &style->background_color);
    cairo_fill (cr);
  }
  cairo_restore (cr);
}

/*
 *  each style state level needs to know how far down it has
 *  painted background,.. on background increment we do all of them..
 *  .. floats are problematic - maybe draw them in second layer.
 *
 */

static void
_mrg_draw_background_increment2 (Mrg *mrg, MrgState *state, 
    MrgHtmlState *html_state, void *data, int last)
{
  MrgHtml *ctx = data;
  MrgStyle *style = &state->style;
  float gap = style->font_size * style->line_height;

  int width = style->width;
  if (style->background_color.alpha <= 0.0001)
    return;
  if (style->display == MRG_DISPLAY_INLINE &&
      (!style->float_))
    return;

  if (last)
    gap += style->padding_bottom;

  if (!width)
  {
    MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
    if (geo->width)
      width = geo->width;
    else
      width = mrg_edge_right (mrg) - mrg_edge_left (mrg); // XXX : err
  }

  if (html_state->ptly == 0)
  {
    mrg_box_fill (mrg, style,
      html_state->block_start_x - style->padding_left,
      (html_state->block_start_y - mrg_em (mrg) - style->padding_top),
      width + style->padding_left + style->padding_right,
      style->padding_top + gap);
    
    html_state->ptly = 
      html_state->block_start_y - mrg_em (mrg) - style->padding_top +
      (style->padding_top + gap);
  }
  else
  {
    if (( (mrg_y (mrg) - style->font_size) - html_state->ptly) + gap > 0)
    {
      mrg_box_fill (mrg, style,
          html_state->block_start_x - style->padding_left,
          html_state->ptly,
          width + style->padding_left + style->padding_right,
          ((mrg_y (mrg) - style->font_size) - html_state->ptly) + gap);

      html_state->ptly = mrg_y (mrg) - style->font_size  + gap;
    }
  }
}

static void
_mrg_draw_background_increment (Mrg *mrg, void *data, int last)
{
  MrgHtml *ctx = &mrg->html;
  int state;
  for (state = 0; state <= ctx->state_no; state++)
  {
    _mrg_draw_background_increment2 (mrg,
        &mrg->states[mrg->state_no - (ctx->state_no) + state],
        &ctx->states[state],
        data, last);
  }

}

void _mrg_layout_pre (Mrg *mrg, MrgHtml *ctx)
{
  MrgStyle *style;
  float dynamic_edge_left, dynamic_edge_right;

  ctx->state_no++;
  ctx->state = &ctx->states[ctx->state_no];
  *ctx->state = ctx->states[ctx->state_no-1];

  style = mrg_style (mrg);

  ctx->state->original_x = mrg_x (mrg);
  ctx->state->original_y = mrg_y (mrg);

  if (ctx->state_no)
  {
    dynamic_edge_right = _mrg_parent_dynamic_edge_right(ctx);
    dynamic_edge_left = _mrg_parent_dynamic_edge_left(ctx);
  }
  else
  {
    dynamic_edge_right = mrg_edge_right(mrg);
    dynamic_edge_left = mrg_edge_left(mrg);
  }

  if (style->clear & MRG_CLEAR_RIGHT)
    clear_right (ctx);
  if (style->clear & MRG_CLEAR_LEFT)
    clear_left (ctx);


  if (style->display == MRG_DISPLAY_BLOCK ||
      style->display == MRG_DISPLAY_LIST_ITEM)
  {
    if (style->padding_left + style->margin_left + style->border_left_width
        != 0)
    {
      mrg_set_edge_left (mrg, mrg_edge_left (mrg) +
          (style->padding_left + style->margin_left + style->border_left_width));
    }
    if (style->padding_right + style->margin_right + style->border_right_width
        != 0)
    {
      mrg_set_edge_right (mrg, mrg_edge_right (mrg) -
          (style->padding_right + style->margin_right + style->border_right_width));
    }

    if (style->margin_top > ctx->state->vmarg)
    mrg_set_edge_top (mrg, mrg_y (mrg) + style->border_top_width + (style->margin_top - ctx->state->vmarg));
    else
    {
      /* XXX: just ignoring vmarg when top-margin is negative? */
      mrg_set_edge_top (mrg, mrg_y (mrg) + style->border_top_width + (style->margin_top));
    }

    ctx->state->block_start_x = mrg_edge_left (mrg);
    ctx->state->block_start_y = mrg_y (mrg);
  }

  if (style->display == MRG_DISPLAY_LIST_ITEM)
  {
    float x = mrg->x;
    _mrg_draw_background_increment (mrg, ctx, 0);
    mrg->x -= mrg_em (mrg) * 1;
    mrg_print (mrg, "•"); //⚫"); //●");
    mrg->x = x;
  }

  switch (style->position)
  {
    case MRG_POSITION_RELATIVE:
      /* XXX: deal with style->right and style->bottom */
      cairo_translate (mrg_cr (mrg), style->left, style->top);
    case MRG_POSITION_STATIC:


      if (style->float_ == MRG_FLOAT_RIGHT)
      {
        float width = style->width;

        if (width == 0.0)
        {
          MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
          if (geo->width)
            width = geo->width;
          else
            width = mrg_edge_right (mrg) - mrg_edge_left (mrg);
        }

        width = (width + style->padding_right + style->padding_left + style->border_left_width + style->border_right_width);


        if (width + style->margin_left + style->margin_right >
            mrg_edge_right(mrg)-mrg_edge_left(mrg))
        {
          clear_both (ctx);
          mrg_set_edge_left (mrg, mrg_edge_right (mrg) - width);
          mrg_set_edge_right (mrg, mrg_edge_right (mrg) - (style->margin_right + style->padding_right + style->border_right_width));

        }
        else
        {
        while (dynamic_edge_right - dynamic_edge_left < width + style->margin_left + style->margin_right)
        {
          mrg_set_xy (mrg, mrg_x (mrg), mrg_y (mrg) + 1.0);
          dynamic_edge_right = _mrg_parent_dynamic_edge_right(ctx);
          dynamic_edge_left = _mrg_parent_dynamic_edge_left(ctx);
        }

        mrg_set_edge_left (mrg, dynamic_edge_right - width);
        mrg_set_edge_right (mrg, dynamic_edge_right - (style->margin_right + style->padding_right + style->border_right_width));

        }

        mrg_set_edge_top (mrg, mrg_y (mrg) + (style->margin_top - ctx->state->vmarg) - mrg_em(mrg));

        ctx->state->block_start_x = mrg_x (mrg);
        ctx->state->block_start_y = mrg_y (mrg);
        ctx->state->floats = 0;

      } else if (style->float_ == MRG_FLOAT_LEFT)
      {
        float left, y;

        float width = style->width;

        if (width == 0.0)
        {
          MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
          if (geo->width)
            width = geo->width;
          else
            width = 4 * mrg_em (mrg);//mrg_edge_right (mrg) - mrg_edge_left (mrg);
        }

        width = (width + style->padding_right + style->padding_left + style->border_left_width + style->border_right_width);

        if (width + style->margin_left + style->margin_right >
            mrg_edge_right(mrg)-mrg_edge_left(mrg))
        {
          clear_both (ctx);
          left = mrg_edge_left (mrg) + style->padding_left + style->border_left_width + style->margin_left;
        }
        else
        {
        while (dynamic_edge_right - dynamic_edge_left < width + style->margin_left + style->margin_right)
        {
          mrg_set_xy (mrg, mrg_x (mrg), mrg_y (mrg) + 1.0);
          dynamic_edge_right = _mrg_parent_dynamic_edge_right(ctx);
          dynamic_edge_left = _mrg_parent_dynamic_edge_left(ctx);
        }
          left = dynamic_edge_left + style->padding_left + style->border_left_width + style->margin_left;
        }

        y = mrg_y (mrg);

        mrg_set_edge_left (mrg, left);
        mrg_set_edge_right (mrg,  left + width +
            style->padding_left + style->border_right_width);
        mrg_set_edge_top (mrg, mrg_y (mrg) + (style->margin_top - ctx->state->vmarg) - mrg_em(mrg));
        ctx->state->block_start_x = mrg_x (mrg);
        ctx->state->block_start_y = y - style->font_size + style->padding_top + style->border_top_width;
        ctx->state->floats = 0;

        /* change cursor point after floating something left; if pushed far
         * down, the new correct
         */
        mrg_set_xy (mrg, ctx->state->original_x = left + width + style->padding_left + style->border_right_width + style->padding_right + style->margin_right + style->margin_left + style->border_left_width,
            y - style->font_size + style->padding_top + style->border_top_width);
      }
      break;
    case MRG_POSITION_ABSOLUTE:
      {
        ctx->state->floats = 0;
        mrg_set_edge_left (mrg, style->left + style->margin_left + style->border_left_width + style->padding_left);
        mrg_set_edge_right (mrg, style->left + style->width);
        mrg_set_edge_top (mrg, style->top + style->margin_top + style->border_top_width + style->padding_top);
        ctx->state->block_start_x = mrg_x (mrg);
        ctx->state->block_start_y = mrg_y (mrg);

      }
      break;
    case MRG_POSITION_FIXED:
      {
        int width = style->width;

        if (!width)
        {
          MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
          if (geo->width)
            width = geo->width;
          else
            width = mrg_edge_right (mrg) - mrg_edge_left (mrg);
        }

#if MRG_CAIRO
        cairo_identity_matrix (mrg_cr (mrg));
        cairo_scale (mrg_cr(mrg), mrg_ddpx (mrg), mrg_ddpx (mrg));
#endif
        ctx->state->floats = 0;

        mrg_set_edge_left (mrg, style->left + style->margin_left + style->border_left_width + style->padding_left);
        mrg_set_edge_right (mrg, style->left + style->margin_left + style->border_left_width + style->padding_left + width);//mrg_width (mrg) - style->padding_right - style->border_right_width - style->margin_right); //style->left + style->width); /* why only padding and not also border?  */
        mrg_set_edge_top (mrg, style->top + style->margin_top + style->border_top_width + style->padding_top);
        ctx->state->block_start_x = mrg_x (mrg);
        ctx->state->block_start_y = mrg_y (mrg);
      }
      break;
  }

  if (style->display == MRG_DISPLAY_BLOCK ||
      style->float_)
  {
     float height = style->height;
     float width = style->width;

     if (!height)
       {
         MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
         if (geo->height)
           height = geo->height;
         else
           height = mrg_em (mrg) * 4;
       }
     if (!width)
       {
         MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);
         if (geo->width)
           width = geo->width;
         else
           width = mrg_em (mrg) * 4;
       }

    if (height  /* XXX: if we knew height of dynamic elements
                        from previous frame, we could use it here */
       && style->overflow == MRG_OVERFLOW_HIDDEN)
       {
         cairo_rectangle (mrg_cr(mrg),
            ctx->state->block_start_x - style->padding_left - style->border_left_width,
            ctx->state->block_start_y - mrg_em(mrg) - style->padding_top - style->border_top_width,
            width + style->border_right_width + style->border_left_width + style->padding_left + style->padding_right, //mrg_edge_right (mrg) - mrg_edge_left (mrg) + style->padding_left + style->padding_right + style->border_left_width + style->border_right_width,
            height + style->padding_top + style->padding_bottom + style->border_top_width + style->border_bottom_width);
         cairo_clip (mrg_cr(mrg));
       }

    ctx->state->ptly = 0;
    _mrg_draw_background_increment (mrg, ctx, 0);
  }
}

#if 0
static void mrg_css_add_class (Mrg *mrg, const char *class_name)
{
  int i;
  MrgStyleNode *node = &mrg->state->style_node;
  for (i = 0; node->classes[i]; i++);
  node->classes[i] = mrg_intern_string (class_name);
}

static void mrg_css_add_pseudo_class (Mrg *mrg, const char *pseudo_class)
{
  int i;
  MrgStyleNode *node = &mrg->state->style_node;
  for (i = 0; node->pseudo[i]; i++);
  node->pseudo[i] = mrg_intern_string (pseudo_class);
}
#endif

void _mrg_layout_post (Mrg *mrg, MrgHtml *ctx)
{
  float vmarg = 0;
  MrgStyle *style = mrg_style (mrg);
  
  /* adjust cursor back to before display */

  if ((style->display == MRG_DISPLAY_BLOCK || style->float_) &&
       style->height != 0.0)
  {
    float diff = style->height - (mrg_y (mrg) - (ctx->state->block_start_y - mrg_em(mrg)));
    mrg_set_xy (mrg, mrg_x (mrg), mrg_y (mrg) + diff);
    if (diff > 0)
      _mrg_draw_background_increment (mrg, ctx, 1);
  }

  /* remember data to store about float, XXX: perhaps better to store
   * straight into parent state?
   */
  if (style->float_)
  {
    int was_float = 0;
    float fx,fy,fw,fh; // these tempvars arent really needed.
    was_float = style->float_;
    fx = ctx->state->block_start_x - style->padding_left - style->border_left_width - style->margin_left;
    fy = ctx->state->block_start_y - mrg_em(mrg) - style->padding_top - style->border_top_width  
      - style->margin_top;

    fw = mrg_edge_right (mrg) - mrg_edge_left (mrg)
      
     + style->padding_left + style->border_left_width + style->margin_left
     + style->padding_right + style->border_right_width + style->margin_right;

    fh = mrg_y (mrg) - (ctx->state->block_start_y - mrg_em(mrg))
         + style->margin_bottom + style->padding_top + style->padding_bottom + style->border_top_width + style->border_bottom_width + style->margin_top + mrg_em (mrg);

    ctx->states[ctx->state_no-1].float_data[ctx->states[ctx->state_no-1].floats].type = was_float;
    ctx->states[ctx->state_no-1].float_data[ctx->states[ctx->state_no-1].floats].x = fx;
    ctx->states[ctx->state_no-1].float_data[ctx->states[ctx->state_no-1].floats].y = fy;
    ctx->states[ctx->state_no-1].float_data[ctx->states[ctx->state_no-1].floats].width = fw;
    ctx->states[ctx->state_no-1].float_data[ctx->states[ctx->state_no-1].floats].height = fh;
    ctx->states[ctx->state_no-1].floats++;
  }

  if (style->display == MRG_DISPLAY_BLOCK || style->float_)
  {
    MrgGeoCache *geo = _mrg_get_cache (ctx, style->id_ptr);

    if (style->width == 0)
    {
#if 0
      if (mrg_y (mrg) == (ctx->state->block_start_y))
        geo->width = mrg_x (mrg) - (ctx->state->block_start_x);
      else
        geo->width = mrg->state->edge_right  - (ctx->state->block_start_x);
#endif
      geo->width = mrg_x (mrg) - (ctx->state->block_start_x);
    }
    else
      geo->width = style->width;

    //:mrg_edge_right (mrg) - mrg_edge_left (mrg);
    if (style->height == 0)
      geo->height = mrg_y (mrg) - (ctx->state->block_start_y - mrg_em(mrg));
    else
      geo->height = style->height;
    geo->gen++;

    mrg_box (mrg,
        ctx->state->block_start_x,
        ctx->state->block_start_y - mrg_em(mrg),
        geo->width,
        geo->height);

    {
      cairo_matrix_t transform;
      cairo_get_matrix (mrg_cr (mrg), &transform);
      double x = mrg->pointer_x;
      double y = mrg->pointer_y;
      cairo_matrix_invert (&transform);
      cairo_matrix_transform_point (&transform, &x, &y);

      if (x >= ctx->state->block_start_x &&
          x <  ctx->state->block_start_x + geo->width &&
          y >= ctx->state->block_start_y - mrg_em (mrg) &&
          y <  ctx->state->block_start_y - mrg_em (mrg) + geo->height)
      {
        geo->hover = 1;
      }
      else
      {
        geo->hover = 0;
      }
    }

    //mrg_edge_right (mrg) - mrg_edge_left (mrg), mrg_y (mrg) - (ctx->state->block_start_y - mrg_em(mrg)));

    if (!style->float_ && style->display == MRG_DISPLAY_BLOCK)
    {
      vmarg = style->margin_bottom;
    
      mrg_set_xy (mrg, 
          mrg_edge_left (mrg),
          mrg_y (mrg) + vmarg + style->border_bottom_width);
    }
  }
  else if (style->display == MRG_DISPLAY_INLINE)
  {
    mrg->x += paint_span_bg_final (mrg, mrg->x, mrg->y, 0);
  }

  if (style->position == MRG_POSITION_RELATIVE)
    cairo_translate (mrg_cr (mrg), -style->left, -style->top);

  if (style->float_ ||
      style->position == MRG_POSITION_ABSOLUTE ||
      style->position == MRG_POSITION_FIXED)
  {
    mrg_set_xy (mrg, ctx->state->original_x,
                     ctx->state->original_y);
  }
  if (ctx->state_no)
    ctx->states[ctx->state_no-1].vmarg = vmarg;

  ctx->state_no--;
  ctx->state = &ctx->states[ctx->state_no];
}

enum {
  HTML_ATT_UNKNOWN = 0,
  HTML_ATT_STYLE,
  HTML_ATT_CLASS,
  HTML_ATT_ID,
  HTML_ATT_HREF,
  HTML_ATT_REL,
  HTML_ATT_SRC
};

static char *entities[][2]={
  {"shy",    ""},   // soft hyphen,. should be made use of in wrapping..
  {"nbsp",   " "},  //
  {"lt",     "<"},
  {"gt",     ">"},
  {"trade",  "™"},
  {"copy",   "©"},
  {"middot", "·"},
  {"bull",   "•"},
  {"Oslash", "Ø"},
  {"oslash", "ø"},
  {"hellip", "…"},
  {"aring",  "å"},
  {"Aring",  "Å"},
  {"aelig",  "æ"},
  {"AElig",  "Æ"},
  {"Aelig",  "Æ"},
  {"laquo",  "«"},
  {"raquo",  "»"},

  /*the above were added as encountered, the rest in anticipation  */

  {"reg",    "®"},
  {"deg",    "°"},
  {"plusmn", "±"},
  {"sup2",   "²"},
  {"sup3",   "³"},
  {"sup1",   "¹"},
  {"ordm",   "º"},
  {"para",   "¶"},
  {"cedil",  "¸"},
  {"bull",   "·"},
  {"amp",   "&"},
  {"mdash",  "–"}, 
  {"apos",   "'"},
  {"quot",   "\""},
  {"iexcl",  "¡"},
  {"cent",   "¢"},
  {"pound",  "£"},
  {"euro",   "€"},
  {"yen",    "¥"},
  {"curren", "¤"},
  {"sect",   "§"},
  {"phi",    "Φ"},
  {"omega",  "Ω"},
  {"alpha",  "α"},

  /* XXX: incomplete */

  {NULL, NULL}
};

static const char *get_attr (MrgHtml *ctx, const char *attr)
{
  int i;
  for (i = 0; i < ctx->attributes; i++)
  {
    if (!strcmp (ctx->attribute[i], attr))
    {
      return ctx->value[i];
    }
  }
  return NULL;
}

static void
mrg_parse_transform (Mrg *mrg, cairo_matrix_t *matrix, const char *str)
{
  if (!strncmp (str, "matrix", 5))
  {
    char *s;
    int numbers = 0;
    double number[12];
    cairo_matrix_init_identity (matrix);
    s = (void*) strchr (str, '(');
    if (!s)
      return;
    s++;
    for (; *s; s++)
    {
      switch (*s)
      {
        case '+':case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
        number[numbers] = mrg_parse_float (mrg, s, &s);
        s--;
        numbers++;
      }
    }
    /* XXX: this needs validation */
    matrix->xx = number[0];
    matrix->yx = number[1];
    matrix->xy = number[2];
    matrix->yy = number[3];
    matrix->x0 = number[4];
    matrix->y0 = number[5];
  }
  else if (!strncmp (str, "scale", 5))
  {
    char *s;
    int numbers = 0;
    double number[12];
    cairo_matrix_init_identity (matrix);
    s = (void*) strchr (str, '(');
    if (!s)
      return;
    s++;
    for (; *s; s++)
    {
      switch (*s)
      {
        case '+':case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
        number[numbers] = mrg_parse_float (mrg, s, &s);
        s--;
        numbers++;
      }
    }
    if (numbers == 1)
      cairo_matrix_scale (matrix, number[0], number[0]);
    else
      cairo_matrix_scale (matrix, number[0], number[1]);
  }
  else if (!strncmp (str, "translate", 5))
  {
    char *s;
    int numbers = 0;
    double number[12];
    cairo_matrix_init_identity (matrix);
    s = (void*) strchr (str, '(');
    if (!s)
      return;
    s++;
    for (; *s; s++)
    {
      switch (*s)
      {
        case '+':case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
        number[numbers] = mrg_parse_float (mrg, s, &s);
        s--;
        numbers++;
      }
    }
    cairo_matrix_translate (matrix, number[0], number[1]);
  }
  else if (!strncmp (str, "rotate", 5))
  {
    char *s;
    int numbers = 0;
    double number[12];
    cairo_matrix_init_identity (matrix);
    s = (void*) strchr (str, '(');
    if (!s)
      return;
    s++;
    for (; *s; s++)
    {
      switch (*s)
      {
        case '+':case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
        number[numbers] = mrg_parse_float (mrg, s, &s);
        s--;
        numbers++;
      }
    }
    cairo_matrix_rotate (matrix, number[0] / 360.0 * 2 * M_PI);
  }
  else
  {
    fprintf (stderr, "unhandled transform: %s\n", str);
    cairo_matrix_init_identity (matrix);
  }
}

static void
mrg_parse_path (Mrg *mrg, const char *str)
{
  cairo_t *cr = mrg_cr (mrg);
  char  command = 'm';
  char *s;
  int numbers = 0;
  double number[12];

  if (!str)
    return;
  cairo_move_to (cr, 0, 0);

  s = (void*)str;
again:
  numbers = 0;

  for (; *s; s++)
  {
    switch (*s)
    {
      case 'z':
      case 'Z':
        cairo_close_path (cr);
        break;
      case 'm':
      case 'a':
      case 'M':
      case 'c':
      case 'C':
      case 'l':
      case 'L':
         command = *s;
         break;


      case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
      number[numbers] = mrg_parse_float (mrg, s, &s);
      s--;
      numbers++;

      switch (command)
      {
        case 'a':
        case 'A':
          if (numbers == 7)
          {
            /// XXX: NYI
            s++;
            goto again;
          }
        case 'm':
          if (numbers == 2)
          {
            cairo_rel_move_to (cr, number[0], number[1]);
            s++;
            goto again;
          }
          break;
        case 'l':
          if (numbers == 2)
          {
            cairo_rel_line_to (cr, number[0], number[1]);
            s++;
            goto again;
          }
          break;
        case 'c':
          if (numbers == 6)
          {
            cairo_rel_curve_to (cr, number[0], number[1],
                                    number[2], number[3],
                                    number[4], number[5]);
            s++;
            goto again;
          }
          break;
        case 'M':
          if (numbers == 2)
          {
            cairo_move_to (cr, number[0], number[1]);
            s++;
            goto again;
          }
          break;
        case 'L':
          if (numbers == 2)
          {
            cairo_line_to (cr, number[0], number[1]);
            s++;
            goto again;
          }
          break;
        case 'C':
          if (numbers == 6)
          {
            cairo_curve_to (cr, number[0], number[1],
                                number[2], number[3],
                                number[4], number[5]);
            s++;
            goto again;
          }
          break;
        default:
          fprintf (stderr, "_%c", *s);
          break;
      }
      break;
      default:
        break;
    }
  }
}

static void
mrg_parse_polygon (Mrg *mrg, const char *str)
{
  cairo_t *cr = mrg_cr (mrg);
  char *s;
  int numbers = 0;
  int started = 0;
  double number[12];

  if (!str)
    return;
  cairo_move_to (cr, 0, 0);

  s = (void*)str;
again:
  numbers = 0;

  for (; *s; s++)
  {
    switch (*s)
    {
      case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7': case '8': case '9':
      number[numbers] = mrg_parse_float (mrg, s, &s);
      s--;
      numbers++;

      if (numbers == 2)
      {
        if (started)
          cairo_line_to (cr, number[0], number[1]);
        else
        {
          cairo_move_to (cr, number[0], number[1]);
          started = 1;
        }
        s++;
        goto again;
      }
      default:
        break;
    }
  }
}

void mrg_xml_render (Mrg *mrg,
                     char *uri_base,
                     char *html,
                     int (*link_cb) (MrgEvent *event, void *href, void *link_data),
                     void *link_data,
                     int (*fetch) (const char *referer, const char *uri, char **contents, long *length, void *fetch_data),
                      void *fetch_data)
{
  MrgXml *xmltok      = xmltok_buf_new (html);
  MrgHtml *ctx        = &mrg->html;
  char tag[64][16];
  int pos             = 0;
  int type            = t_none;
  static int depth    = 0;
  int in_style        = 0;
  int should_be_empty = 0;
  int tagpos          = 0;
  MrgString *style = mrg_string_new ("");
  int whitespaces = 0;

  _mrg_set_wrap_edge_vfuncs (mrg, wrap_edge_left, wrap_edge_right, ctx);
  _mrg_set_post_nl (mrg, _mrg_draw_background_increment, ctx);
  ctx->mrg = mrg;
  ctx->state = &ctx->states[0];

  while (type != t_eof)
  {
    char *data;
    type = xmltok_get (xmltok, &data, &pos);

    if (type == t_tag ||
        type == t_att ||
        type == t_endtag ||
        type == t_closeemptytag ||
        type == t_closetag)
    {
      int i;
      for (i = 0; data[i]; i++)
        data[i] = tolower (data[i]);
    }

    switch (type)
    {
      case t_entity:
        {
          int i;
          int dealt_with = 0;
          if (data[0]=='#')
          {
            int c = atoi (&data[1]);
            mrg_printf (mrg, "%c", c);
          }
          else
          for (i = 0; entities[i][0] && !dealt_with; i++)
            if (!strcmp (data, entities[i][0]))
            {
              mrg_print (mrg, entities[i][1]);
              dealt_with = 1;
            }
          
          
          if (!dealt_with){
            mrg_start (mrg, "dim", (void*)pos);
            mrg_print (mrg, data);
            mrg_end (mrg);
          }
        }
        break;
      case t_word:
        if (in_style)
        {
          mrg_stylesheet_add (mrg, data, uri_base, fetch, fetch_data, NULL);
        }
        else
        {
          mrg_print (mrg, data);
        }
        whitespaces = 0;
        break;

      case t_whitespace:
        if (in_style)
        {
          mrg_stylesheet_add (mrg, data, uri_base, fetch, fetch_data, NULL);
        }
        else
        {
          switch (mrg_style (mrg)->white_space)
          {
            case MRG_WHITE_SPACE_PRE: /* handles as pre-wrap for now */
            case MRG_WHITE_SPACE_PRE_WRAP:
              mrg_print (mrg, data);
              break;
            case MRG_WHITE_SPACE_PRE_LINE:
              switch (*data)
              {
                case ' ':
                  whitespaces ++;
                  if (whitespaces == 1)
                    mrg_print (mrg, " ");
                  break;
                case '\n':
                  whitespaces = 0;
                  break;
              }
              break;
            case MRG_WHITE_SPACE_NOWRAP: /* XXX: handled like normal, this is bad.. */
            case MRG_WHITE_SPACE_NORMAL: 
              whitespaces ++;
              if (whitespaces == 1)
                mrg_print (mrg, " ");
              break;
          }
        }
        break;
      case t_tag:
        ctx->attributes = 0;
        tagpos = pos;
        mrg_string_clear (style);
        break;
      case t_att:
        // XXX: use strncpy
        strcpy (ctx->attribute[ctx->attributes], data);
        break;
      case t_val:
        strcpy (ctx->value[ctx->attributes++], data);
        break;
      case t_endtag:


        if (depth && ((!strcmp (data, "tr") && !strcmp (tag[depth-1], "td"))))
        {
          mrg_end (mrg);
          depth--;
          mrg_end (mrg);
          depth--;
        }
        if (depth && ((!strcmp (data, "tr") && !strcmp (tag[depth-1], "td"))))
        {
          mrg_end (mrg);
          depth--;
          mrg_end (mrg);
          depth--;
        }
        else if (depth && ((!strcmp (data, "dd") && !strcmp (tag[depth-1], "dt")) ||
                      (!strcmp (data, "li") && !strcmp (tag[depth-1], "li")) ||
                      (!strcmp (data, "dt") && !strcmp (tag[depth-1], "dd")) ||
                      (!strcmp (data, "td") && !strcmp (tag[depth-1], "td")) ||
                      (!strcmp (data, "tr") && !strcmp (tag[depth-1], "tr")) ||
                      (!strcmp (data, "dd") && !strcmp (tag[depth-1], "dd")) ||
                      (!strcmp (data, "p") && !strcmp (tag[depth-1], "p"))))
        {
          mrg_end (mrg);
          depth--;
        }

        strcpy (tag[depth], data);
        depth ++;

        {
          char combined[256]="";
          char *klass = (void*)get_attr (ctx, "class");
          /* XXX: spaces in class should be turned into .s making
           * it possible to use multiple classes
           */
          const char *id = get_attr (ctx, "id");

          const char *pseudo = "";

          if (klass)
          {
            klass = strdup (klass);
            if(1){
              int i;
              for (i = 0; klass[i]; i++)
                if (klass[i] == ' ')
                  klass[i]='.';
            }
          }

          //if (mrg_style(mrg)->id_ptr)
          { // XXX : perhaps do this a tiny bit differently?
            MrgGeoCache *geo = _mrg_get_cache (ctx, (void*)tagpos);
            if (geo && geo->hover)
            {
              if (mrg->pointer_down[1])
                pseudo = ":active:hover";
              else
                pseudo = ":hover";
            }
          }
          sprintf (combined, "%s%s%s%s%s%s",
              data,
              klass?".":"",
              klass?klass:"",
              id?"#":"",
              id?id:"", pseudo);

          if (klass)
            free (klass);
          {
            /* collext XML attributes and convert into CSS declarations */
            const char *style_attribute[] ={
              "fill-rule",
              "font-size",
              "font-family",
              "fill-color",
              "fill", 
              "stroke-width",
              "stroke-color",
              "stroke-linecap",
              "stroke-miterlimit",
              "stroke-linejoin",
              "stroke",
              "color",
              "background-color",
              "background",
              NULL};

            int i;
            for (i = 0; i < ctx->attributes; i++)
            {
              int j;
              for (j = 0; style_attribute[j]; j++)
                if (!strcmp (ctx->attribute[i], style_attribute[j]))
                {
                  mrg_string_append_printf (style, "%s: %s;",
                      style_attribute[j], ctx->value[i]);
                  break;
                }
            }
          }
          mrg_string_append_str (style, get_attr (ctx, "style"));
          mrg_start_with_style (mrg, combined, (void*)(tagpos), style->str);
        }

        if (!strcmp (data, "g"))
        {
          const char *transform;
          if ((transform = get_attr (ctx, "transform")))
            {
              cairo_matrix_t matrix;
              mrg_parse_transform (mrg, &matrix, transform);
              cairo_transform (mrg_cr (mrg), &matrix);
            }
        }

        if (!strcmp (data, "polygon"))
        {
          mrg_parse_polygon (mrg, get_attr (ctx, "d"));
          mrg_path_fill_stroke (mrg);
        }

        if (!strcmp (data, "path"))
        {
          mrg_parse_path (mrg, get_attr (ctx, "d"));
          mrg_path_fill_stroke (mrg);
        }

        if (!strcmp (data, "rect"))
        {
          float width, height, x, y;
          const char *val;
          val = get_attr (ctx, "width");
          width = val ? mrg_parse_float (mrg, val, NULL) : 0;
          val = get_attr (ctx, "height");
          height = val ? mrg_parse_float (mrg, val, NULL) : 0;
          val = get_attr (ctx, "x");
          x = val ? mrg_parse_float (mrg, val, NULL) : 0;
          val = get_attr (ctx, "y");
          y = val ? mrg_parse_float (mrg, val, NULL) : 0;

          cairo_rectangle (mrg_cr (mrg), x, y, width, height);
          mrg_path_fill_stroke (mrg);
        }

        if (!strcmp (data, "text"))
        {
          mrg->x = mrg_parse_float (mrg, get_attr (ctx, "x"), NULL);
          mrg->y = mrg_parse_float (mrg, get_attr (ctx, "y"), NULL);
        }

        if (!strcmp (data, "img") && get_attr (ctx, "href"))
        {
          int img_width, img_height;
          const char *href = get_attr (ctx, "href");
          mrg_printf (mrg, "\n");

          if (mrg_query_image (mrg, href, &img_width, &img_height))
          {
            mrg->y += img_height;
            _mrg_draw_background_increment (mrg, &mrg->html, 0);
            mrg_image (mrg, mrg->x, mrg->y - img_height, -1, -1, href);
          }
          else
          {
            mrg_printf (mrg, "[%s]", href);
          }
        }
          
        if (!strcmp (data, "a"))
        {
          if (link_cb && get_attr (ctx, "href")) 
            mrg_text_listen_full (mrg, MRG_CLICK, link_cb, _mrg_resolve_uri (uri_base,  get_attr (ctx, "href")), link_data, (void*)free, NULL); //XXX: free is not invoked according to valgrind
        }

        if (!strcmp (data, "style"))
          in_style = 1;
        else
          in_style = 0;

        should_be_empty = 0;

        if (!strcmp (data, "link"))
        {
          const char *rel;
          if (fetch && (rel=get_attr (ctx, "rel")) && !strcmp (rel, "stylesheet") && get_attr (ctx, "href"))
          {
            char *contents;
            long length;
            fetch (uri_base, get_attr (ctx, "href"), &contents, &length, fetch_data);
            if (contents)
            {
              mrg_stylesheet_add (mrg, contents, uri_base, fetch, fetch_data, NULL);
              free (contents);
            }
          }
        }

        if (!strcmp (data, "link") ||
            !strcmp (data, "meta") ||
            !strcmp (data, "input") ||
            !strcmp (data, "img") ||
            !strcmp (data, "br") ||
            !strcmp (data, "hr"))
        {
          should_be_empty = 1;
          mrg_end (mrg);
          depth--;
        }
        break;

      case t_closeemptytag:

      case t_closetag:
        if (!should_be_empty)
        {
          if (!strcmp (data, "a"))
          {
            mrg_text_listen_done (mrg);
          }
          in_style = 0;
          mrg_end (mrg);
          depth--;

          if (strcmp (tag[depth], data))
          {
            if (!strcmp (tag[depth], "p"))
            {
              mrg_end (mrg);
              depth --;
            } else 
            if (depth > 0 && !strcmp (tag[depth-1], data))
            {
              //fprintf (stderr, "%i: fixing close of %s when %s is open\n", pos, data, tag[depth]);

              mrg_end (mrg);
              depth --;
            }
            else if (depth > 1 && !strcmp (tag[depth-2], data))
            {
              int i;
              fprintf (stderr, "%i: fixing close of %s when %s was open\n", pos, data, tag[depth]);

              for (i = 0; i < 2; i ++)
              {
                depth --;
                mrg_end (mrg);
              }
            }
            else if (depth > 2 && !strcmp (tag[depth-3], data))
            {
              int i;
              fprintf (stderr, "%i: fixing close of %s when %s wass open\n", pos, data, tag[depth]);

              for (i = 0; i < 3; i ++)
              {
                depth --;
                mrg_end (mrg);
              }
            }
            else if (depth > 3 && !strcmp (tag[depth-4], data))
            {
              int i;
              fprintf (stderr, "%i: fixing close of %s when %s wasss open\n", pos, data, tag[depth]);

              for (i = 0; i < 4; i ++)
              {
                depth --;
                mrg_end (mrg);
              }
            }
            else if (depth > 4 && !strcmp (tag[depth-5], data))
            {
              int i;
              fprintf (stderr, "%i: fixing close of %s when %s wassss open\n", pos, data, tag[depth]);

              for (i = 0; i < 5; i ++)
              {
                depth --;
                mrg_end (mrg);
              }

            }
            else
            {
              if (!strcmp (data, "table") && !strcmp (tag[depth], "td"))
              {
                depth--;
                mrg_end (mrg);
                depth--;
                mrg_end (mrg);
              }
              else if (!strcmp (data, "table") && !strcmp (tag[depth], "tr"))
              {
                depth--;
                mrg_end (mrg);
              }
              else
              fprintf (stderr, "%i closed %s but %s is open\n", pos, data, tag[depth]);
            }
          }
        }
        break;
    }
  }

  xmltok_free (xmltok);
  if (depth!=0){
    fprintf (stderr, "html parsing unbalanced, %i open tags.. \n", depth);
    while (depth > 0)
    {
      fprintf (stderr, " %s ", tag[depth-1]);
      mrg_end (mrg);
      depth--;
    }
    fprintf (stderr, "\n");
  }

//  mrg_list_free (&ctx->geo_cache); /* XXX: no point in doing that here */
  mrg_string_free (style, 1);
}

void mrg_print_xml (Mrg *mrg, char *xml)
{
  mrg_xml_render (mrg, NULL, xml, NULL, NULL, NULL, NULL);
}

void
mrg_printf_xml (Mrg *mrg, const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;
  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = malloc(needed);
  va_end (ap);
  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);
  mrg_print_xml (mrg, buffer);
  free (buffer);
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-string.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

// bundled include of mrg-string.h"
// bundled include of mrg.h"
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void mrg_string_init (MrgString *string)
{
  string->allocated_length = 8;
  string->length = 0;
  string->str = malloc (string->allocated_length);
  string->str[0]='\0';
}
static void mrg_string_destroy (MrgString *string)
{
  if (string->str)
    free (string->str);
}

MrgString *mrg_string_new (const char *initial)
{
  MrgString *string = calloc (sizeof (MrgString), 1);
  mrg_string_init (string);
  if (initial)
    mrg_string_append_str (string, initial);
  return string;
}
void mrg_string_clear (MrgString *string)
{
  string->length = 0;
  string->str[string->length]=0;
}

void mrg_string_append_byte (MrgString *string, char  val)
{
  if (string->length + 2 >= string->allocated_length)
    {
      char *old = string->str;
      string->allocated_length *= 2;
      string->str = malloc (string->allocated_length);
      memcpy (string->str, old, string->allocated_length/2);
      free (old);
    }
  string->str[string->length++] = val;
  string->str[string->length] = '\0';
}

void mrg_string_append_unichar (MrgString *string, unsigned int unichar)
{
  char *str;
  char utf8[5];
  utf8[mrg_unichar_to_utf8 (unichar, (unsigned char*)utf8)]=0;
  str = utf8;

  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
      str++;
    }
}

void mrg_string_append_str (MrgString *string, const char *str)
{
  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
      str++;
    }
}

void mrg_string_append_data (MrgString *string, const char *str, int len)
{
  int i;
  for (i = 0; i<len; i++)
    mrg_string_append_byte (string, str[i]);
}

void mrg_string_append_string (MrgString *string, MrgString *string2)
{
  const char *str = mrg_string_get (string2);
  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
      str++;
    }
}

const char *mrg_string_get (MrgString *string)
{
  return string->str;
}

int mrg_string_get_length (MrgString *string)
{
  return string->length;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */
char *mrg_string_dissolve   (MrgString *string)
{
  char *ret = string->str;
  string->str = NULL;
  free (string);
  return ret;
}

void
mrg_string_free (MrgString *string, int freealloc)
{
  if (freealloc)
    {
      mrg_string_destroy (string);
    }
  free (string);
}

void
mrg_string_append_printf (MrgString *string, const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;
  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = malloc(needed);
  va_end (ap);
  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);
  mrg_string_append_str (string, buffer);
  free (buffer);
}

void
mrg_string_set (MrgString *string, const char *new_string)
{
  mrg_string_clear (string);
  mrg_string_append_str (string, new_string);
}

// bundled include of mrg-list.h"
static MrgList *interns = NULL;

const char * mrg_intern_string (const char *str)
{
  MrgList *i;
  for (i = interns; i; i = i->next)
  {
    if (!strcmp (i->data, str))
      return i->data;
  }
  str = strdup (str);
  mrg_list_append (&interns, (void*)str);
  return str;
}


int mrg_utf8_len (const unsigned char first_byte)
{
  if      ((first_byte & 0x80) == 0)
    return 1; /* ASCII */
  else if ((first_byte & 0xE0) == 0xC0)
    return 2;
  else if ((first_byte & 0xF0) == 0xE0)
    return 3;
  else if ((first_byte & 0xF8) == 0xF0)
    return 4;
  return 1;
}

int
mrg_unichar_to_utf8 (unsigned int  ch,
                     unsigned char*dest)
{
/* http://www.cprogramming.com/tutorial/utf8.c  */
/*  Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005 ... */
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

int mrg_utf8_strlen (const char *s)
{
   int count;
   if (!s)
     return 0;
   for (count = 0; *s; s++)
     if ((*s & 0xC0) != 0x80)
       count++;
   return count;
}

const char *mrg_utf8_skip (const char *string, int utf8_length)
{
  const char *s;
  int len;
  int pos = 0;

  if (!string)
    return NULL;

  for (s = string; pos < utf8_length && *s; s += mrg_utf8_len (*s))
    {
      int c;
      len = mrg_utf8_len (*s);
      for (c = 0; c < len; c++)
        {
          if (!s[c])
            return s;
        }
      pos++;
    }
  return s;
}
/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌mrg-http.c▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// bundled include of mrg-string.h"

#define HAS_GETHOSTBYNAME


char *_mrg_http (const char *ip,
                 const char *hostname,
                 int         port,
                 const char *path,
                 int        *length)
{
#ifdef HAS_GETHOSTBYNAME
  struct hostent    * host;
#endif
	struct sockaddr_in  addr;
	int sock;

  sock = socket (PF_INET, SOCK_STREAM, 0);

  //mrg_LOG("http", "GET %s:%i%s", hostname, port, path);

  if ( sock >= 0 )
    {
      MrgString *str = mrg_string_new ("");
      memset (&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons (port);

      if (ip)
      {
        addr.sin_addr.s_addr = inet_addr (ip);
      }
      else
      {
#ifdef HAS_GETHOSTBYNAME
        /* it might be better to do this by ip.. */
        host = gethostbyname (hostname);
        if (!host)
          return NULL;
        addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
#else
        /* XXX: should fall back to try using `hostname` to resolve
         *      the hostname...
         */
        fprintf (stderr, "ip needed on this platform\n");
#endif
      }


      if (connect (sock, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        {	
          int count;
          char s[4096];
          sprintf(s, "GET %s HTTP/1.0\r\n", path);
          write (sock, s, strlen (s));
          if (hostname)
          {
            sprintf(s, "Host: %s\r\n", hostname);
            write (sock, s, strlen (s));
          }
          sprintf(s, "User-Agent: mr/0.0.0\r\n");
          write (sock, s, strlen (s));
          sprintf(s, "\r\n");
          write (sock, s, strlen (s));
          fsync (sock);

          while ((count = read (sock, s, sizeof (s))))
            mrg_string_append_data (str, s, count);
        }
      if (str->length)
        {
          if (strstr (str->str, "HTTP/1.1 200") ||
              strstr (str->str, "HTTP/1.0 200"))
            {
              int start = 0;
              int i;
              char *copy;
              for (i = 0; str->str[i]; i++)
                {
                  if (str->str[i] == '\r' &&
                      str->str[i+1] == '\n' &&
                      str->str[i+2] == '\r' &&
                      str->str[i+3] == '\n')
                    {
                      start = i + 4;
                      break;
                    }
                }
              copy = malloc (str->length + 1 - start);
              memcpy (copy, &(str->str[start]), str->length - start);
              copy[str->length - start] = 0;
              if (length)
                *length = str->length - start;
              //mrg_LOG("http", "got %i bytes", str->length - start);
              mrg_string_free (str, 1);
              fprintf (stderr, "[%s]\n", copy);
              return copy;
            }
          else
            {
              /* XXX: should return error codes back somehow */
              //printf ("\nXXXXX  NON200 response XXXXX:\n\n%s\n", str->str);
              mrg_string_free (str, 1);
            }
        }
      else
        mrg_string_free (str, 1);

      shutdown (sock, SHUT_RDWR);
    }
  if (length)
    *length = -1;
  //mrg_LOG("http", "failed");
  return NULL;
}

typedef struct _MrgHttpConnection MrgHttpConnection;

struct _MrgHttpConnection {
  struct hostent      * host;
  struct sockaddr_in    addr;
  int    sock;
};

static MrgHttpConnection *http_connection_new (const char *ip,
                                               const char *hostname,
                                               int port)
{
  MrgHttpConnection *c;
  c = calloc (sizeof (MrgHttpConnection), 1);

  //mrg_LOG("http", "POST %s:%i%s %i", hostname, port, path, length);

  c->sock = socket (PF_INET, SOCK_STREAM, 0);

  if (c->sock >= 0 )
    {
      memset (&c->addr, 0, sizeof(c->addr));
      c->addr.sin_family = AF_INET;
      c->addr.sin_port = htons (port);

      if (ip)
      {
        c->addr.sin_addr.s_addr = inet_addr (ip);
      }
      else
      {
#ifdef HAS_GETHOSTBYNAME
        /* it might be better to do this by ip.. */
        c->host = gethostbyname (hostname);
        if (!c->host)
          return NULL;
        c->addr.sin_addr.s_addr = *(long*)(c->host->h_addr_list[0]);
#else
        fprintf (stderr, "ip needed on this platform\n");
#endif
      }

      if (connect (c->sock, (struct sockaddr*)&c->addr, sizeof(c->addr)) == 0)
        {
          return c;
        }
    }
  free (c);
  return NULL;
}

static void http_connection_free (MrgHttpConnection *connection)
{
  if (connection->sock)
    close (connection->sock);
  free (connection);
}

char *_mrg_http_post (const char *ip,
                      const char *hostname,
                      int         port,
                      const char *path,
                      const char *body,
                      int         length,
                      int        *ret_length)
{
  MrgHttpConnection *c = http_connection_new (ip, hostname, port);
  if (!c)
    {
      if (ret_length)
        *ret_length = -1;
      fprintf (stderr, "http failed\n");
      return NULL;
    }

    {
      MrgString *str = mrg_string_new ("");
      int count;
      char s[512];
      if (length < 0)
        length = strlen (body);

#define WRITE_DATA(str,len)     write(c->sock, str, len)
#define WRITE(str)              WRITE_DATA(str,strlen(str))

      sprintf(s, "POST %s HTTP/1.0\r\n", path); WRITE (s);
      sprintf(s, "User-Agent: zn/0.0.0\r\n"); WRITE (s);
      sprintf(s, "Content-Length: %d\r\n", length); WRITE (s);
      sprintf(s, "\r\n"); WRITE (s);
      WRITE_DATA(body,length);
      fsync (c->sock); /* write all data, before being able to
                          expect data back */

      while ((count = read (c->sock, s, sizeof (s))))
        mrg_string_append_data (str, s, count);
      if (str->length)
        {
          if (strstr (str->str, "HTTP/1.1 200") ||
              strstr (str->str, "HTTP/1.0 200"))
            {
              int start = 0;
              int i;
              char *copy;
              for (i = 0; str->str[i]; i++)
                {
                  if (str->str[i]   == '\r' &&
                      str->str[i+1] == '\n' &&
                      str->str[i+2] == '\r' &&
                      str->str[i+3] == '\n')
                    {
                      start = i + 4;
                      break;
                    }
                }
              copy = malloc (str->length + 1 - start);
              memcpy (copy, &(str->str[start]), str->length - start);
              copy[str->length - start] = 0;
              if (ret_length)
                *ret_length = str->length - start;
              //mrg_LOG("http", "got %i bytes", str->length - start);
              mrg_string_free (str, 1);

              http_connection_free (c);

              fprintf (stderr, "[%s]\n", copy);
              return copy;
            }
          else
            {
              printf ("%s\n", str->str);
              mrg_string_free (str, 1);
            }
        }
      else
        mrg_string_free (str, 1);
    }

  if (ret_length)
    *ret_length = -1;
  fprintf (stderr, "http failed\n");
  http_connection_free (c);
  return NULL;
}
#endif
