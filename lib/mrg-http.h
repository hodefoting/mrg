#ifndef MRG_HTTP_H
#define MRG_HTTP_H

char *_mrg_http_post (const char *ip,
                      const char *hostname,
                      int         port,
                      const char *path,
                      const char *body,
                      int         length,
                      int        *ret_length);
char *_mrg_http (const char *ip,
                 const char *hostname,
                 int         port,
                 const char *path,
                 int        *length);

#endif
