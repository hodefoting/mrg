/* utf8 ansi/vt100/rxvt terminal escape code interpretation engine
 *
 * built to be a self-contained engine that is utf8 native providing a
 * simple core API which other interfaces can be built on top of.
 *
 * TODO: selection + copy/(bracketed) paste
 *       scrollback
 *       origin mode
 *       special handling of box drawing characters
 *       deal with scroll events
 *       sane API for scrollback
 *       tabs
 *       keyrepeat
 *       alternate screen
 *       256color
 *       dim, hidden
 *       mouse reporting
 *       optimize newline
 *       overlay with commands for scrollback + fontsize change
 *
 * Copyright (c) 2014, 2016, 2018 Øyvind Kolås <pippin@hodefoting.com>
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

/* TODO:
 *   scrollback
 *   selection, copy + paste
 *   runtime font selection
 */

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pty.h>
#include "mrg.h"
#include "mrg-string.h"
#include "mrg-list.h"
#include "mrg-vt.h"

typedef enum {
  MRG_VT_STYLE_RESET          = 0,
  MRG_VT_STYLE_BOLD           = 1,
  MRG_VT_STYLE_DIM            = 2,
  MRG_VT_STYLE_UNDERSCORE     = 4,
  MRG_VT_STYLE_BLINK          = 5,
  MRG_VT_STYLE_REVERSE        = 7,
  MRG_VT_STYLE_HIDDEN         = 8,
  MRG_VT_STYLE_STRIKETHROUGH  = 9,
  MRG_VT_STYLE_FONT0          = 10,
  MRG_VT_STYLE_FONT1          = 11,
  MRG_VT_STYLE_FONT2          = 12,
  MRG_VT_STYLE_FONT3          = 13,
  MRG_VT_STYLE_FONT4          = 14,

  MRG_VT_STYLE_BOLD_OFF       = 22,
  MRG_VT_STYLE_DIM_OFF        = 23,
  MRG_VT_STYLE_UNDERSCORE_OFF = 24,
  MRG_VT_STYLE_BLINK_OFF      = 25,
  MRG_VT_STYLE_REVERSE_OFF    = 27,
  MRG_VT_STYLE_HIDDEN_OFF     = 28,
  MRG_VT_STYLE_STRIKETHROUGH_OFF = 29,
  MRG_VT_STYLE_FG_BLACK       = 30,
  MRG_VT_STYLE_FG_RED,
  MRG_VT_STYLE_FG_GREEN,
  MRG_VT_STYLE_FG_YELLOW,
  MRG_VT_STYLE_FG_BLUE,
  MRG_VT_STYLE_FG_MAGENTA,
  MRG_VT_STYLE_FG_CYAN,
  MRG_VT_STYLE_FG_LIGHT_GRAY = 37,
  MRG_VT_STYLE_FG_DEFAULT    = 39,
  MRG_VT_STYLE_BG_BLACK      = 40,
  MRG_VT_STYLE_BG_RED,
  MRG_VT_STYLE_BG_GREEN,
  MRG_VT_STYLE_BG_YELLOW,
  MRG_VT_STYLE_BG_BLUE,
  MRG_VT_STYLE_BG_MAGENTA,
  MRG_VT_STYLE_BG_CYAN,
  MRG_VT_STYLE_BG_LIGHT_GRAY = 47,
  MRG_VT_STYLE_BG_DEFAULT    = 49,
  MRG_VT_STYLE_FG_DARK_GRAY  = 90,
  MRG_VT_STYLE_FG_LIGHT_RED,
  MRG_VT_STYLE_FG_LIGHT_GREEN,
  MRG_VT_STYLE_FG_LIGHT_YELLOW,
  MRG_VT_STYLE_FG_LIGHT_BLUE,
  MRG_VT_STYLE_FG_LIGHT_MAGENTA,
  MRG_VT_STYLE_FG_LIGHT_CYAN,
  MRG_VT_STYLE_FG_WHITE = 97,
  MRG_VT_STYLE_BG_DARK_GRAY = 100,
  MRG_VT_STYLE_BG_LIGHT_RED,
  MRG_VT_STYLE_BG_LIGHT_GREEN,
  MRG_VT_STYLE_BG_LIGHT_YELLOW,
  MRG_VT_STYLE_BG_LIGHT_BLUE,
  MRG_VT_STYLE_BG_LIGHT_MAGENTA,
  MRG_VT_STYLE_BG_LIGHT_CYAN,
  MRG_VT_STYLE_BG_WHITE,

} MrgVtStyle;

static int mrg_vt_trimlines (MrgVT *vt, int max);

#define ENABLE_STYLE

#ifdef ENABLE_STYLE
#endif

typedef enum {
  TERMINAL_STATE_NEUTRAL          = 0,
  TERMINAL_STATE_GOT_ESC          = 1,
  TERMINAL_STATE_GOT_ESC_SQRPAREN = 2,
  TERMINAL_STATE_GOT_ESC_SEQUENCE = 3,
  TERMINAL_STATE_GOT_ESC_FOO      = 4,
} TerminalState;

#define MAX_COLS 200
#define MAX_ROWS 200

typedef enum {
  STYLE_BOLD          =  1 << 0,
  STYLE_DIM           =  1 << 1,
  STYLE_UNDERLINE     =  1 << 2,
  STYLE_REVERSE       =  1 << 3,
  STYLE_BLINK         =  1 << 4,
  STYLE_HIDDEN        =  1 << 5,
  STYLE_STRIKETHROUGH =  1 << 6,
} TerminalStyle;

struct _MrgVT {
  char      *commandline;
  char      *title;

  MrgList   *lines;
  int        line_count;
  uint32_t   style[MAX_ROWS][MAX_COLS];
  uint32_t   cstyle;
  int        debug;
  int        black_on_white;
  int        origin;
  int        charset;

  TerminalState state;
  long       rev;
  char       utf8_holding[4];

  int        encoding;  // 0 = utf8 1=pc vga 2=ascii

  int        insert_mode;
  int        autowrap;
  int        utf8_expected;
  int        utf8_pos;
  int        cursor_x;
  int        cursor_y;
  int        cols;
  int        rows;
  MrgString *current_line;
  int        pty;
  pid_t      pid;
  int        cr_on_lf;
  int        cursor_visible;
  int        saved_x;
  int        saved_y;
  uint32_t   saved_style;
  int        saved_charset;
  int        cursor_key_application;
  int        scroll_top;
  int        scroll_bottom;
  int        lines_scrollback;

  char       argument_buf[64];
  uint8_t    tabs[MAX_COLS];
  int        argument_buf_len;
  int        inert;

  int        idle_poller;
  int        done;
  int        result;
  Mrg       *mrg;
};

long mrg_vt_rev (MrgVT *vt)
{
  return vt->rev;
}

void mrg_vt_feed_byte (MrgVT *vt, int byte);
static void vtcmd_reset_device (MrgVT *vt, const char *sequence);

#define log(args...) \
  if (vt->debug) \
    fprintf (stderr, args);

static void mrg_vt_set_title (MrgVT *vt, const char *new_title)
{
  if (vt->inert) return;
  if (vt->title)
    free (vt->title);
  vt->title = strdup (new_title);

  if (vt->mrg)
    mrg_set_title (vt->mrg, new_title);
}

const char *mrg_vt_get_title (MrgVT *vt)
{
  return vt->title;
}

static int mrg_pty_poll (Mrg *mrg, void *data)
{
#if 0
  static long prev_rev = 0; // XXX: makes multi instance hard - ok for now; and just glitchy when trouble
#endif
  MrgVT *vt = data;
  if (vt->inert) return 0;
  mrg_vt_poll (vt);
#if 0
  if (mrg_vt_rev (vt) != prev_rev)
  {
    mrg_queue_draw (mrg, NULL);
    prev_rev = mrg_vt_rev (vt);
  }
#endif
  return 1;
}

static MrgList *vts = NULL;

static void mrg_vt_run_command (MrgVT *vt, const char *command);
static void vtcmd_set_top_and_bottom_margins (MrgVT *vt, const char *sequence);
static void _mrg_vt_move_to (MrgVT *vt, int y, int x);

