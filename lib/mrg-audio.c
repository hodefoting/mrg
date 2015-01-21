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
