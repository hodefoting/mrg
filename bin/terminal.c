/* mrg - MicroRaptor Gui
 * Copyright (c) 2014, 2016 Øyvind Kolås <pippin@hodefoting.com>
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

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <sys/stat.h>
#include <errno.h>

/* this terminal implements a subset of ANSI/vt100,
 */

/* TODO: 
 *   font size changes
 *   utf8 handling (make more use of mrgstring api)
 *   bold/reverse video 
 *   color
 *   scrollback
 */

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pty.h>
#include "mrg.h"
#include "mrg-string.h"
#include "mrg-list.h"

typedef struct _Vt100 Vt100;
struct _Vt100 {
  int        pty;
  pid_t      pid;
  MrgList   *lines;
  MrgString *buffer;
  int        cursor_x;
  int        cursor_y;
  double     font_size;
  double     line_spacing;
  float      cell_width;
  float      cell_height;

  int        cursor_visible;
  int        saved_x;
  int        saved_y;
  int        cols;
  int        rows;
  int        line_wrap;
  int        reverse_video;
  int        bold;
  int        cursor_key_application;
  int        scroll_top;
  int        scroll_bottom;
  void      *user_data;
  char      *commandline;
  char       argument_buf[64];
  int        argument_buf_len;
};

static void vt100_feed_byte (Vt100 *vt100, int byte);
static void vt100_set_term_size (Vt100 *vt100, int icols, int irows);
static void vtcmd_reset_device (Vt100 *vt100, const char *sequence);

#define DEFAULT_ROWS      25
#define DEFAULT_COLS      80
#define DEFAULT_FONT_SIZE 11.0
#define DEFAULT_LINE_SPACING 1.3

static void vt100_run_command (Vt100 *vt100, const char *command);
Vt100 *vt100_new (const char *command)
{
  Vt100 *vt100 = calloc (sizeof (Vt100), 1);
  vt100->cursor_x               = 1;
  vt100->cursor_y               = 1;
  vt100->cursor_visible         = 1;
  vt100->lines                  = NULL;
  vt100->buffer                 = NULL;
  vt100->saved_x                = 1;
  vt100->saved_y                = 1;
  vt100->cell_width             = DEFAULT_FONT_SIZE;
  vt100->cell_height            = DEFAULT_FONT_SIZE * DEFAULT_LINE_SPACING;
  vt100->cols                   = DEFAULT_COLS;
  vt100->rows                   = DEFAULT_ROWS;
  vt100->line_wrap              = 0;
  vt100->reverse_video          = 0;
  vt100->bold                   = 0;
  vt100->cursor_key_application = 0;
  vt100->scroll_top             = 1;
  vt100->scroll_bottom          = 40;
  vt100->font_size              = DEFAULT_FONT_SIZE;
  vt100->line_spacing           = DEFAULT_LINE_SPACING;
  vt100->argument_buf_len       = 0;
  vt100->argument_buf[0]        = 0;
  vt100->user_data              = NULL;
  vt100->commandline            = NULL;

  if (command)
  {
    vt100_run_command (vt100, command);
    vt100->commandline = strdup (command);
  }

  vt100_set_term_size (vt100, vt100->cols, vt100->rows-2); // makes top few lines be visible - but bit of a hack
  vtcmd_reset_device (vt100, NULL);
  return vt100;
}

/* To get functional scrollback, tweak trimlines... and fix bugs/workarounds
   relying on the behavior of continous trimming
 */
static int vt100_trimlines (Vt100 *vt100, int max)
{
  MrgList *chop_point = NULL;
  MrgList *l;
  int i;
  for (l = vt100->lines, i = 0; l && i < max-1; l = l->next, i++);

  if (l)
  {
    chop_point = l->next;
    l->next = NULL;
  }

  while (chop_point)
  {
    mrg_string_free (chop_point->data, 1);
    mrg_list_remove (&chop_point, chop_point->data);
  }
  return 0;
}

static void vt100_set_term_size (Vt100 *vt100, int icols, int irows)
{
  struct winsize ws;
  vt100->rows = ws.ws_row = irows;
  vt100->cols = ws.ws_col = icols;
  ws.ws_xpixel = ws.ws_col * 8;
  ws.ws_ypixel = ws.ws_row * 8;
  ioctl(vt100->pty, TIOCSWINSZ, &ws);
}