static void vtcmd_reset_device (MrgVT *vt, const char *sequence)
{
  while (vt->lines)
  {
    mrg_string_free (vt->lines->data, 1);
    mrg_list_remove (&vt->lines, vt->lines->data);
  }
  if (getenv ("VT_DEBUG"))
    vt->debug = 1;
  vt->encoding = 0;
  vt->lines = NULL;
  vt->line_count = 0;
  vt->cr_on_lf = 1;
  vtcmd_set_top_and_bottom_margins (vt, "[r");
  vt->cursor_key_application = 0;
  vt->autowrap       = 1;
  vt->cstyle         = 0;
  vt->cursor_visible = 1;

  vt->saved_x                = 1;
  vt->saved_y                = 1;
  vt->saved_style            = 1;

  vt->cursor_key_application = 0;
  vt->argument_buf_len       = 0;
  vt->argument_buf[0]        = 0;
  vt->done                   = 0;
  vt->result                 = -1;
  vt->state                  = TERMINAL_STATE_NEUTRAL,
  vt->commandline            = NULL;

  /* populate lines */
  for (int i=0; i<vt->rows;i++)
  {
    vt->current_line = mrg_string_new ("");
    mrg_list_prepend (&vt->lines, vt->current_line);
    vt->line_count++;
  }

  for (int i = 0; i < MAX_COLS; i++)
    vt->tabs[i] = i % 8 == 0? 1 : 0;

  _mrg_vt_move_to (vt, 1, 1);
}


MrgVT *mrg_vt_new (Mrg *mrg, const char *command)
{
  MrgVT *vt                  = calloc (sizeof (MrgVT), 1);
  vt->cursor_visible         = 1;
  vt->lines                  = NULL;
  vt->line_count             = 0;
  vt->current_line           = NULL;
  vt->cols                   = DEFAULT_COLS;
  vt->rows                   = DEFAULT_ROWS;
  vt->scroll_top             = 1;
  vt->scroll_bottom          = DEFAULT_ROWS - 1;
  vt->lines_scrollback       = DEFAULT_SCROLLBACK;

  vt->cursor_key_application = 0;
  vt->argument_buf_len       = 0;
  vt->argument_buf[0]        = 0;
  vt->done                   = 0;
  vt->result                 = -1;
  vt->state                  = TERMINAL_STATE_NEUTRAL,
  vt->commandline            = NULL;

  if (command)
  {
    mrg_vt_run_command (vt, command);
    vt->commandline = strdup (command);
  }

  mrg_vt_set_term_size (vt, vt->cols, vt->rows); // makes top few lines be visible - but bit of a hack
  vtcmd_reset_device (vt, NULL);

  if (mrg)
  {
    vt->idle_poller = mrg_add_idle (mrg, mrg_pty_poll, vt);
    vt->mrg = mrg;
  }

  mrg_list_prepend (&vts, vt);

  return vt;
}

/* To get functional scrollback, tweak trimlines... and fix bugs/workarounds
   relying on the behavior of continous trimming
 */
static int mrg_vt_trimlines (MrgVT *vt, int max)
{
  MrgList *chop_point = NULL;
  MrgList *l;
  int i;

  max += vt->lines_scrollback; /* needed for scrollback escape */

  if (vt->line_count < max)
    return 0;

  for (l = vt->lines, i = 0; l && i < max-1; l = l->next, i++);

  if (l)
  {
    chop_point = l->next;
    l->next = NULL;
  }

  while (chop_point)
  {
    mrg_string_free (chop_point->data, 1);
    mrg_list_remove (&chop_point, chop_point->data);
    vt->line_count--;
  }
  return 0;
}

void mrg_vt_set_term_size (MrgVT *vt, int icols, int irows)
{
  struct winsize ws;
  vt->rows = ws.ws_row = irows;
  vt->cols = ws.ws_col = icols;
  ws.ws_xpixel = ws.ws_col * 8;
  ws.ws_ypixel = ws.ws_row * 8;
  ioctl(vt->pty, TIOCSWINSZ, &ws);
  mrg_vt_trimlines (vt, vt->rows);

  vt->scroll_top = 1;
  vt->scroll_bottom = vt->rows; // bottom;
}

static void mrg_vt_argument_buf_reset (MrgVT *vt, const char *start)
{
  if (start)
  {
    strcpy (vt->argument_buf, start);
    vt->argument_buf_len = strlen (start);
  }
  else
    vt->argument_buf[vt->argument_buf_len=0]=0;
}

static inline void mrg_vt_argument_buf_add (MrgVT *vt, int ch)
{
  if (vt->argument_buf_len < 62)
  {
    vt->argument_buf[vt->argument_buf_len] = ch;
    vt->argument_buf[++vt->argument_buf_len] = 0;
  }
}

static void _mrg_vt_move_to (MrgVT *vt, int y, int x)
{
  int i;
  x = x < 1 ? 1 : (x > vt->cols ? vt->cols : x);
  y = y < 1 ? 1 : (y > vt->rows ? vt->rows : y);

  vt->cursor_x = x;
  vt->cursor_y = y;

  i = vt->rows - y;

  MrgList *l;
  for (l = vt->lines; l && i >= 1; l = l->next, i--);
  if (l)
  {
    vt->current_line = l->data;
  }
  else
  {
    for (; i > 0; i--)
      {
        vt->current_line = mrg_string_new ("");
        mrg_list_append (&vt->lines, vt->current_line);
        vt->line_count++;
      }
  }
  mrg_vt_trimlines (vt, vt->rows);
}

static void _mrg_vt_add_str (MrgVT *vt, const char *str)
{
  if (vt->cursor_x > vt->cols)
  {
    if (vt->autowrap) {
      mrg_vt_feed_byte (vt, '\n');
      vt->cursor_x = 1;
    }
    else
      vt->cursor_x = vt->cols;
  }

  if (vt->insert_mode)
   {
     fprintf (stderr, "insert mode testing...\n");
     vt->style[vt->cursor_y][vt->cursor_x] = vt->cstyle;
     mrg_string_insert_utf8 (vt->current_line, vt->cursor_x - 1, str);
   }
  else
   {
     vt->style[vt->cursor_y][vt->cursor_x] = vt->cstyle;
     mrg_string_replace_utf8 (vt->current_line, vt->cursor_x - 1, str);
   }
  vt->cursor_x ++;
}

static void _mrg_vt_backspace (MrgVT *vt)
{
  if (vt->current_line)
  {
    vt->cursor_x --;
    if (vt->cursor_x < 1)
      vt->cursor_x = 1;
  }
}

static void vtcmd_set_top_and_bottom_margins (MrgVT *vt, const char *sequence)
{
  int top = 1, bottom = vt->rows;
  if (strlen (sequence) != 2)
  {
    sscanf (sequence, "[%i;%ir", &top, &bottom);
  }
  vt->scroll_top = top;
  vt->scroll_bottom = bottom;
}

static void vt_scroll_style (MrgVT *vt, int amount)
{
  if (amount > 0)
  {
    for (int row = vt->scroll_bottom; row > vt->scroll_top; row--)
      memcpy (&vt->style[row][0],
              &vt->style[row-1][0],
              sizeof(vt->style[0]));
    memset (vt->style[vt->scroll_top], 0, sizeof (vt->style[0]));
  }
  else
  {
    for (int row = vt->scroll_top; row < vt->scroll_bottom; row++)
      memcpy (&vt->style[row][0],
              &vt->style[row+1][0],
              sizeof(vt->style[0]));
    memset (vt->style[vt->scroll_bottom], 0, sizeof (vt->style[0]));
  }
}

