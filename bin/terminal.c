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

/* this terminal implements a subset of ANSI/vt100,
 */

/* TODO: 
 *   font size changes
 *   refactor to have a nicer API and encapsulated core..
 *   utf8 handling (make more use of mrgstring api)
 *   bold/reverse video 
 *   color
 *   scrollback
 */
#if 0

typedef struct _Cell Cell;

struct _Cell {
  char c[5]; /* up to 4 utf8 and terminator */
  int  bold:1;
  int  reverse:1;
  int  underline:1;
};

/* the whole core engine should be wrapped like this: */

 vt100_new        (vt100, w, h);
 vt100_set_size   (vt100, w, h);
 vt100_feed_char  (vt100, c);
 vt100_key_event  (vt100, str);
 vt100_cell       (vt100, char *utf8, int *bold, int *reverse, int *underline, int *fg, int *bg);
 vt100_clear_dirt (vt100);
 vt100_get_dirty  (vt100, int *lines);
 vt100_get_text   (vt100, int col, int row, int len, char *res);

#endif

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include "mrg.h"
#include "mrg-string.h"
#include "mrg-list.h"

static int        cursor_x       = 1;
static int        cursor_y       = 1;
static pid_t      pid;
static int        pty;
static MrgList   *lines          = NULL;
static MrgString *buffer         = NULL; /* < buffer of current line */
static int        cols           = 80;
static int        rows           = 25;
static int        scroll_top     = 1;
static int        scroll_bottom  = 40;
static double     fontsize       = 11.0;
static int        cursor_key_application = 0;
static int        line_wrap      = 0;
static int        reverse_video  = 0;
static int        bold           = 0;
static int        cursor_visible = 1;
static float      cell_width     = 11;
static float      cell_height    = 11;
static int        saved_x        = 1;
static int        saved_y        = 1;


#include <sys/stat.h>
#include <errno.h>

