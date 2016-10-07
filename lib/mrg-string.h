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

#ifndef MRG_STRING_H
#define MRG_STRING_H

#include "mrg-utf8.h"

typedef struct _MrgString MrgString;

struct _MrgString
{
  char *str;
  int   length;
  int   allocated_length;
}  __attribute((packed));

MrgString   *mrg_string_new            (const char *initial);
void         mrg_string_free           (MrgString  *string, int freealloc);
char        *mrg_string_dissolve       (MrgString  *string);
const char  *mrg_string_get            (MrgString  *string);
int          mrg_string_get_length     (MrgString  *string);
void         mrg_string_set            (MrgString  *string, const char *new_string);
void         mrg_string_clear          (MrgString  *string);
void         mrg_string_append_str     (MrgString  *string, const char *str);
void         mrg_string_append_byte    (MrgString  *string, char  val);
void         mrg_string_append_string  (MrgString  *string, MrgString *string2);
void         mrg_string_append_unichar (MrgString  *string, unsigned int unichar);
void         mrg_string_append_data    (MrgString  *string, const char *data, int len);
void         mrg_string_append_printf  (MrgString  *string, const char *format, ...);


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
