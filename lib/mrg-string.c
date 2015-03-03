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
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void mrg_string_init (MrgString *string)
{
  string->allocated_length = 8;
  string->length = 0;
  string->str = malloc (string->allocated_length);
  string->str[0]='\0';
}
static void mrg_string_destroy (MrgString *string)
{
  if (string->str)
    free (string->str);
}

MrgString *mrg_string_new (const char *initial)
{
  MrgString *string = calloc (sizeof (MrgString), 1);
  mrg_string_init (string);
  if (initial)
    mrg_string_append_str (string, initial);
  return string;
}

void mrg_string_clear (MrgString *string)
{
  string->length = 0;
  string->str[string->length]=0;
}

void mrg_string_append_byte (MrgString *string, char  val)
{
  if (string->length + 2 >= string->allocated_length)
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

void mrg_string_append_unichar (MrgString *string, unsigned int unichar)
{
  char *str;
  char utf8[5];
  utf8[mrg_unichar_to_utf8 (unichar, (unsigned char*)utf8)]=0;
  str = utf8;

  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
      str++;
    }
}

void mrg_string_append_str (MrgString *string, const char *str)
{
  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
      str++;
    }
}

void mrg_string_append_data (MrgString *string, const char *str, int len)
{
  int i;
  for (i = 0; i<len; i++)
    mrg_string_append_byte (string, str[i]);
}

void mrg_string_append_string (MrgString *string, MrgString *string2)
{
  const char *str = mrg_string_get (string2);
  while (str && *str)
    {
      mrg_string_append_byte (string, *str);
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
  mrg_string_append_str (string, buffer);
  free (buffer);
}

void
mrg_string_set (MrgString *string, const char *new_string)
{
  mrg_string_clear (string);
  mrg_string_append_str (string, new_string);
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


int mrg_utf8_len (const unsigned char first_byte)
{
  if      ((first_byte & 0x80) == 0)
    return 1; /* ASCII */
  else if ((first_byte & 0xE0) == 0xC0)
    return 2;
  else if ((first_byte & 0xF0) == 0xE0)
    return 3;
  else if ((first_byte & 0xF8) == 0xF0)
    return 4;
  return 1;
}

int
mrg_unichar_to_utf8 (unsigned int  ch,
                     unsigned char*dest)
{
/* http://www.cprogramming.com/tutorial/utf8.c  */
/*  Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005 ... */
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

int mrg_utf8_strlen (const char *s)
{
   int count;
   if (!s)
     return 0;
   for (count = 0; *s; s++)
     if ((*s & 0xC0) != 0x80)
       count++;
   return count;
}

const char *mrg_utf8_skip (const char *string, int utf8_length)
{
  const char *s;
  int len;
  int pos = 0;

  if (!string)
    return NULL;

  for (s = string; pos < utf8_length && *s; s += mrg_utf8_len (*s))
    {
      int c;
      len = mrg_utf8_len (*s);
      for (c = 0; c < len; c++)
        {
          if (!s[c])
            return s;
        }
      pos++;
    }
  return s;
}
