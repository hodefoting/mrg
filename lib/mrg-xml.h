/* mrg - MicroRaptor Gui
 * Copyright (c) 2002, 2003, Øyvind Kolås <pippin@hodefoting.com>
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

#ifndef XMLTOK_H
#define XMLTOK_H

#include <stdio.h>

#define inbufsize 4096

typedef struct _MrgXml MrgXml;

enum
{
  t_none = 0,
  t_whitespace,
  t_prolog,
  t_dtd,
  t_comment,
  t_word,
  t_tag,
  t_closetag,
  t_closeemptytag,
  t_endtag,
  t_att = 10,
  t_val,
  t_eof,
  t_entity,
  t_error
};

MrgXml *xmltok_new (FILE * file_in);
MrgXml *xmltok_buf_new (char *membuf);
void    xmltok_free (MrgXml *t);
int     xmltok_lineno (MrgXml *t);
int     xmltok_get (MrgXml *t, char **data, int *pos);

#endif /*XMLTOK_H */
