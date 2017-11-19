#include <string.h>
#include "mrg-internal.h"
#include "mmm.h"

int mrg_pcm_init (Mrg *mrg)
{
  return -1;
}

int mrg_pcm_write (Mrg *mrg, const int8_t *data, int frames)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_write (mrg->backend_data, data, frames);
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
  return 0;
}

void mrg_pcm_set_sample_rate (Mrg *mrg, int sample_rate)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    mmm_pcm_set_sample_rate (mrg->backend_data, sample_rate);
  }
}

void mrg_pcm_set_format (Mrg *mrg, MrgAudioFormat format)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    mmm_pcm_set_format (mrg->backend_data, format);
  }
}

MrgAudioFormat mrg_pcm_get_format (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_get_format (mrg->backend_data);
  }
  return MMM_s16;
}

int mrg_pcm_get_sample_rate (Mrg *mrg)
{
  if (!strcmp (mrg->backend->name, "mmm") ||
      !strcmp (mrg->backend->name, "mmm-client"))
  {
    return mmm_pcm_get_sample_rate (mrg->backend_data);
  }
  return 48000;
}
