-- ffi lua binding for microraptor gui by Øyvind Kolås, public domain
--
--
local ffi = require('ffi')
local cairo = require('cairo')
local C = ffi.load('mrg')
local M = setmetatable({C=C},{__index = C})

ffi.cdef[[

typedef struct _MrgList MrgList;

struct _MrgList {
  void *data;
  MrgList *next;
  void (*freefunc)(void *data, void *freefunc_data);
  void *freefunc_data;
}
;
void mrg_list_prepend_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data);

int mrg_list_length (MrgList *list);

void mrg_list_prepend (MrgList **list, void *data);

void mrg_list_append_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data);

void mrg_list_append (MrgList **list, void *data);

void mrg_list_remove (MrgList **list, void *data);

void mrg_list_free (MrgList **list);

MrgList *mrg_list_nth (MrgList *list, int no);

MrgList *mrg_list_find (MrgList *list, void *data);

void mrg_list_sort (MrgList **list, 
    int(*compare)(const void *a, const void *b, void *userdata),
    void *userdata);

void
mrg_list_insert_sorted (MrgList **list, void *data,
                       int(*compare)(const void *a, const void *b, void *userdata),
                       void *userdata);

typedef struct _Mrg Mrg;
typedef struct _MrgColor     MrgColor;
typedef struct _MrgStyle     MrgStyle;
typedef struct _MrgRectangle MrgRectangle;
typedef struct _MrgEvent     MrgEvent;

enum _MrgType {
  MRG_PRESS          = 1 << 0,
  MRG_MOTION         = 1 << 1,
  MRG_RELEASE        = 1 << 2,
  MRG_ENTER          = 1 << 3,
  MRG_LEAVE          = 1 << 4,
  MRG_TAP            = 1 << 5, /* NYI */
  MRG_TAP_AND_HOLD   = 1 << 6, /* NYI */
  MRG_DRAG_PRESS     = 1 << 7,
  MRG_DRAG_MOTION    = 1 << 8,
  MRG_DRAG_RELEASE   = 1 << 9,
  MRG_KEY_DOWN       = 1 << 10,
  MRG_KEY_UP         = 1 << 11,

  MRG_TAPS     = (MRG_TAP | MRG_TAP_AND_HOLD),
  MRG_POINTER  = (MRG_PRESS | MRG_MOTION | MRG_RELEASE),
  MRG_CROSSING = (MRG_ENTER | MRG_LEAVE),
  MRG_DRAG     = (MRG_DRAG_PRESS | MRG_DRAG_MOTION | MRG_DRAG_RELEASE),
  MRG_KEY      = (MRG_KEY_DOWN | MRG_KEY_UP),
  MRG_ANY      = (MRG_POINTER | MRG_DRAG | MRG_CROSSING | MRG_KEY | MRG_TAPS),
};


typedef enum _MrgType MrgType;

typedef void (*MrgCb) (MrgEvent *event,
                       void     *data,
                       void     *data2);

typedef void(*MrgDestroyNotify) (void     *data);
typedef int(*MrgTimeoutCb) (Mrg *mrg, void  *data);
typedef int(*MrgIdleCb)    (Mrg *mrg, void  *data);

typedef void (*MrgNewText) (const char *new_text, void *data);
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

void mrg_restarter_add_path (Mrg *mrg, const char *path);
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


void  mrg_set_line_spacing (Mrg *mrg, float line_spacing);
float mrg_line_spacing     (Mrg *mrg); 

int mrg_print (Mrg *mrg, const char *string);

int   mrg_print_get_xy     (Mrg *mrg, const char *str, int no, float *x, float *y);


int mrg_print_xml (Mrg *mrg, const char *string);

void mrg_listen      (Mrg     *mrg,
                      MrgType  types,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2);

void mrg_listen_full (Mrg     *mrg,
                      MrgType  types,
                      MrgCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2,  void *finalize_data),
                      void    *finalize_data);