static int trimlines (int max)
{
  MrgList *chop_point = NULL;
  MrgList *l;
  int i;
  for (l = lines, i = 0; l && i < max-1; l = l->next, i++);

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

static void set_term_size (Mrg *mrg, int icols, int irows)
{
  struct winsize ws;
  //int width = icols * fontsize * 0.6;
  //int height = irows * fontsize * 1.3;

  rows = ws.ws_row = irows;
  cols = ws.ws_col = icols;
  ws.ws_xpixel = ws.ws_col * 8;
  ws.ws_ypixel = ws.ws_row * 8;
  ioctl(pty, TIOCSWINSZ, &ws);
}

static char argument_buf[64] = "";
static int  argument_buf_len = 0;

static void argument_buf_reset (const char *start)
{
  if (start)
  {
    strcpy (argument_buf, start);
    argument_buf_len = strlen (start);
  }
  else
    argument_buf[argument_buf_len=0]=0;
}

static void argument_buf_add (int ch)
{
  if (argument_buf_len < 62)
  {
    argument_buf[argument_buf_len] = ch;
    argument_buf[++argument_buf_len] = 0;
  }
}

static void move_to (Mrg *mrg, int y, int x)
{
  int i;
  if (x < 1)
    x = 1;
  if (y < 1)
    y = 1;

  cursor_x = x;
  cursor_y = y;

  i = rows - y;
  MrgList *l;
  for (l = lines; l && i >= 1; l = l->next, i--);

  if (l)
    buffer = l->data;
  else
    buffer = lines->data;

  mrg_queue_draw (mrg, NULL);

  trimlines (rows);
}

static void add_byte (Mrg *mrg, int byte)
{
  if (cursor_x >= buffer->length)
    {
      int i;
      for (i = 0; i < cursor_x - buffer->length; i++)
        mrg_string_append_byte (buffer, ' ');
    }
  if (buffer->length > cursor_x - 1 && cursor_x >= 1)
  {
    buffer->str[cursor_x - 1] = byte;
  }
  else
  {
    mrg_string_append_byte (buffer, byte);
  }
  cursor_x ++;
  mrg_queue_draw (mrg, NULL);
}

static void backspace (Mrg *mrg)
{
  if (buffer)
  {
    cursor_x --;
    if (cursor_x < 1)
      cursor_x = 1;
  }
  mrg_queue_draw (mrg, NULL);
}

static void set_scroll_margins (Mrg *mrg, const char *sequence)
{
  int top, bottom;
  if (strlen (sequence) == 2)
  {
    top = 1; bottom = rows;
  }
  else
  {
    sscanf (sequence, "[%i;%ir", &top, &bottom);
  }
  //fprintf (stderr, "should set scroll margins top:%i bottom:%i\n", top, bottom);
  scroll_top = top;
  scroll_bottom = bottom;
};

typedef struct Sequence {
  const char *prefix;
  const char *suffix;
  void (*handler) (Mrg *mrg, const char *sequence);
} Sequence;

static void set_cursor_key_to_application (Mrg *mrg, const char *sequence)
{
  cursor_key_application = 1;
};

static void set_cursor_key_to_cursor (Mrg *mrg, const char *sequence)
{
  cursor_key_application = 0;
};

static void reset_device (Mrg *mrg)
{
  int i;
  for (i = 0; i < rows + 4; i++)
  {
    buffer = mrg_string_new ("");
    mrg_list_prepend (&lines, buffer);
  }
  set_scroll_margins (mrg, "[r");
  move_to (mrg, 1, 1);
  cursor_key_application = 0;
  line_wrap = 0;
  bold = 0;
  reverse_video = 0;
  cursor_visible = 1;
}

static void move_cursor (Mrg *mrg, const char *sequence)
{
  int y = 0, x = 0;
  sscanf (sequence, "[%i;%i", &y, &x);
  move_to (mrg, y, x);
};

static int parse_int (const char *arg, int def_val)
{
  if (strlen (arg) == 2)
    return def_val;
  return atoi (arg+1);
}

static void goto_column (Mrg *mrg, const char *sequence)
{
  int x = parse_int (sequence, 1);
  move_to (mrg, cursor_y, x);
}

static void goto_row (Mrg *mrg, const char *sequence)
{
  int y = parse_int (sequence, 1);
  move_to (mrg, y, cursor_x);
}

static void move_cursor_right (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  move_to (mrg, cursor_y, cursor_x + n);
}

static void move_cursor_left (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  move_to (mrg, cursor_y, cursor_x - n);
}

static void move_cursor_up (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  move_to (mrg, cursor_y - n, cursor_x);
}

static void move_cursor_down (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  move_to (mrg, cursor_y + n, cursor_x);
}

static void move_cursor_down_and_first_col (Mrg *mrg, const char *sequence)
{
  move_cursor_down (mrg, sequence);
  move_to (mrg, cursor_y, 1);
}

static void move_cursor_up_and_first_col (Mrg *mrg, const char *sequence)
{
  move_cursor_up (mrg, sequence);
  move_to (mrg, cursor_y, 1);
}

static void clear_line (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of line
      buffer->str[cursor_x-1] = 0;
      buffer->length = strlen (buffer->str);
      break;
    case 1: // clear from beginning to cursor
      {
        int i;
        for (i = 0; i < cursor_x-1; i++)
        {
          if (i < buffer->length)
            buffer->str[i] = ' ';
        }
      }
      break;
    case 2: // clear entire line
      mrg_string_set (buffer, "");
      mrg_queue_draw (mrg, NULL);
      break;
  }
}

static void clear_lines (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 0);

  switch (n)
  {
    case 0: // clear to end of screen
      buffer->str[cursor_x-1] = 0;
      buffer->length = strlen (buffer->str);

      {
        MrgList *l;
        for (l = lines; l->data != buffer; l = l->next)
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
        for (i = 0; i < cursor_x-1; i++)
        {
          if (i < buffer->length)
            buffer->str[i] = ' ';
        }
      }
      {
        MrgList *l;
        int there_yet = 0;
        int no = 0;

        for (l = lines; l && no < rows; l = l->next, no ++)
        {
          MrgString *buf = l->data;
          if (there_yet)
          {
            buf->str[0] = 0;
            buf->length = 0;
          }
          if (buf == buffer)
          {
            there_yet = 1;
          }
        }
      }
      break;
    case 2: // clear entire screen but keep cursor;
      {
        int tx = cursor_x;
        int ty = cursor_y;
        reset_device (mrg);
        move_to (mrg, tx, ty);  // XXX: some implementation moves cursor to home?!
      }
      break;
  }
}

