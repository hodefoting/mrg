/* lowish level detection of file modification for live-coding
 * reinit purposes, running applications should respawn when
 * their dependencies in on disk
 *   binaries / lua code  change
 *
 * could be made to use file monitoring - to enable doing it by
 * default and not only 
 *
 * todo:
 *    implement persistance of state in a couple of apps
 *
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include "mrg.h"
#include "mrg-list.h"
#include "mrg-internal.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static int mrg_restarter_id = 0;
static int mrg_restart_id = 0;
static MrgList *restarter_list = NULL;

typedef struct _MrgRestarterEntry  MrgRestarterEntry;
struct _MrgRestarterEntry
{
  char   *path;
  time_t  mtime;
};

static time_t path_get_mtime (const char *path)
{
#if 0
  struct stat buf;
  if (!stat (path, &buf))
    return buf.st_mtime;
  return 0;
#else
  /* XXX this is really ugly XXX but luajit crashes if the above is used */
  char buf[512], cmd[512]="";
  FILE *ls;
  long ret;
  sprintf (cmd, "ls -la %s -l --time-style=+%%s | cut -f 6 -d ' '" , path);
  ls = popen(cmd, "r");
  while (fgets(buf, sizeof(buf), ls) != 0) {
    ret = atol (buf);
    break;
  }
  pclose(ls);
  return (time_t)ret;
#endif
}

static MrgRestarterEntry *entry_new (const char *path)
{
  time_t mtime;
  MrgRestarterEntry *entry = NULL;
  entry = calloc (1, sizeof (MrgRestarterEntry));
  entry->path = strdup (path);
  mtime = path_get_mtime (entry->path);
  if (mtime)
  {
    entry->mtime = mtime;
  }
  return entry;
}

int mrg_restart_cb (Mrg *mrg, void *data)
{
  char *cmdline = NULL;
  long cmdline_length;
  char path_exe[512]="";
  _mrg_file_get_contents ("/proc/self/cmdline", &cmdline, &cmdline_length);
  readlink ("/proc/self/exe", path_exe, 512);

  char *argv[32]={0,0,0,0,0,0,0};
  int i, a;
  argv[0] = cmdline;
  a=0;
  for (i = 0; i < cmdline_length-1; i++)
  {
    if (cmdline[i] == 0)
    {
      a++;
      argv[a] = &cmdline[i+1];
    }
  }
  if (strstr (path_exe, " (deleted)")) /* XXX: locale dependent? */
  {
    *strstr (path_exe, " (deleted") = 0;
  }
  fprintf (stderr, "execv: %s %s %s %s\n", path_exe, argv[0], argv[1], argv[2]);
  setenv ("MRG_RESTARTER", "foo", 1);
  if (mrg->backend->mrg_restart)
    mrg->backend->mrg_restart (mrg);
  execv (path_exe, argv);
  return 0;
}

int mrg_restarter_cb (Mrg *mrg, void *data)
{
  MrgList *l;
  for (l = restarter_list; l; l = l->next)
  {
    MrgRestarterEntry *entry = l->data;
    time_t mtime = path_get_mtime (entry->path);
    if (mtime && mtime != entry->mtime)
    {
      entry->mtime = mtime;
      if (mrg_restart_id != 0)
        mrg_remove_idle (mrg, mrg_restart_id);
      mrg_restart_id = mrg_add_timeout (mrg, 1000, mrg_restart_cb, NULL);        
    }
  }
  return 1;
  /* XXX: the timeout currently turns into an idle and thus fires
   * much more rapidly than it should
   */
}

void mrg_restarter_add_path (Mrg *mrg, const char *path)
{
  mrg_list_prepend (&restarter_list, entry_new (path));

  if (!mrg_restarter_id)
    mrg_restarter_id = mrg_add_timeout (mrg, 1000, mrg_restarter_cb, NULL);        
}

void mrg_restarter_init (Mrg *mrg)
{
  char *cmdline = NULL;
  long cmdline_length;
  char path_exe[512]="";
  _mrg_file_get_contents ("/proc/self/cmdline", &cmdline, &cmdline_length);
  readlink ("/proc/self/exe", path_exe, 512);

  char *argv[32]={0,0,0,0,0,0,0};
  int i, a;
  argv[0] = cmdline;
  a=0;
  for (i = 0; i < cmdline_length-1; i++)
  {
    if (cmdline[i] == 0)
    {
      a++;
      argv[a] = &cmdline[i+1];
    }
  }
  if (strstr (path_exe, "luajit"))
  {
    char script_path[1024];
    realpath (argv[1], script_path);
    mrg_restarter_add_path (mrg, script_path);
    /* TODO: add recursive travarsal of require's */
  }
  else
  {
    if (!strstr (path_exe, "host"))
      mrg_restarter_add_path (mrg, path_exe);
  }
  fprintf (stderr, "[[[%s]\n", getenv ("MRG_RESTARTER"));
}
