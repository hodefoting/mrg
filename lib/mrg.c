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

#include "mrg-config.h"
#include "mrg-internal.h"

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
  mrg->style = mrg_string_new ("");

  mrg_set_mrg_get_contents (mrg, mrg_get_contents_default, NULL);
  mrg->style_global = mrg_string_new ("");

  {
    const char *global_css_uri = "mrg:theme.css";

    if (getenv ("MRG_CSS"))
      global_css_uri = getenv ("MRG_CSS");

      char *contents;
      long length;
      mrg_get_contents (mrg, NULL, global_css_uri, &contents, &length);
      if (contents)
      {
        mrg_string_set (mrg->style_global, contents);
        free (contents);
      }
  }
}

#if MRG_MMM
extern MrgBackend mrg_backend_mmm;
extern MrgBackend mrg_backend_mmm_client;
#endif
#if MRG_SDL
extern MrgBackend mrg_backend_sdl;
#endif
#if MRG_NCT
extern MrgBackend mrg_backend_nct;
#endif
#if MRG_MEM
extern MrgBackend mrg_backend_mem;
#endif
#if MRG_GTK
extern MrgBackend mrg_backend_gtk;
#endif

static MrgBackend *backends[]={
#if MRG_MMM
  &mrg_backend_mmm_client,
#endif
#if MRG_SDL
  &mrg_backend_sdl,
#endif
#if MRG_GTK
  &mrg_backend_gtk,
#endif
#if MRG_MMM
  &mrg_backend_mmm,
#endif
#if MRG_NCT
  &mrg_backend_nct,
#endif
#if MRG_MEM
  &mrg_backend_mem,
#endif
  NULL
};

static Mrg *mrg_new2 (int width, int height, const char *backend)
{
  Mrg *mrg = NULL;
  int i;
  for (i = 0; backends[i]; i++)
  {
    if (!strcmp (backends[i]->name, backend))
    {
      mrg = backends[i]->mrg_new (width, height);
      if (!mrg)
        fprintf (stderr, "failed to initialize [%s] mrg backend\n", backend);
      return mrg;
    }
  }

  fprintf (stderr, "Unrecognized microraptor backend: %s\n", backend);
  fprintf (stderr, " recognized backends:");
  for (i = 0; backends[i]; i++)
  {
    fprintf (stderr, " %s", backends[i]->name);
  }
  fprintf (stderr, "\n");
  exit (-1);
  return NULL;
}

void mrg_style_defaults (Mrg *mrg);

Mrg *mrg_new (int width, int height, const char *backend)
{
  Mrg *mrg = NULL;

  MRG_INFO("new %i %i", width, height);

  if (!backend)
  {
    if (getenv ("MMM_PATH") && !getenv ("MMM_COMPOSITOR"))
      backend = "mmm"; /* default to mmm compositing when available */
    else
      backend = getenv ("MRG_BACKEND");
  }

  if (backend)
    mrg = mrg_new2 (width, height, backend);
  else
  {
    int i;
    for (i = 0; backends[i] && mrg == NULL; i++)
    {
      if (strcmp (backends[i]->name, "mem"))
        mrg = backends[i]->mrg_new (width, height); 
    }
    if (!mrg)
    {
      fprintf (stderr, "Unable to initialize any mrg backend\n");
      exit (-1);
    }
  }
  if (mrg)
    mrg_style_defaults (mrg);
  mrg->edited_str = mrg_string_new ("");
  return mrg;
}

