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

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>

typedef struct _AppMap AppMap;

typedef enum {
  APP_FLAG_DEFAULTS = 0,
  APP_FLAG_HIDE = 1 << 0
} AppFlags;

struct _AppMap
{
  char    *name;
  char    *arguments_help;
  int    (*main) (int argc, char **argv);
  AppFlags flags;
  char    *description;
};

int host_main (int argc, char **argv);
int dir_main        (int argc, char **argv);
int edit_main       (int argc, char **argv);
int terminal_main   (int argc, char **argv);
int browser_main    (int argc, char **argv);
int session_main    (int argc, char **argv);
int acoustics_main  (int argc, char **argv);
int taskman_main    (int argc, char **argv);

AppMap mains[]={

  {"browser", "[uri]",
    browser_main,    APP_FLAG_DEFAULTS,
    "XHTML renderer rigged up as webbrowser"},

  {"host", "",
    host_main, APP_FLAG_HIDE,
    "Framebuffer multiplexer (window manager)"},


  {"edit", "<file>",
    edit_main,       APP_FLAG_DEFAULTS,
    "text-editor"},

  {"terminal", "[command]",
    terminal_main,   APP_FLAG_DEFAULTS,
    "Terminal emulator, if no command is provided \"sh\" is used."},

  {"session", "",
    session_main,   APP_FLAG_DEFAULTS,
    "launch a session (runs hosts in a loop)"},
    
  {"taskman", "",
    taskman_main,   APP_FLAG_DEFAULTS,
    "visual task manager"},

  {"acoustics", "",
    acoustics_main, APP_FLAG_DEFAULTS,
    "audio host daemon, monitors clients for pcm data, when it exists mixes the data and feeds it to the audio device. "},

  {"dir", "<directory | file>",
    dir_main,        APP_FLAG_DEFAULTS,
    "filesystem explorer"},

  {NULL, NULL, NULL, 0, NULL}
};

static int help (char *command)
{
  int i;

  if (command)
  {
    for (i = 0; mains[i].name; i++)
      if (!strcmp (mains[i].name, command))
      {
        printf ("Usage: %s %s\n", command, mains[i].arguments_help);
        printf ("\n");
        printf ("%s\n", mains[i].description);
        return -1;
      }
    printf ("mrg help doesn't know about the command '%s'\n", command);
    return -1;
  }

  printf ("Usage: mrg <command> [options]\n"
      "\n"
      "mrg is a multi-call binary; commands can be invoked through symlinks\n"
      "or by using the command as the first argument of mrg itself, mrg called\n"
      "without any argument is the same as invoking the session command.\n"
      "\n"
      "Currently defined commands:\n");

  for (i = 0; mains[i].name; i++)
    printf ("  %s %s\n", mains[i].name, mains[i].arguments_help);
  printf ("\n");
  return 0;
}

static int arg_is_help (const char *arg)
{
  if (!arg)
    return 0;
  if (!strcmp (arg, "-h") ||
      !strcmp (arg, "--help") ||
      !strcmp (arg, "help"))
    return 1;
  return 0;
}

int main (int argc, char **argv)
{
  char *base = basename (argv[0]);
  int i;

  if (arg_is_help(argv[1]))
  {
    for (i = 0; mains[i].name; i++)
    {
      if (!strncmp (base, "mrg-", 4) &&
          !strcmp (&base[4], mains[i].name))
        return help(mains[i].name);
      if (!strcmp (base, mains[i].name))
        return help(mains[i].name);
    }

    return help (argv[2]);
  }

  for (i = 0; mains[i].name; i++)
  {
    if (!strncmp (base, "mrg-", 4) &&
        !strcmp (&base[4], mains[i].name))
    {
      if (arg_is_help (argv[1]))
        return help (mains[i].name);
      else
        return mains[i].main(argc, argv);
    }

    if (!strcmp (base, mains[i].name))
    {
      if (arg_is_help (argv[1]))
        return help (mains[i].name);
      else
        return mains[i].main(argc, argv);
    }

    if (argv[1] && !strcmp (argv[1], mains[i].name))
    {
      if (arg_is_help (argv[2]))
        return help (mains[i].name);
      else
        return mains[i].main(argc - 1, &argv[1]);
    }
  }

  return mains[sizeof(mains)/sizeof(mains[0])-2].main (argc, argv);
  return help (NULL);
//  return session_main (argc, argv);
}
