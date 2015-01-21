
#ifndef MRG_CONFIG_H
#define MRG_CONFIG_H

 /* tweak these numbers to change which features are built into MRG and not,
  * when used as an amalgamation that is only included; these compile-time
  * flags can be set before including the amalgamation.
  */

#ifndef MRG_GTK
#define MRG_GTK 0
#endif
#ifndef MRG_SDL
#define MRG_SDL 0
#endif
#ifndef MRG_NCT
#define MRG_NCT 1
#endif
#ifndef MRG_CAIRO
#define MRG_CAIRO 1
#endif
#ifndef MRG_MEM
#define MRG_MEM 1
#endif
#ifndef MRG_MMM
#define MRG_MMM 1
#endif

#ifndef MRG_URI_LOG
#define MRG_URI_LOG   0
#endif

#ifndef MRG_LOG
#define MRG_LOG       1 // set to 0 to not compile any of the log messages
#endif

#endif
