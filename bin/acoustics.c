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

/* daemons should have their process files in the same directory,
 * permitting the system to be self introspectable.
 *
 * drag and drop configuration using filesystem as repository.
 *
 * desktop view,. listing contents of a specific folder, client windows
 * rendered on top. 
 *
 * how to determine that something is an mmm app?
 */

#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mrg.h"
#include <dirent.h>
#include "mmm.h"
#include <unistd.h>
#include "mrg-list.h"
#include <sys/stat.h>
#include <errno.h>

#if MRG_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#endif

#include "mrg-internal.h" // XXX: eeek

typedef struct Client
{
  char  *filename;
  Mmm   *mmm;
  long   pid;
} Client;

#if MRG_SDL
static const int freq = 44100;
#else
static const int freq = 8000;
#endif

typedef struct Acoustics
{
  Mrg     *mrg;
  char    *fbdir;
  MrgList *clients;
  FILE    *dev_audio;
} Acoustics;

static void enable_audio  (Acoustics *acoustics);
static void disable_audio (Acoustics *acoustics);

static Acoustics *acoustics;


static int pid_is_alive (long pid)
{
  char path[256];
  struct stat sts;
  sprintf (path, "/proc/%li", pid);
  if (stat(path, &sts) == -1 && errno == ENOENT) {
    return 0;
  }
  return 1;
}

static void validate_client (Acoustics *acoustics, const char *client_name)
{
  MrgList *l;
  for (l = acoustics->clients; l; l = l->next)
  {
    Client *client = l->data;
    if (client->filename && 
        !strcmp (client->filename, client_name))
    {
      return;
    }
  }

  {
    Client *client = calloc (sizeof (Client), 1);

    char tmp[256];
    sprintf (tmp, "%s/%s", acoustics->fbdir, client_name);
    client->mmm = mmm_host_open (tmp);
    client->pid = mmm_client_pid (client->mmm);
    if (!client->mmm)
    {
      fprintf (stderr, "acoustics failed to open\n");
      return;
    }

    {
      const char *title = mmm_get_title (client->mmm);
      if (title && !strcmp (title, "acoustics"))
      {
        if (getpid () != client->pid && pid_is_alive (client->pid))
          exit(-2);
      }
    }

    client->filename = strdup (client_name);
    mrg_list_append (&acoustics->clients, client);
  }
}

static void acoustics_monitor_dir (Acoustics *acoustics)
{
  MrgList *l;
again:
  for (l = acoustics->clients; l; l = l->next)
  {
    Client *client = l->data;
    if (!pid_is_alive (client->pid))
    {
      free (client);
      mrg_list_remove (&acoustics->clients, client);
      goto again;
    }
  }

  DIR *dir = opendir (acoustics->fbdir);
  struct dirent *ent;
  
  while ((ent = readdir (dir)))
  {
    if (ent->d_name[0]!='.')
      validate_client (acoustics, ent->d_name);
  }
  closedir (dir);
}

#include <stdio.h>
#include <time.h>

