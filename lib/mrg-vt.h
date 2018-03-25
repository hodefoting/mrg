
typedef struct _MrgVT MrgVT;

MrgVT      *mrg_vt_new                (Mrg *mrg, const char *commandline);

const char *mrg_vt_find_shell_command (void);

int         mrg_vt_get_result         (MrgVT *vt);
int         mrg_vt_is_done            (MrgVT *vt);
void        mrg_vt_poll               (MrgVT *vt);
long        mrg_vt_rev                (MrgVT *vt);
void        mrg_vt_destroy            (MrgVT *vt);
void        mrg_vt_set_term_size      (MrgVT *vt, int cols, int rows);

/* this is how mrg/mmm based key-events are fed into the vt engine
 */
void        mrg_vt_feed_keystring     (MrgVT *vt, const char *str);

/* not needed when passing a commandline for command to
 * run, but could be used for injecting commands, or
 * output from stored shell commands/sessions to display
 */
void        mrg_vt_feed_byte          (MrgVT *vt, int byte);

#define DEFAULT_SCROLLBACK   0
#define DEFAULT_ROWS         24
#define DEFAULT_COLS         80

const char *mrg_vt_get_commandline    (MrgVT *vt);

int         mrg_vt_get_line_count     (MrgVT *vt);


const char *mrg_vt_get_line           (MrgVT *vt, int no);

void        mrg_vt_set_scrollback_lines (MrgVT *vt, int scrollback_lines);
int         mrg_vt_get_scrollback_lines (MrgVT *vt);

int         mrg_vt_get_cols             (MrgVT *vt);
int         mrg_vt_get_rows             (MrgVT *vt);

int         mrg_vt_get_cursor_x         (MrgVT *vt);
int         mrg_vt_get_cursor_y         (MrgVT *vt);

void        mrg_vt_draw                 (MrgVT *vt, Mrg *mrg, double x, double y, float font_size, float line_spacing);