static void vt100_argument_buf_reset (Vt100 *vt100, const char *start)
{
  if (start)
  {
    strcpy (vt100->argument_buf, start);
    vt100->argument_buf_len = strlen (start);
  }
  else
    vt100->argument_buf[vt100->argument_buf_len=0]=0;
}

static void vt100_argument_buf_add (Vt100 *vt100, int ch)
{
  if (vt100->argument_buf_len < 62)
  {
    vt100->argument_buf[vt100->argument_buf_len] = ch;
    vt100->argument_buf[++vt100->argument_buf_len] = 0;
  }
}

static void _vt100_move_to (Vt100 *vt100, int y, int x)
{
  int i;
  if (x < 1)
    x = 1;
  if (y < 1)
    y = 1;

  vt100->cursor_x = x;
  vt100->cursor_y = y;

  i = vt100->rows - y;
  MrgList *l;
  for (l = vt100->lines; l && i >= 1; l = l->next, i--);

  if (l)
    vt100->buffer = l->data;
  else
    vt100->buffer = vt100->lines?vt100->lines->data:NULL;

  if (vt100->user_data)
    mrg_queue_draw (vt100->user_data, NULL);

  vt100_trimlines (vt100, vt100->rows);
}

static void _vt100_add_byte (Vt100 *vt100, int byte)
{
  if (vt100->cursor_x >= vt100->buffer->length)
    {
      int i;
      for (i = 0; i < vt100->cursor_x - vt100->buffer->length; i++)
        mrg_string_append_byte (vt100->buffer, ' ');
    }
  if (vt100->buffer->length > vt100->cursor_x - 1 && vt100->cursor_x >= 1)
  {
    vt100->buffer->str[vt100->cursor_x - 1] = byte;
  }
  else
  {
    mrg_string_append_byte (vt100->buffer, byte);
  }

  vt100->cursor_x ++; /* this would depend on the validity of utf8.. */

  if (vt100->user_data)
    mrg_queue_draw (vt100->user_data, NULL);
}

static void _vt100_backspace (Vt100 *vt100)
{
  if (vt100->buffer)
  {
    vt100->cursor_x --;
    if (vt100->cursor_x < 1)
      vt100->cursor_x = 1;
  }
  if (vt100->user_data)
    mrg_queue_draw (vt100->user_data, NULL);
}

static void vtcmd_set_scroll_margins (Vt100 *vt100, const char *sequence)
{
  int top, bottom;
  if (strlen (sequence) == 2)
  {
    top = 1; bottom = vt100->rows;
  }
  else
  {
    sscanf (sequence, "[%i;%ir", &top, &bottom);
  }
  vt100->scroll_top = top;
  vt100->scroll_bottom = bottom;
};

typedef struct Sequence {
  const char *prefix;
  const char *suffix;
  void (*vtcmd) (Vt100 *vt100, const char *sequence);
} Sequence;

static void vtcmd_set_cursor_key_to_application (Vt100 *vt100, const char *sequence)
{
  vt100->cursor_key_application = 1;
};

static void vtcmd_set_cursor_key_to_cursor (Vt100 *vt100, const char *sequence)
{
  vt100->cursor_key_application = 0;
};

static void vtcmd_reset_device (Vt100 *vt100, const char *sequence)
{
  if (vt100->buffer)
    mrg_string_free (vt100->buffer, TRUE);
  vt100->lines = NULL;

  vtcmd_set_scroll_margins (vt100, "[r");
  _vt100_move_to (vt100, 1, 1);
  vt100->cursor_key_application = 0;
  vt100->line_wrap = 0;
  vt100->bold = 0;
  vt100->reverse_video = 0;
  vt100->cursor_visible = 1;
  vt100->buffer = mrg_string_new ("");
  mrg_list_prepend (&vt100->lines, vt100->buffer);
  for (int i=0; i<vt100->rows * 10;i++)
    vt100_feed_byte (vt100, '\n');
}

static void vtcmd_move_cursor (Vt100 *vt100, const char *sequence)
{
  int y = 0, x = 0;
  sscanf (sequence, "[%i;%i", &y, &x);
  _vt100_move_to (vt100, y, x);
};

static int parse_int (const char *arg, int def_val)
{
  if (strlen (arg) == 2)
    return def_val;
  return atoi (arg+1);
}

