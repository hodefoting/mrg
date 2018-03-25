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

#include "mrg-string.h"
#include "mrg-utf8.h"

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void mrg_string_init (MrgString *string)
{
  string->allocated_length = 8; // XXX: working around a bug in string or vt
  string->length = 0;
  string->utf8_length = 0;
  string->str = malloc (string->allocated_length);
  string->str[0]='\0';
}

static void mrg_string_destroy (MrgString *string)
{
  if (string->str)
  {
    free (string->str);
    string->str = NULL;
  }
}

void mrg_string_clear (MrgString *string)
{
  string->length = 0;
  string->utf8_length = 0;
  string->str[string->length]=0;
}

static inline void _mrg_string_append_byte (MrgString *string, char  val)
{
  if ((val & 0xC0) != 0x80)
    string->utf8_length++;
  if (string->length + 1 >= string->allocated_length)
    {
      char *old = string->str;
      string->allocated_length *= 2;
      string->str = malloc (string->allocated_length);
      memcpy (string->str, old, string->allocated_length/2);
      free (old);
    }
  string->str[string->length++] = val;
  string->str[string->length] = '\0';
}
void mrg_string_append_byte (MrgString *string, char  val)
{
  _mrg_string_append_byte (string, val);
}

void mrg_string_append_unichar (MrgString *string, unsigned int unichar)
{
  char *str;
  char utf8[5];
  utf8[mrg_unichar_to_utf8 (unichar, (unsigned char*)utf8)]=0;
  str = utf8;

  while (str && *str)
    {
      _mrg_string_append_byte (string, *str);
      str++;
    }
}

static inline void _mrg_string_append_str (MrgString *string, const char *str)
{
  while (str && *str)
    {
      _mrg_string_append_byte (string, *str);
      str++;
    }
}
void mrg_string_append_str (MrgString *string, const char *str)
{
  _mrg_string_append_str (string, str);
}

MrgString *mrg_string_new (const char *initial)
{
  MrgString *string = calloc (sizeof (MrgString), 1);
  mrg_string_init (string);
  if (initial)
    _mrg_string_append_str (string, initial);
  return string;
}

void mrg_string_append_data (MrgString *string, const char *str, int len)
{
  int i;
  for (i = 0; i<len; i++)
    _mrg_string_append_byte (string, str[i]);
}

void mrg_string_append_string (MrgString *string, MrgString *string2)
{
  const char *str = mrg_string_get (string2);
  while (str && *str)
    {
      _mrg_string_append_byte (string, *str);
      str++;
    }
}

const char *mrg_string_get (MrgString *string)
{
  return string->str;
}

int mrg_string_get_length (MrgString *string)
{
  return string->length;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */
char *mrg_string_dissolve   (MrgString *string)
{
  char *ret = string->str;
  string->str = NULL;
  free (string);
  return ret;
}

void
mrg_string_free (MrgString *string, int freealloc)
{
  if (freealloc)
    {
      mrg_string_destroy (string);
    }
  free (string);
}

void
mrg_string_append_printf (MrgString *string, const char *format, ...)
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
  _mrg_string_append_str (string, buffer);
  free (buffer);
}

void
mrg_string_set (MrgString *string, const char *new_string)
{
  mrg_string_clear (string);
  _mrg_string_append_str (string, new_string);
}

#include "mrg-list.h"
static MrgList *interns = NULL;

const char * mrg_intern_string (const char *str)
{
  MrgList *i;
  for (i = interns; i; i = i->next)
  {
    if (!strcmp (i->data, str))
      return i->data;
  }
  str = strdup (str);
  mrg_list_append (&interns, (void*)str);
  return str;
}

void mrg_string_replace_utf8 (MrgString *string, int pos, const char *new_glyph)
{
  int new_len = mrg_utf8_len (*new_glyph);
  int old_len = string->utf8_length;
  char tmpg[3]=" ";
  if (new_len <= 1 && new_glyph[0] < 32)
  {
    tmpg[0]=new_glyph[0]+64;
    new_glyph = tmpg;
  }

  if (pos == old_len)
  {
    _mrg_string_append_str (string, new_glyph);
    return;
  }

  {
    for (int i = old_len; i <= pos; i++)
    {
      _mrg_string_append_byte (string, ' ');
      old_len++;
    }
  }

  if (string->length + new_len  > string->allocated_length)
  {
    char *tmp;
    char *defer;
    string->allocated_length = string->length + new_len;
    tmp = calloc (string->allocated_length, 1);
    strcpy (tmp, string->str);
    defer = string->str;
    string->str = tmp;
    free (defer);
  }

  char *p = (void*)mrg_utf8_skip (string->str, pos);
  int prev_len = mrg_utf8_len (*p);
  char *rest;
  if (*p == 0 || *(p+prev_len) == 0)
  {
    rest = strdup("");
  }
  else
  {
    rest = strdup (p + prev_len);
  }

  memcpy (p, new_glyph, new_len);
  memcpy (p + new_len, rest, strlen (rest) + 1);
  string->length += new_len;
  string->length -= prev_len;
  free (rest);

  string->utf8_length = mrg_utf8_strlen (string->str);
}

void mrg_string_insert_utf8 (MrgString *string, int pos, const char *new_glyph)
{
  int new_len = mrg_utf8_len (*new_glyph);
  int old_len = string->utf8_length;
  char tmpg[3]=" ";
  if (new_len <= 1 && new_glyph[0] < 32)
  {
    tmpg[0]=new_glyph[0]+64;
    new_glyph = tmpg;
  }

  if (pos == old_len)
  {
    _mrg_string_append_str (string, new_glyph);
    return;
  }

  {
    for (int i = old_len; i <= pos; i++)
    {
      _mrg_string_append_byte (string, ' ');
      old_len++;
    }
  }

  if (string->length + new_len + 1  > string->allocated_length)
  {
    char *tmp;
    char *defer;
    string->allocated_length = string->length + new_len + 1;
    tmp = calloc (string->allocated_length, 1);
    strcpy (tmp, string->str);
    defer = string->str;
    string->str = tmp;
    free (defer);
  }

  char *p = (void*)mrg_utf8_skip (string->str, pos);
  int prev_len = mrg_utf8_len (*p);
  char *rest;
  if (*p == 0 || *(p+prev_len) == 0)
  {
    rest = strdup("");
  }
  else
  {
    rest = strdup (p);
  }

  memcpy (p, new_glyph, new_len);
  memcpy (p + new_len, rest, strlen (rest) + 1);
  string->length += new_len;
  free (rest);

  string->utf8_length = mrg_utf8_strlen (string->str);
}

int mrg_string_get_utf8_length (MrgString  *string)
{
  //return mrg_utf8_strlen (string->str);
  return string->utf8_length;
}

void mrg_string_remove_utf8 (MrgString *string, int pos)
{
  int old_len = string->utf8_length;

  {
    for (int i = old_len; i <= pos; i++)
    {
      _mrg_string_append_byte (string, ' ');
      old_len++;
    }
  }

  char *p = (void*)mrg_utf8_skip (string->str, pos);
  int prev_len = mrg_utf8_len (*p);
  char *rest;
  if (*p == 0 || *(p+prev_len) == 0)
  {
    rest = strdup("");
  }
  else
  {
    rest = strdup (p + prev_len);
  }

  memcpy (p, rest, strlen (rest) + 1);
  string->length -= prev_len;
  free (rest);

  string->utf8_length = mrg_utf8_strlen (string->str);
}

