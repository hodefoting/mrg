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


#include <sys/time.h>
#include <string.h>
#include "mrg.h"

static struct timeval start_time;

#define usecs(time)    ((time.tv_sec - start_time.tv_sec) * 1000000 + time.     tv_usec)

static void
init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

long
_mrg_ticks (void)
{
  struct timeval measure_time;
  init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}

#undef usecs

int mrg_quit_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_quit (event->mrg);
  return 1;
}

void mrg_cairo_set_source_color (cairo_t *cr, MrgColor *color)
{
  cairo_set_source_rgba (cr, color->red, color->green, color->blue, color->alpha);
}

void mrg_color_set (MrgColor *color, float red, float green, float blue, float alpha)
{
  color->red = red;
  color->green = green;
  color->blue = blue;
}

