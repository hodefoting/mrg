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
}
