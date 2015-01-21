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

#if MRG_MEM
#include "mrg-internal.h"

typedef struct MrgMem {
  unsigned char *pixels;
  int allocated;
} MrgMem;

static unsigned char *mrg_mem_get_pixels (Mrg *mrg, int *rowstride)
{
  MrgMem *mem = mrg->backend_data;

  if (mem->allocated < mrg->width * mrg->height * 4)
  {
    int allocated = mrg->width * mrg->height * 4;
    free (mem->pixels);
    mem->pixels = calloc (allocated, 1);
    mem->allocated = allocated;
  }

  if (rowstride)
    *rowstride = mrg->width * 4;
  return mem->pixels;
}

static void mrg_mem_flush (Mrg *mrg)
{
#if MRG_CAIRO
  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
#endif
}

static void mrg_mem_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  fprintf (stderr, "error: main of mem backend invoked\n");
  return;
}

static void mrg_mem_destroy (Mrg *mrg)
{
  free (mrg->backend_data);
}

static Mrg *_mrg_mem_new (int width, int height);

MrgBackend mrg_backend_mem = {
  "mem",
  _mrg_mem_new,
  mrg_mem_main,
  mrg_mem_get_pixels, 
  NULL, /* mrg_cr, */
  mrg_mem_flush,
  NULL, /* mrg_prepare */
  NULL, /* mrg_clear */
  NULL, /* mrg_queue_draw */
  mrg_mem_destroy,
  NULL, /* mrg_warp_pointer, */
  NULL, /* mrg_fullscreen, */
  NULL, /* mrg_gtk_set_position, */
  NULL, /* mrg_gtk_get_position, */
  NULL, /* mrg_gtk_set_title, */
  NULL, /* mrg_gtk_get_title, */
};

static Mrg *_mrg_mem_new (int width, int height)
{
  Mrg *mrg;
  MrgMem *backend = calloc (sizeof (MrgMem), 1);

  mrg = calloc (sizeof (Mrg), 1);
#if MRG_CAIRO
  backend->allocated = width * height * 4;
  backend->pixels = calloc (backend->allocated, 1);
#endif

  mrg->backend = &mrg_backend_mem;
  mrg->backend_data = backend;
  _mrg_init (mrg, width, height);

  return mrg;
}
#endif
