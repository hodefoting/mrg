#include <string.h>
#include "mrg-internal.h"
#include "mmm.h"

#include <pthread.h>
#include <alsa/asoundlib.h>
#include <alloca.h>

#define DESIRED_PERIOD_SIZE 800

static float          host_freq     = 48000;
static MrgAudioFormat host_format   = MRG_s16S;
static float          client_freq   = 48000;
static MrgAudioFormat client_format = MRG_s16S;
static int            pcm_queued    = 0;
static int            pcm_cur_left  = 0;
static MrgList       *pcm_list;

/* todo: only start audio thread on first write - enabling dynamic choice
 * of sample-rate? or is it better to keep to opening 48000 as a standard
 * and do better internal resampling for others?
 */

static snd_pcm_t *alsa_open (char *dev, int rate, int channels)
{
   snd_pcm_hw_params_t *hwp;
   snd_pcm_sw_params_t *swp;
   snd_pcm_t *h;
   int r;
   int dir;
   snd_pcm_uframes_t period_size_min;
   snd_pcm_uframes_t period_size_max;
   snd_pcm_uframes_t period_size;
   snd_pcm_uframes_t buffer_size;

   if ((r = snd_pcm_open(&h, dev, SND_PCM_STREAM_PLAYBACK, 0) < 0))
           return NULL;

   hwp = alloca(snd_pcm_hw_params_sizeof());
   memset(hwp, 0, snd_pcm_hw_params_sizeof());
   snd_pcm_hw_params_any(h, hwp);

   snd_pcm_hw_params_set_access(h, hwp, SND_PCM_ACCESS_RW_INTERLEAVED);
   snd_pcm_hw_params_set_format(h, hwp, SND_PCM_FORMAT_S16_LE);
   snd_pcm_hw_params_set_rate(h, hwp, rate, 0);
   snd_pcm_hw_params_set_channels(h, hwp, channels);
   dir = 0;
   snd_pcm_hw_params_get_period_size_min(hwp, &period_size_min, &dir);
   dir = 0;
   snd_pcm_hw_params_get_period_size_max(hwp, &period_size_max, &dir);

   period_size = DESIRED_PERIOD_SIZE;

   dir = 0;
   r = snd_pcm_hw_params_set_period_size_near(h, hwp, &period_size, &dir);
   r = snd_pcm_hw_params_get_period_size(hwp, &period_size, &dir);
   buffer_size = period_size * 4;
   r = snd_pcm_hw_params_set_buffer_size_near(h, hwp, &buffer_size);
   r = snd_pcm_hw_params(h, hwp);
   swp = alloca(snd_pcm_sw_params_sizeof());
   memset(hwp, 0, snd_pcm_sw_params_sizeof());
   snd_pcm_sw_params_current(h, swp);
   r = snd_pcm_sw_params_set_avail_min(h, swp, period_size);
   snd_pcm_sw_params_set_start_threshold(h, swp, 0);
   r = snd_pcm_sw_params(h, swp);
   r = snd_pcm_prepare(h);

   return h;
}