static void vtcmd_goto_column (Vt100 *vt100, const char *sequence)
{
  int x = parse_int (sequence, 1);
  _vt100_move_to (vt100, vt100->cursor_y, x);
}

static void vtcmd_goto_row (Vt100 *vt100, const char *sequence)
{
  int y = parse_int (sequence, 1);
  _vt100_move_to (vt100, y, vt100->cursor_x);
}

static void vtcmd_move_cursor_right (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  _vt100_move_to (vt100, vt100->cursor_y, vt100->cursor_x + n);
}

static void vtcmd_move_cursor_left (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  _vt100_move_to (vt100, vt100->cursor_y, vt100->cursor_x - n);
}

static void vtcmd_move_cursor_up (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  _vt100_move_to (vt100, vt100->cursor_y - n, vt100->cursor_x);
}

static void vtcmd_move_cursor_down (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  _vt100_move_to (vt100, vt100->cursor_y + n, vt100->cursor_x);
}

static void vtcmd_move_cursor_down_and_first_col (Vt100 *vt100, const char *sequence)
{
  vtcmd_move_cursor_down (vt100, sequence);
  _vt100_move_to (vt100, vt100->cursor_y, 1);
}

static void vtcmd_move_cursor_up_and_first_col (Vt100 *vt100, const char *sequence)
{
  vtcmd_move_cursor_up (vt100, sequence);
  _vt100_move_to (vt100, vt100->cursor_y, 1);
}

static void vtcmd_clear_line (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of line
      vt100->buffer->str[vt100->cursor_x-1] = 0;
      vt100->buffer->length = strlen (vt100->buffer->str);
      break;
    case 1: // clear from beginning to cursor
      {
        int i;
        for (i = 0; i < vt100->cursor_x-1; i++)
        {
          if (i < vt100->buffer->length)
            vt100->buffer->str[i] = ' ';
        }
      }
      break;
    case 2: // clear entire line
      mrg_string_set (vt100->buffer, "");
      if (vt100->user_data)
        mrg_queue_draw (vt100->user_data, NULL);
      break;
  }
}

static void vtcmd_clear_lines (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of screen
      vt100->buffer->str[vt100->cursor_x-1] = 0;
      vt100->buffer->length = strlen (vt100->buffer->str);

      {
        MrgList *l;
        for (l = vt100->lines; l->data != vt100->buffer; l = l->next)
        {
          MrgString *buf = l->data;
          buf->str[0] = 0;
          buf->length = 0;
        }
      }
      break;
    case 1: // clear from beginning to cursor
      {
        int i;
        for (i = 0; i < vt100->cursor_x-1; i++)
        {
          if (i < vt100->buffer->length)
            vt100->buffer->str[i] = ' ';
        }
      }
      {
        MrgList *l;
        int there_yet = 0;
        int no = 0;

        for (l = vt100->lines; l && no < vt100->rows; l = l->next, no ++)
        {
          MrgString *buf = l->data;
          if (there_yet)
          {
            buf->str[0] = 0;
            buf->length = 0;
          }
          if (buf == vt100->buffer)
          {
            there_yet = 1;
          }
        }
      }
      break;
    case 2: // clear entire screen but keep cursor;
      {
        int tx = vt100->cursor_x;
        int ty = vt100->cursor_y;
        vtcmd_reset_device (vt100, "");
        _vt100_move_to (vt100, tx, ty);  // XXX: some implementation moves cursor to home?!
      }
      break;
  }
}

static void vtcmd_set_style (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 0); // works until color
  switch (n)
  {
    case 0:  // clear all style 
      vt100->reverse_video = 0;
      vt100->bold = 0;
      break;
    case 7:  // Inverse video ON
      vt100->reverse_video = 1;
      break;
    case 27: // Inverse Video OFF
      vt100->reverse_video = 0;
      break;
    case 1:  // Alternate Intensity ON
      vt100->bold = 1;
      break;
    case 22: // Alternate Intensity OFF
      vt100->bold = 0;
      break;
    case 4:  // Underline ON
    case 24: // Underline OFF
    case 5:  // Blink ON
    case 25: // Blink OFF
      break;
  }
}

static void vtcmd_reverse_scroll (Vt100 *vt100, const char *sequence)
{
  fprintf (stderr, "%s NYI %s\n", __FUNCTION__, sequence);
}

