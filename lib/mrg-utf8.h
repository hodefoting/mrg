#ifndef MRG_UTF8_H
#define MRG_UTF8_H

int          mrg_utf8_len              (const unsigned char first_byte);
int          mrg_utf8_strlen           (const char *s);
const char  *mrg_utf8_skip             (const char *string, int utf8_length);
int          mrg_unichar_to_utf8       (unsigned int ch,
                                        unsigned char*dest);

#endif
