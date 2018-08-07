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

#include "mrg.h"
#include "mrg-xml.h"
#include <string.h>
#include <math.h>
#include "mrg-internal.h"

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
}
void _mrg_border_bottom (Mrg *mrg, int x, int y, int width, int height)
{
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
}

void _mrg_border_top_r (Mrg *mrg, int x, int y, int width, int height)
{
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
}
void _mrg_border_bottom_r (Mrg *mrg, int x, int y, int width, int height)
{
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
}

void _mrg_border_top_l (Mrg *mrg, int x, int y, int width, int height)
{
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
}
void _mrg_border_bottom_l (Mrg *mrg, int x, int y, int width, int height)
{
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
}


void _mrg_border_top_m (Mrg *mrg, int x, int y, int width, int height)
{
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
}
void _mrg_border_bottom_m (Mrg *mrg, int x, int y, int width, int height)
{
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
}
void _mrg_border_left (Mrg *mrg, int x, int y, int width, int height)
{
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
}
void _mrg_border_right (Mrg *mrg, int x, int y, int width, int height)
{
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
      /* fallthrough */

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
      } /* XXX: maybe spot for */
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

        cairo_identity_matrix (mrg_cr (mrg));
        cairo_scale (mrg_cr(mrg), mrg_ddpx (mrg), mrg_ddpx (mrg));
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
      style->display == MRG_DISPLAY_INLINE_BLOCK ||
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
      double x = mrg_pointer_x (mrg);
      double y = mrg_pointer_y (mrg);
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

int
mrg_parse_svg_path (Mrg *mrg, const char *str)
{
  cairo_t *cr = mrg_cr (mrg);
  char  command = 'm';
  char *s;
  int numbers = 0;
  double number[12];

  if (!str)
    return -1;
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
      if (*s == '-')
        number[numbers] = mrg_parse_float (mrg, s, &s);
      else
      {
        number[numbers] = mrg_parse_float (mrg, s, &s);
        s--;
      }
        numbers++;

      switch (command)
      {
        case 'a':
          /* fallthrough */
        case 'A':
          if (numbers == 7)
          {
            /// XXX: NYI
            s++;
            goto again;
          }
          /* fallthrough */
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
  return 0;
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
                     void (*link_cb) (MrgEvent *event, void *href, void *link_data),
                     void *link_data,
                     void *(finalize)(void *listen_data, void *listen_data2, void *finalize_data),
                     void *finalize_data,
                     char *html_)
{
  char *html;
  MrgXml *xmltok;
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

  html = malloc (strlen (html_) + 3);
  sprintf (html, "%s ", html_);
  xmltok = xmltok_buf_new (html);

  {
    int no = mrg->text_listen_count;
    mrg->text_listen_data1[no] = link_data;
    mrg->text_listen_data2[no] = html_;
    mrg->text_listen_finalize[no] = (void*)finalize;
    mrg->text_listen_finalize_data[no] = finalize_data;
    mrg->text_listen_count++;
  }

  _mrg_set_wrap_edge_vfuncs (mrg, wrap_edge_left, wrap_edge_right, ctx);
  _mrg_set_post_nl (mrg, _mrg_draw_background_increment, ctx);
  ctx->mrg = mrg;
  ctx->state = &ctx->states[0];

  while (type != t_eof)
  {
    char *data = NULL;
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
            mrg_start (mrg, "dim", (void*)((size_t)pos));
            mrg_print (mrg, data);
            mrg_end (mrg);
          }
        }
        break;
      case t_word:
        if (in_style)
        {
          mrg_stylesheet_add (mrg, data, uri_base, MRG_STYLE_XML, NULL);
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
          mrg_stylesheet_add (mrg, data, uri_base, MRG_STYLE_XML, NULL);
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
        if (ctx->attributes < MRG_XML_MAX_ATTRIBUTES-1)
          strncpy (ctx->attribute[ctx->attributes], data, MRG_XML_MAX_ATTRIBUTE_LEN-1);
        break;
      case t_val:
        if (ctx->attributes < MRG_XML_MAX_ATTRIBUTES-1)
          strncpy (ctx->value[ctx->attributes++], data, MRG_XML_MAX_VALUE_LEN-1);
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
            MrgGeoCache *geo = _mrg_get_cache (ctx, (void*)(size_t)(tagpos));
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
            /* collect XML attributes and convert into CSS declarations */
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
          mrg_start_with_style (mrg, combined, (void*)((size_t)tagpos), style->str);
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
          mrg_parse_svg_path (mrg, get_attr (ctx, "d"));
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
          if ((rel=get_attr (ctx, "rel")) && !strcmp (rel, "stylesheet") && get_attr (ctx, "href"))
          {
            char *contents;
            long length;
            mrg_get_contents (mrg, uri_base, get_attr (ctx, "href"), &contents, &length);
            if (contents)
            {
              mrg_stylesheet_add (mrg, contents, uri_base, MRG_STYLE_XML, NULL);
              free (contents);
            }
          }
        }

        if (!strcmp (data, "img") && get_attr (ctx, "src"))
        {
          int img_width, img_height;
          const char *src = get_attr (ctx, "src");

          if (mrg_query_image (mrg, src, &img_width, &img_height))
          {
            float width = mrg_style(mrg)->width;
            float height = mrg_style(mrg)->height;

            if (width < 1)
            {
               width = img_width;
            }
            if (height < 1)
            {
               height = img_height *1.0 / img_width * width;
            }

            _mrg_draw_background_increment (mrg, &mrg->html, 0);
            mrg->y += height;

            mrg_image (mrg,
            mrg->x,
            mrg->y - height,
            width,
            height, src, NULL, NULL);

            mrg->x += width;
          }
          else
          {
            mrg_printf (mrg, "![%s]", src);
          }
        }
#if 1
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
#endif
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
              fprintf (stderr, "%i: fixing close of %s when %s is open\n", pos, data, tag[depth]);

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
  free (html);
}

void mrg_xml_renderf (Mrg *mrg,
                      char *uri_base,
                      void (*link_cb) (MrgEvent *event, void *href, void *link_data),
                      void *link_data,
                      char *format,
                      ...)
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
  mrg_xml_render (mrg, uri_base, link_cb, link_data, NULL, NULL, buffer);
  free (buffer);
}

void mrg_print_xml (Mrg *mrg, char *xml)
{
  mrg_xml_render (mrg, NULL, NULL, NULL, NULL, NULL, xml);
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
