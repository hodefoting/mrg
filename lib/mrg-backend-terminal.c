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

#include "mrg-config.h"
#if MRG_NCT

#include "mrg-internal.h"
#define NCHANTERM_HEADER_ONLY
#include "nchanterm.c"

#include <math.h> // remove after removing sqrt

static int mrg_black_on_white;

typedef struct MrgNct {
  Nchanterm     *term;
  unsigned char *nct_pixels;
} MrgNct;

static char *qblocks[]={
  " ",//0
  "▘",//1
  "▝",//2
  "▀",//3
  "▖",//4
  "▌",//5
  "▞",//6
  "▛",//7
  "▗",//8
  "▚",//9
  "▐",//10
  "▜",//11
  "▄",//12
  "▙",//13
  "▟",//14
  "█",//15
  NULL};

static char *utf8_gray_scale[]={" ","░","▒","▓","█","█", NULL};
#if 0
static char *horblocks[] = {" ","▏","▎","▍","▌","▋","▊","▉","█", NULL};
static char *verblocks[] = {" ","▁","▂","▃","▄","▅","▆","▇","█", NULL};

/* sorted so that drawing is bit operations */
static char *lscale[]={
//   " ","╵","╷","│","╶","╰","╭","├","╴","╯","╮","┤","─","┴","┬","┼",NULL};
     " ","╵","╷","│","╶","└","┌","├","╴","┘","┐","┤","─","┴","┬","┼",NULL};
#endif

static void set_gray (Nchanterm *n, int x, int y, float value)
{
  int i = value * 5.0;
  if (i > 5)
    i = 5;
  if (i < 0)
    i = 0;
  nct_set (n, x, y, utf8_gray_scale[i]);
}

/* draws with 4 input subpixels per output subpixel, the routine is
 * written to deal with an input that has 2x2 input pixels per
 * output pixel.
 */
static inline void draw_rgb_cell (Nchanterm *n, int x, int y,
                                  float b[4], float g[4], float r[4])
{
  float sum[3] = {0,0,0};
  int i;
  int bestbits = 0;
  for (i=0; i<4; i++)
    {
      sum[0] += r[i]/4;
      sum[1] += g[i]/4;
      sum[2] += b[i]/4;
    }
  {
    /* go through all fg/bg combinations with all color mixtures and
     * compare with our desired average color output
     */
    int fg, bg;
    float mix;

    int best_fg = 7;
    int best_bg = 0;
    float best_mix = 1.0;
    float best_distance = 1.0;
    int use_geom = 0;

#define POW2(a) ((a)*(a))

    for (fg = 0; fg < 8; fg ++)
      for (bg = 0; bg < 8; bg ++)
        for (mix = 0.0; mix <= 1.0; mix+=0.25)
          {
            int frgb[3] = { (fg & 1)!=0,
                            (fg & 2)!=0,
                            (fg & 4)!=0};
            int brgb[3] = { (bg & 1)!=0,
                            (bg & 2)!=0,
                            (bg & 4)!=0};
            float resrgb[3];
            float distance;
            int c;
            for (c = 0; c < 3; c++)
              resrgb[c] = (frgb[c] * mix + brgb[c] * (1.0-mix)) ;

            distance = sqrtf(POW2(resrgb[0] - sum[0])+
                       POW2(resrgb[1] - sum[1])+
                       POW2(resrgb[2] - sum[2]));
            if (distance < best_distance)
              {
                best_distance = distance;
                best_fg = fg;
                best_bg = bg;
                best_mix = mix;
              }
          }
#if 0
    if (best_mix <= 0.0) /* prefer to draw blocks than to not do so */
      {
        int tmp = best_fg;
        best_fg = best_bg;
        best_bg = tmp;
        best_mix = 1.0-best_mix;
      }
#endif

#if 1
  if (best_bg == 7 && best_fg == 7)
  {
    best_fg = 7;
    best_bg = 0;
    best_mix = 1.0;
  }
  if (best_fg == 0 && best_mix >=1.0 )
  {
    best_bg = 0;
    best_fg = 7;
    best_mix = 0.0;
  }
#endif

  {
    int totbits;
    for (totbits = 0; totbits < 15; totbits++)
        {
          float br[4],bg[4],bb[4];
          int i;
          float distance = 0;

          for (i=0;i<4;i++)
            {
              br[i] = ((totbits >> (i))&1) ? ((best_fg & 1) != 0) :
                                             ((best_bg & 1) != 0);
              bg[i] = ((totbits >> (i))&1) ? ((best_fg & 2) != 0) :
                                             ((best_bg & 2) != 0);
              bb[i] = ((totbits >> (i))&1) ? ((best_fg & 4) != 0) :
                                             ((best_bg & 4) != 0);
            }

          for (i=0;i<4;i++)
            distance += sqrt (POW2(br[i] - r[i])+
                              POW2(bg[i] - g[i])+
                              POW2(bb[i] - b[i]));
#define GEOM_FACTOR  0.10
          if (distance * GEOM_FACTOR < best_distance)
            {
              best_distance = distance/4 * GEOM_FACTOR;
              use_geom = 1;
              bestbits = totbits;
            }
        }
  }

  /* XXX:
   * do another pass checking 1/4th filled permutations,
   * these should give better large curves.
   */


  if (mrg_black_on_white)
  {
    if (best_fg == 7) best_fg = 0;
    else if (best_fg == 0) best_fg = 7;
    if (best_bg == 7) best_bg = 0;
    else if (best_bg == 0) best_bg = 7;
  }

  nct_fg_color (n, best_fg);
  nct_bg_color (n, best_bg);

  if (use_geom)
    nct_set (n, x, y, qblocks[bestbits]);
  else
    set_gray (n, x, y, best_mix);
  }
}

