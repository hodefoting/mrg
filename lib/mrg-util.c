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


#include <string.h>
#include "mrg.h"


void mrg_quit_cb (MrgEvent *event, void *data1, void *data2)
{
  mrg_quit (event->mrg);
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