static void vt_scroll (MrgVT *vt, int amount)
{
  int remove_no, insert_before;
  if (amount < 0)
    {
      remove_no = vt->scroll_top;
      insert_before = vt->scroll_bottom;
    }
  else
    {
      remove_no = vt->scroll_bottom;
      insert_before = vt->scroll_top;
    }
  MrgList *l;
  int i;

  for (i=vt->rows, l = vt->lines; i > 0 && l; l=l->next, i--)
  {
    if (i == remove_no)
    {
      mrg_string_free (l->data, 1);
      mrg_list_remove (&vt->lines, l->data);
      break;
    }
  }

  if (amount > 0 && vt->scroll_top == 1)
  {
    MrgString *new_line = mrg_string_new ("");
    mrg_list_append (&vt->lines, new_line);
    vt->current_line = new_line;
  }
  else
  {
    for (i=vt->rows, l = vt->lines; l; l=l->next, i--)
    {
      if (i == insert_before)
      {
        MrgString *new_line = mrg_string_new ("");
        mrg_list_insert_before (&vt->lines, l, new_line);
        vt->current_line = new_line;
        break;
      }
    }

    if (i != insert_before)
    {
      MrgString *new_line = mrg_string_new ("");
      mrg_list_append (&vt->lines, new_line);
      vt->current_line = new_line;
    }
  }
  /* not updating line count since we should always remove one and add one */

  vt_scroll_style (vt, amount);
}

typedef struct Sequence {
  const char *prefix;
  char        suffix;
  void (*vtcmd) (MrgVT *vt, const char *sequence);
} Sequence;

static void vtcmd_set_cursor_key_to_application (MrgVT *vt, const char *sequence)
{
  vt->cursor_key_application = 1;
};

static void vtcmd_set_cursor_key_to_cursor (MrgVT *vt, const char *sequence)
{
  vt->cursor_key_application = 0;
};

static inline int parse_int (const char *arg, int def_val)
{
  if (!isdigit (arg[1]) || strlen (arg) == 2)
    return def_val;
  return atoi (arg+1);
}

static void vtcmd_cursor_position (MrgVT *vt, const char *sequence)
{
  int y = 1, x = 1;
  const char *semi;
  if (sequence[0] != 'H')
  {
  y = parse_int (sequence, 1);
  if ( (semi = strchr (sequence, ';')))
  {
    x = parse_int (semi, 1);
  }
  }
  _mrg_vt_move_to (vt, y, x);
};


static void vtcmd_goto_column (MrgVT *vt, const char *sequence)
{
  int x = parse_int (sequence, 1);
  _mrg_vt_move_to (vt, vt->cursor_y, x);
}

static void vtcmd_goto_row (MrgVT *vt, const char *sequence)
{
  int y = parse_int (sequence, 1);
  _mrg_vt_move_to (vt, y, vt->cursor_x);
}

static void vtcmd_cursor_forward (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  for (int i = 0; i < n; i++)
    _mrg_vt_move_to (vt, vt->cursor_y, vt->cursor_x + 1);
}

static void vtcmd_cursor_backward (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  for (int i = 0; i < n; i++)
    _mrg_vt_move_to (vt, vt->cursor_y, vt->cursor_x - 1);
}

static void vtcmd_cursor_up (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);

  for (int i = 0; i < n; i++)
  {
    if (vt->cursor_y == vt->scroll_top)
    {
      vt_scroll (vt, 1);
      _mrg_vt_move_to (vt, vt->scroll_top, vt->cursor_x);
    }
    else
    {
      _mrg_vt_move_to (vt, vt->cursor_y-1, vt->cursor_x);
    }
  }
}

static void vtcmd_cursor_down (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  for (int i = 0; i < n; i++)
  {
    if (vt->cursor_y == vt->scroll_bottom)
    {
      vt_scroll (vt, -1);
      _mrg_vt_move_to (vt, vt->scroll_bottom, vt->cursor_x);
    }
    else
    {
      _mrg_vt_move_to (vt, vt->cursor_y + 1, vt->cursor_x);
    }
  }
}

static void vtcmd_next_line (MrgVT *vt, const char *sequence)
{
  vtcmd_cursor_down (vt, sequence);
  _mrg_vt_move_to (vt, vt->cursor_y, 1);
}

static void vtcmd_cursor_up_and_first_col (MrgVT *vt, const char *sequence)
{
  vtcmd_cursor_up (vt, sequence);
  _mrg_vt_move_to (vt, vt->cursor_y, 1);
}

static void vtcmd_erase_in_line (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of line
      {
        char *p = (char*)mrg_utf8_skip (vt->current_line->str, vt->cursor_x-1);
        if (p) *p = 0;
        vt->current_line->length = strlen (vt->current_line->str);
        vt->current_line->utf8_length = mrg_utf8_strlen (vt->current_line->str); //XXX
      }
      break;
    case 1: // clear from beginning to cursor
      {
        int i;
        for (i = 0; i < vt->cursor_x-1; i++)
        {
          mrg_string_replace_utf8 (vt->current_line, i, " ");
        }
        vt->current_line->length = strlen (vt->current_line->str);
        vt->current_line->utf8_length = mrg_utf8_strlen (vt->current_line->str); //XXX
      }
      break;
    case 2: // clear entire line
      mrg_string_set (vt->current_line, "");
      break;
  }
}

static void vtcmd_erase_in_display (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of screen
      {
        char *p = (char*)mrg_utf8_skip (vt->current_line->str, vt->cursor_x-1);
        if (p) *p = 0;
        vt->current_line->length = strlen (vt->current_line->str);
        vt->current_line->utf8_length = mrg_utf8_strlen (vt->current_line->str);
      }
      {
        MrgList *l;
        for (l = vt->lines; l->data != vt->current_line; l = l->next)
        {
          MrgString *buf = l->data;
          buf->str[0] = 0;
          buf->length = 0;
          buf->utf8_length = 0;
        }
      }
      break;
    case 1: // clear from beginning to cursor
      {
        int i;
        for (i = 0; i < vt->cursor_x-1; i++)
        {
          mrg_string_replace_utf8 (vt->current_line, i, " ");
        }
      }
      {
        MrgList *l;
        int there_yet = 0;
        int no = 0;

        for (l = vt->lines; l && no < vt->rows; l = l->next, no ++)
        {
          MrgString *buf = l->data;
          if (there_yet)
          {
            buf->str[0] = 0;
            buf->length = 0;
            buf->utf8_length = 0;
          }
          if (buf == vt->current_line)
          {
            there_yet = 1;
          }
        }
      }
      break;
    case 2: // clear entire screen but keep cursor;
      {
        int tx = vt->cursor_x;
        int ty = vt->cursor_y;
        vtcmd_reset_device (vt, "");
        _mrg_vt_move_to (vt, ty, tx);
      }
      break;
  }
}

static void vtcmd_screen_alignment_display (MrgVT *vt, const char *sequence)
{
  for (int y = 1; y <= vt->rows; y++)
  {
    _mrg_vt_move_to (vt, y, 1);
    for (int x = 1; x <= vt->cols; x++)
      {
        _mrg_vt_add_str (vt, "E");
      }
  }
}

static void vtcmd_set_style (MrgVT *vt, const char *sequence)
{
  const char *s = sequence + 1;
  while (*s)
  {
  int n = parse_int (s - 1, 0); // works until color

  switch (n)
  {
    case MRG_VT_STYLE_RESET:    // clear all style
      vt->cstyle = 0;
      break;
    case MRG_VT_STYLE_BOLD:     // Alternate Intensity ON
      vt->cstyle ^= STYLE_BOLD;
      break;
    case MRG_VT_STYLE_BOLD_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_BOLD);
      break;
    case MRG_VT_STYLE_FONT0:
      vt->charset = 0;
      break;
    case MRG_VT_STYLE_FONT1:
      vt->charset = 1;
      break;
    case MRG_VT_STYLE_FONT2:
    case MRG_VT_STYLE_FONT3:
    case MRG_VT_STYLE_FONT4:
      break;


    case MRG_VT_STYLE_DIM:
      vt->cstyle |= STYLE_DIM;
      break;
    case MRG_VT_STYLE_DIM_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_DIM);
      break;
    case MRG_VT_STYLE_UNDERSCORE:
      vt->cstyle |= STYLE_UNDERLINE;
      break;
    case MRG_VT_STYLE_UNDERSCORE_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_UNDERLINE);
      break;
    case MRG_VT_STYLE_STRIKETHROUGH:
      vt->cstyle |= STYLE_STRIKETHROUGH;
      break;
    case MRG_VT_STYLE_STRIKETHROUGH_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_STRIKETHROUGH);
      break;

    case MRG_VT_STYLE_BLINK:
      vt->cstyle |= STYLE_BLINK;
      break;
    case MRG_VT_STYLE_BLINK_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_BLINK);
      break;
    case MRG_VT_STYLE_HIDDEN:
      vt->cstyle |= STYLE_HIDDEN;
      break;
    case MRG_VT_STYLE_HIDDEN_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_HIDDEN);
      break;
    case MRG_VT_STYLE_REVERSE:
      vt->cstyle |= STYLE_REVERSE;
      break;
    case MRG_VT_STYLE_REVERSE_OFF:
      vt->cstyle ^= (vt->cstyle & STYLE_REVERSE);
      break;