static void set_style (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 0); // works until color
  switch (n)
  {
    case 0:  // clear all style 
      reverse_video = 0;
      bold = 0;
      break;
    case 7:  // Inverse video ON
      reverse_video = 1;
      break;
    case 27: // Inverse Video OFF
      reverse_video = 0;
      break;
    case 1:  // Alternate Intensity ON
      bold = 1;
      break;
    case 22: // Alternate Intensity OFF
      bold = 0;
      break;
    case 4:  // Underline ON
    case 24: // Underline OFF
    case 5:  // Blink ON
    case 25: // Blink OFF
      break;
  }
}

static void reverse_scroll (Mrg *mrg, const char *sequence)
{
  fprintf (stderr, "%s NYI\n", __FUNCTION__);
}

static void forward_scroll (Mrg *mrg, const char *sequence)
{
  fprintf (stderr, "%s NYI\n", __FUNCTION__);
}

static void ignore (Mrg *mrg, const char *sequence)
{
}

static void clear_all_tabs (Mrg *mrg, const char *sequence)
{
}

static void set_tab_at_current_column (Mrg *mrg, const char *sequence)
{
}

static void cursor_position_report (Mrg *mrg, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[%i;%iR", cursor_y, cursor_x);
  write (pty, buf, strlen(buf));
}

static void status_report (Mrg *mrg, const char *sequence)
{
  char buf[64];
  sprintf (buf, "\033[0n"); // we're always OK :)
  write (pty, buf, strlen(buf));
}

static void identify (Mrg *mrg, const char *sequence)
{
  char *buf = "\033[?6c";          // identify as vt102
  write (pty, buf, strlen(buf));
}

static void clear_current_tab (Mrg *mrg, const char *sequence)
{
}

static void enable_line_wrap (Mrg *mrg, const char *sequence)
{
  line_wrap = 1;
}

static void disable_line_wrap (Mrg *mrg, const char *sequence)
{
  line_wrap = 0;
}


static void show_cursor (Mrg *mrg, const char *sequence)
{
  cursor_visible = 1;
}

static void hide_cursor (Mrg *mrg, const char *sequence)
{
  cursor_visible = 0;
}

static void save_cursor_position (Mrg *mrg, const char *sequence)
{
  saved_x = cursor_x;
  saved_y = cursor_y;
}

static void restore_cursor_position (Mrg *mrg, const char *sequence)
{
  move_to (mrg, saved_x, saved_y);
}

static void insert_n_tabs (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  
  while (n--)
  {
    do {
      add_byte (mrg, ' ');
    } while ( ((buffer->length) % 8));
  }
}

static void insert_blanks (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
    add_byte (mrg, ' ');
}

static void insert_blank_lines (Mrg *mrg, const char *sequence)
{
  int n = parse_int (sequence, 1);
  while (n--)
  {
    if (lines->data == buffer)
    {
      mrg_list_prepend (&lines, mrg_string_new (""));
      buffer = lines->data;
    }
    cursor_y ++;
  }
  move_to (mrg, cursor_y, 1); // XXX: the 1 should depend on a mode
}

/* XXX: the sequnece matching is overkill,.. since the suffix is always 0 or 1
 * chars..
 */

Sequence sequences[]={
  {"[6n",  "",   cursor_position_report},
  {"[5n",  "",   status_report},
  {"[0g",  "",   clear_current_tab},
  {"[3g",  "",   clear_all_tabs},
  {"H" ,   "",   set_tab_at_current_column},
  {"Z" ,   "",   identify},
  {"[",    "f",  move_cursor},
  {"[",    "r",  set_scroll_margins},
  {"[",    "A",  move_cursor_up},
  {"[",    "B",  move_cursor_down},
  {"[",    "e",  move_cursor_down},
  {"[",    "C",  move_cursor_right},
  {"[",    "a",  move_cursor_right},
  {"[",    "D",  move_cursor_left},
  {"[",    "E",  move_cursor_down_and_first_col},
  {"[",    "F",  move_cursor_up_and_first_col},
  {"[",    "G",  goto_column},
  {"[",    "`",  goto_column},
  {"[",    "H",  move_cursor},
  {"[",    "I",  insert_n_tabs},
  {"[",    "J",  clear_lines},
  {"[",    "K",  clear_line},
  {"[",    "L",  insert_blank_lines},
  {"[",    "@",  insert_blanks},
  {"[",    "d",  goto_row},
  {"[s",   "",   save_cursor_position},
  {"7",    "",   save_cursor_position}, // ( + attr )
  {"[r",   "",   restore_cursor_position},
  {"8",    "",   restore_cursor_position}, // ( + attr )

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

  {"[",     "m", set_style},
  {"M",     "",  reverse_scroll},
  {"D",     "",  forward_scroll},
  {"=",     "",  ignore}, // keypad mode change
  {">",     "",  ignore}, // keypad mode change
  {"c",     "",  (void*)reset_device},
  {"!p",    "",  (void*)reset_device},
  {"[?1h",  "",  set_cursor_key_to_application},
  {"[?1l",  "",  set_cursor_key_to_cursor},
  {"[?25h", "", show_cursor},
  {"[?25l", "", hide_cursor},
  {"[7h",   "",  enable_line_wrap},
  {"[7l",   "",  disable_line_wrap},
  {NULL, NULL, NULL}
};

static void handle_sequence (Mrg *mrg, const char *sequence)
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
        sequences[i].handler (mrg, sequence);
        return;
      }
    }
  }
  fprintf (stderr, "ESC%s\n", sequence);
}