static int  audio_muted = 100;
static inline void monitor_audio (Acoustics *acoustics, uint8_t *buf, int frames)
{
  MrgList *l;

  int got_data = 0;

  for (l = acoustics->clients; l; l = l->next)
  {
    Client *client = l->data;
    int8_t data[8192*2];
    int16_t *dst = (void*) buf;
    int read;
    int remaining = frames;
    int requested = remaining;
    do {
      int16_t *src = (void*) &data[0];
      requested = remaining;

      float factor = mmm_pcm_get_sample_rate (client->mmm) * 1.0 / freq;

      if (factor < 1.001 && factor > 0.999)
      {
        read = mmm_pcm_read (client->mmm, data, remaining);
        if (read)
        {
          int i;
          remaining -= read;
          for (i = 0; i < read; i ++)
          {
#if 0
            static int prev = 0;
            int delta = prev-*src;
            if (delta != -1)
              fprintf (stderr, "%i ", delta<0?-delta:delta);
            prev = *src;
#endif
            *(dst++) += *(src++);
          }
          got_data ++;
        }
      } else if (factor > 1.0)
      { /* get more, scale down. the most horrible resampler */
        int request = remaining * factor; // XXX: keep track of fraction
        read = mmm_pcm_read (client->mmm, data, request);
        if (read) {
          int i;
          int outpos = 0;
          int bpf = mmm_pcm_bytes_per_frame (client->mmm);

          for (i = 0; i < read / bpf / factor; i++)
          {
            int j;
            for (j = 0; j < bpf; j++)
            {
              data[outpos * bpf + j] = data[i * bpf + j];
            }
            outpos++;
          }
        }
        if (read)
        {
          int i;
          remaining -= read / factor;
          for (i = 0; i < read; i ++)
          {
            *(dst++) += *(src++);
          }
          got_data ++;
        }
      }
      else
      {
        fprintf (stderr, "uhm %f %f %f\n",
          mmm_pcm_get_sample_rate (client->mmm) * 1.0, freq * 1.0, factor);
      }
    } while ((read == requested) && remaining > 0);

    if (remaining)
    {
      fprintf (stderr, "%p %i pcm underrun\n", client, remaining);
    }
  }

  if (!got_data)
  {
    audio_muted++;
    if (audio_muted > 4)
    {
      disable_audio (acoustics);
    }
  }
}

#if MRG_SDL
void audio_callback (void *data, uint8_t *buf, int bytes)
{
  Acoustics *acoustics = data;
  uint8_t target_buf[MMM_PCM_BUFFER_SIZE]={0,};
  memset (target_buf, 0, sizeof target_buf);
  monitor_audio (acoustics, target_buf, bytes / 2);
  memcpy (buf, target_buf, bytes);
}
#else

/*
 * ** This routine converts from linear to ulaw
 * **
 * ** Craig Reese: IDA/Supercomputing Research Center
 * ** Joe Campbell: Department of Defense
 * ** 29 September 1989
 * **
 * ** References:
 * ** 1) CCITT Recommendation G.711  (very difficult to follow)
 * ** 2) "A New Digital Technique for Implementation of Any
 * **     Continuous PCM Companding Law," Villeret, Michel,
 * **     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
 * **     1973, pg. 11.12-11.17
 * ** 3) MIL-STD-188-113,"Interoperability and Performance Standards
 * **     for Analog-to_Digital Conversion Techniques,"
 * **     17 February 1987
 * **
 * ** Input: Signed 16 bit linear sample
 * ** Output: 8 bit ulaw sample
 * */

//#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

