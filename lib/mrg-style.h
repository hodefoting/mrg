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
                         int priority,
                         char **error);

void mrg_css_set (Mrg *mrg, const char *css);
void mrg_css_add (Mrg *mrg, const char *css);


void mrg_set_style (Mrg *mrg, const char *style);
void mrg_set_stylef (Mrg *mrg, const char *format, ...);


void  mrg_set_line_height (Mrg *mrg, float line_height);
float  mrg_line_height (Mrg *mrg);


/* XXX: doesnt feel like it belongs here */
void mrg_image (Mrg *mrg, float x0, float y0, float width, float height, const char *path);

#endif