typedef enum {
  TERMINAL_STATE_NEUTRAL = 0,
  TERMINAL_STATE_GOT_ESC = 1,
  TERMINAL_STATE_GOT_ESC_SQLPAREN = 2,
  TERMINAL_STATE_GOT_ESC_SQRPAREN = 3,
} TerminalState;

static void bell (Mrg *mrg)
{
}

static void feed_byte (Mrg *mrg, int byte)
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
        case '\a': /* BELl */ bell (mrg); break;
        case '\b': /* BS */ backspace (mrg); break;
        case '\t': /* HT tab */
          do {
            add_byte (mrg, ' ');
          } while ( ((buffer->length) % 8));
          break;
        case '\n': /* LF line ffed */
        case '\v': /* VT vertical tab */
        case '\f': /* VF form feed */
          if (lines->data == buffer)
          {
            mrg_list_prepend (&lines, mrg_string_new (""));
            buffer = lines->data;
          }
          move_to (mrg, cursor_y + 1, 1); // XXX the col should depend on mode
          break;
        case '\r': /* CR carriage return */
          move_to (mrg, cursor_y, 1); 
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
          add_byte (mrg, byte);
          break;
      }
      break;
    case TERMINAL_STATE_GOT_ESC:
      if (byte == '[')
      {
        argument_buf_reset("[");
        state = TERMINAL_STATE_GOT_ESC_SQLPAREN;
      }
      else if (byte == ']')
      {
        argument_buf_reset("]");
        state = TERMINAL_STATE_GOT_ESC_SQRPAREN;
      }
      else
      {
        char tmp[3]=" ";
        tmp[0]=byte;
        handle_sequence (mrg, tmp);
        state= TERMINAL_STATE_NEUTRAL;
      }
      break;
    case TERMINAL_STATE_GOT_ESC_SQLPAREN:
      if (byte >= '@' && byte <= '~')
      {
        argument_buf_add (byte);
        handle_sequence (mrg, argument_buf);
        state=0;
      }
      else
      {
        argument_buf_add (byte);
      }
      break;
    case TERMINAL_STATE_GOT_ESC_SQRPAREN:
      if (byte == '\a')
      {
        mrg_set_title (mrg, argument_buf + 3);
        // XXX: use handle sequence here as well,.. for consistency
        state= TERMINAL_STATE_NEUTRAL;
      }
      else
      {
        argument_buf_add (byte);
      }
      break;
  }
}