uint32_t mrg_ms (Mrg *mrg);

int mrg_pointer_press    (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_pointer_release  (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_pointer_motion   (Mrg *mrg, float x, float y, int device_no, uint32_t time);
int mrg_key_press        (Mrg *mrg, unsigned int keyval, const char *string, uint32_t time);

/* these deal with pointer_no 0 only
 */
void  mrg_warp_pointer (Mrg *mrg, float x, float y);
float mrg_pointer_x    (Mrg *mrg);
float mrg_pointer_y    (Mrg *mrg);

float mrg_em (Mrg *mrg);
void  mrg_set_xy (Mrg *mrg, float x, float y);
float  mrg_x (Mrg *mrg);
float  mrg_y (Mrg *mrg);

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

struct _MrgEvent {
  MrgType  type;
  Mrg     *mrg;
  long     time;
  MrgModifierState state;

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
  int stop_propagate;
};
void mrg_event_stop_propagate (MrgEvent *event);

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

int mrg_get_cursor_pos  (Mrg *mrg);
void mrg_set_cursor_pos (Mrg *mrg, int pos);

void mrg_edit_start (Mrg *mrg,
                     MrgNewText update_string,
                     void *user_data);

void mrg_edit_start_full (Mrg *mrg,
                           MrgNewText update_string,
                           void *user_data,
                           MrgDestroyNotify destroy,
                           void *destroy_data);
void  mrg_edit_end (Mrg *mrg);

void  mrg_set_edge_left   (Mrg *mrg, float edge);
void  mrg_set_edge_top    (Mrg *mrg, float edge);
void  mrg_set_edge_right  (Mrg *mrg, float edge);
void  mrg_set_edge_bottom (Mrg *mrg, float edge);
float mrg_edge_left       (Mrg *mrg);
float mrg_edge_top        (Mrg *mrg);
float mrg_edge_right      (Mrg *mrg);
float mrg_edge_bottom     (Mrg *mrg);



typedef struct _MrgHost   MrgHost;
typedef struct _MrgClient MrgClient;

void       mrg_host_add_client_mrg   (MrgHost     *host,
                                      Mrg         *mrg,
                                      float        x,
                                      float        y);
MrgHost   *mrg_host_new              (Mrg *mrg, const char *path);
void       mrg_host_destroy          (MrgHost *host);
void       mrg_host_set_focused      (MrgHost *host, MrgClient *client);
MrgClient *mrg_host_get_focused      (MrgHost *host);
void       mrg_host_monitor_dir      (MrgHost *host);
void       mrg_host_register_events  (MrgHost *host);
MrgList   *mrg_host_clients          (MrgHost *host);

void       mrg_client_render_sloppy  (MrgClient *client, float x, float y);
int        mrg_client_get_pid        (MrgClient *client);
void       mrg_client_kill           (MrgClient *client);
void       mrg_client_raise_top      (MrgClient *client);
void       mrg_client_render         (MrgClient *client, Mrg *mrg, float x, float y);
void       mrg_client_maximize       (MrgClient *client);
float      mrg_client_get_x          (MrgClient *client);
float      mrg_client_get_y          (MrgClient *client);
void       mrg_client_set_x          (MrgClient *client, float x);
void       mrg_client_set_y          (MrgClient *client, float y);
void       mrg_client_get_size       (MrgClient *client, int *width, int *height);
void       mrg_client_set_size       (MrgClient *client, int width,  int height);
const char *mrg_client_get_title     (MrgClient *client);
void        host_add_client_mrg      (MrgHost     *host,
                                      Mrg         *mrg,
                                      float        x,
                                      float        y);

]]

function M.new(width,height, backend)
  return ffi.gc(
           C.mrg_new(width, height, backend),
           C.mrg_destroy)