#define set_fg_idx(idx) \
    vt->cstyle ^= (vt->cstyle & (31<<8));\
    vt->cstyle |=  ((idx)<<8);


    case MRG_VT_STYLE_FG_DEFAULT: set_fg_idx(0);break;
    case MRG_VT_STYLE_FG_BLACK:   set_fg_idx(16); break;
    case MRG_VT_STYLE_FG_RED:     set_fg_idx(1); break;
    case MRG_VT_STYLE_FG_GREEN:   set_fg_idx(2); break;;
    case MRG_VT_STYLE_FG_YELLOW:  set_fg_idx(3); break;;
    case MRG_VT_STYLE_FG_BLUE:    set_fg_idx(4); break;;
    case MRG_VT_STYLE_FG_MAGENTA: set_fg_idx(5); break;;
    case MRG_VT_STYLE_FG_CYAN:    set_fg_idx(6); break;;

    case MRG_VT_STYLE_FG_DARK_GRAY:   set_fg_idx(7); break;;
    case MRG_VT_STYLE_FG_LIGHT_GRAY:  set_fg_idx(8); break;;
    case MRG_VT_STYLE_FG_LIGHT_RED:    set_fg_idx(9); break;;
    case MRG_VT_STYLE_FG_LIGHT_GREEN:  set_fg_idx(10); break;;
    case MRG_VT_STYLE_FG_LIGHT_YELLOW:     set_fg_idx(11); break;;
    case MRG_VT_STYLE_FG_LIGHT_BLUE:         set_fg_idx(12); break;;
    case MRG_VT_STYLE_FG_LIGHT_MAGENTA:      set_fg_idx(13); break;;
    case MRG_VT_STYLE_FG_LIGHT_CYAN:         set_fg_idx(14); break;;
    case MRG_VT_STYLE_FG_WHITE:              set_fg_idx(15); break;;

#define set_bg_idx(idx) \
    vt->cstyle ^= (vt->cstyle & (31<<16));\
    vt->cstyle |= ((idx)<<16);

    case MRG_VT_STYLE_BG_DEFAULT: set_bg_idx(0); break;
    case MRG_VT_STYLE_BG_BLACK: set_bg_idx(16); break;
    case MRG_VT_STYLE_BG_RED: set_bg_idx(1); break;
    case MRG_VT_STYLE_BG_GREEN: set_bg_idx(2); break;
    case MRG_VT_STYLE_BG_YELLOW: set_bg_idx(3); break;
    case MRG_VT_STYLE_BG_BLUE: set_bg_idx(4); break;
    case MRG_VT_STYLE_BG_MAGENTA: set_bg_idx(5); break;
    case MRG_VT_STYLE_BG_CYAN: set_bg_idx(6); break;
    case MRG_VT_STYLE_BG_LIGHT_GRAY: set_bg_idx(7); break;
    case MRG_VT_STYLE_BG_DARK_GRAY:  set_bg_idx(8); break;
    case MRG_VT_STYLE_BG_LIGHT_RED: set_bg_idx(9); break;
    case MRG_VT_STYLE_BG_LIGHT_GREEN: set_bg_idx(10); break;
    case MRG_VT_STYLE_BG_LIGHT_YELLOW: set_bg_idx(11); break;
    case MRG_VT_STYLE_BG_LIGHT_BLUE: set_bg_idx(12); break;
    case MRG_VT_STYLE_BG_LIGHT_MAGENTA: set_bg_idx(13); break;
    case MRG_VT_STYLE_BG_LIGHT_CYAN: set_bg_idx(14); break;
    case MRG_VT_STYLE_BG_WHITE: set_bg_idx(15); break;

    default:
      fprintf (stderr, "mrgvt: unhandled style code %i in sequence '%s'\n", n, sequence);
      break;

#undef set_col
  }
    while(*s && *s != ';') s++;
    if (*s == ';') s++;
  }
}

static void vtcmd_ignore (MrgVT *vt, const char *sequence)
{
}

static void vtcmd_clear_all_tabs (MrgVT *vt, const char *sequence)
{
  memset (vt->tabs, 0, sizeof (vt->tabs));
}

static void vtcmd_clear_current_tab (MrgVT *vt, const char *sequence)
{
  vt->tabs[vt->cursor_x-1] = 0;
}

static void vtcmd_set_tab_at_current_column (MrgVT *vt, const char *sequence)
{
  vt->tabs[vt->cursor_x-1] = 1;
}

static void vtcmd_cursor_position_report (MrgVT *vt, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[%i;%iR", vt->cursor_y, vt->cursor_x);
  write (vt->pty, buf, strlen(buf));
}

static void vtcmd_status_report (MrgVT *vt, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[0n"); // we're always OK :)
  write (vt->pty, buf, strlen(buf));
}

static void vtcmd_device_attributes (MrgVT *vt, const char *sequence)
{
  char *buf = "\033[?1;6c";          // identify as vt102
  write (vt->pty, buf, strlen(buf));
}

static void vtcmd_save_cursor_position (MrgVT *vt, const char *sequence)
{
  vt->saved_x = vt->cursor_x;
  vt->saved_y = vt->cursor_y;
  vt->saved_style = vt->cstyle;
  vt->saved_charset  = vt->charset;
}

static void vtcmd_restore_cursor_position (MrgVT *vt, const char *sequence)
{
  _mrg_vt_move_to (vt, vt->saved_y, vt->saved_x);
  vt->cstyle = vt->saved_style;
  vt->charset = vt->saved_charset;
}

static void vtcmd_insert_n_tabs (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
    do {
      _mrg_vt_add_str (vt, " ");
    } while ( ((vt->current_line->length) % 8));
  }
}

static void vtcmd_erase_n_chars (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
     mrg_string_replace_utf8 (vt->current_line, vt->cursor_x - 1, " ");
  }
}

static void vtcmd_delete_n_chars (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
     mrg_string_remove_utf8 (vt->current_line, vt->cursor_x - 1);
  }
}

static void vtcmd_delete_n_lines (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  for (int a = 0; a < n; a++)
  {
    int i;
    MrgList *l;
    mrg_list_remove (&vt->lines, vt->current_line);
    for (i=vt->rows, l = vt->lines; l; l=l->next, i--)
    {
      if (i == vt->scroll_bottom)
      {
        vt->current_line = mrg_string_new ("");
        mrg_list_insert_before (&vt->lines, l, vt->current_line);
        break;
      }
    }
    _mrg_vt_move_to (vt, vt->cursor_y, vt->cursor_x);
    vt_scroll_style (vt, -1);
  }
}

static void vtcmd_insert_blanks (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
     mrg_string_insert_utf8 (vt->current_line, vt->cursor_x - 1, " ");
  }
}

static void vtcmd_scroll_up (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
    vt_scroll (vt, -1);
}

static void vtcmd_scroll_down (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
    vt_scroll (vt, 1);
}

static void vtcmd_insert_blank_lines (MrgVT *vt, const char *sequence)
{
  int n = parse_int (sequence, 1); // XXX this seems like it might be wrong
  while (n--)
  {
    vt_scroll (vt, 1);
  }
}

static void vtcmd_set_replace_mode (MrgVT *vt, const char *sequence)
{
  vt->insert_mode = 0;
}
static void vtcmd_set_insert_mode (MrgVT *vt, const char *sequence)
{
  vt->insert_mode = 1;
}
static void vtcmd_set_autowrap_mode (MrgVT *vt, const char *sequence)
{
  vt->autowrap = 1;
}
static void vtcmd_set_noautowrap_mode (MrgVT *vt, const char *sequence)
{
  vt->autowrap = 0;
}
static void vtcmd_show_cursor (MrgVT *vt, const char *sequence)
{
  vt->cursor_visible = 1;
}
static void vtcmd_hide_cursor (MrgVT *vt, const char *sequence)
{
  vt->cursor_visible = 0;
}
static void vtcmd_set_origin (MrgVT *vt, const char *sequence)
{
  vt->origin = 1;
  _mrg_vt_move_to (vt, 1, 1);
}

