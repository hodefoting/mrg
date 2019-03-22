#ifndef MRG_AUDIO_H
#define MRG_AUDIO_H

#include <stdint.h>

/* This enum should be kept in sync with the corresponding mmm enum.
 */
typedef enum {
  MRG_f32,
  MRG_f32S,
  MRG_s16,
  MRG_s16S
} MrgPCM;

void   mrg_pcm_set_format        (Mrg *mrg, MrgPCM format);
MrgPCM mrg_pcm_get_format        (Mrg *mrg);
int    mrg_pcm_get_sample_rate   (Mrg *mrg);
void   mrg_pcm_set_sample_rate   (Mrg *mrg, int sample_rate);
int    mrg_pcm_get_frame_chunk   (Mrg *mrg);
int    mrg_pcm_get_queued        (Mrg *mrg);
float  mrg_pcm_get_queued_length (Mrg *mrg);
int    mrg_pcm_queue             (Mrg *mrg, const int8_t *data, int frames);

#endif