end

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

  image_size       = function (mrg, path)
    local rw = ffi.new'int[1]'
    local rh = ffi.new'int[1]'
    rw[0] = -1
    rh[0] = -1
    C.mrg_query_image (mrg, path, rw, rh)
    return rw[0], rh[0]
  end,


  css_set          = function (...) C.mrg_css_set(...) end,
  set_style        = function (...) C.mrg_set_style(...) end,
  css_add          = function (...) C.mrg_css_add(...) end,
  set_em           = function (...) C.mrg_set_em(...) end,

  edit_start = function (mrg, cb, data)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (finalize_data)
      cb_fun:free();
      notify_fun:free();
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    local wrap_cb = function(new_text,data)
      cb(ffi.string(new_text),data)
    end
    cb_fun = ffi.cast ("MrgNewText", wrap_cb)
    return C.mrg_edit_start_full (mrg, cb_fun, data, notify_fun, NULL)
  end,

  edit_end = function (...) C.mrg_edit_end (...) end,
  set_edge_top     = function (...) C.mrg_set_edge_top(...) end,
  edge_top         = function (...) return C.mrg_get_edge_top(...) end,
  set_edge_bottom     = function (...) C.mrg_set_edge_bottom(...) end,
  edge_bottom         = function (...) return C.mrg_get_edge_bottom(...) end,
  set_edge_left     = function (...) C.mrg_set_edge_left(...) end,
  edge_left         = function (...) return C.mrg_get_edge_left(...) end,
  set_edge_right     = function (...) C.mrg_set_edge_right(...) end,
  edge_right         = function (...) return C.mrg_get_edge_right(...) end,
  set_rem          = function (...) C.mrg_set_rem(...) end,
  main             = function (...) C.mrg_main(...) end,
  start            = function (mrg, cssid) C.mrg_start(mrg, cssid, NULL) end,
  start_with_style = function (mrg, cssid, style) C.mrg_start_with_style(mrg, cssid, NULL, style) end,
  restarter_add_path = function (...) C.mrg_restarter_add_path(...) end,
  close            = function (...) C.mrg_end(...) end,
  queue_draw       = function (...) C.mrg_queue_draw(...) end,
  ms               = function (...) return C.mrg_ms(...) end,
  add_binding = function(mrg,key,action,label,cb,db_data)
    local notify_fun, cb_fun;
    local notify_cb = function (data1, data2,finalize_data)
      cb_fun:free();
      notify_fun:free();
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
    end

    notify_fun = ffi.cast ("MrgCb", notify_cb)
    cb_fun = ffi.cast ("MrgCb", cb)
    return C.mrg_text_listen_full (mrg, types, cb_fun, data1, data2, notify_fun, NULL)
  end,
  listen           = function (mrg, types, cb, data1, data2)
    local notify_fun, cb_fun;
    local notify_cb = function (data1, data2, finalize_data)
      cb_fun:free();
      notify_fun:free();
    end
    notify_fun = ffi.cast ("MrgCb", notify_cb)
    cb_fun = ffi.cast ("MrgCb", cb)
    return C.mrg_listen_full (mrg, types, cb_fun, data1, data2, notify_fun, NULL)
  end,
  print_get_xy     = function (mrg, str, no) 
                         local rx = ffi.new'float[1]'
                         local ry = ffi.new'float[1]'
                         C.mrg_print_get_xy(mrg, str, no, rx, ry)
                         return rx[0], ry[0]
                       end,
  get_cursor_pos   = function (...) return C.mrg_get_cursor_pos(...) end,
  set_cursor_pos   = function (...) C.mrg_set_cursor_pos(...) end,
  print            = function (...) C.mrg_print(...) end,
  print_xml        = function (...) C.mrg_print_xml(...) end,
  cr               = function (...) return C.mrg_cr (...) end,
  width            = function (...) return C.mrg_width (...) end,
  height           = function (...) return C.mrg_height (...) end,
  set_fullscreen   = function (...) C.mrg_set_fullscreen(...) end,
  is_fullscreen    = function (...) return C.mrg_is_fullscreen(...) ~= 0 end,
  x                = function (...) return C.mrg_x (...) end,
  y                = function (...) return C.mrg_y (...) end,
  xy               = function (mrg) return mrg:x(), mrg:y() end,
  em               = function (...) return C.mrg_em (...) end,
  rem              = function (...) return C.mrg_rem (...) end,
  set_xy           = function (...) C.mrg_set_xy (...) end,
  set_size         = function (...) C.mrg_set_size (...) end,
  set_position     = function (...) C.mrg_set_position(...) end,
  set_title        = function (...) C.mrg_set_title (...) end,
  get_title        = function (...) return C.mrg_get_title (...) end,
  set_font_size    = function (...) C.mrg_set_font_size (...) end,

  pointer_press = function (...) C.mrg_pointer_press (...) end,
  pointer_release = function (...) C.mrg_pointer_release (...) end,
  pointer_motion = function (...) C.mrg_pointer_motion (...) end,
  key_press      = function (...) C.mrg_key_press (...) end,

  add_idle = function (mrg, ms, cb, data1)
    local notify_fun, cb_fun;
    local notify_cb = function (a,b)
      cb_fun:free();
      notify_fun:free();
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    cb_fun = ffi.cast ("MrgIdleCb", cb)
    return C.mrg_add_idle_full (mrg, cb_fun, data1, notify_fun, NULL)
  end,

  add_timeout = function (mrg, ms, cb, data1)
    local notify_fun, cb_fun;
    local notify_cb = function (a,b)
      cb_fun:free();
      notify_fun:free();
    end
    notify_fun = ffi.cast ("MrgDestroyNotify", notify_cb)
    cb_fun = ffi.cast ("MrgTimeoutCb", cb)
    return C.mrg_add_timeout_full (mrg, ms, cb_fun, data1, notify_fun, NULL)
  end,

  host_new = function (...) 
             return C.mrg_host_new(...)
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
ffi.metatype('MrgEvent',     {__index = { 

  stop_propagation = function (...) return C.mrg_event_stop_propagate (...) end,

}})
ffi.metatype('MrgColor',     {__index = { }})
ffi.metatype('MrgStyle',     {__index = { }})
ffi.metatype('MrgRectangle', {__index = { }})
ffi.metatype('MrgList',      {__index = { }})
ffi.metatype('MrgHost',      {__index = {
  set_focused     = function (...) C.mrg_host_set_focused (...) end,
  focused         = function (...) return C.mrg_host_get_focused (...) end,
  monitor_dir     = function (...) C.mrg_host_monitor_dir (...) end,
  register_events = function (...) C.mrg_host_register_events (...) end,
  clients         = function (...) 
    local ret = {}
    local iter = C.mrg_host_clients (...)
    while iter ~= NULL do
      local client 
      ret[#ret+1] = ffi.cast("MrgClient*", iter.data)
      iter = iter.next
    end
    return ret
  end,

  add_client_mrg  = function (...) C.mrg_host_add_client_mrg (...) end

}})
ffi.metatype('MrgClient',    {__index = {
  render_sloppy = function (...) C.mrg_client_render_sloppy (...) end,
  render        = function (...) C.mrg_client_render (...) end,
  pid           = function (...) return C.mrg_client_get_pid (...) end,
  kill          = function (...) C.mrg_client_kill (...) end,
  raise_top     = function (...) C.mrg_client_raise_top (...) end,
  maximize      = function (...) C.mrg_client_maximize (...) end,
  title         = function (...) return C.mrg_client_get_title (...) end,
  x             = function (...) return C.mrg_client_get_x (...) end,
  y             = function (...) return C.mrg_client_get_y (...) end,
  set_x         = function (...) C.mrg_client_set_x (...) end,
  set_y         = function (...) C.mrg_client_set_y (...) end,
  set_xy        = function (client, x, y) client:set_x(x) client:set_y(y) end,
  xy            = function (client) return client:x(), client:y() end,
  set_size      = function (...) C.mrg_client_set_size (...) end,
  size          = function (client)
    local rw = ffi.new'int[1]'
    local rh = ffi.new'int[1]'
    C.mrg_client_get_size (client, rw, rh)
    return rw[0], rh[0]
  end,
 }})

  M.MOTION = C.MRG_MOTION;
  M.ENTER = C.MRG_ENTER;
  M.LEAVE = C.MRG_LEAVE;
  M.PRESS = C.MRG_PRESS;
  M.RELEASE = C.MRG_RELEASE;
  M.TAP = C.MRG_TAP;
  M.TAP_AND_HOLD = C.MRG_TAP_AND_HOLD;
  M.DRAG_MOTION = C.MRG_DRAG_MOTION;
  M.DRAG_PRESS = C.MRG_DRAG_PRESS;
  M.DRAG_RELEASE = C.MRG_DRAG_RELEASE;
  M.DRAG = C.MRG_DRAG;



local keyboard_visible = false

local keys={
  {x=20,   y=25, w=44, label='esc'},
  {x=20,   y=65, label='`', shifted='~'},
  {x=60,   y=65, label='1', shifted='!'},
  {x=100,  y=65, label='2', shifted='@'},
  {x=140,  y=65, label='3', shifted='#'},
  {x=180,  y=65, label='4', shifted='$'},
  {x=220,  y=65, label='5', shifted='%'},
  {x=260,  y=65, label='6', shifted='^'},
  {x=300,  y=65, label='7', shifted='&'},
  {x=340,  y=65, label='8', shifted='*'},
  {x=380,  y=65, label='9', shifted='('},
  {x=420,  y=65, label='0', shifted=')'},
  {x=460,  y=65, label='-', shifted='_'},
  {x=500,  y=65, label='=', shifted='+'},
  {x=540,  w=40, y=65, label='⌫', code='backspace'},

  {x=400,  w=40, y=225, label='ins', code='insert'},
  {x=450,  w=40, y=225, label='del', code='delete'},

  {x=20,   y=105, label='↹', code='tab'},
  {x=65,   y=105, label='q', shifted='Q'},
  {x=105,  y=105, label='w', shifted='W'},
  {x=145,  y=105, label='e', shifted='E'},
  {x=185,  y=105, label='r', shifted='R'},
  {x=225,  y=105, label='t', shifted='T'},
  {x=265,  y=105, label='y', shifted='Y'},
  {x=305,  y=105, label='u', shifted='U'},
  {x=345,  y=105, label='i', shifted='I'},
  {x=385,  y=105, label='o', shifted='O'},
  {x=425,  y=105, label='p', shifted='P'},
  {x=465,  y=105, label='[', shifted='{'},
  {x=505,  y=105, label=']', shifted='}'},
  {x=545,  y=105, label='\\', shifted='|'},

  {x=70,   y=145, label='a', shifted='A'},
  {x=110,  y=145, label='s', shifted='S'},
  {x=150,  y=145, label='d', shifted='D'},
  {x=190,  y=145, label='f', shifted='F'},
  {x=230,  y=145, label='g', shifted='G'},
  {x=270,  y=145, label='h', shifted='H'},
  {x=310,  y=145, label='j', shifted='J'},
  {x=350,  y=145, label='k', shifted='K'},
  {x=390,  y=145, label='l', shifted='L'},
  {x=430,  y=145, label=';', shifted=':'},
  {x=470,  y=145, label='\'', shifted='"'},
  {x=510, w=50, y=145, label='⏎', code='return' },

  {x=10,   w=60, y=185, label='shift', type='modal'},  -- ⇧
  {x=75,   y=185, label='z', shifted='Z'},
  {x=115,  y=185, label='x', shifted='X'},
  {x=155,  y=185, label='c', shifted='C'},
  {x=195,  y=185, label='v', shifted='V'},
  {x=235,  y=185, label='b', shifted='B'},
  {x=275,  y=185, label='n', shifted='N'},
  {x=315,  y=185, label='m', shifted='M'},
  {x=355,  y=185, label=',', shifted='<'},
  {x=395,  y=185, label='.', shifted='>'},
  {x=435,  y=185, label='/', shifted='?'},
  {x=475,  w=60, y=185, label='shift', type='modal'},

  {x=20, w=50, y=225, label='ctrl', type='modal'},
  {x=75, w=40,  y=225, label='alt', type='modal'},
  {x=120,  y=225, w=230, label=' ', code='space'},

  {x=540,  y=185, label='↑', code='up'},
  {x=500,  y=225, label='←', code='left'},
  {x=540,  y=225, label='↓', code='down'},
  {x=580,  y=225, label='→', code='right'},

  {x=590,  y=145, label='⌨', type='keyboard'},
}

local pan_y = 0
local pan_x = 0
local shifted = false
local alted = false
local ctrld = false

M.draw_keyboard = function (mrg)
  local em = 20
  local cr = mrg:cr()

  local dim = mrg:width() / 620

  cr:set_font_size(em)

  if not keyboard_visible then
    cr:rectangle (mrg:width() - 4 * em, mrg:height() - 4 * em, 4 * em, 4 * em)
    mrg:listen(M.TAP, function(event) 
      keyboard_visible = true
      mrg:queue_draw(nil)
    end)
    cr:set_source_rgba(0,0,0,0.3)
    cr:fill()
  else
    cr:translate(0,mrg:height()-250* dim)
    cr:scale(dim, dim)
    cr:translate(pan_x, pan_y)
    for k,v in ipairs(keys) do
      cr:new_path()
      if not v.w then v.w = 34 end
      if v.w then
        cr:rectangle(v.x - em * 0.8, v.y - em, v.w, em * 1.8)
      else
        cr:arc (v.x, v.y, em, 0, 3.1415*2)
      end
      mrg:listen(M.TAP, function(event)
        if v.type == 'keyboard' then
          keyboard_visible = false
        else
          local keystr = v.label
          if shifted and v.shifted then keystr = v.shifted end
          if v.code then keystr = v.code end
          if alted  then keystr = 'alt-' .. keystr end
          if ctrled then keystr = 'control-' .. keystr end
          mrg:key_press(0, keystr, 0)
        end
        mrg:queue_draw(nil)
        event.stop_propagate = 1
      end)
      if v.type == 'modal' then

        mrg:listen(M.TAP, function(event)
          if v.label == 'shift' then
            shifted = not shifted
            mrg:queue_draw(nil)
          end
          if v.label == 'ctrl' then
            ctrled = not ctrled
            mrg:queue_draw(nil)
          end
          if v.label == 'alt' then
            alted = not alted
            mrg:queue_draw(nil)
          end
        end)
        if (v.label == 'shift' and shifted) or
           (v.label == 'alt' and alted) or
           (v.label == 'ctrl' and ctrled)
          then
          cr:set_source_rgba (0,0,0,0.5)
          cr:stroke_preserve()
        end

      end
      cr:set_source_rgba (0.0,0.0,0.0,0.25)
      cr:fill ()
      cr:move_to (v.x - 6, v.y + 4)
      cr:set_source_rgba (1,1,1,1.0)
      if shifted then
        if v.shifted then
          cr:show_text (v.shifted)
        else
          cr:show_text (v.label)
        end
      else
        cr:show_text (v.label)
      end
    end

    cr:rectangle(50,5,470,35)
    cr:set_source_rgba(0,0,1,0.2)
    mrg:listen(M.DRAG, function(event)
      pan_y = pan_y + event.delta_y
      --pan_x = pan_x + event.delta_x
      mrg:queue_draw(nil)
      event.stop_propagate = 1
    end)
    cr:fill()
  end
end





return M