static void vtcmd_set_no_origin (MrgVT *vt, const char *sequence)
{
  vt->origin = 0;
  _mrg_vt_move_to (vt, 1, 1);
}

static void vtcmd_set_cr_on_lf (MrgVT *vt, const char *sequence)
{
  vt->cr_on_lf = 1;
}

static void vtcmd_set_cr_on_lf_off (MrgVT *vt, const char *sequence)
{
  vt->cr_on_lf = 0;
}

static void vtcmd_set_default_font (MrgVT *vt, const char *sequence)
{
  vt->charset = 0;
}

static void vtcmd_set_alternate_font (MrgVT *vt, const char *sequence)
{
  vt->charset = 1;
}

static char* charmap_cp437[]={
" ","☺","☻","♥","♦","♣","♠","•","◘","○","◙","♂","♀","♪","♫","☼",
"►","◄","↕","‼","¶","§","▬","↨","↑","↓","→","←","∟","↔","▲","▼",
" ","!","\"","#","$","%","&","'","(",")","*","+",",","-",".","/",
"0","1","2","3","4","5","6","7","8","9",":",";","<","=",">","?",
"@","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O",
"P","Q","R","S","T","U","V","W","X","Y","Z","[","\\","]","^","_",
"`","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o",
"p","q","r","s","t","u","v","w","x","y","z","{","|","}","~","⌂",
"Ç","ü","é","â","ä","à","å","ç","ê","ë","è","ï","î","ì","Ä","Å",
"É","æ","Æ","ô","ö","ò","û","ù","ÿ","Ö","Ü","¢","£","¥","₧","ƒ",
"á","í","ó","ú","ñ","Ñ","ª","º","¿","⌐","¬","½","¼","¡","«","»",
"░","▒","▓","│","┤","╡","╢","╖","╕","╣","║","╗","╝","╜","╛","┐",
"└","┴","┬","├","─","┼","╞","╟","╚","╔","╩","╦","╠","═","╬","╧",
"╨","╤","╥","╙","╘","╒","╓","╫","╪","┘","┌","█","▄","▌","▐","▀",
"α","ß","Γ","π","Σ","σ","µ","τ","Φ","Θ","Ω","δ","∞","φ","ε","∩",
"≡","±","≥","≤","⌠","⌡","÷","≈","°","∙","·","√","ⁿ","²","■"," "};

static char* charmap[]={
" ","!","\"","#","$","%","&","'","(",")","*","+",",","-",".","/","0",
"1","2","3","4","5","6","7","8","9",":",";","<","=",">","?",
"@","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P",
"Q","R","S","T","U","V","W","X","Y","Z","[","\\","]","^","_",
"◆","▒","␉","␌","␍","␊","°","±","␤","␋","┘","┐","┌","└","┼","⎺","⎻",
"─","⎼","⎽","├","┤","┴","┬","│","≤","≥","π","≠","£","·"," "};

static Sequence sequences[]={
/*
  prefix   suffix  command */

  {"[",     'm',   vtcmd_set_style},
  {"[",     'A',   vtcmd_cursor_up},
  {"[",     'B',   vtcmd_cursor_down},
  {"[",     'C',   vtcmd_cursor_forward},
  {"[",     'D',   vtcmd_cursor_backward},
  {"[",     'E',   vtcmd_next_line},
  {"[",     'F',   vtcmd_cursor_up_and_first_col},
  {"[",     'G',   vtcmd_goto_column},
  {"[",     'H',   vtcmd_cursor_position},
  {"[",     'f',   vtcmd_cursor_position},
  {"[",     'I',   vtcmd_insert_n_tabs},
  {"[",     'J',   vtcmd_erase_in_display},
  {"[",     'K',   vtcmd_erase_in_line},
  {"[",     'L',   vtcmd_insert_blank_lines},
  {"[",     'M',   vtcmd_delete_n_lines},
  {"[",     'P',   vtcmd_delete_n_chars},
  {"[",     'X',   vtcmd_erase_n_chars},
  {"[",     'S',   vtcmd_scroll_up},
  {"[",     'T',   vtcmd_scroll_down},
  /*  [ Z - cursor backward tabulation n tab stops */

  {"[6n",    0,   vtcmd_cursor_position_report},
  {"D",      0,   vtcmd_cursor_down}, /* INDex */
  {"E",      0,   vtcmd_next_line},
  {"M",      0,   vtcmd_cursor_up}, /* reverse index */

  {"[5n",    0,   vtcmd_status_report},
  {"[0g",    0,   vtcmd_clear_current_tab},
  {"[3g",    0,   vtcmd_clear_all_tabs},
  {"[6l",    0,   vtcmd_set_origin},
  {"[6h",    0,   vtcmd_set_no_origin},
  {"[4l",    0,   vtcmd_set_replace_mode},
  {"[4h"  ,  0,   vtcmd_set_insert_mode},
  {"[?7l",   0,   vtcmd_set_noautowrap_mode},
  {"[?7h",   0,   vtcmd_set_autowrap_mode},
  {"[?1049h",0,   vtcmd_ignore}, // save_cursor_go_alternate
  {"[?1049l",0,   vtcmd_ignore}, // restore_cursor_go_mainstream
  {"[?3h",   0,   vtcmd_reset_device},
  {"[?3l",   0,   vtcmd_reset_device},
  {"[20h",   0,   vtcmd_set_cr_on_lf},
  {"[20l",   0,   vtcmd_set_cr_on_lf_off},
  {"7",      0,   vtcmd_save_cursor_position},
  {"8",      0,   vtcmd_restore_cursor_position},
  {"H" ,     0,   vtcmd_set_tab_at_current_column},
  {"[0c" ,   0,   vtcmd_device_attributes},
  {"[",      'r',  vtcmd_set_top_and_bottom_margins},
  {"[",      'e',  vtcmd_cursor_down},
  {"[",      'a',  vtcmd_cursor_forward},
  {"[",      '`',  vtcmd_goto_column},
  {"[",      '@',  vtcmd_insert_blanks},
  {"[",      'd',  vtcmd_goto_row},
  {"[s",     0,   vtcmd_save_cursor_position},
  {"[u",     0,   vtcmd_restore_cursor_position},
  {")B",     0,   vtcmd_set_default_font}, // set_default_font
  {")A",     0,   vtcmd_set_default_font}, // set_default_font
  {")0",     0,   vtcmd_set_alternate_font}, // set_alternate_font
  {"(B",     0,   vtcmd_set_default_font}, // set_default_font
  {"(A",     0,   vtcmd_set_default_font}, // set_default_font
  {"(0",     0,   vtcmd_set_alternate_font}, // set_alternate_font
  {"#8",     0,   vtcmd_screen_alignment_display},
  {"=",      0,   vtcmd_ignore},  // keypad mode change
  {">",      0,   vtcmd_ignore},  // keypad mode change
  {"c",      0,   vtcmd_reset_device},
  {"[!p",    0,   vtcmd_reset_device},
  {"[?1h",   0,   vtcmd_set_cursor_key_to_application},
  {"[?1l",   0,   vtcmd_set_cursor_key_to_cursor},
  {"[?25h",  0,   vtcmd_show_cursor},
  {"[?25l",  0,   vtcmd_hide_cursor},
  {"[?12l",  0,   vtcmd_ignore}, // stop_blinking_cursor
  {"[?2004l", 0,  vtcmd_ignore}, // reset_bracketed_paste_mode
  {"[?2004h", 0,  vtcmd_ignore}, // set_bracketed_paste_mode
  {NULL, 0, NULL}
};

