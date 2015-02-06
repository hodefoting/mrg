-- ffi lua binding for microraptor gui by Øyvind Kolås, public domain
--
--
local ffi = require('ffi')
local cairo = require('cairo')
local C = ffi.load('mrg')
local M = setmetatable({C=C},{__index = C})

ffi.cdef[[
typedef struct _Mrg Mrg;
typedef struct _MrgColor MrgColor;
typedef struct _MrgStyle MrgStyle;
typedef struct _MrgRectangle MrgRectangle;
typedef struct _MrgEvent MrgEvent;

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


typedef enum _MrgType MrgType;

typedef int (*MrgCb) (MrgEvent *event,
                      void     *data,
                      void     *data2);

typedef void(*MrgDestroyNotify) (void     *data);
typedef int(*MrgTimeoutCb) (Mrg *mrg, void  *data);
typedef int(*MrgIdleCb)    (Mrg *mrg, void  *data);

typedef void (*UiRenderFun)(Mrg *mrg, void *ui_data);

void  mrg_destroy       (Mrg *mrg);

void  mrg_set_size      (Mrg *mrg, int width, int height);
void  mrg_set_position  (Mrg *mrg, int x, int y);
void  mrg_get_position  (Mrg *mrg, int *x, int *y);

void  mrg_set_title     (Mrg *mrg, const char *title);
const char *mrg_get_title (Mrg *mrg);

int   mrg_width         (Mrg *mrg);
int   mrg_height        (Mrg *mrg);

Mrg *mrg_new(int width, int height, const char *backend);

void  mrg_set_ui        (Mrg *mrg, UiRenderFun, void *ui_data);

void  mrg_main          (Mrg *mrg);
void  mrg_quit          (Mrg *mrg);
cairo_t *mrg_cr         (Mrg *mrg);
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
int mrg_get_contents (Mrg         *mrg,
                      const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length);

int
mrg_get_contents_default (const char  *referer,
                          const char  *input_uri,
                          char       **contents,
                          long        *length,
                          void        *ignored_user_data);

void mrg_set_mrg_get_contents (Mrg *mrg,
  int (*mrg_get_contents) (const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length,
                      void        *get_contents_data),
  void *get_contents_data);


void mrg_render_to_mrg (Mrg *mrg, Mrg *mrg2, float x, float y);

int mrg_add_idle (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data), void *   idle_data);
int mrg_add_timeout (Mrg *mrg, int ms, int (*idle_cb)(Mrg *mrg, void *          idle_data), void *idle_data);
void mrg_remove_idle (Mrg *mrg, int handle);

int mrg_add_idle_full (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data),     void *idle_data,
                       void (*destroy_notify)(void *destroy_data),    void *destroy_data);

int mrg_add_timeout_full (Mrg *mrg, int ms, MrgTimeoutCb idle_cb, void *idle_data,
                       void (*destroy_notify)(void *destroy_data),    void *destroy_data);


int mrg_print (Mrg *mrg, const char *string);
int mrg_print_xml (Mrg *mrg, const char *string);

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
                      void   (*finalize)(void *listen_data, void *listen_data2,  void *finalize_data),
                      void    *finalize_data);

/* these deal with pointer_no 0 only
 */
void  mrg_warp_pointer (Mrg *mrg, float x, float y);
float mrg_pointer_x    (Mrg *mrg);
float mrg_pointer_y    (Mrg *mrg);

float mrg_em (Mrg *mrg);
void  mrg_set_xy (Mrg *mrg, float x, float y);

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

void  mrg_text_listen (Mrg *mrg, MrgType types,
                       MrgCb cb, void *data1, void *data2);

void  mrg_text_listen_full (Mrg *mrg, MrgType types,
                            MrgCb cb, void *data1, void *data2,
          void (*finalize)(void *listen_data, void *listen_data2, void *       finalize_data),
          void  *finalize_data);

void  mrg_text_listen_done (Mrg *mrg);

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
  MrgFillRule         fill_rule;
  MrgFontStyle        font_style;
  MrgFontWeight       font_weight;
  MrgLineCap          stroke_linecap;
  MrgLineJoin         stroke_linejoin;
  MrgTextAlign        text_align;
  MrgFloat            float_;
  MrgClear            clear;
  MrgOverflow         overflow;
  MrgDisplay          display;
  MrgPosition         position;
  MrgBoxSizing        box_sizing;
  MrgVerticalAlign    vertical_align;
  MrgWhiteSpace       white_space;
  MrgUnicodeBidi      unicode_bidi;
  MrgDirection        direction;
  MrgListStyle        list_style;
  unsigned char       stroke;
  unsigned char       fill;
  unsigned char       width_auto;
  unsigned char       margin_left_auto;
  unsigned char       margin_right_auto;
  unsigned char       print_symbols;
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
                         int priority,
                         char **error);

void mrg_css_set (Mrg *mrg, const char *css);
void mrg_css_add (Mrg *mrg, const char *css);

void mrg_set_style (Mrg *mrg, const char *style);


void  mrg_set_line_height (Mrg *mrg, float line_height);
void  mrg_set_font_size (Mrg *mrg, float font_size);
float  mrg_line_height (Mrg *mrg);

/* XXX: doesnt feel like it belongs here */
void mrg_image (Mrg *mrg, float x0, float y0, float width, float height, const char *path);

]]

function M.new(width,height, backend) return C.mrg_new(width, height, backend) end

