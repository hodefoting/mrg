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

#if MRG_CAIRO
static float _mrg_text_shift (Mrg *mrg)
{
  //MrgStyle *style = mrg_style (mrg);
  float ascent, descent;
  _mrg_get_ascent_descent (mrg, &ascent, &descent);
  return (descent * 0.9); // XXX
}
#endif

const char * hl_punctuation[] =
{";", ",", "(", ")", "{", "}", NULL};
const char * hl_operators [] =
{"-", "+", "=", "*", "/", "return", "<", ">", 
 "if", "else", "break", "case", NULL};
const char * hl_types[] =
{"int", "cairo_t", "Mrg", "float", "double",
  "char", "const", "static", "void", "NULL",
  "#include", "#define", NULL};

static int is_one_of (const char *word, const char **words)
{
  int i;
  for (i = 0; words[i]; i++)
  {
    if (!strcmp (words[i], word))
      return 1;
  }
  return 0;
}

static int is_a_number (const char *word)
{
  int yep = 1;
  int i;
  for (i = 0; word[i]; i++)
  {
    if (word[i] < '0' || word[i] > '9')
      yep = 0;
  }
  return yep;
}

/* the syntax highlighting is done with static globals; deep in the text
 * rendering, this permits the editing code to recognize which string is
 * edited and directly work with pointer arithmetic on that instead of
 * marked up xml for the highlighting.
 */
enum {
  MRG_HL_NEUTRAL      = 0,
  MRG_HL_NEXT_NEUTRAL = 1,
  MRG_HL_STRING       = 2,
  MRG_HL_STRING_ESC   = 3,
  MRG_HL_SLASH        = 4,
  MRG_HL_LINECOMMENT  = 5,
  MRG_HL_COMMENT      = 6,
  MRG_HL_COMMENT_STAR = 7,
};

static int state = MRG_HL_NEUTRAL;
static int mrg_syntax_hl = 0;
void mrg_syntax_hl_start (Mrg *mrg)
{
  mrg_syntax_hl = 1;
  state = MRG_HL_NEUTRAL;
}

void mrg_syntax_hl_stop (Mrg *mrg)
{
  mrg_syntax_hl = 0;
}

static void mrg_hl_token (cairo_t *cr, const char *word)
{
  switch (state)
  {
    case MRG_HL_NEUTRAL:
      if (!strcmp (word, "\""))
      {
        state = MRG_HL_STRING;
      }
      else if (!strcmp (word, "/"))
      {
        state = MRG_HL_SLASH;
      }
      break;
    case MRG_HL_SLASH:
      if (!strcmp (word, "/"))
      {
        state = MRG_HL_LINECOMMENT;
      } else if (!strcmp (word, "*"))
      {
        state = MRG_HL_COMMENT;
      } else
      {
        state = MRG_HL_NEUTRAL;
      }
      break;
    case MRG_HL_LINECOMMENT:
      if (!strcmp (word, "\n"))
      {
        state = MRG_HL_NEUTRAL;
      }
      break;
    case MRG_HL_COMMENT:
      if (!strcmp (word, "*"))
      {
        state = MRG_HL_COMMENT_STAR;
      }
      break;
    case MRG_HL_COMMENT_STAR:
      if (!strcmp (word, "/"))
      {
        state = MRG_HL_NEUTRAL;
      }
      else
      {
        state = MRG_HL_COMMENT;
      }
      break;
    case MRG_HL_STRING:
      if (!strcmp (word, "\""))
      {
        state = MRG_HL_NEXT_NEUTRAL;
      }
      else if (!strcmp (word, "\\"))
      {
        state = MRG_HL_STRING_ESC;
      }
      break;
    case MRG_HL_STRING_ESC:
      state = MRG_HL_STRING;
      break;
    case MRG_HL_NEXT_NEUTRAL:
      state = MRG_HL_NEUTRAL;
      break;
  }

  switch (state)
  {
    case MRG_HL_NEUTRAL:
      if (is_a_number (word))
        cairo_set_source_rgb (cr, 0.5, 0.0, 0.0);
      else if (is_one_of (word, hl_punctuation))
        cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
      else if (is_one_of (word, hl_operators))
        cairo_set_source_rgb (cr, 0, 0.5, 0);
      else if (is_one_of (word, hl_types))
        cairo_set_source_rgb (cr, 0.2, 0.2, 0.5);
      else 
        cairo_set_source_rgb (cr, 0, 0, 0);
      break;
    case MRG_HL_STRING:
        cairo_set_source_rgb (cr, 1, 0, 0.5);
      break;
    case MRG_HL_COMMENT:
    case MRG_HL_COMMENT_STAR:
    case MRG_HL_LINECOMMENT:
        cairo_set_source_rgb (cr, 0.4, 0.4, 1);
      break;

  }

  cairo_show_text (cr, word);
}

