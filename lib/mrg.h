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

#ifndef MICRO_RAPTOR_GUI
#define MICRO_RAPTOR_GUI
#include "mrg-config.h"

/* if we've got GTK we've always got cairo */
#if MRG_GTK
#undef MRG_CAIRO
#define MRG_CAIRO 1
#endif

#if MRG_CAIRO
#include <cairo.h>
#endif

#define MRG_VERSION_MAJOR 0
#define MRG_VERSION_MINOR 0
#define MRG_VERSION_MICRO 1

/* Mrg is fed with all press, release and motion events and synthesizes
 * generated callbacks as appropriate.
 *
 */
typedef struct _Mrg          Mrg;
typedef struct _MrgColor     MrgColor;
typedef struct _MrgStyle     MrgStyle;
typedef struct _MrgRectangle MrgRectangle;

typedef void (*MrgDestroyNotify) (void *data);
typedef void (*MrgNewText)       (const char *new_text, void *data);
typedef void (*UiRenderFun)      (Mrg *mrg, void *ui_data);

Mrg        *mrg_new           (int width, int height, const char *backend);
void        mrg_destroy       (Mrg *mrg);
void        mrg_set_size      (Mrg *mrg, int width, int height);
void        mrg_set_position  (Mrg *mrg, int x, int y);
void        mrg_get_position  (Mrg *mrg, int *x, int *y);
void        mrg_set_title     (Mrg *mrg, const char *title);
const char *mrg_get_title (Mrg *mrg);
int         mrg_width         (Mrg *mrg);
int         mrg_height        (Mrg *mrg);
void        mrg_set_ui        (Mrg *mrg, UiRenderFun, void *ui_data);
void        mrg_main          (Mrg *mrg);
void        mrg_quit          (Mrg *mrg);
cairo_t    *mrg_cr            (Mrg *mrg);
void        mrg_queue_draw    (Mrg *mrg, MrgRectangle *rectangle);
int         mrg_is_terminal   (Mrg *mrg);
void        mrg_set_fullscreen (Mrg *mrg, int fullscreen);
int         mrg_is_fullscreen (Mrg *mrg);
float       mrg_ddpx          (Mrg *mrg);
unsigned char *mrg_get_pixels (Mrg *mrg, int *rowstride);

/*********************************************/

Mrg *mrg_new_mrg (Mrg *parent, int width, int height); // NYI

typedef struct _MrgImage MrgImage;

/* force a ui render, this is included for use with the headless backend.
 */
void  mrg_ui_update (Mrg *mrg);
MrgImage *mrg_query_image (Mrg *mrg, const char *path, 
                           int *width,
                           int *height);

/* built in http / local file URI fetcher, this is the callback interface
 * that needs to be implemented for mrg_xml_render if external resources (css
 * files / png images) are to be retrieved and rendered.
 */
int mrg_get_contents (Mrg         *mrg,
                      const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length);

int mrg_get_contents_default (const char  *referer,
                              const char  *input_uri,
                              char       **contents,
                              long        *length,
                              void        *ignored_user_data);

/* set the function used to implement mrg_get_contents() - both for external
 * calls and internal calls.
 */
void mrg_set_mrg_get_contents (Mrg *mrg,
  int (*mrg_get_contents) (const char  *referer,
                      const char  *input_uri,
                      char       **contents,
                      long        *length,
                      void        *get_contents_data),
  void *get_contents_data);

void mrg_render_to_mrg (Mrg *mrg, Mrg *mrg2, float x, float y);
int mrg_add_idle       (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data);
int mrg_add_timeout      (Mrg *mrg, int ms, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data);
void mrg_remove_idle (Mrg *mrg, int handle);

int mrg_add_idle_full  (Mrg *mrg, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data,
                        MrgDestroyNotify destroy_notify,
                        void *destroy_data);
int mrg_add_timeout_full (Mrg *mrg, int ms, int (*idle_cb)(Mrg *mrg, void *idle_data), void *idle_data,
                          MrgDestroyNotify destroy_notify,
                          void *destroy_data);

#include "mrg-events.h"
#include "mrg-text.h"
#include "mrg-style.h"
#include "mrg-util.h"
#include "mrg-audio.h"

#ifdef __GTK_H__ /* This is only declared if mrg.h is included after gtk.h */

GtkWidget *mrg_gtk_new (void (*ui_update)(Mrg *mrg, void *user_data),
                        void *user_data);

#endif

#include <stddef.h>

#endif
