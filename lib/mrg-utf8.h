#ifndef MRG_UTF8_H
#define MRG_UTF8_H

#include <string.h>

static inline int mrg_utf8_len (const unsigned char first_byte)
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

static inline int mrg_utf8_strlen (const char *s)
{
   int count;
   if (!s)
     return 0;
   for (count = 0; *s; s++)
     if ((*s & 0xC0) != 0x80)
       count++;
   return count;
}

static inline const char *mrg_utf8_skip (const char *s, int utf8_length)
{
   int count;
   if (!s)
     return NULL;
   for (count = 0; *s; s++)
   {
     if ((*s & 0xC0) != 0x80)
       count++;
     if (count == utf8_length+1)
       return s;
   }
   return s;
}

int          mrg_unichar_to_utf8       (unsigned int   ch,
                                        unsigned char *dest);
unsigned int mrg_utf8_to_unichar       (unsigned char *utf8);

#endif