/* hook syntax highlighter in here..  */
void mrg_hl_text (cairo_t *cr, const char *text)
{
  if (!mrg_syntax_hl)
  {
    cairo_show_text (cr, text);
    return;
  }
  int i;
  MrgString *word = mrg_string_new ("");
  for (i = 0; i < text[i]; i++)
  {
    switch (text[i])
    {
      case ';':
      case '-':
      case '>':
      case '<':
      case '=':
      case '+':
      case '"':
      case '*':
      case '/':
      case '\\':
      case '[':
      case ']':
      case ')':
      case ',':
      case '(':
        if (word->length)
        {
          mrg_hl_token (cr, word->str);
          mrg_string_set (word, "");
        }
        mrg_string_append_byte (word, text[i]);
        mrg_hl_token (cr, word->str);
        mrg_string_set (word, "");
        break;
      default:
        cairo_set_source_rgb (cr, 0,0,0);
        mrg_string_append_byte (word, text[i]);
        break;
    }
  }
  if (word->length)
    mrg_hl_token (cr, word->str);

  mrg_string_free (word, 1);
  //cairo_show_text (cr, text);
}

/* x and y in cairo user units ; returns x advance in user units  */
float mrg_draw_string (Mrg *mrg, MrgStyle *style, 
                      float x, float y,
                      const char *string,
                      int utf8_len)
{
#if MRG_CAIRO
  double new_x, old_x;
  char *temp_string = NULL;
  cairo_t *cr = mrg_cr (mrg);

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

    /* when syntax highlighting,.. should do it as a coloring
     * directly here..
     */
#if 0
    cairo_show_text (cr, string);
#else
    mrg_hl_text (cr, string);
#endif
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
#else
  return 0;
#endif
}

float mrg_addstr (Mrg *mrg, float x, float y, const char *string, int utf8_length);


float paint_span_bg_final (Mrg   *mrg, float x, float y,
                           float  width)
{
#if MRG_CAIRO
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
#else
  return 0;
#endif
}

float paint_span_bg (Mrg   *mrg, float x, float y,
                     float  width)
{
#if MRG_CAIRO
  MrgStyle *style = mrg_style (mrg);
  cairo_t *cr = mrg_cr (mrg);
  if (!cr)
    return 0.0;
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
#else
  return 0;
#endif
}


float
mrg_addstr (Mrg *mrg, float x, float y, const char *string, int utf8_length)
{
  float wwidth = measure_word_width (mrg, string);
  float left_pad;
  left_pad = paint_span_bg (mrg, x, y, wwidth);

  {
    double tx = x;
    double ty = y;
    cairo_user_to_device (mrg_cr (mrg), &tx, &ty);
    if (ty > mrg->height * 2 ||
        tx > mrg->width * 2 ||
        tx < -mrg->width * 2 ||
        ty < -mrg->height * 2)
    {
    }
    else
    mrg_draw_string (mrg, &mrg->state->style, x + left_pad, y, string, utf8_length);
  }

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

/* calling this for a full text is horribly slow */
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
  if (x) *x = mrg->x + no; // XXX: only correct for nct/monospace

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
    int best = mrg->cursor_pos;
    float best_y = cy;
    float best_score = 1000000000000.0;
    for (no = mrg->cursor_pos - 1; no>= mrg->cursor_pos - 256 && no > 0; no--)
    {
      float x, y;
      float attempt_score = 0.0;
      mrg_set_xy (mrg, e_x, e_y);
      mrg_print_get_xy (mrg, *mrg->edited, no, &x, &y);

      if (y < cy && best_y == cy)
        best_y = y;

      if (y < cy)
        attempt_score = (best_y - y);
      else
        attempt_score = 1000.0;

      attempt_score += fabs(cx-x) / 10000000.0;

      if (attempt_score < best_score)
      {
        best_score = attempt_score;
        best = no;
      }
    }
    mrg->cursor_pos = best;
  }

  if (mrg->cursor_pos < 0)
    mrg->cursor_pos = 0;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

int mrg_get_cursor_pos (Mrg *mrg)
{
  return mrg->cursor_pos;
}

void mrg_set_cursor_pos (Mrg *mrg, int pos)
{
  mrg->cursor_pos = pos;
  mrg_queue_draw (mrg, NULL);
}

