#ifndef _HOST_H
#define _HOST_H

#include <mrg.h>

typedef struct _Host   Host;
typedef struct _Client Client;

void  add_client_mrg (Host        *host,
                      Mrg         *mrg,
                      float        x,
                      float        y);
int host_client_has_focus (Host *host);

Host *host_new       (Mrg *mrg, const char *path);
void  host_destroy   (Host *host);
void  host_render    (Mrg *mrg, Host *host);

#endif
