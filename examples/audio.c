/*
 * Copyright (c) 2014 Øyvind Kolås <pippin@hodefoting.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "mrg.h"
#include <math.h>

static long frames = 1;

static void ui (Mrg *mrg, void *data)
{
  mrg_printf (mrg, "hoping for sound %li\n", frames);

  mrg_add_binding (mrg, "control-q", NULL, NULL, mrg_quit_cb, NULL);
}
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int audio_cb (Mrg *mrg, void *data)
{
  int count = 8192 * 4;
  int16_t buf[count * 2];
  int i;
  count = mrg_pcm_get_frame_chunk (mrg)-2;

  for (i = 0; i < count; i++)
  {
    frames++;
    buf[i] = sin(frames/ 44100.0 * 880 * M_PI) * 31000;
  }
  mrg_pcm_write (mrg, (void*)buf, count);
  return 1;
}

void *audio_thread (void *mrg)
{
  while (1)
  {
    if (!audio_cb (mrg, NULL))
      usleep (2000);
  }
  return NULL;
}

int main (int argc, char **argv)
{
  Mrg *mrg = mrg_new (400, 300, NULL);
  mrg_set_ui (mrg, ui, argv[1]?argv[1]:"world");
#if 0
  pthread_t thread_id;
  pthread_create (&thread_id, NULL, audio_thread, mrg);
#else
  mrg_add_idle (mrg, audio_cb, NULL);
#endif
  mrg_main (mrg);
  return 0;
}