static void handle_sequence (MrgVT *vt, const char *sequence)
{
  int i;
  log ("[ESC]%s", sequence);
  for (i = 0; sequences[i].prefix; i++)
  {
    if (!strncmp (sequence, sequences[i].prefix, strlen(sequences[i].prefix)))
    {
      int i0;
      int mismatch = 0;
      i0 = strlen (sequence)-1;
      if (sequences[i].suffix && (sequence[i0] != sequences[i].suffix))
        mismatch = 1;
      if (!mismatch)
      {
        sequences[i].vtcmd (vt, sequence);
        return;
      }
    }
  }
  log (" <-unhandled\n");
}

static void mrg_vt_line_feed (MrgVT *vt)
{
  if (vt->scroll_top == 1 && vt->scroll_bottom == vt->rows)
  {
    if (vt->lines->data == vt->current_line)
    {
      mrg_list_prepend (&vt->lines, mrg_string_new (""));
      vt->line_count++;
      vt->current_line = vt->lines->data;
    }

    vt->cursor_y++;

    if (vt->cursor_y > vt->scroll_bottom){
      vt->cursor_y = vt->scroll_bottom;
      vt_scroll_style (vt, -1);
    }

    if (vt->cr_on_lf)
      _mrg_vt_move_to (vt, vt->cursor_y, 1);
    else
      _mrg_vt_move_to (vt, vt->cursor_y, vt->cursor_x);
  }
  else
  {
    if (vt->cursor_y == vt->scroll_bottom)
    {
      vt_scroll (vt, -1);
      _mrg_vt_move_to (vt, vt->cursor_y, 1);
    }
    else
    {
      _mrg_vt_move_to (vt, vt->cursor_y + 1, 1);
    }
  }
}

static void mrg_vt_bell (MrgVT *vt)
{
}

static void _mrg_vt_htab (MrgVT *vt)
{
  do {
    _mrg_vt_add_str (vt, " ");
  } while ( ! vt->tabs[vt->cursor_x-1]);
}

void mrg_vt_feed_byte (MrgVT *vt, int byte)
{
  switch (vt->encoding)
  {
    case 0: /* utf8 */
    if (!vt->utf8_expected)
    {
      vt->utf8_expected = mrg_utf8_len (byte) - 1;
      vt->utf8_pos = 0;
    }
    if (vt->utf8_expected)
    {
      vt->utf8_holding[vt->utf8_pos++] = byte;
      if (vt->utf8_pos == vt->utf8_expected + 1)
      {
        vt->utf8_holding[vt->utf8_pos] = 0;
        vt->utf8_expected = 0;
        vt->utf8_pos = 0;
        byte='A';
      }
      else
      {
        return;
      }
    }
    else
    {
      vt->utf8_holding[0] = byte;
      vt->utf8_holding[1] = 0;
    }
    break;
    case 1:
      if ( ! (byte>=0 && byte < 256))
        byte = 255;
      strcpy (&vt->utf8_holding[0], &charmap_cp437[byte][0]);
      vt->utf8_expected = mrg_utf8_len (byte) - 1;
    break;
    default:
      vt->utf8_holding[0] = byte & 127;
      vt->utf8_holding[1] = 0;
    break;
  }

  {
  switch (vt->state)
  {
    case TERMINAL_STATE_NEUTRAL:
      switch (byte)
      {
        case '\0': break;
        case 1:    /* SOH start of heading */
        case 2:    /* STX start of text */
        case 3:    /* ETX end of text */
        case 4:    /* EOT end of transmission */
        case 5:    /* ENQuiry */
        case 6:    /* ACKnolwedge */
        case '\a': /* BELl */ mrg_vt_bell (vt); break;
        case '\b': /* BS */   _mrg_vt_backspace (vt); break;
        case '\t': /* HT tab */ _mrg_vt_htab (vt); break;
        case '\v': _mrg_vt_move_to (vt, vt->cursor_y+1, vt->cursor_x); break; /* VT */
        case '\n': /* LF line ffed */
        case '\f': /* VF form feed */
          log ("[LF]\n");
          mrg_vt_line_feed (vt);
          break;
        case '\r': /* CR carriage return */
           log ("[CR]");
          _mrg_vt_move_to (vt, vt->cursor_y, 1);
          break;
        case 14: /* SO shift in */
        case 15: /* SI shift out */
        case 16: /* DLE data link escape */
        case 17: /* DC1 device control 1 */
        case 18: /* DC2 device control 2 */
        case 19: /* DC3 device control 3 */
        case 20: /* DC4 device control 4 */
        case 21: /* NAK negative ack */
        case 22: /* SYNchronous idle */
        case 23: /* ETB end of trans. blk */
        case 24: /* CANcel */
        case 25: /* EM  end of medium */
        case 26: /* SUBstitute */
          break;
        case 27: /* ESCape */
          vt->state = TERMINAL_STATE_GOT_ESC;
          break;
        case 28: /* FS file separator */
        case 29: /* GS group separator */
        case 30: /* RS record separator */
        case 31: /* US unit separator */
          break;
        default:
          log ("%s", vt->utf8_holding);
          if (vt->charset)
           if ((vt->utf8_holding[0] > ' ') && (vt->utf8_holding[0] <= '~'))
           {
             _mrg_vt_add_str (vt, charmap[vt->utf8_holding[0]-' ']);
             break;
           }

          _mrg_vt_add_str (vt, vt->utf8_holding);
          break;
      }
      break;
    case TERMINAL_STATE_GOT_ESC:
      switch (byte)
      {
        case ')':
        case '#':
        case '(':
          {
            char tmp[]={byte, '\0'};
            mrg_vt_argument_buf_reset(vt, tmp);
            vt->state = TERMINAL_STATE_GOT_ESC_FOO;
          }
          break;
        case '[':
        case '%':
        case '+':
        case '*':
          {
            char tmp[]={byte, '\0'};
            mrg_vt_argument_buf_reset(vt, tmp);
            vt->state = TERMINAL_STATE_GOT_ESC_SEQUENCE;
          }
          break;
        case ']':
          {
            char tmp[]={byte, '\0'};
            mrg_vt_argument_buf_reset(vt, tmp);
            vt->state = TERMINAL_STATE_GOT_ESC_SQRPAREN;
          }
          break;
        default:
          {
            char tmp[]={byte, '\0'};
            tmp[0]=byte;
            handle_sequence (vt, tmp);
            vt->state = TERMINAL_STATE_NEUTRAL;
          }
          break;
      }
      break;
    case TERMINAL_STATE_GOT_ESC_FOO:
      mrg_vt_argument_buf_add (vt, byte);
      handle_sequence (vt, vt->argument_buf);
      vt->state = TERMINAL_STATE_NEUTRAL;
      break;
    case TERMINAL_STATE_GOT_ESC_SEQUENCE:
      if (byte >= '@' && byte <= '~')
      {
        mrg_vt_argument_buf_add (vt, byte);
        handle_sequence (vt, vt->argument_buf);
        vt->state = TERMINAL_STATE_NEUTRAL;
      }
      else
      {
        switch (byte)
        {
          case '\t': _mrg_vt_htab (vt); break;
          case '\b': _mrg_vt_backspace (vt); break;
          case '\r': vt->cursor_x = 1; break;
          case '\v': _mrg_vt_move_to (vt, vt->cursor_y+1, vt->cursor_x); break;
          case '\n':
          case '\f': mrg_vt_line_feed (vt); break;
          default:
            mrg_vt_argument_buf_add (vt, byte);
        }
      }
      break;
    case TERMINAL_STATE_GOT_ESC_SQRPAREN:
      // XXX: use handle sequence here as well,.. for consistency
      if (byte == '\a')
      {
        mrg_vt_set_title (vt, vt->argument_buf + 3);
        // XXX: the above was commented out, since it causes crashes when fuzzing
        // even with just cat /dev/urandom
        vt->state = TERMINAL_STATE_NEUTRAL;
      }
      else
      {
        mrg_vt_argument_buf_add (vt, byte);
      }
      break;
  }
  }
}

