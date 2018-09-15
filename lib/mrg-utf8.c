#include <string.h>
#include "mrg-utf8.h"


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

unsigned int mrg_utf8_to_unichar (unsigned char *utf8)
{
  unsigned char c = utf8[0];
  if ((c & 0x80) == 0)
    return c;
  else if ((c & 0xE0) == 0xC0)
    return ((utf8[0] & 0x1F) << 6) |
            (utf8[1] & 0x3F);
  else if ((c & 0xF0) == 0xE0)
    return ((utf8[0] & 0xF)  << 12)|
           ((utf8[1] & 0x3F) << 6) |
            (utf8[2] & 0x3F);
  else if ((c & 0xF8) == 0xF0)
    return ((utf8[0] & 0x7)  << 18)|
           ((utf8[1] & 0x3F) << 12)|
           ((utf8[2] & 0x3F) << 6) |
            (utf8[3] & 0x3F);
  else if ((c & 0xFC) == 0xF8)
    return ((utf8[0] & 0x3)  << 24)|
           ((utf8[1] & 0x3F) << 18)|
           ((utf8[2] & 0x3F) << 12)|
           ((utf8[3] & 0x3F) << 6) |
            (utf8[4] & 0x3F);
  else if ((c & 0xFE) == 0xFC)
    return ((utf8[0] & 0x1)  << 30)|
           ((utf8[1] & 0x3F) << 24)|
           ((utf8[2] & 0x3F) << 18)|
           ((utf8[3] & 0x3F) << 12)|
           ((utf8[4] & 0x3F) << 6) |
            (utf8[5] & 0x3F);
  return 0;
}