void mrg_destroy (Mrg *mrg)
{
  if (mrg->backend->mrg_destroy)
    mrg->backend->mrg_destroy (mrg);
  mrg_string_free (mrg->edited_str, 1);
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

  if (mrg->backend->mrg_queue_draw)
    mrg->backend->mrg_queue_draw (mrg, &rect_copy);
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

void  mrg_set_ui (Mrg *mrg, void (*ui)(Mrg *mrg, void *ui_data),
                  void *ui_data)
{
  mrg->ui_update = ui;
  mrg->user_data = ui_data;
}

void mrg_main (Mrg *mrg)
{
  if (!mrg->title)
  {
    mrg->title = strdup ("micro raptor gui");//__progname;
  }
  mrg_set_title (mrg, mrg->title);
  mrg->backend->mrg_main (mrg, mrg->ui_update, mrg->user_data);
}

cairo_t *mrg_cr (Mrg *mrg)
{
  if (mrg->backend->mrg_cr)
    return mrg->backend->mrg_cr (mrg);
  else
  {
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
  }
}

static long  frame_start;
static long  frame_end;

int  _mrg_bindings_key_down (MrgEvent *event, void *data1, void *data2);
void mrg_text_edit_bindings (Mrg *mrg);
void mrg_focus_bindings (Mrg *mrg);

typedef struct IdleCb {
  int (*cb) (Mrg *mrg, void *idle_data);
  void *idle_data;

  void (*destroy_notify)(void *destroy_data);
  void *destroy_data;

  int   ticks_remaining;
  int   is_idle;
  int   id;
} IdleCb;

static long prev_ticks = 0;

void _mrg_idle_iteration (Mrg *mrg)
{
  MrgList *l;
  MrgList *to_remove = NULL;
  long ticks = _mrg_ticks ();
  long tick_delta = ticks - prev_ticks;
  prev_ticks = ticks;

  if (!mrg->idles)
  {
    return;
  }
  for (l = mrg->idles; l; l = l->next)
  {
    IdleCb *item = l->data;

    if (item->ticks_remaining >= 0)
      item->ticks_remaining -= tick_delta;

    if (item->ticks_remaining < 0)
    {
      if (item->cb (mrg, item->idle_data) == FALSE)
        mrg_list_prepend (&to_remove, item);
    }
  }
  for (l = to_remove; l; l = l->next)
  {
    IdleCb *item = l->data;
    if (item->destroy_notify)
      item->destroy_notify (item->destroy_data);
    mrg_list_remove (&mrg->idles, l->data);
  }
}

void _mrg_text_prepare (Mrg *mrg);

void mrg_prepare (Mrg *mrg)
{

  mrg->state->fg = 0; /* XXX: move to terminal? what about reverse-video? */
  mrg->state->bg = 7;

  frame_start = _mrg_ticks ();

  mrg_string_set (mrg->edited_str, "");
  mrg->got_edit = 0;
  mrg_clear (mrg);
  mrg->in_paint ++;

  _mrg_text_prepare (mrg);

  mrg_style_defaults (mrg);

  if (mrg->backend->mrg_prepare)
    mrg->backend->mrg_prepare (mrg);

  mrg_start (mrg, "document", NULL);

  {
    cairo_t *cr = mrg_cr (mrg);
    cairo_save (cr);
    cairo_scale (cr, mrg->ddpx, mrg->ddpx);
    /* gtk does it's own clipping/exposure handling  */
    if (mrg->do_clip) 
    {
      cairo_rectangle (cr,
          mrg->dirty.x,
          mrg->dirty.y,
          mrg->dirty.width,
          mrg->dirty.height);
      cairo_clip (cr);
    }
    mrg_cairo_set_source_color (cr, &mrg_style(mrg)->background_color);
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);
  }

  mrg_listen (mrg, MRG_KEY_DOWN, _mrg_bindings_key_down, NULL, NULL);


#if 0
  else
    mrg_focus_bindings (mrg);
#endif
}

void mrg_flush  (Mrg *mrg)
{
  cairo_new_path (mrg_cr (mrg));

  //_mrg_debug_overlays (mrg);
  mrg_end (mrg);

  if (mrg->got_edit && mrg->text_edit_blocked <= 0)
  {
    mrg_text_edit_bindings (mrg);
  }

  cairo_restore (mrg_cr (mrg));

  if (mrg->backend->mrg_flush)
    mrg->backend->mrg_flush (mrg);

  _mrg_set_clean (mrg);
  mrg->in_paint --;
  frame_end = _mrg_ticks ();
  //fprintf (stderr, "(%f)", (frame_end - frame_start) / 1000.0);
}

void mrg_warp_pointer (Mrg *mrg, float x, float y)
{
  if (mrg->backend->mrg_warp_pointer)
    mrg->backend->mrg_warp_pointer (mrg, x, y);
  mrg->pointer_x = x;
  mrg->pointer_y = y;
}

void mrg_set_fullscreen (Mrg *mrg, int fullscreen)
{
  if (mrg->backend->mrg_fullscreen)
    mrg->backend->mrg_fullscreen (mrg, fullscreen);
}

int mrg_is_fullscreen (Mrg *mrg)
{
  return mrg->fullscreen;
}

int  mrg_is_terminal (Mrg *mrg)
{
  return mrg->glyphs != NULL;
}

static void mrg_parse_style_id (Mrg          *mrg,
                                const char   *style_id,
                                MrgStyleNode *node)
{
  const char *p;
  char temp[128] = "";
  int  temp_l = 0;
  if (!style_id)
  {
    return; // XXX: why does this happen?
  }

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

  if (mrg->in_paint)
    cairo_save (mrg_cr (mrg));

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
  if (mrg->in_paint)
    cairo_restore (mrg_cr (mrg));
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

  if (mrg->style_global->length)
  {
    mrg_stylesheet_add (mrg, mrg->style_global->str, NULL, MRG_STYLE_GLOBAL, NULL);
  }

  if (mrg->style->length)
    mrg_stylesheet_add (mrg, mrg->style->str, NULL, MRG_STYLE_GLOBAL, NULL);
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

/* setting edge top has the additional side effect of doing cursor
 * positioning
 *
 */
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
  if (mrg->backend->mrg_get_pixels)
    return mrg->backend->mrg_get_pixels (mrg, rowstride);
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

  cairo_new_path (cr);
  cairo_rectangle (cr, 0, 0, mrg_width (mrg), mrg_height (mrg));

  mrg_listen (mrg2, MRG_PRESS,   mrg_mrg_press, mrg, NULL);
  mrg_listen (mrg2, MRG_MOTION,  mrg_mrg_motion, mrg, NULL);
  mrg_listen (mrg2, MRG_RELEASE, mrg_mrg_release, mrg, NULL);
  cairo_new_path (cr);

  cairo_restore (cr);
}

