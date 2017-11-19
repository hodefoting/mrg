#ifndef MRG_AUDIO_H
#define MRG_AUDIO_H

#include <stdint.h>

int mrg_pcm_init  (Mrg *mrg);
int mrg_pcm_write (Mrg *mrg, const int8_t *data, int frames);
int mrg_pcm_get_frame_chunk (Mrg *mrg);

int mrg_pcm_get_sample_rate (Mrg *mrg);
void mrg_pcm_set_sample_rate (Mrg *mrg, int sample_rate);

typedef enum {
  MRG_f32,
  MRG_f32S,
  MRG_s16,
  MRG_s16S
} MrgAudioFormat;


void mrg_pcm_set_format (Mrg *mrg, MrgAudioFormat format);
MrgAudioFormat mrg_pcm_get_format (Mrg *mrg);


#endif

