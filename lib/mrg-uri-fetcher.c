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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg-list.h"
#include "mrg-config.h"
#include "mrg-string.h"

int
_mrg_file_get_contents (const char  *path,
                   char       **contents,
                   long        *length)
{
  FILE *file;
  long  size;
  long  remaining;
  char *buffer;

  file = fopen (path,"rb");

  if (!file)
    return -1;

  if (!strncmp (path, "/proc", 4))
  {
    buffer = calloc(2048, 1);
    *contents = buffer;
    *length = fread (buffer, 1, 2047, file);
    buffer[*length] = 0;
    return 0;
  }
  else
  {
    fseek (file, 0, SEEK_END);
    *length = size = remaining = ftell (file);
    rewind (file);
    buffer = malloc(size + 8);
  }

  if (!buffer)
    {
      fclose(file);
      return -1;
    }

  remaining -= fread (buffer, 1, remaining, file);
  if (remaining)
    {
      fclose (file);
      free (buffer);
      return -1;
    }
  fclose (file);
  *contents = buffer;
  buffer[size] = 0;
  return 0;
}

#include "mrg-http.h"

enum
{
  URI_STATE_IN_PROTOCOL = 0,
  URI_STATE_IN_HOST,
  URI_STATE_IN_PORT,
  URI_STATE_E_S1,
  URI_STATE_E_S2,
  URI_STATE_IN_PATH,
  URI_STATE_IN_FRAGMENT,
};

int split_uri (char *uri,
               char **protocol,
               char **host,
               char **port,
               char **path,
               char **fragment)
{
  char *p;
  *protocol =
  *host =
  *port =
  *path =
  *fragment = NULL;

  if (strstr (uri, "//") || strchr(uri, ':'))
  {
    int mr = URI_STATE_IN_PROTOCOL;

    if (protocol)
      *protocol = uri;

    if (uri[0] == '/' &&
        uri[1] == '/')
    {
      mr = URI_STATE_E_S1;
      *protocol = NULL;
    }

    for (p = uri; *p; p++)
    {
      switch (mr)
      {
        case URI_STATE_IN_PROTOCOL:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_E_S1;
              break;
          }
          break;
        case URI_STATE_E_S1:
          switch (*p)
          {
            default:
              mr = URI_STATE_IN_HOST;
              if (path) *path = p;
              break;
            case '/':
              mr = URI_STATE_E_S2;
              break;
          }
          break;
        case URI_STATE_E_S2:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_IN_HOST;
              if (host) *host = p+1;
              break;
          }
          break;
        case URI_STATE_IN_HOST:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_IN_PORT;
              if (port) *port = p+1;
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PORT:
          switch (*p)
          {
            default:
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PATH:
          switch (*p)
          {
            default:
              break;
            case '#':
              *p = '\0';
              mr = URI_STATE_IN_FRAGMENT;
              if (fragment) *fragment = p+1;
              break;
          }
          break;
      }
    }
  }
  else
  {

    int mr = URI_STATE_IN_HOST;
    if (protocol)
      *protocol = NULL;

    if (uri[0]=='/')
    {
      if (host)
        *host = NULL;
      if (port)
        *port = NULL;
      *uri = '\0';
      mr = URI_STATE_IN_PATH;
      if (path) *path = uri+1;
    }
    else 
    {
      mr = URI_STATE_IN_PATH;
      if (path) *path = uri;
    }

    for (p = uri; *p; p++)
    {
      switch (mr)
      {
        case URI_STATE_IN_PROTOCOL:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_E_S1;
              break;
          }
          break;
        case URI_STATE_E_S1:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_E_S2;
              break;
          }
          break;
        case URI_STATE_E_S2:
          switch (*p)
          {
            default:
              // XXX ?
              break;
            case '/':
              mr = URI_STATE_IN_HOST;
              if (host) *host = p+1;
              break;
          }
          break;
        case URI_STATE_IN_HOST:
          switch (*p)
          {
            default:
              break;
            case ':':
              *p = '\0';
              mr = URI_STATE_IN_PORT;
              if (port) *port = p+1;
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PORT:
          switch (*p)
          {
            default:
              break;
            case '/':
              *p = '\0';
              mr = URI_STATE_IN_PATH;
              if (path) *path = p+1;
              break;
          }
          break;
        case URI_STATE_IN_PATH:
          switch (*p)
          {
            default:
              break;
            case '#':
              *p = '\0';
              mr = URI_STATE_IN_FRAGMENT;
              if (fragment) *fragment = p+1;
              break;
          }
          break;
      }
    }
  }
  if (*protocol && (*protocol)[0] == 0)
    *protocol = NULL;
  return 0;
}

