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

void  mrg_set_xy          (Mrg *mrg, float x, float y);
float mrg_x               (Mrg *mrg);
float mrg_y               (Mrg *mrg);
int   mrg_print           (Mrg *mrg, const char *str);
void  mrg_printf          (Mrg *mrg, const char *format, ...);
void  mrg_print_xml       (Mrg *mrg, char *xml);
void  mrg_printf_xml      (Mrg *mrg, const char *format, ...);

/* text layout handling */
float mrg_em              (Mrg *mrg);
void  mrg_set_em          (Mrg *mrg, float em);
float mrg_rem             (Mrg *mrg);
void  mrg_set_rem         (Mrg *mrg, float em);

void  mrg_set_edge_left   (Mrg *mrg, float edge);
void  mrg_set_edge_top    (Mrg *mrg, float edge);
void  mrg_set_edge_right  (Mrg *mrg, float edge);
void  mrg_set_edge_bottom (Mrg *mrg, float edge);
float mrg_edge_left       (Mrg *mrg);
float mrg_edge_top        (Mrg *mrg);
float mrg_edge_right      (Mrg *mrg);
float mrg_edge_bottom     (Mrg *mrg);

void  mrg_set_font_size   (Mrg *mrg, float size);

void  mrg_set_line_spacing (Mrg *mrg, float line_spacing);
float mrg_line_spacing     (Mrg *mrg); 
int   mrg_print_get_xy     (Mrg *mrg, const char *str, int no, float *x, float *y);
void  mrg_set_wrap         (Mrg *mrg, int do_wrap);
float mrg_draw_string      (Mrg *mrg, MrgStyle *style, 
                            float x, float y,
                            const char *string,
                            int utf8_len);
void mrg_edit_string       (Mrg *mrg, char **string,
                            void (*update_string)(Mrg *mrg,
                            char **string_loc,
                            const char *new_string,
                            void  *user_data),
                            void *user_data);

int mrg_get_cursor_pos  (Mrg *mrg);
void mrg_set_cursor_pos (Mrg *mrg, int pos);

/* should accept at lest 4-8 stacked cb's */
void  mrg_text_listen (Mrg *mrg, MrgType types,
                       MrgCb cb, void *data1, void *data2);

void  mrg_text_listen_full (Mrg *mrg, MrgType types,
                            MrgCb cb, void *data1, void *data2,
          void (*finalize)(void *listen_data, void *listen_data2, void *finalize_data),
          void  *finalize_data);

void  mrg_text_listen_done (Mrg *mrg);


/*  renders within configured edges
 */
void mrg_xml_render (Mrg *mrg,
                     char *uri_base,
                     int (*link_cb) (MrgEvent *event, void *href, void *link_data),
                     void *link_data,
                     char *html);

void mrg_xml_renderf (Mrg *mrg,
                      char *uri_base,
                      int (*link_cb) (MrgEvent *event, void *href, void *link_data),
                      void *link_data,
                      char *format,
                      ...);

void _mrg_block_edit (Mrg *mrg);
void _mrg_unblock_edit (Mrg *mrg);

void mrg_syntax_hl_start (Mrg *mrg);
void mrg_syntax_hl_stop (Mrg *mrg);