/*  draw a 32bit RGBA file,..
 */
static void nct_buf (Nchanterm *n, int x0, int y0, int w, int h,
                     unsigned char *buf, int rw, int rh)
{
  int u, v;

  if (!buf)
    return;

  for (u = 0; u < w; u++)
    for (v = 0; v < h; v++)
      {
        float r[4], g[4], b[4];
        float xo, yo;
        int no = 0;
        for (yo = 0.0; yo <= 0.5; yo += 0.5)
          for (xo = 0.0; xo <= 0.5; xo += 0.5, no++)
            {
              int c = 0;

              /* do a set of samplings to get a crude higher
               * quality box down-sampler?, do this crunching
               * based on a scaling factor.
               */

              float uo = 0.0, vo = 0.0;
              r[no]=g[no]=b[no]=0.0;
              //for (uo = 0.0 ; uo <= 0.5; uo+= 0.1)
              //for (vo = 0.0 ; vo <= 0.5; vo+= 0.1)
              //
              // using nearest neighbour is best for non photos..
              // we do not want the added AA for crisp edges
                  {
                    int x, y;
                    x = ((u+xo+uo) * 1.0 / w) * rw;
                    y = ((v+yo+vo) * 1.0 / h) * rh;
                    if (x<0) x = 0;
                    if (y<0) y = 0;
                    if (x>=rw) x = rw-1;
                    if (y>=rh) y = rh-1;
                      
                    r[no] += buf [(y * rw + x) * 4 + 0] / 255.0;
                    g[no] += buf [(y * rw + x) * 4 + 1] / 255.0;
                    b[no] += buf [(y * rw + x) * 4 + 2] / 255.0;
                    c++;
                  }
              r[no] /= c;
              g[no] /= c;
              b[no] /= c;
            }
        draw_rgb_cell (n, x0+u, y0+v, r, g, b);
      }
}


static unsigned char *mrg_nct_get_pixels (Mrg *mrg, int *rowstride)
{
  MrgNct *backend = mrg->backend_data;

  if (rowstride)
    *rowstride = mrg->width * 4;
  return backend->nct_pixels;
}

static void mrg_nct_flush (Mrg *mrg)
{
  MrgNct *backend = mrg->backend_data;

  nct_clear (backend->term);
  nct_buf (backend->term, 1, 1, nct_width(backend->term), nct_height (backend->term),
           backend->nct_pixels, mrg->width, mrg->height);

  {
    int x, y;
    int w = nct_width(backend->term);
    int h = nct_height(backend->term);
    int offset = 0;
    for (y = 0; y < h; y ++)
      for (x = 0; x < w; x ++)
      {
        if (mrg->glyphs[offset] != 0)
        {
          int style = mrg->styles[offset/4];
          int fg = style & 7;
          int bg = (style/8) & 7;
          int attr = style / 64;

          if (mrg_black_on_white)
          {
            if (fg == 7) fg = 0;
            else if (fg == 0) fg = 7;
            if (bg == 7) bg = 0;
            else if (bg == 0) bg = 7;
          }

          {
            if (bg == 7 && !(attr & MRG_REVERSE)) {
              nct_fg_color (backend->term, bg);
              nct_bg_color (backend->term, fg);
              attr |= MRG_REVERSE;
            }
            else
            {
              nct_fg_color (backend->term, fg);
              nct_bg_color (backend->term, bg);
            }
          }

          nct_set_attr (backend->term, attr);
          nct_set (backend->term, x+1, y, (char *)&mrg->glyphs[offset]);
        }
        offset += 4;
      }
  }

  nct_flush (backend->term);

  if (mrg->cr)
  {
    cairo_destroy (mrg->cr);
    mrg->cr = NULL;
  }
}

#include <stdio.h>
#include <math.h>

static int was_down = 0;