static int cmd_down (MrgEvent *event, void *data1, void *data2)
{
  Mrg *mrg = event->mrg;
  float e_x, e_y, e_s, e_e, e_em;
  float cx, cy;

  if (!mrg->edited || !(*mrg->edited))
    return 0;
 
  mrg_get_edit_state (mrg, &e_x, &e_y, &e_s, &e_e, &e_em);
  mrg_set_edge_left (mrg, e_s - mrg->state->style.padding_left);
  mrg_set_edge_right (mrg, e_e + mrg->state->style.padding_right);
  mrg_set_xy (mrg, e_x, e_y);
  mrg_print_get_xy (mrg, *mrg->edited, mrg->cursor_pos, &cx, &cy);

  {
    int no;
    int best = mrg->cursor_pos;
    float best_score = 10000000000.0;
    float best_y = cy;
    int strl = mrg_utf8_strlen (*mrg->edited);
    for (no = mrg->cursor_pos + 1; no < mrg->cursor_pos + 256 && no < strl; no++)
    {
      float x, y;
      float attempt_score = 0.0;
      mrg_set_xy (mrg, e_x, e_y);
      mrg_print_get_xy (mrg, *mrg->edited, no, &x, &y);

      if (y > cy && best_y == cy)
        best_y = y;

      if (y > cy)
        attempt_score = (y - best_y);
      else
        attempt_score = 1000.0;

      attempt_score += fabs(cx-x) / 10000000.0;

      if (attempt_score <= best_score)
      {
        best_score = attempt_score;
        best = no;
      }
    }
    mrg->cursor_pos = best;
  }

  if (mrg->cursor_pos >= mrg_utf8_strlen (*mrg->edited))
    mrg->cursor_pos = mrg_utf8_strlen (*mrg->edited) - 1;
  mrg_queue_draw (mrg, NULL);
  return 1;
}

static int cmd_page_down (MrgEvent *event, void *data1, void *data2)
{
  int i;
  for (i = 0; i < 6; i++)
    cmd_down (event, data1, data2);
  return 1;
}

static int cmd_page_up (MrgEvent *event, void *data1, void *data2)
{
  int i;
  for (i = 0; i < 6; i++)
    cmd_up (event, data1, data2);
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
  mrg_add_binding (mrg, "escape",    NULL, "Stop editing", cmd_escape,      NULL);
  mrg_add_binding (mrg, "return",    NULL, "insert newline", cmd_return,    NULL);
  mrg_add_binding (mrg, "home",      NULL, "Go to start of text", cmd_home, NULL);
  mrg_add_binding (mrg, "end",       NULL, "Go to end of text", cmd_end,    NULL);
  mrg_add_binding (mrg, "left",      NULL, "Move cursor left", cmd_left,    NULL);
  mrg_add_binding (mrg, "right",     NULL, "Move cursor right", cmd_right,  NULL);
  mrg_add_binding (mrg, "up",        NULL, "Move cursor up", cmd_up,        NULL);
  mrg_add_binding (mrg, "down",      NULL, "Move cursor down", cmd_down,    NULL);
  mrg_add_binding (mrg, "page-up",   NULL, "Move cursor up", cmd_page_up,     NULL);
  mrg_add_binding (mrg, "page-down", NULL, "Move cursor down", cmd_page_down, NULL);
  mrg_add_binding (mrg, "backspace", NULL, "Remove character left of cursor", cmd_backspace, NULL);
  mrg_add_binding (mrg, "delete",    NULL, "Remove selected character", cmd_delete, NULL);
  mrg_add_binding (mrg, "unhandled", NULL, "Insert if key name is one char", cmd_unhandled, NULL);
}

void mrg_edit_string (Mrg *mrg, char **string,
                      void (*update_string)(Mrg *mrg,
                        char **string_loc,
                        const char *new_string,
                        void  *user_data),
                      void *user_data)
{
  if (mrg->edited == string)
    return;
  mrg->edited = string;
  mrg->update_string = update_string;
  mrg->update_string_user_data = user_data;
  if (string)
    mrg->cursor_pos = mrg_utf8_strlen (*string);
  else
    mrg->cursor_pos = 0;
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

void  mrg_set_font_size   (Mrg *mrg, float size)
{
    mrg_set_stylef (mrg, "font-size:%fpx;", size);
}

void _mrg_block_edit (Mrg *mrg)
{
  mrg->text_edit_blocked = 1;
}
void _mrg_unblock_edit (Mrg *mrg)
{
  mrg->text_edit_blocked = 0;
}
