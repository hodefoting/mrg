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

#ifndef MRG_UTIL_H__
#define MRG_UTIL_H__


int mrg_quit_cb (MrgEvent *event, void *data1, void *data2);

void mrg_cairo_set_source_color (cairo_t *cr, MrgColor *color);

void mrg_color_set (MrgColor *color, float red, float green, float blue, float alpha);
int mrg_in_dirty_rect (Mrg *mrg,
                        int x, int y,
                        int width, int height);

#define EM(value)           (value*mrg_em(mrg))
#define PERCENT_X(value)    (value*mrg_width(mrg))
#define PERCENT_Y(value)    (value*mrg_height(mrg))

#endif