static void vtcmd_forward_scroll (Vt100 *vt100, const char *sequence)
{
  fprintf (stderr, "%s NYI %s\n", __FUNCTION__, sequence);
}

static void vtcmd_ignore (Vt100 *vt100, const char *sequence)
{
}

static void vtcmd_clear_all_tabs (Vt100 *vt100, const char *sequence)
{
}

static void vtcmd_set_tab_at_current_column (Vt100 *vt100, const char *sequence)
{
}

static void vtcmd_cursor_position_report (Vt100 *vt100, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[%i;%iR", vt100->cursor_y, vt100->cursor_x);
  write (vt100->pty, buf, strlen(buf));
}

static void vtcmd_status_report (Vt100 *vt100, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[0n"); // we're always OK :)
  write (vt100->pty, buf, strlen(buf));
}

static void vtcmd_identify (Vt100 *vt100, const char *sequence)
{
  char *buf = "\033[?6c";          // identify as vt102
  write (vt100->pty, buf, strlen(buf));
}

static void vtcmd_clear_current_tab (Vt100 *vt100, const char *sequence)
{
}

static void vtcmd_enable_line_wrap (Vt100 *vt100, const char *sequence)
{
  vt100->line_wrap = 1;
}

static void vtcmd_disable_line_wrap (Vt100 *vt100, const char *sequence)
{
  vt100->line_wrap = 0;
}

static void vtcmd_show_cursor (Vt100 *vt100, const char *sequence)
{
  vt100->cursor_visible = 1;
}

static void vtcmd_hide_cursor (Vt100 *vt100, const char *sequence)
{
  vt100->cursor_visible = 0;
}

static void vtcmd_save_cursor_position (Vt100 *vt100, const char *sequence)
{
  vt100->saved_x = vt100->cursor_x;
  vt100->saved_y = vt100->cursor_y;
}

static void vtcmd_restore_cursor_position (Vt100 *vt100, const char *sequence)
{
  _vt100_move_to (vt100, vt100->saved_x, vt100->saved_y);
}

static void vtcmd_insert_n_tabs (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  
  while (n--)
  {
    do {
      _vt100_add_byte (vt100, ' ');
    } while ( ((vt100->buffer->length) % 8));
  }
}

static void vtcmd_insert_blanks (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
    _vt100_add_byte (vt100, ' ');
}

static void vtcmd_insert_blank_lines (Vt100 *vt100, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
    if (vt100->lines->data == vt100->buffer)
    {
      mrg_list_prepend (&vt100->lines, mrg_string_new (""));
      vt100->buffer = vt100->lines->data;
    }
    vt100->cursor_y ++;
  }
  _vt100_move_to (vt100, vt100->cursor_y, 1); // XXX: the 1 should depend on a mode
}

Sequence sequences[]={
/*
  The suffix is always 0 or 1 chars, and could be replaced with a char instead of a string - as an optimization.

  Legend:
  prefix   suffix command */
  {"[6n",  "",    vtcmd_cursor_position_report},
  {"[5n",  "",    vtcmd_status_report},
  {"[0g",  "",    vtcmd_clear_current_tab},
  {"[3g",  "",    vtcmd_clear_all_tabs},
  {"H" ,   "",    vtcmd_set_tab_at_current_column},
  {"Z" ,   "",    vtcmd_identify},
  {"[",    "f",   vtcmd_move_cursor},
  {"[",    "r",   vtcmd_set_scroll_margins},
  {"[",    "A",   vtcmd_move_cursor_up},
  {"[",    "B",   vtcmd_move_cursor_down},
  {"[",    "e",   vtcmd_move_cursor_down},
  {"[",    "C",   vtcmd_move_cursor_right},
  {"[",    "a",   vtcmd_move_cursor_right},
  {"[",    "D",   vtcmd_move_cursor_left},
  {"[",    "E",   vtcmd_move_cursor_down_and_first_col},
  {"[",    "F",   vtcmd_move_cursor_up_and_first_col},
  {"[",    "G",   vtcmd_goto_column},
  {"[",    "`",   vtcmd_goto_column},
  {"[",    "H",   vtcmd_move_cursor},
  {"[",    "I",   vtcmd_insert_n_tabs},
  {"[",    "J",   vtcmd_clear_lines},
  {"[",    "K",   vtcmd_clear_line},
  {"[",    "L",   vtcmd_insert_blank_lines},
  {"[",    "@",   vtcmd_insert_blanks},
  {"[",    "d",   vtcmd_goto_row},
  {"[s",   "",    vtcmd_save_cursor_position},
  {"7",    "",    vtcmd_save_cursor_position}, // ( + attr )
  {"[r",   "",    vtcmd_restore_cursor_position},
  {"8",    "",    vtcmd_restore_cursor_position}, // ( + attr )

  /*  [ S - scroll N lines up
   *  [ T - scroll N lines down
   *  [ M - delete N blank lines
   *  [ X - erase  n chars
   *  [ P - delete n chars
   *  [ Z - cursor backward tabulation n tab stops
   *  [ d - move to row
   *
   *  M reverse index (scroll if reaching margin)
   *  E next line (and scroll?)
   *  S scroll up P lines (xterm)
   */

  {"[",     "m", vtcmd_set_style},
  {"M",     "",  vtcmd_reverse_scroll},
  {"D",     "",  vtcmd_forward_scroll},
  {"=",     "",  vtcmd_ignore}, // keypad mode change
  {">",     "",  vtcmd_ignore}, // keypad mode change
  {"c",     "",  vtcmd_reset_device},
  {"!p",    "",  vtcmd_reset_device},
  {"[?1h",  "",  vtcmd_set_cursor_key_to_application},
  {"[?1l",  "",  vtcmd_set_cursor_key_to_cursor},
  {"[?25h", "",  vtcmd_show_cursor},
  {"[?25l", "",  vtcmd_hide_cursor},
  {"[7h",   "",  vtcmd_enable_line_wrap},
  {"[7l",   "",  vtcmd_disable_line_wrap},
  {NULL, NULL, NULL}
};

static void handle_sequence (Vt100 *vt100, const char *sequence)
{
  int i;
  for (i = 0; sequences[i].prefix; i++)
  {
    if (!strncmp (sequence, sequences[i].prefix, strlen(sequences[i].prefix)))
    {
      int i0, i1;
      int mismatch = 0;
      i0 = strlen (sequence)-1;
      i1 = strlen (sequences[i].suffix)-1;
      while (i1 >= 0 && !mismatch)
      {
        if (sequence[i0] != sequences[i].suffix[i1])
          mismatch = 1;
        i0--;
        i1--;
      }
      if (i1>-1)
        mismatch = 1;
      if (!mismatch)
      {
        sequences[i].vtcmd (vt100, sequence);
        return;
      }
    }
  }
  fprintf (stderr, "ESC%s\n", sequence);
}

typedef enum {
  TERMINAL_STATE_NEUTRAL          = 0,
  TERMINAL_STATE_GOT_ESC          = 1,
  TERMINAL_STATE_GOT_ESC_SQLPAREN = 2,
  TERMINAL_STATE_GOT_ESC_SQRPAREN = 3,
} TerminalState;

static void vt100_bell (Vt100 *vt100)
{
}

static void vt100_feed_byte (Vt100 *vt100, int byte)
{
  static TerminalState state = TERMINAL_STATE_NEUTRAL;
  switch (state)
  {
    case TERMINAL_STATE_NEUTRAL:
      switch (byte)
      {
        case '\0': /* NUL */
        case 1:    /* SOH start of heading */
        case 2:    /* STX start of text */
        case 3:    /* ETX end of text */
        case 4:    /* EOT end of transmission */
        case 5:    /* ENQuiry */
        case 6:    /* ACKnolwedge */
          break;
        case '\a': /* BELl */ vt100_bell (vt100); break;
        case '\b': /* BS */   _vt100_backspace (vt100); break;
        case '\t': /* HT tab */
          do {
            _vt100_add_byte (vt100, ' ');
          } while ( ((vt100->buffer->length) % 8));
          break;
        case '\n': /* LF line ffed */
        case '\v': /* VT vertical tab */
        case '\f': /* VF form feed */
          if (vt100->lines->data == vt100->buffer)
          {
            mrg_list_prepend (&vt100->lines, mrg_string_new (""));
            vt100->buffer = vt100->lines->data;
          }
          _vt100_move_to (vt100, vt100->cursor_y + 1, 1); // XXX the col should depend on mode
          break;
        case '\r': /* CR carriage return */
          _vt100_move_to (vt100, vt100->cursor_y, 1); 
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
          state= TERMINAL_STATE_GOT_ESC;
          break;
        case 28: /* FS file separator */
        case 29: /* GS group separator */
        case 30: /* RS record separator */
        case 31: /* US unit separator */
          break;
        default:
          _vt100_add_byte (vt100, byte);
          break;
      }
      break;
    case TERMINAL_STATE_GOT_ESC:
      if (byte == '[')
      {
        vt100_argument_buf_reset(vt100, "[");
        state = TERMINAL_STATE_GOT_ESC_SQLPAREN;
      }
      else if (byte == ']')
      {
        vt100_argument_buf_reset(vt100, "]");
        state = TERMINAL_STATE_GOT_ESC_SQRPAREN;
      }
      else
      {
        char tmp[3]=" ";
        tmp[0]=byte;
        handle_sequence (vt100, tmp);
        state= TERMINAL_STATE_NEUTRAL;
      }
      break;
    case TERMINAL_STATE_GOT_ESC_SQLPAREN:
      if (byte >= '@' && byte <= '~')
      {
        vt100_argument_buf_add (vt100, byte);
        handle_sequence (vt100, vt100->argument_buf);
        state=0;
      }
      else
      {
        vt100_argument_buf_add (vt100, byte);
      }
      break;
    case TERMINAL_STATE_GOT_ESC_SQRPAREN:
      if (byte == '\a')
      {
        if (vt100->user_data)
        {
          mrg_set_title (vt100->user_data, vt100->argument_buf + 3);
        }
        // XXX: use handle sequence here as well,.. for consistency
        state= TERMINAL_STATE_NEUTRAL;
      }
      else
      {
        vt100_argument_buf_add (vt100, byte);
      }
      break;
  }
}

static void vt100_poll (Vt100 *vt100)
{
  char buf[256];
  int len = read(vt100->pty, buf, sizeof (buf));
  if (len > 0)
  {
    int i;
    for (i = 0; i < len; i++)
      vt100_feed_byte (vt100, buf[i]);
    vt100_poll (vt100);
  }
  else
  {
    usleep (16000); /* XXX: due to recursion this always happens, giving
                            a (given the buf size, large) rate limit on the
                            terminal */
  }
  if (vt100->cursor_y > vt100->rows)
    vt100->cursor_y = vt100->rows;
}

/******/

static int mrg_pty_poll (Mrg *mrg, void *data)
{
  vt100_poll (data);
  return 1;
}

static void vt100_feed_keystring (Vt100 *vt100, const char *str)
{
  if (vt100->cursor_key_application)
  {
    if (!strcmp (str, "up"))         { str = "\033OA"; goto done; }
    else if (!strcmp (str, "down"))  { str = "\033OB"; goto done; }
    else if (!strcmp (str, "right")) { str = "\033OC"; goto done; }
    else if (!strcmp (str, "left"))  { str = "\033OD"; goto done; }
  }

       if (!strcmp (str, "up"))              str = "\033[A";
  else if (!strcmp (str, "down"))            str = "\033[B";
  else if (!strcmp (str, "right"))           str = "\033[C";
  else if (!strcmp (str, "left"))            str = "\033[D";
  else if (!strcmp (str, "shift-up"))        str = "\033[1;2A";
  else if (!strcmp (str, "shift-down"))      str = "\033[1;2B";
  else if (!strcmp (str, "shift-right"))     str = "\033[1;2C";
  else if (!strcmp (str, "shift-left"))      str = "\033[1;2D";
  else if (!strcmp (str, "alt-up"))          str = "\033[1;3A";
  else if (!strcmp (str, "alt-down"))        str = "\033[1;3B";
  else if (!strcmp (str, "alt-right"))       str = "\033[1;3C";
  else if (!strcmp (str, "alt-left"))        str = "\033[1;3D";
  else if (!strcmp (str, "alt-shift-up"))    str = "\033[1;4A";
  else if (!strcmp (str, "alt-shift-down"))  str = "\033[1;4B";
  else if (!strcmp (str, "alt-shift-right")) str = "\033[1;4C";
  else if (!strcmp (str, "alt-shift-left"))  str = "\033[1;4D";
  else if (!strcmp (str, "control-up"))      str = "\033[1;5A";
  else if (!strcmp (str, "control-down"))    str = "\033[1;5B";
  else if (!strcmp (str, "control-right"))   str = "\033[1;5C";
  else if (!strcmp (str, "control-left"))    str = "\033[1;5D";
  else if (!strcmp (str, "insert"))          str = "\033[2~";
  else if (!strcmp (str, "delete"))          str = "\033[3~";
  else if (!strcmp (str, "control-delete"))  str = "\033[3;5~";
  else if (!strcmp (str, "shift-delete"))    str = "\033[3;2~";
  else if (!strcmp (str, "control-shift-delete")) str = "\033[3;6~";
  else if (!strcmp (str, "page-up"))              str = "\033[5~";
  else if (!strcmp (str, "page-down"))            str = "\033[6~";
  else if (!strcmp (str, "return"))               str = "\r";
  else if (!strcmp (str, "space"))                str = " ";
  else if (!strcmp (str, "control-a")) str = "\001";
  else if (!strcmp (str, "control-b")) str = "\002";
  else if (!strcmp (str, "control-c")) str = "\003";
  else if (!strcmp (str, "control-d")) str = "\004";
  else if (!strcmp (str, "control-e")) str = "\005";
  else if (!strcmp (str, "control-f")) str = "\006";
  else if (!strcmp (str, "control-g")) str = "\007";
  else if (!strcmp (str, "control-h")) str = "\010";
  else if (!strcmp (str, "control-i")) str = "\011";
  else if (!strcmp (str, "control-j")) str = "\012";
  else if (!strcmp (str, "control-k")) str = "\013";
  else if (!strcmp (str, "control-l")) str = "\014";
  else if (!strcmp (str, "control-m")) str = "\015";
  else if (!strcmp (str, "control-n")) str = "\016";
  else if (!strcmp (str, "control-o")) str = "\017";
  else if (!strcmp (str, "control-p")) str = "\020";
  else if (!strcmp (str, "control-q")) str = "\021";
  else if (!strcmp (str, "control-r")) str = "\022";
  else if (!strcmp (str, "control-s")) str = "\023";
  else if (!strcmp (str, "control-t")) str = "\024";
  else if (!strcmp (str, "control-u")) str = "\025";
  else if (!strcmp (str, "control-v")) str = "\026";
  else if (!strcmp (str, "control-w")) str = "\027";
  else if (!strcmp (str, "control-x")) str = "\030";
  else if (!strcmp (str, "control-y")) str = "\031";
  else if (!strcmp (str, "control-z")) str = "\032";
  else if (!strcmp (str, "escape"))    str = "\033";
  else if (!strcmp (str, "tab"))       str = "\t";
  else if (!strcmp (str, "backspace")) str = "\b";
  else if (!strcmp (str, "shift-tab")) str = "\033[Z";
  else if (!strcmp (str, "home"))      str = "\033[1~";
  else if (!strcmp (str, "end"))       str = "\033[4~";
  else if (!strcmp (str, "F1"))        str = "\033[11~";
  else if (!strcmp (str, "F2"))        str = "\033[12~";
  else if (!strcmp (str, "F3"))        str = "\033[13~";
  else if (!strcmp (str, "F4"))        str = "\033[14~";
  else if (!strcmp (str, "F5"))        str = "\033[15~";
  else if (!strcmp (str, "F6"))        str = "\033[16~";
  else if (!strcmp (str, "F7"))        str = "\033[17~";
  else if (!strcmp (str, "F8"))        str = "\033[18~";
  else if (!strcmp (str, "F9"))        str = "\033[19~";
  else if (!strcmp (str, "F10"))       str = "\033[21~";
  else if (!strcmp (str, "F11"))       str = "\033[22~";
  else if (!strcmp (str, "F12"))       str = "\033[23~";

done:
  if (strlen (str))
    write (vt100->pty, str, strlen (str));
}

static const char *vt100_find_shell_command (Vt100 *vt100)
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

static void vt100_run_command (Vt100 *vt100, const char *command)
{
  vt100->pid = forkpty (&vt100->pty, NULL, NULL, NULL);
  if (vt100->pid == 0)
  {
    unsetenv ("TERM");
    unsetenv ("COLUMNS");
    unsetenv ("LINES");
    unsetenv ("TERMCAP");
    unsetenv ("COLOR_TERM");
    setenv ("TERM", "vt100", 1);
    system (command);
  }
  else if (vt100->pid < 0)
  {
    fprintf (stderr, "forkpty failed (%s)\n", command);
  }
  fcntl(vt100->pty, F_SETFL, O_NONBLOCK);
}

const char *vt100_get_commandline (Vt100 *vt100)
{
  return vt100->commandline;
}

/***********************************************************************/

static void event_handler (MrgEvent *event, void *data1, void *data2)
{
  Vt100 *vt100 = data1;
  const char *str = event->key_name;

  if (!str)
  {
    mrg_event_stop_propagate (event);
    return;
  }

  if (!strcmp (str, "resize-event")) {
    Mrg *mrg = event->mrg;
    str = "";

    vt100_set_term_size (vt100, (int)(mrg_width (mrg) / vt100->cell_width)-2, (int)(mrg_height (mrg) / vt100->cell_height)-1);
    vt100_trimlines (vt100, vt100->rows);
  }
  else vt100_feed_keystring (vt100, str);

  mrg_event_stop_propagate (event);
}

static Mrg *mrg_tmp;

static void render_vt100 (Mrg *mrg, void *vt100_data)
{
  Vt100 *vt100 = vt100_data;
  mrg_tmp = mrg;
  mrg_set_edge_left (mrg, mrg_em (mrg));
  mrg_set_edge_right (mrg, mrg_em (mrg));
  mrg_set_edge_top (mrg, mrg_em (mrg));

  mrg_start (mrg, "terminal", NULL);
#if MRG_CAIRO
  mrg_cairo_set_source_color (mrg_cr (mrg), &mrg_style(mrg)->background_color);
  cairo_paint (mrg_cr (mrg));
#endif

  mrg_print (mrg, " "); // XXX: hack; something in the CSS? causes first printed bit to be different

  /* draw terminal lines */
  {
    MrgList *l;
    float y = mrg_height (mrg) - mrg_em (mrg);  /* XXX: strip down to grid with no scrollback */
    float x = mrg_em (mrg); /* ... and remove this logic? */
    int no = 1;

    for (l = vt100->lines; l && y > -64.0; l = l->next, no++)
    {
      mrg_set_xy (mrg, x, y);
      mrg_print (mrg, ((MrgString*)(l->data))->str);
      y -= mrg_em (mrg) * vt100->line_spacing;
    }
  }

  /* draw cursor */
  {
    cairo_t *cr = mrg_cr (mrg);
    float cw = mrg_x (mrg);
    float cy;
    float em = mrg_em (mrg);
    mrg_print (mrg, " ");
    cw = mrg_x (mrg) - cw;

    vt100->cell_width = cw;
    vt100->cell_height = em * vt100->line_spacing;

    cy = mrg_height (mrg) - em;
    cy -= (vt100->rows - vt100->cursor_y + 1) * vt100->cell_height;
    mrg_end (mrg);

    mrg_start (mrg, "cursor", NULL);
    cairo_rectangle (mrg_cr (mrg),
              (vt100->cursor_x-1) * cw + em,
              cy,
              vt100->cell_width,
              vt100->cell_height);

    cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 0.15);
    cairo_fill (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_translate (cr, 0.5, 0.5);
    cairo_rectangle (mrg_cr (mrg),
              (vt100->cursor_x-1) * cw + em,
              cy,
              cw,
              vt100->line_spacing * em);
    cairo_stroke (cr);
    mrg_end (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_listen (mrg, MRG_KEY_DOWN, event_handler, vt100, NULL);
}

void
signal_child (int a) {
  if (mrg_tmp)
    mrg_quit (mrg_tmp);
  exit (0); /* XXX: make it less brutal? */
}


int terminal_main (int argc, char **argv)
{
  Mrg *mrg;
  mrg = mrg_new ((DEFAULT_COLS+1) * DEFAULT_FONT_SIZE * 0.65,
                 (DEFAULT_ROWS+1) * DEFAULT_FONT_SIZE * DEFAULT_LINE_SPACING, NULL);

  signal (SIGCHLD, signal_child);
  Vt100 *vt100 = vt100_new (argv[1]?argv[1]:vt100_find_shell_command(NULL));
  vt100->user_data = mrg;
  mrg_set_ui (mrg, render_vt100, vt100);
  mrg_add_idle (mrg, mrg_pty_poll, vt100);
  mrg_set_title (mrg, vt100_get_commandline (vt100));
  mrg_main (mrg);
  mrg_destroy (mrg);
  return 0;
}