ffi.metatype('Mrg', {__index = {
  -- maybe we should add a _full version to this as well, then all callback
  -- things in the code would be lifecycle managed.
  set_ui           = function (mrg, uifun, uidata) C.mrg_set_ui(mrg, uifun, uidata) end,
  style            = function (...) return C.mrg_style (...) end;
  width            = function (...) return C.mrg_width (...) end,
  height           = function (...) return C.mrg_height (...) end,
  pointer_x        = function (...) return C.mrg_pointer_x (...) end,
  pointer_y        = function (...) return C.mrg_pointer_y (...) end,
  text_listen_done = function (...) C.mrg_text_listen_done(...) end,
  warp_pointer     = function (...) C.mrg_warp_pointer(...) end,
  quit             = function (...) C.mrg_quit(...) end,
  image            = function (...) C.mrg_image(...) end,
  css_set          = function (...) C.mrg_css_set(...) end,
  set_style        = function (...) C.mrg_set_style(...) end,
  css_add          = function (...) C.mrg_css_add(...) end,
  set_em           = function (...) C.mrg_set_em(...) end,
  set_rem          = function (...) C.mrg_set_rem(...) end,
  main             = function (...) C.mrg_main(...) end,
  start            = function (mrg, cssid) C.mrg_start(mrg, cssid, NULL) end,
  start_with_style = function (mrg, cssid, style) C.mrg_start_with_style(mrg, cssid, NULL, style) end,
  close            = function (...) C.mrg_end(...) end,
  queue_draw       = function (...) C.mrg_queue_draw(...) end,

  add_binding = function(mrg,key,action,label,cb,db_data)
    local notify_fun, cb_fun;
    local notify_cb = function (data1, data2,finalize_data)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    cb_fun = ffi.cast ("MrgCb", cb)
    return C.mrg_add_binding_full (mrg, key, action,label,cb_fun, cb_data, notify_fun, NULL)
  end,

  text_listen      = function (mrg, types, cb, data1, data2)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (data1, data2,finalize_data)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end
    notify_fun = ffi.cast ("MrgCb", notify_cb)
    cb_fun = ffi.cast ("MrgCb", cb)
    return C.mrg_text_listen_full (mrg, types, cb_fun, data1, data2, notify_fun, NULL)
  end,
  listen           = function (mrg, types, x, y, w, h, cb, data1, data2)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (data1, data2, finalize_data)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end
    notify_fun = ffi.cast ("MrgCb", notify_cb)
    cb_fun = ffi.cast ("MrgCb", cb)
    return C.mrg_listen_full (mrg, types, x, y, w, h, cb_fun, data1, data2, notify_fun, NULL)
  end,
  print            = function (...) C.mrg_print(...) end,
  print_xml        = function (...) C.mrg_print_xml(...) end,
  cr               = function (...) return C.mrg_cr (...) end,
  width            = function (...) return C.mrg_width (...) end,
  height           = function (...) return C.mrg_height (...) end,
  x                = function (...) return C.mrg_x (...) end,
  y                = function (...) return C.mrg_y (...) end,
  em               = function (...) return C.mrg_em (...) end,
  rem              = function (...) return C.mrg_rem (...) end,
  set_xy           = function (...) C.mrg_set_xy (...) end,
  set_size         = function (...) C.mrg_set_size (...) end,
  set_position     = function (...) C.mrg_set_position(...) end,
  set_title        = function (...) C.mrg_set_title (...) end,
  get_title        = function (...) return C.mrg_get_title (...) end,
  set_font_size    = function (...) C.mrg_set_font_size (...) end,

  add_idle = function (mrg, ms, cb, data1)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (a,b)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    cb_fun = ffi.cast ("MrgIdleCb", cb)
    return C.mrg_add_idle_full (mrg, cb_fun, data1, notify_fun, NULL)
  end,

  add_timeout = function (mrg, ms, cb, data1)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (a,b)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    cb_fun = ffi.cast ("MrgTimeoutCb", cb)
    return C.mrg_add_timeout_full (mrg, ms, cb_fun, data1, notify_fun, NULL)
  end,

  remove_idle      = function (...) return C.mrg_remove_idle (...) end,
  edge_left        = function (...) return C.mrg_edge_left (...) end,
  set_edge_left    = function (...) C.mrg_set_edge_left (...) end,
  edge_right       = function (...) return C.mrg_edge_right (...) end,
  set_edge_right   = function (...) C.mrg_set_edge_right (...) end,
  edge_top         = function (...) return C.mrg_edge_top (...) end,
  set_edge_top     = function (...) C.mrg_set_edge_top (...) end,
  edge_bottom      = function (...) return C.mrg_edge_bottom (...) end,
  set_edge_bottom  = function (...) C.mrg_set_edge_bottom (...) end,
}})
ffi.metatype('MrgEvent',     {__index = { }})
ffi.metatype('MrgColor',     {__index = { }})
ffi.metatype('MrgStyle',     {__index = { }})
ffi.metatype('MrgRectangle', {__index = { }})

  M.MOTION = C.MRG_MOTION;
  M.ENTER = C.MRG_ENTER;
  M.LEAVE = C.MRG_LEAVE;
  M.PRESS = C.MRG_PRESS;
  M.RELEASE = C.MRG_RELEASE;
  M.DRAG_MOTION = C.MRG_DRAG_MOTION;
  M.DRAG_PRESS = C.MRG_DRAG_PRESS;
  M.DRAG_RELEASE = C.MRG_DRAG_RELEASE;

return M