#include "../data.inc"

typedef struct {
  const char *path;
  int (*cb)(const char *referer, const char *uri, char **contents, long *length);
} UriDispatch;

#include <sys/utsname.h>
#include <cairo.h>
#include <pixman.h>
#if MRG_SDL
#include <SDL/SDL.h>
#endif
#if MRG_GTK
#include <gtk/gtk.h>
#endif
#include "mrg.h"

static int
about_version_cb (const char *referer,
                  const char *uri,
                  char **contents,
                  long  *length)
{
  MrgString *str = mrg_string_new ("<html><body>");
  struct utsname uname_info;
  ;
  uname (&uname_info);

  mrg_string_append_printf (str, "<h2>Micro Raptor Gui</h2>");

  mrg_string_append_printf (str, "<p>verison %i.%i.%i</p>", MRG_VERSION_MAJOR, MRG_VERSION_MINOR, MRG_VERSION_MICRO);
  

  mrg_string_append_printf (str, "<dl>");
  mrg_string_append_printf (str, "<dt>Kernel: </dt><dd>%s %s %s</dd>", uname_info.sysname, uname_info.machine, uname_info.release);
  mrg_string_append_printf (str, "<dt>Cairo: </dt><dd>%i.%i.%i</dd>", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
  mrg_string_append_printf (str, "<dt>Pixman: </dt><dd>%i.%i.%i</dd>", PIXMAN_VERSION_MAJOR, PIXMAN_VERSION_MINOR, PIXMAN_VERSION_MICRO);

#if MRG_GTK
  mrg_string_append_printf (str, "<dt>Gtk: </dt><dd>%i.%i.%i</dd>", GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
#endif

#if MRG_SDL
  mrg_string_append_printf (str, "<dt>SDL: </dt><dd>%i.%i.%i</dd>", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
#endif

  mrg_string_append_printf (str, "</dl>");

  mrg_string_append_printf (str, "</body></html> ");

  *contents = mrg_string_dissolve (str);
  *length = strlen (*contents);
  return 0;
}

UriDispatch uri_dispatchers[]=
{
  {"",        about_version_cb},
  {"version", about_version_cb},
  {NULL, NULL}
};

static int
_mrg_internal_get_contents (const char *referer,
                            const char *uri,
                            char **contents,
                            long  *length)
{
  int i;
  
  for (i = 0; uri_dispatchers[i].path; i++)
  {
    if (!strcmp (&uri[4], uri_dispatchers[i].path))
      return uri_dispatchers[i].cb(referer,uri,contents,length);
  }

  for (i = 0; mrg_data[i].path; i++)
  {
    if (!strcmp (mrg_data[i].path, &uri[4]))
      {
        *length = mrg_data[i].length;
        *contents = malloc (*length);
        memcpy (*contents, mrg_data[i].data, *length);
        return 0;
      }
  }
  return -1;
}

static int
_mr_get_contents (const char  *referer,
                 const char  *uri,
                 char       **contents,
                 long        *length)
{
  char *uri_dup;
  char *protocol = NULL;
  char *host = NULL;
  char *port = NULL;
  char *path = NULL;
  char *fragment = NULL;

  if (!strncmp (uri, "mrg:", 4))
  {
    return _mrg_internal_get_contents (referer, uri, contents, length);
  }

  uri_dup = strdup (uri);
  split_uri (uri_dup, &protocol, &host, &port, &path, &fragment);
  if (protocol && !strcmp (protocol, "http"))
  {
    int len;
    char *pathdup = malloc (strlen (path) + 2);
    pathdup[0] = '/';
    strcpy (&pathdup[1], path);
   // fprintf (stderr, "%s %i\n",host, port?atoi(port):80);
    char *cont = _mrg_http (NULL, host, port?atoi(port):80, pathdup, &len);
    *contents = cont;
    *length = len;
    //fprintf (stderr, "%s\n", cont);
    //
    fprintf (stderr, "%i\n", len);
    free (uri_dup);
    return 0;
  }
  else if (protocol && !strcmp (protocol, "file"))
  {
    char *path2 = malloc (strlen (path) + 2);
    int ret;
    sprintf (path2, "/%s", path);
    ret = _mrg_file_get_contents (path2, contents, length);
    free (path2);
    free (uri_dup);
    fprintf (stderr, "%i\n", (int)*length);
    return ret;
  }
  else
  {
    char *c = NULL;
    long  l = 0;
    int ret;
    free (uri_dup);
    ret = _mrg_file_get_contents (uri, &c, &l);
    if (contents) *contents = c;
    if (length) *length = l;
    fprintf (stderr, "%li\n", l);
    return ret;
  }

  return -1;
}

typedef struct _CacheEntry {
  char *uri;
  char *contents;
  long  length;
} CacheEntry;

static MrgList *cache = NULL;

char *_mrg_resolve_uri (const char *base_uri, const char *uri)
{
  char *ret;
  char *uri_dup = strdup (uri);

  if (!base_uri)
    return uri_dup;

  char *base_dup = strdup (base_uri);

  char *protocol = NULL;
  char *host = NULL;
  char *port = NULL;
  char *path = NULL;
  char *fragment = NULL;
  char *base_protocol = NULL;
  char *base_host = NULL;
  char *base_port = NULL;
  char *base_path = NULL;
  char *base_fragment = NULL;

  //int retlen = 0;
  int samehost = 0;

  split_uri (uri_dup, &protocol, &host, &port, &path, &fragment);
  split_uri (base_dup, &base_protocol, &base_host, &base_port, &base_path, &base_fragment);

  if (!protocol)
    protocol = base_protocol;
  if (!host)
  {
    host = base_host;
    port = base_port;
    samehost = 1;
  }

  ret = malloc (
      (path?strlen (path):0)
      + (fragment?strlen (fragment):0) +
      (host?strlen (host):0) + 640);
  if (protocol)
  {
    if (uri[0] == '/' && uri[1] != '/')
      sprintf (ret, "%s://%s%s%s%s", protocol, host, port?":":"", port?port:"", uri);
    else if (uri[0] == '.' && uri[1] == '.' && uri[2] == '/' &&
             uri[3] == '.' && uri[4] == '.')
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;

      sprintf (ret, "c%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", &path[6], fragment?"#":"", fragment?fragment:"");
    }
    else if (uri[0] == '.' && uri[1] == '.')
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      if (base_path[strlen (base_path)-1] == '/')
        base_path[strlen (base_path)-1] = 0;
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else
        base_path[0]=0;

      sprintf (ret, "%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", &path[3], fragment?"#":"", fragment?fragment:"");
    }
    else
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      else if (strchr (base_path, '/'))
        strchr (base_path, '/')[1] = 0;
      else
        base_path[0] = 0;
      
      if (host)
      sprintf (ret, "%s://%s%s%s/%s%s%s%s", protocol, host, port?":":"", port?port:"", samehost?base_path:"", path, fragment?"#":"", fragment?fragment:"");
      else
      sprintf (ret, "%s:%s%s%s%s", protocol, samehost?base_path:"", path, fragment?"#":"", fragment?fragment:"");

    }
  }
  else
  {
    if (uri[0] == '/')
      sprintf (ret, "%s", path);
    else
    {
      if (strrchr (base_path, '/'))
        strrchr (base_path, '/')[1] = 0;
      sprintf (ret, "/%s%s", base_path, path);
    }
  }

  free (uri_dup);
  free (base_dup);
  return ret;
}