static int mrg_nct_consume_events (Mrg *mrg)
{
  MrgNct *backend = mrg->backend_data;
  int ix, iy;
  const char *event = NULL;
    {
      float x, y;
      event = nct_get_event (backend->term, 50, &ix, &iy);

      x = floor((ix * 1.0) / nct_width (backend->term) * mrg->width);
      y = floor((iy * 1.0) / nct_height (backend->term) * mrg->height) - 1;

      if (!strcmp (event, "mouse-press"))
      {
        mrg_pointer_press (mrg, x, y, 0);
        was_down = 1;
      } else if (!strcmp (event, "mouse-release"))
      {
        mrg_pointer_release (mrg, x, y, 0);
      } else if (!strcmp (event, "mouse-motion"))
      {
        nct_set_cursor_pos (backend->term, ix, iy);
        nct_flush (backend->term);
        if (was_down)
        {
          mrg_pointer_release (mrg, x, y, 0);
          was_down = 0;
        }
        mrg_pointer_motion (mrg, x, y, 0);
      } else if (!strcmp (event, "mouse-drag"))
      {
        mrg_pointer_motion (mrg, x, y, 0);
      } else if (!strcmp (event, "size-changed"))
      {
        int width = nct_sys_terminal_width ();
        int height = nct_sys_terminal_height ();
        nct_set_size (backend->term, width, height);
        width *= CPX;
        height *= CPX;
        free (mrg->glyphs);
        free (mrg->styles);
        free (backend->nct_pixels);
        backend->nct_pixels = calloc (width * height * 4, 1);
        mrg->glyphs = calloc ((width/CPX) * (height/CPX) * 4, 1);
        mrg->styles = calloc ((width/CPX) * (height/CPX) * 1, 1);
        mrg_set_size (mrg, width, height);
        mrg_queue_draw (mrg, NULL);
      }
      else
      {
        if (!strcmp (event, "esc"))
          mrg_key_press (mrg, 0, "escape");
        else if (!strcmp (event, "space"))
          mrg_key_press (mrg, 0, " ");
        else if (!strcmp (event, "enter"))
          mrg_key_press (mrg, 0, "\n");
        else if (!strcmp (event, "return"))
          mrg_key_press (mrg, 0, "\n");
        else
        mrg_key_press (mrg, 0, event);
      }
    }
    
    if (nct_has_event (backend->term, 25))
      return mrg_nct_consume_events (mrg);
  return 1;
}

static void mrg_nct_main (Mrg *mrg,
                          void (*ui_update)(Mrg *mrg, void *user_data),
                          void *user_data)
{
  while (!_mrg_has_quit (mrg))
  {
    if (_mrg_is_dirty (mrg))
      mrg_ui_update (mrg);
    if (!mrg_nct_consume_events (mrg))
      usleep (100);
  }
}

static void mrg_nct_prepare (Mrg *mrg)
{
}

static void mrg_nct_clear (Mrg *mrg)
{
  memset (mrg->glyphs, 0, (mrg->width/CPX) * (mrg->height/CPX) * 4);
  memset (mrg->styles, 0, (mrg->width/CPX) * (mrg->height/CPX));
}

static void mrg_nct_warp_pointer (Mrg *mrg, float x, float y)
{
  MrgNct *backend = mrg->backend_data;

  mrg->pointer_x = x;
  mrg->pointer_y = y;
  nct_set_cursor_pos (backend->term, x/CPX, y/CPX);
  nct_flush (backend->term);
}

static void mrg_nct_destroy (Mrg *mrg)
{
  free (mrg->backend_data);
}

static Mrg *_mrg_terminal_new (int width, int height);

MrgBackend mrg_backend_nct = {
  "terminal",
  _mrg_terminal_new,
  mrg_nct_main,
  mrg_nct_get_pixels,
  NULL, /* mrg_cr, */
  mrg_nct_flush,
  mrg_nct_prepare,
  mrg_nct_clear,
  NULL, /* mrg_queue_draw, */
  mrg_nct_destroy,
  mrg_nct_warp_pointer,
  NULL, /* fullscreen */
  NULL, /* get position */
  NULL, /* set position */
  NULL, /* mrg_nct_set_title, XXX: this is possible! */
  NULL, /* mrg_nct_get_title, */
};

static Mrg *_mrg_terminal_new (int width, int height)
{
  Mrg *mrg;
  MrgNct *backend = calloc (sizeof (MrgNct), 1);

  backend->term = nct_new ();
  nct_clear (backend->term);
  nct_flush (backend->term);
  nct_mouse (backend->term, NC_MOUSE_DRAG);
        
  width = nct_sys_terminal_width () * CPX;
  height = nct_sys_terminal_height () * CPX;

  setvbuf(stdin, NULL, _IONBF, 0); 

  mrg = calloc (sizeof (Mrg), 1);
  backend->nct_pixels = calloc (width * height * 4, 1);
  mrg->glyphs = calloc ((width/CPX) * (height/CPX) * 4, 1);
  mrg->styles = calloc ((width/CPX) * (height/CPX) * 1, 1);

  mrg->backend = &mrg_backend_nct;
  mrg->backend_data = backend;

  _mrg_init (mrg, width, height);
  mrg->ddpx = 0.25;

  mrg->fullscreen = 1;

  if (getenv ("MRG_BLACK_ON_WHITE"))
    mrg_black_on_white = 1;

  return mrg;
}
#endif
