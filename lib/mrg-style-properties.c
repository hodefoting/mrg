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
    cairo_set_line_width (mrg_cr (mrg), s->line_width);
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
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
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
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
    }
  else if (!strcmp (name, "font-family"))
    {
      strncpy (s->font_family, value, 63);
      s->font_family[63]=0;
      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
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
      cairo_set_line_join (mrg_cr (mrg), s->stroke_linejoin);
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
      cairo_set_line_cap (mrg_cr (mrg), s->stroke_linecap);
    }
  else if (!strcmp (name, "font-family"))
    {
      strncpy (s->font_family, value, 63);
      s->font_family[63]=0;

      cairo_select_font_face (mrg_cr (mrg),
          s->font_family,
          s->font_style,
          s->font_weight);
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