void mrg_remove_idle (Mrg *mrg, int handle)
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
    if (item->id == handle)
      mrg_list_prepend (&to_remove, item);
  }
  for (l = to_remove; l; l = l->next)
  {
    IdleCb *item = l->data;
    if (item->destroy_notify)
      item->destroy_notify (item->destroy_data);
    mrg_list_remove (&mrg->idles, l->data);
  }
}

int mrg_add_timeout_full (Mrg *mrg, int ms, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data,
                          void (*destroy_notify)(void *destroy_data), void *destroy_data)
{
  IdleCb *item = calloc (sizeof (IdleCb), 1);
  item->cb = idle_cb;
  item->idle_data = idle_data;
  item->id = ++mrg->idle_id;
  item->ticks_remaining = ms * 1000;
  item->destroy_notify = destroy_notify;
  item->destroy_data = destroy_data;
  mrg_list_append (&mrg->idles, item);
  return item->id;
}

int mrg_add_timeout (Mrg *mrg, int ms, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data)
{
  return mrg_add_timeout_full (mrg, ms, idle_cb, idle_data, NULL, NULL);
}

int mrg_add_idle_full (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data,
                                 void (*destroy_notify)(void *destroy_data), void *destroy_data)
{
  IdleCb *item = calloc (sizeof (IdleCb), 1);
  item->cb = idle_cb;
  item->idle_data = idle_data;
  item->id = ++mrg->idle_id;
  item->ticks_remaining = -1;
  item->is_idle = 1;
  item->destroy_notify = destroy_notify;
  item->destroy_data = destroy_data;
  mrg_list_append (&mrg->idles, item);
  return item->id;
}

int mrg_add_idle (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data)
{
  return mrg_add_idle_full (mrg, idle_cb, idle_data, NULL, NULL);
}

void  mrg_set_position  (Mrg *mrg, int x, int y)
{
  if (mrg->backend->mrg_set_position)
  {
    //int ox, oy;
    //int dx, dy;
    //mrg_get_position (mrg, &ox, &oy);
    //dx = x - ox;
    //dy = y - oy;

    mrg->backend->mrg_set_position (mrg, x, y);
  }
}

void  mrg_get_position  (Mrg *mrg, int *x, int *y)
{
  if (mrg->backend->mrg_get_position)
    mrg->backend->mrg_get_position (mrg, x, y);
}

void  mrg_set_title     (Mrg *mrg, const char *title)
{
  if (title != mrg->title)
  {
    if (mrg->title)
      free (mrg->title);
    mrg->title = title?strdup (title):NULL;
  }
  if (mrg->backend->mrg_set_title)
    mrg->backend->mrg_set_title (mrg, title);
}

const char *mrg_get_title (Mrg *mrg)
{
  if (mrg->backend->mrg_get_title)
    return mrg->backend->mrg_get_title (mrg);
  return mrg->title;
}

void mrg_set_mrg_get_contents (Mrg *mrg,
  int (*mrg_get_contents) (const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length,
                      void        *get_contents_data),
  void *get_contents_data)

{
  mrg->mrg_get_contents = mrg_get_contents;
  mrg->get_contents_data = get_contents_data;
}

int
mrg_get_contents (Mrg         *mrg,
                  const char  *referer,
                  const char  *input_uri,
                  char       **contents,
                  long        *length)
{
  if (mrg->mrg_get_contents)
  {
    int ret;

    ret = mrg->mrg_get_contents (referer, input_uri, contents, length, mrg->get_contents_data);


    return ret;
  }
  else
  {
    *contents = NULL;
    *length = 0;
    return -1;
  }
}

#if MRG_LOG

void __mrg_log (Mrg        *mrg,
                const char *file,
                const char *function,
                int         line_no,
                int         type,
                const char *message)
{
  static int inited = 0;
  static int log_level = 0;
  if (!inited)
  {
    inited = 1;
    if (getenv ("MRG_LOG_LEVEL"))
      log_level = atoi(getenv("MRG_LOG_LEVEL"));
  }
  if (type > log_level)
    return;
  switch (type)
  {
    case MRG_LOG_INFO:
      fprintf (stderr, "INFO %s:%i : %s\n", function, line_no, message);
      break;
  }
}

void _mrg_log (Mrg        *mrg,
               const char *file,
               const char *function,
               int         line_no,
               int         type,
               const char *format, ...)
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
  __mrg_log (mrg, file, function, line_no, type, buffer);
  free (buffer);
}



#endif


int mrg_in_dirty_rect (Mrg *mrg,
                        int x, int y,
                        int width, int height)
{
   if (x > mrg->dirty.x + mrg->dirty.width ||
       y > mrg->dirty.y + mrg->dirty.height ||
       x + width < mrg->dirty.x ||
       y + height < mrg->dirty.y)
     return 0;
   return 1;
}
