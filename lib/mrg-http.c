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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "mrg-string.h"

#define HAS_GETHOSTBYNAME


char *_mrg_http (const char *ip,
                 const char *hostname,
                 int         port,
                 const char *path,
                 int        *length)
{
#ifdef HAS_GETHOSTBYNAME
  struct hostent    * host;
#endif
	struct sockaddr_in  addr;
	int sock;

  sock = socket (PF_INET, SOCK_STREAM, 0);

  //mrg_LOG("http", "GET %s:%i%s", hostname, port, path);

  if ( sock >= 0 )
    {
      MrgString *str = mrg_string_new ("");
      memset (&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons (port);

      if (ip)
      {
        addr.sin_addr.s_addr = inet_addr (ip);
      }
      else
      {
#ifdef HAS_GETHOSTBYNAME
        host = gethostbyname (hostname);
        if (!host)
          return NULL;
        addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
#else
        /* XXX: should fall back to try using `hostname` to resolve
         *      the hostname...
         */
        fprintf (stderr, "ip needed on this platform\n");
#endif
      }


      if (connect (sock, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        {	
          int count;
          char s[4096];
          sprintf(s, "GET %s HTTP/1.0\r\n", path);
          write (sock, s, strlen (s));
          if (hostname)
          {
            sprintf(s, "Host: %s\r\n", hostname);
            write (sock, s, strlen (s));
          }
          sprintf(s, "User-Agent: mr/0.0.0\r\n");
          write (sock, s, strlen (s));
          sprintf(s, "\r\n");
          write (sock, s, strlen (s));
          fsync (sock);

          while ((count = read (sock, s, sizeof (s))))
            mrg_string_append_data (str, s, count);
        }
      if (str->length)
        {
          if (strstr (str->str, "HTTP/1.1 200") ||
              strstr (str->str, "HTTP/1.0 200"))
            {
              int start = 0;
              int i;
              char *copy;
              for (i = 0; str->str[i]; i++)
                {
                  if (str->str[i] == '\r' &&
                      str->str[i+1] == '\n' &&
                      str->str[i+2] == '\r' &&
                      str->str[i+3] == '\n')
                    {
                      start = i + 4;
                      break;
                    }
                }
              copy = malloc (str->length + 1 - start);
              memcpy (copy, &(str->str[start]), str->length - start);
              copy[str->length - start] = 0;
              if (length)
                *length = str->length - start;
              //mrg_LOG("http", "got %i bytes", str->length - start);
              mrg_string_free (str, 1);
              fprintf (stderr, "[%s]\n", copy);
              return copy;
            }
          else
            {
              /* XXX: should return error codes back somehow */
              //printf ("\nXXXXX  NON200 response XXXXX:\n\n%s\n", str->str);
              mrg_string_free (str, 1);
            }
        }
      else
        mrg_string_free (str, 1);

      shutdown (sock, SHUT_RDWR);
    }
  if (length)
    *length = -1;
  //mrg_LOG("http", "failed");
  return NULL;
}

typedef struct _MrgHttpConnection MrgHttpConnection;

struct _MrgHttpConnection {
  struct hostent      * host;
  struct sockaddr_in    addr;
  int    sock;
};

static MrgHttpConnection *http_connection_new (const char *ip,
                                               const char *hostname,
                                               int port)
{
  MrgHttpConnection *c;
  c = calloc (sizeof (MrgHttpConnection), 1);

  //mrg_LOG("http", "POST %s:%i%s %i", hostname, port, path, length);

  c->sock = socket (PF_INET, SOCK_STREAM, 0);

  if (c->sock >= 0 )
    {
      memset (&c->addr, 0, sizeof(c->addr));
      c->addr.sin_family = AF_INET;
      c->addr.sin_port = htons (port);

      if (ip)
      {
        c->addr.sin_addr.s_addr = inet_addr (ip);
      }
      else
      {
#ifdef HAS_GETHOSTBYNAME
        /* it might be better to do this by ip.. */
        c->host = gethostbyname (hostname);
        if (!c->host)
          return NULL;
        c->addr.sin_addr.s_addr = *(long*)(c->host->h_addr_list[0]);
#else
        fprintf (stderr, "ip needed on this platform\n");
#endif
      }

      if (connect (c->sock, (struct sockaddr*)&c->addr, sizeof(c->addr)) == 0)
        {
          return c;
        }
    }
  free (c);
  return NULL;
}

static void http_connection_free (MrgHttpConnection *connection)
{
  if (connection->sock)
    close (connection->sock);
  free (connection);
}

char *_mrg_http_post (const char *ip,
                      const char *hostname,
                      int         port,
                      const char *path,
                      const char *body,
                      int         length,
                      int        *ret_length)
{
  MrgHttpConnection *c = http_connection_new (ip, hostname, port);
  if (!c)
    {
      if (ret_length)
        *ret_length = -1;
      fprintf (stderr, "http failed\n");
      return NULL;
    }

    {
      MrgString *str = mrg_string_new ("");
      int count;
      char s[512];
      if (length < 0)
        length = strlen (body);

#define WRITE_DATA(str,len)     write(c->sock, str, len)
#define WRITE(str)              WRITE_DATA(str,strlen(str))

      sprintf(s, "POST %s HTTP/1.0\r\n", path); WRITE (s);
      sprintf(s, "User-Agent: zn/0.0.0\r\n"); WRITE (s);
      sprintf(s, "Content-Length: %d\r\n", length); WRITE (s);
      sprintf(s, "\r\n"); WRITE (s);
      WRITE_DATA(body,length);
      fsync (c->sock); /* write all data, before being able to
                          expect data back */

      while ((count = read (c->sock, s, sizeof (s))))
        mrg_string_append_data (str, s, count);
      if (str->length)
        {
          if (strstr (str->str, "HTTP/1.1 200") ||
              strstr (str->str, "HTTP/1.0 200"))
            {
              int start = 0;
              int i;
              char *copy;
              for (i = 0; str->str[i]; i++)
                {
                  if (str->str[i]   == '\r' &&
                      str->str[i+1] == '\n' &&
                      str->str[i+2] == '\r' &&
                      str->str[i+3] == '\n')
                    {
                      start = i + 4;
                      break;
                    }
                }
              copy = malloc (str->length + 1 - start);
              memcpy (copy, &(str->str[start]), str->length - start);
              copy[str->length - start] = 0;
              if (ret_length)
                *ret_length = str->length - start;
              //mrg_LOG("http", "got %i bytes", str->length - start);
              mrg_string_free (str, 1);

              http_connection_free (c);

              fprintf (stderr, "[%s]\n", copy);
              return copy;
            }
          else
            {
              printf ("%s\n", str->str);
              mrg_string_free (str, 1);
            }
        }
      else
        mrg_string_free (str, 1);
    }

  if (ret_length)
    *ret_length = -1;
  fprintf (stderr, "http failed\n");
  http_connection_free (c);
  return NULL;
}
