#ifndef _MRG_HOST_H
#define _MRG_HOST_H

#include "mrg-list.h"

typedef struct _MrgHost   MrgHost;
typedef struct _MrgClient MrgClient;

MrgHost   *mrg_host_new              (Mrg *mrg, const char *path);
void       mrg_host_destroy          (MrgHost *host);
void       mrg_host_add_client_mrg   (MrgHost     *host,
                                      Mrg         *mrg,
                                      float        x,
                                      float        y);
void       mrg_host_set_focused      (MrgHost *host, MrgClient *client);
MrgClient *mrg_host_get_focused      (MrgHost *host);
void       mrg_host_monitor_dir      (MrgHost *host);
void       mrg_host_register_events  (MrgHost *host);
MrgList   *mrg_host_clients          (MrgHost *host);
void mrg_host_get_default_size (MrgHost *host, int *width, int *height);
void mrg_host_set_default_size (MrgHost *host, int width, int height);

void       mrg_client_render_sloppy  (MrgClient *client, float x, float y);
int        mrg_client_get_pid        (MrgClient *client);
void       mrg_client_kill           (MrgClient *client);
void       mrg_client_render         (MrgClient *client, Mrg *mrg, float x, float y);
void       mrg_client_maximize       (MrgClient *client);
float      mrg_client_get_x          (MrgClient *client);
float      mrg_client_get_y          (MrgClient *client);
void       mrg_client_set_x          (MrgClient *client, float x);
void       mrg_client_set_y          (MrgClient *client, float y);
void       mrg_client_get_size       (MrgClient *client, int *width, int *height);
void       mrg_client_set_size       (MrgClient *client, int width,  int height);
const char *mrg_client_get_title     (MrgClient *client);

void        mrg_client_send_message  (MrgClient *client, const char *message);
const char *mrg_client_get_message   (MrgClient *client);
int         mrg_client_has_message   (MrgClient *client);
const char *mrg_client_get_value     (MrgClient *client, const char *name);
void        mrg_client_set_value     (MrgClient *client, const char *name, const char *value);

void       mrg_client_raise_top      (MrgClient *client); /* XXX: use set_stack_order(1) instead */
void mrg_client_set_stack_order (MrgClient *client, int zpos);
int  mrg_client_get_stack_order (MrgClient *client);

#define TITLE_BAR_HEIGHT 18
//#define TITLE_BAR_HEIGHT 0

#endif