static inline unsigned char
linear2ulaw(int sample)
{
  static int exp_lut[256] = { 0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,
  4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;          /* set aside the sign */
  if (sign != 0) sample = -sample;              /* get magnitude */
  if (sample > CLIP) sample = CLIP;             /* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[(sample >> 7) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
  if (ulawbyte == 0) ulawbyte = 0x02;   /* optional CCITT trap */
#endif

  return(ulawbyte);
}

#endif

static void render_ui (Mrg *mrg, void *data)
{
//  Acoustics *acoustics = data;
}

static void init_env (Acoustics *acoustics)
{
  char buf[512];
  if (acoustics->fbdir)
    return;
  acoustics->fbdir = "/tmp/mrg";
  acoustics->fbdir = getenv ("MMM_PATH");
  sprintf (buf, "mkdir %s &> /dev/null", acoustics->fbdir);
  system (buf);
}

Acoustics *acoustics_new (void)
{
  Acoustics *acoustics = calloc (sizeof (acoustics), 1);
  acoustics->dev_audio = fopen ("/dev/audio", "w");
  return acoustics;
}

void acoustics_destroy (Acoustics *acoustics)
{
  free (acoustics);
}

static int acoustics_idle_check (Mrg *mrg, void *data)
{
  Acoustics *acoustics = data;
  MrgList *l;
  static int i = 0;
  i++;
  if (i % 50 == 0)
    acoustics_monitor_dir (acoustics);
  
  if (audio_muted == 100)
  {
    for (l = acoustics->clients; l; l = l->next)
    {
      Client *client = l->data;
      if (mmm_pcm_get_queued_frames (client->mmm))
      {
        enable_audio (acoustics);
        break;
      }
    }
  }
  //
  for (l = acoustics->clients; l; l = l->next)
  {
    Client *client = l->data;

    int x, y, width, height;
    if (mmm_get_damage (client->mmm, &x, &y, &width, &height))
    {
      if (width)
      {
        MrgRectangle rect = {x + mmm_get_x (client->mmm), mmm_get_y (client->mmm), width, height};
        //fprintf (stderr, "%i %i %i %i",  x, y, width, height);
        mrg_queue_draw (mrg, &rect);
      }
      else
      {
        MrgRectangle rect = {mmm_get_x (client->mmm), mmm_get_y (client->mmm),
                             mmm_get_width (client->mmm), mmm_get_height (client->mmm)};
        mrg_queue_draw (mrg, &rect);
      }
    }
  }

#if !MRG_SDL && 0
  {
    static int prev_ticks = -1;
    int ticks, delta, i;

    if (prev_ticks==-1) prev_ticks = mrg_ms (mrg);
    ticks = mrg_ms (mrg);
    delta = ticks - prev_ticks;
    prev_ticks = ticks;

    // XXX: measure number of ticks; then write
    int bytes = delta * 8000.0 / 1000.0;
    uint16_t target_buf[MMM_PCM_BUFFER_SIZE]={0,};
    uint8_t temp2[MMM_PCM_BUFFER_SIZE];

    memset (target_buf, 0, sizeof target_buf);
    monitor_audio (acoustics, (void*)target_buf, bytes);
    for (i = 0; i < bytes; i++)
    {
      temp2[i] = linear2ulaw(target_buf[i]);
    }

    // XXX: convert to ulaw
    fwrite (temp2, bytes, 1, acoustics->dev_audio);
    // XXX: write to /dev/audio
  }
#endif

  return 1;
}

#if MRG_SDL
static void enable_audio (Acoustics *acoustics)
{
  static int first = 1;
  if (first) // XXX: hack since CloseAudio crashes pulseaudio
  {
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec desire_spec;
    SDL_AudioSpec actual_spec;
    desire_spec.freq = freq;
    desire_spec.format = AUDIO_S16SYS;
    desire_spec.channels = 1;
    desire_spec.samples = 512 * freq / 44100;
    desire_spec.callback = audio_callback;
    desire_spec.userdata = acoustics;
    SDL_OpenAudio(&desire_spec, &actual_spec);
    first = 0;
  }
  SDL_PauseAudio(0);
  audio_muted = 0;
}

static void disable_audio (Acoustics *acoustics)
{
  audio_muted = 99;
  SDL_PauseAudio(1);
#if 0 // XXX: crashes pulseaudio
  SDL_CloseAudio();
#endif
  audio_muted = 100;
}
#else

static void enable_audio (Acoustics *acoustics)
{
}

static void disable_audio (Acoustics *acoustics)
{
}

#endif

int acoustics_main (int argc, char **argv)
{
  Mrg *mrg;
  if (!getenv ("MMM_PATH"))
  {
    fprintf (stderr, "acoustics error MMM_PATH not set\n");
    return -1;
  }
  return 0;

  mrg = mrg_new (16, 16, NULL);
  mrg_set_title (mrg, "acoustics");
  acoustics = acoustics_new ();
  acoustics->mrg = mrg;
  mrg_set_ui (mrg, render_ui, acoustics);

  init_env (acoustics);
  mrg_add_idle (mrg, acoustics_idle_check, acoustics);
  mrg_main (mrg);
  mrg_destroy (mrg);

  acoustics_destroy (acoustics);
  return 0;
}