void mrg_vt_poll (MrgVT *vt)
{
  unsigned char buf[2048];
  int count = 0;
{
  int len;
a:
  len = read(vt->pty, buf, sizeof (buf));
  if (len > 0)
  {
    int i;
    for (i = 0; i < len; i++)
      mrg_vt_feed_byte (vt, buf[i]);
    count += len;
    if (count < 1024 * 256)
    {
      if (len < sizeof (buf))
        usleep (1000); /* to give pipe chance to fill  */
      goto a;
    }
  }

  if (vt->cursor_y > vt->rows)
    vt->cursor_y = vt->rows;
  if (count >0)
  {
    if (vt->mrg)
      mrg_queue_draw (vt->mrg, NULL);
    vt->rev ++;
   }
  else
   {
     usleep (1000);
   }
}
}

/******/

static const char *keymap_application[][2]={
  {"up",    "\033OA" },
  {"down",  "\033OB" },
  {"right", "\033OC" },
  {"left",  "\033OD" },
};
static const char *keymap_general[][2]={
#if 0
  {"up",             "\033[A"},
  {"down",           "\033[B"},
  {"right",          "\033[C"},
  {"left",           "\033[D"},
#endif
  {"up",    "\033OA" },
  {"down",  "\033OB" },
  {"right", "\033OC" },
  {"left",  "\033OD" },


  {"shift-up",       "\033[1},2A"},
  {"shift-down",     "\033[1},2B"},
  {"shift-right",    "\033[1},2C"},
  {"shift-left",     "\033[1},2D"},
  {"alt-up",         "\033[1},3A"},
  {"alt-down",       "\033[1},3B"},
  {"alt-right",      "\033[1},3C"},
  {"alt-left",       "\033[1},3D"},
  {"alt-shift-up",   "\033[1},4A"},
  {"alt-shift-down", "\033[1},4B"},
  {"alt-shift-right","\033[1},4C"},
  {"alt-shift-left", "\033[1},4D"},
  {"control-up",     "\033[1},5A"},
  {"control-down",   "\033[1},5B"},
  {"control-right",  "\033[1},5C"},
  {"control-left",   "\033[1},5D"},
  {"insert",         "\033[2~"},
  {"delete",         "\033[3~"},
  {"control-delete", "\033[3},5~"},
  {"shift-delete",   "\033[3},2~"},
  {"control-shift-delete",  "\033[3},6~"},
  {"page-up",        "\033[5~"},
  {"page-down" ,     "\033[6~"},
  {"return",         "\r"},
  {"space",          " "},
  {"control-space",  " "},
  {"shift-space",    " "},
  {"control-a",      "\001"},
  {"control-b",      "\002"},
  {"control-c",      "\003"},
  {"control-d",      "\004"},
  {"control-e",      "\005"},
  {"control-f",      "\006"},
  {"control-g",      "\007"},
  {"control-h",      "\010"},
  {"control-i",      "\011"},
  {"control-j",      "\012"},
  {"control-k",      "\013"},
  {"control-l",      "\014"},
  {"control-m",      "\015"},
  {"control-n",      "\016"},
  {"control-o",      "\017"},
  {"control-p",      "\020"},
  {"control-q",      "\021"},
  {"control-r",      "\022"},
  {"control-s",      "\023"},
  {"control-t",      "\024"},
  {"control-u",      "\025"},
  {"control-v",      "\026"},
  {"control-w",      "\027"},
  {"control-x",      "\030"},
  {"control-y",      "\031"},
  {"control-z",      "\032"},
  {"escape",         "\033"},
  {"tab",            "\t"},
  {"backspace",      "\b"},
  {"shift-tab",      "\033[Z"},
  {"home",           "\033[1~"},
  {"end",            "\033[4~"},
  {"F1",             "\033[11~"},
  {"F2",             "\033[12~"},
  {"F3",             "\033[13~"},
  {"F4",             "\033[14~"},
  {"F5",             "\033[15~"},
  {"F6",             "\033[16~"},
  {"F7",             "\033[17~"},
  {"F8",             "\033[18~"},
  {"F9",             "\033[19~"},
  {"F10",            "\033[21~"},
  {"F11",            "\033[22~"},
  {"F12",            "\033[23~"},
};

void mrg_vt_feed_keystring (MrgVT *vt, const char *str)
{
  if (vt->cursor_key_application)
  {
    for (int i = 0; i<sizeof (keymap_application)/sizeof(keymap_application[0]); i++)
      if (!strcmp (str, keymap_application[i][0]))
       { str = keymap_application[i][1]; goto done; }
  }

  for (int i = 0; i<sizeof (keymap_general)/sizeof(keymap_general[0]); i++)
  if (!strcmp (str, keymap_general[i][0]))
    { str = keymap_general[i][1];
      break;
    }
done:
  if (strlen (str))
    write (vt->pty, str, strlen (str));
}

const char *mrg_vt_find_shell_command (void)
{
  int i;
  const char *command = NULL;
  struct stat stat_buf;
  static char *alts[][2] ={
    {"/bin/bash",     "/bin/bash -i"},
    {"/usr/bin/bash", "/usr/bin/bash -i"},
    {"/bin/sh",       "/bin/sh -i"},
    {"/usr/bin/sh",   "/usr/bin/sh -i"},
    {NULL, NULL}
  };
  for (i = 0; alts[i][0] && !command; i++)
  {
    lstat (alts[i][0], &stat_buf);
    if (S_ISREG(stat_buf.st_mode) || S_ISLNK(stat_buf.st_mode))
      command = alts[i][1];
  }
  return command;
}

static void signal_child (int signum)
{
  pid_t pid;
  int   status;
  while ((pid = waitpid (-1, &status, WNOHANG)) != -1)
    {
      if (pid)
      {
        for (MrgList *l = vts; l; l=l->next)
        {
          MrgVT *vt = l->data;
          if (vt->pid == pid)
            {
              vt->done = 1;
              vt->result = status;
            }
        }
      }
    }
}

static int reaper_started = 0;

static void mrg_vt_run_command (MrgVT *vt, const char *command)
{
  struct winsize ws;
  if (!reaper_started)
  {
    reaper_started = 1;
    signal (SIGCHLD, signal_child);
  }

  ws.ws_row = vt->rows;
  ws.ws_col = vt->cols;
  ws.ws_xpixel = ws.ws_col * 8;
  ws.ws_ypixel = ws.ws_row * 8;

  vt->pid = forkpty (&vt->pty, NULL, NULL, &ws);
  if (vt->pid == 0)
  {
    int i;
    for (i = 3; i<64;i++)close(i);/*to be sure xcb is closed*/
    unsetenv ("TERM");
    unsetenv ("COLUMNS");
    unsetenv ("LINES");
    unsetenv ("TERMCAP");
    unsetenv ("COLOR_TERM");
    unsetenv ("VTE_VERSION");
    //setenv ("TERM", "ansi", 1);
    //setenv ("TERM", "vt102", 1);
    //setenv ("TERM", "vt100", 1);
    setenv ("TERM", "xterm", 1);
    vt->result = system (command);
    exit(0);
  }
  else if (vt->pid < 0)
  {
    fprintf (stderr, "forkpty failed (%s)\n", command);
  }
  fcntl(vt->pty, F_SETFL, O_NONBLOCK);
}

const char *mrg_vt_get_commandline (MrgVT *vt)
{
  return vt->commandline;
}

void mrg_vt_destroy (MrgVT *vt)
{

  while (vt->lines)
  {
    mrg_string_free (vt->lines->data, 1);
    mrg_list_remove (&vt->lines, vt->lines->data);
    vt->line_count--;
  }


  mrg_list_remove (&vts, vt);
  if (vt->mrg && vt->idle_poller)
  {
    mrg_remove_idle (vt->mrg, vt->idle_poller);
    vt->idle_poller = 0;
  }
  kill (vt->pid, 9);
  close (vt->pty);
  free (vt);
}

int mrg_vt_get_line_count (MrgVT *vt)
{
  return vt->line_count;
  return mrg_list_length (vt->lines);
}

const char *mrg_vt_get_line (MrgVT *vt, int no)
{
  MrgList *l= mrg_list_nth (vt->lines, no);
  MrgString *str;
  if (!l)
    return NULL;
  str = l->data;
  return str->str;
}

int mrg_vt_get_cols (MrgVT *vt)
{
  return vt->cols;
}

int mrg_vt_get_rows (MrgVT *vt)
{
  return vt->rows;
}

