#ifndef _HOST_H
#define _HOST_H

#include <mrg.h>

typedef struct _MrgHost   MrgHost;
typedef struct _MrgClient MrgClient;

void       mrg_host_add_client_mrg   (MrgHost     *host,
                                      Mrg         *mrg,
                                      float        x,
                                      float        y);
MrgHost   *mrg_host_new              (Mrg *mrg, const char *path);
void       mrg_host_destroy          (MrgHost *host);
void       mrg_host_set_focused      (MrgHost *host, MrgClient *client);
MrgClient *mrg_host_get_focused      (MrgHost *host);
int        mrg_host_client_has_focus (MrgHost *host);
void       mrg_host_render_client    (MrgHost *host, MrgClient *client, float x, float y);
void       mrg_host_monitor_dir      (MrgHost *host);
void       mrg_host_register_events  (MrgHost *host);
MrgList   *mrg_host_clients          (MrgHost *host);
int        mrg_client_get_pid        (MrgClient *client);
void       mrg_client_kill           (MrgClient *client);

#endif