/* */

static int pty_poll (Mrg *mrg, void *data)
{
  char buf[256];
  int len = read(pty, buf, sizeof (buf));
  if (len > 0)
  {
    int i;
    for (i = 0; i < len; i++)
      feed_byte (mrg, buf[i]);
    pty_poll (mrg, NULL);
  }
  else
  {
    usleep (16000); /* XXX: due to recursion this always happens, giving
                            a (given the buf size, large) rate limit on the
                            terminal */
  }
  if (cursor_y > rows)
    cursor_y = rows;

  return 1;
}

static int event_handler (MrgEvent *event, void *data1, void *data2)
{
  const char *str = event->key_name;

  if (!str)
    return 1;

  if (cursor_key_application)
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
  else if (!strcmp (str, "resize-event")) {
    Mrg *mrg = event->mrg;
    str = "";

    set_term_size (mrg, (int)(mrg_width (mrg) / cell_width)-2, (int)(mrg_height (mrg) / cell_height)-1);
  //  trimlines (rows);
  }

done:
  if (strlen (str))
    write (pty, str, strlen (str));

  return 1;
}

static void render_ui (Mrg *mrg, void *data)
{
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
    float x = mrg_em (mrg); /* ... and remove this logic */
    int no = 1;

    if (mrg_list_length (lines) < rows)
      y -= mrg_em(mrg) * 1.2 * (rows-mrg_list_length (lines));

    for (l = lines; l; l = l->next, no++)
    {
      mrg_set_xy (mrg, x, y);
      mrg_print (mrg, ((MrgString*)(l->data))->str);
      y -= mrg_em (mrg) * 1.2;
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

    cell_width = cw;
    cell_height = em * 1.2;

    cy = mrg_height (mrg) - em;
    cy -= (rows - cursor_y + 1) * 1.2 * em;
    mrg_end (mrg);

    mrg_start (mrg, "cursor", NULL);
    cairo_rectangle (mrg_cr (mrg),
              (cursor_x-1) * cw + em,
              cy,
              cw,
              1.2 * em);
    cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 0.15);
    cairo_fill (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_translate (cr, 0.5, 0.5);
    cairo_rectangle (mrg_cr (mrg),
              (cursor_x-1) * cw + em,
              cy,
              cw,
              1.2 * em);
    cairo_stroke (cr);
    mrg_end (mrg);
  }

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
  mrg_listen (mrg, MRG_KEY_DOWN, event_handler, NULL, NULL);
}

void
signal_child (int a) {
  exit (0); /* XXX: make it less brutal */
}

int terminal_main (int argc, char **argv)
{
  Mrg *mrg = mrg_new ((cols+1) * fontsize * 0.65, (rows+1) * fontsize * 1.2, NULL);

  reset_device (mrg);

  mrg_set_ui (mrg, render_ui, NULL);
  char *command = "sh";

  if (argv[1])
    command = argv[1];

  /* XXX, command should be arg of -e ..., why not handle some more rxvt/xterm
   * like args?
   */
  mrg_set_title (mrg, command);

  pid = forkpty (&pty, NULL, NULL, NULL);

  if (pid == 0)
  {
    unsetenv ("COLUMNS");
    unsetenv ("LINES");
    unsetenv ("TERMCAP");
    unsetenv ("COLOR_TERM");
    setenv ("TERM", "vt100", 1);
    if (execlp(command, command, NULL))
    {
      fprintf (stderr, "failed to start shell\n");
      exit(-1);
    }
  }
  else if (pid < 0)
  {
    fprintf (stderr, "forkpty failed (%m)\n");
    return -1;
  }

  signal (SIGCHLD, signal_child);
  fcntl(pty, F_SETFL, O_NONBLOCK);
  set_term_size (mrg, cols, rows);
  mrg_add_idle (mrg, pty_poll, NULL);
  mrg_main (mrg);
  mrg_destroy (mrg);
  return 0;
}