int mrg_vt_get_cursor_x (MrgVT *vt)
{
  return vt->cursor_x;
}

int mrg_vt_get_cursor_y (MrgVT *vt)
{
  return vt->cursor_y;
}

static void event_handler (MrgEvent *event, void *data1, void *data2)
{
  MrgVT *vt = data1;
  const char *str = event->string;
  int cw = (uint8_t*)(data2)-((uint8_t*)NULL);

  if (!str)
  {
    mrg_event_stop_propagate (event);
    return;
  }

  if (!strcmp (str, "resize-event")) {
    Mrg *mrg = event->mrg;
    str = "";

    mrg_vt_set_term_size (vt, mrg_width (mrg) / cw, mrg_height (mrg)/cw);
  }
  else mrg_vt_feed_keystring (vt, str);

  mrg_event_stop_propagate (event);
}

void mrg_vt_draw (MrgVT *vt, Mrg *mrg, double x, double y0, float font_size, float line_spacing)
{
//   mrg_start (mrg, "terminal", NULL);
   mrg_set_edge_left (mrg, x);
   mrg_set_edge_right (mrg, x);
// mrg_set_edge_top (mrg, y);


  mrg_set_stylef (mrg, "font-size: %f; font-family: monospace;", font_size);

  int   cw;
  int   ch;
  {
    mrg_print (mrg, " "); // XXX: hack for measurements; something in the CSS? causes first printed bit to be different
    cw  = mrg_x (mrg);
    mrg_print (mrg, " ");  // measure the space of the presumed monospace font
    cw = mrg_x (mrg) - cw;
    ch = cw * line_spacing;
  }

#if MRG_CAIRO
  //mrg_cairo_set_source_color (mrg_cr (mrg), &mrg_style(mrg)->background_color);
  cairo_set_source_rgb (mrg_cr (mrg), 0,0,0);
  cairo_rectangle (mrg_cr (mrg), x, y0, cw * vt->cols, ch * vt->rows);
  cairo_fill (mrg_cr (mrg));
  //cairo_paint (mrg_cr (mrg));
#endif

  /* draw terminal lines */
  {
    int count = mrg_vt_get_line_count (vt);
    int row = 1;
    uint32_t set_style = 9999;

    float y = y0 + ch * vt->rows;

    for (row = 0; row < count; row ++)
    {
      const char *data = mrg_vt_get_line (vt, row);
      if (data)
      {
        const char *d = data;
        mrg_set_xy (mrg, x, y);
        for (int col = 1; *d; d = mrg_utf8_skip (d, 1), col++)
        {
          char data2[6]=" ";
          data2[0]=d[0];
          memcpy (data2, d, mrg_utf8_len (*d));
          data2[mrg_utf8_len (*d)]=0;

          if (vt->style[vt->rows-row][col] != set_style)
          {
            MrgString *style = mrg_string_new ("");
            set_style = vt->style[vt->rows-row][col];

            if (set_style & STYLE_BOLD)
              mrg_string_append_str (style, "font-weight:bold;");
            else
              mrg_string_append_str (style, "font-weight:normal;");

            if (set_style & STYLE_UNDERLINE)
              mrg_string_append_str (style, "text-decoration:underline;");
            else if (set_style & STYLE_STRIKETHROUGH)
              mrg_string_append_str (style, "text-decoration:linethrough;");
            else
              mrg_string_append_str (style, "text-decoration:none;");

            {
              int color;
              if (set_style & STYLE_REVERSE)
                color = (set_style >> 8) & 31;
              else
                color = (set_style >> 16) & 31;
              switch (color)
              {
                case 0:
            if (set_style & STYLE_REVERSE)
              mrg_string_append_str (style, "background-color:white;");
            else
              mrg_string_append_str (style, "background-color:black;");
                  break;
                case 16: mrg_string_append_str (style, "background-color:black;"); break;
                case 1:  mrg_string_append_str (style, "background-color:#933;"); break;
                case 2:  mrg_string_append_str (style, "background-color:#393;"); break;
                case 3:  mrg_string_append_str (style, "background-color:#993;"); break;
                case 4:  mrg_string_append_str (style, "background-color:#339;"); break;
                case 5:  mrg_string_append_str (style, "background-color:#939;"); break;
                case 6:  mrg_string_append_str (style, "background-color:#399;"); break;
                case 7:  mrg_string_append_str (style, "background-color:#999;"); break;
                case 8:  mrg_string_append_str (style, "background-color:#666;"); break;
                case 9:  mrg_string_append_str (style, "background-color:#f77;"); break;
                case 10: mrg_string_append_str (style, "background-color:#7f7;"); break;
                case 11: mrg_string_append_str (style, "background-color:#ff7;"); break;
                case 12: mrg_string_append_str (style, "background-color:#87f;"); break;
                case 13: mrg_string_append_str (style, "background-color:#f7f;"); break;
                case 14: mrg_string_append_str (style, "background-color:#7ff;"); break;
                case 15: mrg_string_append_str (style, "background-color:#fff;"); break;
                default: mrg_string_append_str (style, "background-color:purple;"); break;
              }
            }

            {
              int color;
              if (set_style & STYLE_REVERSE)
                color = (set_style >> 16) & 31;
              else
                color = (set_style >> 8) & 31;

              switch (color)
              {
                case 0:

            if (set_style & STYLE_REVERSE)
              mrg_string_append_str (style, "color:black;");
            else
              mrg_string_append_str (style, "color:white;");
break;
                case 16: mrg_string_append_str (style, "color:black;"); break;
                case 1: mrg_string_append_str (style, "color:red;"); break;
                case 2: mrg_string_append_str (style, "color:green;"); break;
                case 3: mrg_string_append_str (style, "color:yellow;"); break;
                case 4: mrg_string_append_str (style, "color:blue;"); break;
                case 5: mrg_string_append_str (style, "color:magenta;"); break;
                case 6: mrg_string_append_str (style, "color:cyan;"); break;
                case 7: mrg_string_append_str (style, "color:#777;"); break;
                case 8: mrg_string_append_str (style, "color:#aaa;"); break;
                case 9: mrg_string_append_str (style, "color:#f77;"); break;
                case 10: mrg_string_append_str (style, "color:#7f7;"); break;
                case 11: mrg_string_append_str (style, "color:#ff7;"); break;
                case 12: mrg_string_append_str (style, "color:#87f;"); break;
                case 13: mrg_string_append_str (style, "color:#f7f;"); break;
                case 14: mrg_string_append_str (style, "color:#7ff;"); break;
                case 15: mrg_string_append_str (style, "color:#fff;"); break;
                default: mrg_string_append_str (style, "color:purple;"); break;
              }
            }
            mrg_set_style (mrg, style->str);
            mrg_string_free (style, 1);
          }
          mrg_print (mrg, data2);
        }
        y -= ch;
      }
    }
  }

#define MIN(a,b)  ((a)<(b)?(a):(b))
  /* draw cursor */
  if (vt->cursor_visible)
  {
    cairo_t *cr = mrg_cr (mrg);
    float cursor_x = mrg_vt_get_cursor_x (vt);
    float cursor_y = mrg_vt_get_cursor_y (vt);

    mrg_start (mrg, "cursor", NULL);
    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1.0);
    cairo_set_line_width (cr, 1.0);
    cairo_rectangle (mrg_cr (mrg),
               x + (cursor_x - 1) * cw,
               y0 + (cursor_y - 1) * ch,
               cw, ch);
    cairo_stroke_preserve (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 0.0, 0.3333);
    cairo_fill (cr);
    mrg_end (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_listen (mrg, MRG_KEY_DOWN, event_handler, vt, (((uint8_t*)NULL)+cw));
}

int mrg_vt_is_done (MrgVT *vt)
{
  return vt->done;
}

int mrg_vt_get_result (MrgVT *vt)
{
  /* we could block - at least for a while, here..? */
  return vt->result;
}

void mrg_vt_set_scrollback_lines (MrgVT *vt, int scrollback_lines)
{
  vt->lines_scrollback = scrollback_lines;
}

int  mrg_vt_get_scrollback_lines (MrgVT *vt)
{
  return vt->lines_scrollback;
}