static  snd_pcm_t *h = NULL;
static void *alsa_audio_start(Mrg *mrg)
{
//  Lyd *lyd = aux;
  int c;
  int16_t data[81920*4];

  /* The audio handler is implemented as a mixer that adds data on top
   * of 0s, XXX: it should be ensured that minimal work is there is
   * no data available.
   */
  for (;;)
  {
    int client_channels = mmm_pcm_audio_format_get_channels (client_format);
    int is_float = 0;

    if (client_format == MRG_f32 ||
        client_format == MRG_f32S)
      is_float = 1;

    c = snd_pcm_wait(h, 1000);

    if (c >= 0)
       c = snd_pcm_avail_update(h);

    if (c > 1000) c = 1000; // should use max mmm buffer sizes

    if (c == -EPIPE)
      snd_pcm_prepare(h);

    if (c > 0)
    {
      int i;
      for (i = 0; i < c && pcm_cur_left; i ++)
      {
        if (pcm_cur_left)
        {
          uint32_t *packet_sizep = (pcm_list->data);
          uint32_t packet_size = *packet_sizep;
          uint16_t left = 0, right = 0;

          if (is_float)
          {
            float *packet = (pcm_list->data);
            packet += 4;
            packet += (packet_size - pcm_cur_left) * client_channels;
            left = right = packet[0] * (1<<15);
            if (client_channels > 1)
              right = packet[0] * (1<<15);
          }
          else // s16
          {
            uint16_t *packet = (pcm_list->data);
            packet += 8;
            packet += (packet_size - pcm_cur_left) * client_channels;

            left = right = packet[0];
            if (client_channels > 1)
              right = packet[1];
          }
          data[i * 2 + 0] = left;
          data[i * 2 + 1] = right;

          pcm_cur_left--;
          if (pcm_cur_left <= 0)
          {
            free (pcm_list->data);
            mrg_list_remove (&pcm_list, pcm_list->data);
            if (pcm_list)
            {
              uint32_t *packet_sizep = (pcm_list->data);
              uint32_t packet_size = *packet_sizep;
              pcm_cur_left = packet_size;
            }
          }
        }
      }

    c = snd_pcm_writei(h, data, c);
    if (c < 0)
      c = snd_pcm_recover (h, c, 0);
     }else{
      if (getenv("LYD_FATAL_UNDERRUNS"))
        {
          printf ("dying XXxx need to add API for this debug\n");
          //printf ("%i", lyd->active);
          exit(0);
        }
      fprintf (stderr, "alsa underun\n");
      //exit(0);
    }
  }
}

int mrg_pcm_init (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return 0;
  }
  else
  {
     pthread_t tid;
     h = alsa_open("default", host_freq, mmm_pcm_audio_format_get_channels (host_format));
  if (!h) {
    fprintf(stderr, "mrg unable to open ALSA device (%d channels, %f Hz), dying\n",
            mmm_pcm_audio_format_get_channels (host_format), host_freq);
    return -1;
  }
  pthread_create(&tid, NULL, (void*)alsa_audio_start, mrg);
  }
  return 0;
}

int mrg_pcm_write (Mrg *mrg, const int8_t *data, int frames)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_write (mrg->backend_data, data, frames);
  }
  else
  {
    float factor = client_freq * 1.0 / host_freq;
    int   scaled_frames = frames / factor;
    int   bpf = mmm_pcm_audio_format_bytes_per_frame (client_format);

    uint8_t *packet = malloc (scaled_frames * mmm_pcm_audio_format_bytes_per_frame (client_format) + 16);
    *((uint32_t *)packet) = scaled_frames;

    /* we do both allocations and resampling at the pcm queuing stage.. at it
     * is not on the actual audio playout thread doesn't have to, this sounds
     * better than the resampling currently done in mmm / compositor
     */
    if (factor > 0.999 && factor < 1.0001)
    {
       memcpy (packet + 16, data, frames * bpf);
    }
    else
    {
      int i;
      for (i = 0; i < scaled_frames; i++)
      {
        int source_frame = i * factor;
        memcpy (packet + 16 + bpf * i, data + source_frame * bpf, bpf);
      }
    }
    if (pcm_list == NULL)
      pcm_cur_left = frames;
    mrg_list_append (&pcm_list, packet);
    pcm_queued += frames;

    return frames;
  }
  return 0;
}

int mrg_pcm_get_frame_chunk (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_get_frame_chunk (mrg->backend_data);
  }
  return 1000; // XXX: get queue length and do a cutoff
}

void mrg_pcm_set_sample_rate (Mrg *mrg, int sample_rate)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    mmm_pcm_set_sample_rate (mrg->backend_data, sample_rate);
  }
  else
    client_freq = sample_rate;
}

void mrg_pcm_set_format (Mrg *mrg, MrgAudioFormat format)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    mmm_pcm_set_format (mrg->backend_data, format);
  }
  else
    client_format = format;
}

MrgAudioFormat mrg_pcm_get_format (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_get_format (mrg->backend_data);
  }
  return client_format;
}

int mrg_pcm_get_sample_rate (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_get_sample_rate (mrg->backend_data);
  }
  return client_freq;
}