/* caching uri fetcher
 */
int
mrg_get_contents_default (const char  *referer,
                          const char  *input_uri,
                          char       **contents,
                          long        *length,
                          void        *ignored_user_data)
{
  MrgList *i;

  /* should resolve before mrg_get_contents  */
  char *uri = _mrg_resolve_uri (referer, input_uri);

  for (i = cache; i; i = i->next)
  {
    CacheEntry *entry = i->data;
    if (!strcmp (entry->uri, uri))
    {
      *contents = malloc (entry->length + 1);
      memcpy (*contents, entry->contents, entry->length);
      (*contents)[entry->length]=0;
      free (uri);
      if (length) *length = entry->length;
      if (length)
      {
        return 0;
      }
      else
      {
        return -1;
      }
    }
  }

  {
    CacheEntry *entry = calloc (sizeof (CacheEntry), 1);
    char *c = NULL;
    long  l = 0;

    entry->uri = uri;
    _mr_get_contents (referer, uri, &c, &l);
    if (c){
      entry->contents = c;
      entry->length = l;
    } else
    {
      entry->contents = NULL;
      entry->length = 0;
    }
    mrg_list_prepend (&cache, entry);

#if MRG_URI_LOG
    if (c)
      fprintf (stderr, "%li\t%s\n", l, uri);
    else
      fprintf (stderr, "FAIL\t%s\n", uri);
#endif

  }


  return mrg_get_contents_default (referer, input_uri, contents, length, ignored_user_data);
}
