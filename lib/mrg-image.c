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
#define  STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct _MrgImage
{
  char *path;
  cairo_surface_t *surface;
};

static int compute_size (int w, int h)
{
  return w * h * 4 + sizeof (MrgImage) + 1024;
}

static long image_cache_size = 0;
static int image_cache_max_size_mb = 128; 

static MrgList *image_cache = NULL;

static void free_image (void *data, void *foo)
{
  MrgImage *image = data;
  free (image->path);
  cairo_surface_destroy (image->surface);
  free (data);
}

static void trim_cache (void)
{
  while (image_cache_size > image_cache_max_size_mb * 1024 * 1024)
  {
    MrgImage *image;
    int item = mrg_list_length (image_cache);
    int w, h;
    item = random() % item;
    image = mrg_list_nth (image_cache, item)->data;
    w = cairo_image_surface_get_width (image->surface);
    h = cairo_image_surface_get_height (image->surface);
    mrg_list_remove (&image_cache, image);
    image_cache_size -= compute_size (w, h);
  }
}

void mrg_set_image_cache_mb (Mrg *mrg, int new_max_size)
{
  image_cache_max_size_mb = new_max_size;
  trim_cache ();
}

int mrg_get_image_cache_mb (Mrg *mrg)
{
  return image_cache_max_size_mb;
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
  trim_cache ();
  {
    if (strstr (path, "png") || strstr (path, "PNG"))
    {
      /* use cairo and thus the full libpng to try decoding png images 
       */
      cairo_surface_t *surface = cairo_image_surface_create_from_png (path);
      if (surface)
      {
        int w = cairo_image_surface_get_width (surface);
        int h = cairo_image_surface_get_height (surface);
        MrgImage *image = malloc (sizeof (MrgImage));
        image->path = strdup (path);
        image->surface = surface;
        mrg_list_prepend_full (&image_cache, image, (void*)free_image, NULL);

        image_cache_size += compute_size (w, h);

        return mrg_query_image (mrg, path, width, height);
      }
    }
    else /* some other type of file, try with stb image */
    {
      char *contents = NULL;
      long length;
      _mrg_file_get_contents (path, &contents, &length);
      if (contents)
      {
          int w, h, comp;
          unsigned char *data;

          data = stbi_load_from_memory ((void*)contents, length, &w, &h, &comp, 4);
          if (data)
          {
          MrgImage *image = malloc (sizeof (MrgImage));
          image->surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, w, h);
          image->path = strdup (path);
          {
            int i;
            char *src = (void*) data;
            char *dst = (void*)cairo_image_surface_get_data (image->surface);
            /* XXX: this depends on endianness of platform */
            for (i = 0; i < w * h; i++)
            {
              dst[i*4 + 0] = src[i*4 + 2];
              dst[i*4 + 1] = src[i*4 + 1];
              dst[i*4 + 2] = src[i*4 + 0];
              dst[i*4 + 3] = src[i*4 + 3];
            }
          }
          mrg_list_prepend_full (&image_cache, image,
                                 (void*)free_image, NULL);
          free (data);
          free (contents);

          image_cache_size +=
              compute_size (w, h);


          return mrg_query_image (mrg, path, width, height);
        }
        free (contents);
      }
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
