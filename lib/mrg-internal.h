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

#include "mrg.h"
#include "mrg-list.h"
#include "mrg-string.h"
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

typedef struct _MrgBackend MrgBackend;
struct _MrgBackend {
  const char *name;
  Mrg *             (*mrg_new) (int width, int height);


  /* backend callbacks */
  void (*mrg_main) (Mrg *mrg,
                    void (*ui_update)(Mrg *mrg, void *user_data),
                                                void *user_data);
  unsigned char    *(*mrg_get_pixels)   (Mrg *mrg, int *rowstride);
  cairo_t          *(*mrg_cr)           (Mrg *mrg);
  void              (*mrg_flush)        (Mrg *mrg);
  void              (*mrg_prepare)      (Mrg *mrg);
  void              (*mrg_clear)        (Mrg *mrg);
  void              (*mrg_queue_draw)   (Mrg *mrg, MrgRectangle *rectangle);
  void              (*mrg_destroy)      (Mrg *mrg);
  void              (*mrg_warp_pointer) (Mrg *mrg, float x, float y);
  void              (*mrg_fullscreen)   (Mrg *mrg, int fullscreen);

  void              (*mrg_set_position) (Mrg *mrg, int x, int y);
  void              (*mrg_get_position) (Mrg *mrg, int *x, int *y);
  void              (*mrg_set_title)    (Mrg *mrg, const char *title);
  const char *      (*mrg_get_title)    (Mrg *mrg);
};

struct _Mrg {
  float          rem;

  MrgHtml        html;

  cairo_t       *cr;
  int            width;
  int            height;
  float          ddpx;

  MrgList       *stylesheet;
  void          *css_parse_state;

  MrgString     *style;
  MrgString     *style_global;

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
  void          *backend_data;
  int            do_clip;

  MrgEvent drag_event;

  int (*mrg_get_contents) (const char  *referer,
                           const char  *input_uri,
                           char       **contents,
                           long        *length,
                           void        *get_contents_data);
  void *get_contents_data;

  void (*ui_update)(Mrg *mrg, void *user_data);
  void *user_data;

  MrgBackend *backend;

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
  int          idle_id;

};

int _mrg_file_get_contents (const char  *path,
                            char       **contents,
                            long        *length);

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

void *mrg_mmm (Mrg *mrg);

#if MRG_LOG

#define MRG_LOG_ERROR    0
#define MRG_LOG_FATAL   -1
#define MRG_LOG_WARNING  1
#define MRG_LOG_LOG      5
#define MRG_LOG_INFO    10

void _mrg_log (Mrg *mrg,
               const char *file,
               const char *function,
               int line_no,
               int type,
               const char *format, ...);

#define MRG_STYLE_INTERNAL 10
#define MRG_STYLE_GLOBAL   15
#define MRG_STYLE_XML      20
#define MRG_STYLE_APP      20
#define MRG_STYLE_INLINE   25
#define MRG_STYLE_CODE     30


#define MRG_INFO(a...)   _mrg_log(mrg,__FILE__,__FUNCTION__, __LINE__,MRG_LOG_INFO, a)
#define MRG_WARNING(...) _mrg_log(mrg,__FILE__,__FUNCTION__, __LINE__,MRG_LOG_WARNING a,
#define MRG_ERROR(...)   _mrg_log(mrg,__FILE__,__FUNCTION__, __LINE__,MRG_LOG_ERROR, a)
#define MRG_FATAL(...)   do{_mrg_log(mrg,__FILE__,__FUNCTION__,__LINE__,MRG_LOG_ERROR, a);assert(0);}while(0)

#else

#define MRG_INFO(a...)    do{}while(0);
#define MRG_WARNING(a...) do{}while(0);
#define MRG_ERROR(a...)   do{fprintf(stderr,awhile(0);
#define MRG_FATAL(a...)   exit(-1)

#endif

#endif
