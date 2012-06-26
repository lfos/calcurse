/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2012 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#ifndef CALCURSE_H
#define CALCURSE_H

#include "config.h"

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#elif defined HAVE_NCURSESW_NCURSES_H
#include <ncursesw/ncurses.h>
#else
#error "Missing ncurses header. Aborting..."
#endif

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>

#include "llist.h"
#include "htable.h"
#include "llist_ts.h"

/* Internationalization. */
#if ENABLE_NLS
#include <locale.h>
#include <libintl.h>
#undef _
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(String) (String)
#define dcgettext(String) (String)
#define ngettext(String1,String2,n) ((n) == 1 ? (String1) : (String2))
#define bindtextdomain(String) (String)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif /* ENABLE_NLS */

/* Paths configuration. */
#define DIR_NAME         ".calcurse/"
#define TODO_PATH_NAME   "todo"
#define APTS_PATH_NAME   "apts"
#define CONF_PATH_NAME   "conf"
#define KEYS_PATH_NAME   "keys"
#define CPID_PATH_NAME   ".calcurse.pid"
#define DPID_PATH_NAME   ".daemon.pid"
#define DLOG_PATH_NAME   "daemon.log"
#define NOTES_DIR_NAME   "notes/"

#define TODO_PATH        DIR_NAME TODO_PATH_NAME
#define APTS_PATH        DIR_NAME APTS_PATH_NAME
#define CONF_PATH        DIR_NAME CONF_PATH_NAME
#define KEYS_PATH        DIR_NAME KEYS_PATH_NAME
#define CPID_PATH        DIR_NAME CPID_PATH_NAME
#define DLOG_PATH        DIR_NAME DLOG_PATH_NAME
#define DPID_PATH        DIR_NAME DPID_PATH_NAME
#define NOTES_DIR        DIR_NAME NOTES_DIR_NAME

#define DEFAULT_EDITOR   "vi"
#define DEFAULT_PAGER    "less"

#define ATTR_FALSE    0
#define ATTR_TRUE     1
#define ATTR_LOWEST   2
#define ATTR_LOW      3
#define ATTR_MIDDLE   4
#define ATTR_HIGH     5
#define ATTR_HIGHEST  6

#define STATUSHEIGHT  2
#define MAX_NOTESIZ   40
#define TMPEXTSIZ     6

/* Format for appointment hours is: HH:MM */
#define HRMIN_SIZE 6

/* Maximum number of colors available. */
#define NBUSERCOLORS  6

/* Side bar width acceptable boundaries. */
#define SBARMINWIDTH     32
#define SBARMAXWIDTHPERC 50

/* Related to date manipulation. */
#define YEARINMONTHS    12
#define YEARINDAYS      365
#define TM_YEAR_BASE    1900

#define WEEKINDAYS      7
#define DAYINHOURS      24
#define HOURINMIN       60
#define MININSEC        60

#define WEEKINHOURS     (WEEKINDAYS * DAYINHOURS)
#define WEEKINMIN       (WEEKINHOURS * HOURINMIN)
#define WEEKINSEC       (WEEKINMIN * MININSEC)
#define DAYINMIN        (DAYINHOURS * HOURINMIN)
#define DAYINSEC        (DAYINMIN * MININSEC)
#define HOURINSEC       (HOURINMIN * MININSEC)

/* Calendar window. */
#define CALHEIGHT       12

/* Key definitions. */
#define CTRLVAL                 0x1F
#define CTRL(x)                 ((x) & CTRLVAL)
#define ESCAPE                  27
#define TAB                     9
#define SPACE                   32

#define KEYS_KEYLEN         3   /* length of each keybinding */
#define KEYS_LABELEN        8   /* length of command description */
#define KEYS_CMDS_PER_LINE  6   /* max number of commands per line */

/* Size of the hash table the note garbage collector uses. */
#define NOTE_GC_HSIZE 1024

#define ERROR_MSG(...) do {                                                   \
  char msg[BUFSIZ];                                                           \
  int len;                                                                    \
                                                                              \
  len = snprintf (msg, BUFSIZ, "%s: %d: ", __FILE__, __LINE__);               \
  snprintf (msg + len, BUFSIZ - len, __VA_ARGS__);                            \
  if (ui_mode == UI_CURSES)                                                   \
    fatalbox (msg);                                                           \
  else                                                                        \
    fprintf (stderr, "%s\n", msg);                                            \
} while (0)

#define WARN_MSG(...) do {                                                    \
  char msg[BUFSIZ];                                                           \
                                                                              \
  snprintf (msg, BUFSIZ, __VA_ARGS__);                                        \
  if (ui_mode == UI_CURSES)                                                   \
    warnbox (msg);                                                            \
  else                                                                        \
    fprintf (stderr, "%s\n", msg);                                            \
} while (0)

#define EXIT(...) do {                                                        \
  ERROR_MSG(__VA_ARGS__);                                                     \
  if (ui_mode == UI_CURSES)                                                   \
    exit_calcurse (EXIT_FAILURE);                                             \
  else                                                                        \
    exit (EXIT_FAILURE);                                                      \
} while (0)

#define EXIT_IF(cond, ...) do {                                               \
  if ((cond))                                                                 \
    EXIT(__VA_ARGS__);                                                        \
} while (0)

#define RETURN_IF(cond, ...) do {                                             \
  if ((cond))                                                                 \
    {                                                                         \
      ERROR_MSG(__VA_ARGS__);                                                 \
      return;                                                                 \
    }                                                                         \
} while (0)

#define RETVAL_IF(cond, val, ...) do {                                        \
  if ((cond))                                                                 \
    {                                                                         \
      ERROR_MSG(__VA_ARGS__);                                                 \
      return (val);                                                           \
    }                                                                         \
} while (0)

#define STRINGIFY(x)  #x
#define TOSTRING(x)   STRINGIFY(x)
#define __FILE_POS__   __FILE__ ":" TOSTRING(__LINE__)

#define UTF8_MAXLEN 6
#define UTF8_LENGTH(ch) ((unsigned char)ch >= 0xFC ? 6 : \
    ((unsigned char)ch >= 0xF8 ? 5 : \
    ((unsigned char)ch >= 0xF0 ? 4 : \
    ((unsigned char)ch >= 0xE0 ? 3 : \
    ((unsigned char)ch >= 0xC0 ? 2 : 1)))))
#define UTF8_ISCONT(ch) ((unsigned char)ch >= 0x80 && \
    (unsigned char)ch <= 0xBF)

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

/* General configuration variables. */
struct conf {
  unsigned auto_save;
  unsigned auto_gc;
  unsigned periodic_save;
  unsigned confirm_quit;
  unsigned confirm_delete;
  unsigned system_dialogs;
  unsigned progress_bar;
  const char *editor;
  const char *pager;
  char output_datefmt[BUFSIZ];  /* format for displaying date */
  int input_datefmt;            /* format for reading date */
};

/* Daemon-related configuration. */
struct dmon_conf {
  unsigned enable;              /* launch daemon automatically when exiting */
  unsigned log;                 /* log daemon activity */
};

/* Input date formats. */
enum datefmt {
  DATEFMT_MMDDYYYY = 1,
  DATEFMT_DDMMYYYY,
  DATEFMT_YYYYMMDD,
  DATEFMT_ISO,
  DATEFMT_MAX
};

#define DATE_FORMATS (DATEFMT_MAX - 1)

#define DATEFMT(datefmt) (datefmt == DATEFMT_MMDDYYYY ? "%m/%d/%Y" :          \
    (datefmt == DATEFMT_DDMMYYYY ? "%d/%m/%Y" :                               \
    (datefmt == DATEFMT_YYYYMMDD ? "%Y/%m/%d" : "%Y-%m-%d")))

#define DATEFMT_DESC(datefmt) (datefmt == DATEFMT_MMDDYYYY ?                  \
    _("mm/dd/yyyy") : (datefmt == DATEFMT_DDMMYYYY ? _("dd/mm/yyyy") :        \
    (datefmt == DATEFMT_YYYYMMDD ? _("yyyy/mm/dd") : _("yyyy-mm-dd"))))

struct date {
  unsigned dd;
  unsigned mm;
  unsigned yyyy;
};

/* Appointment definition. */
struct apoint {
  long start;                   /* seconds since 1 jan 1970 */
  long dur;                     /* duration of the appointment in seconds */

#define APOINT_NULL      0x0
#define APOINT_NOTIFY    0x1    /* Item needs to be notified */
#define APOINT_NOTIFIED  0x2    /* Item was already notified */
  int state;

  char *mesg;
  char *note;
};

/* Event definition. */
struct event {
  int id;                       /* event identifier */
  long day;                     /* seconds since 1 jan 1970 */
  char *mesg;
  char *note;
};

/* Todo item definition. */
struct todo {
  char *mesg;
  int id;
  char *note;
};

/* Number of items in current day. */
struct day_items_nb {
  unsigned nb_events;
  unsigned nb_apoints;
};

struct excp {
  long st;                      /* beggining of the considered day, in seconds */
};

enum recur_type {
  RECUR_NO,
  RECUR_DAILY,
  RECUR_WEEKLY,
  RECUR_MONTHLY,
  RECUR_YEARLY,
  RECUR_TYPES
};

/* To describe an item's repetition. */
struct rpt {
  enum recur_type type;         /* repetition type */
  int freq;                     /* repetition frequence */
  long until;                   /* ending date for repeated event */
};

/* Recurrent appointment definition. */
struct recur_apoint {
  struct rpt *rpt;              /* information about repetition */
  llist_t exc;                  /* days when the item should not be repeated */
  long start;                   /* beggining of the appointment */
  long dur;                     /* duration of the appointment */
  char state;                   /* 8 bits to store item state */
  char *mesg;                   /* appointment description */
  char *note;                   /* note attached to appointment */
};

/* Reccurent event definition. */
struct recur_event {
  struct rpt *rpt;              /* information about repetition */
  llist_t exc;                  /* days when the item should not be repeated */
  int id;                       /* event type */
  long day;                     /* day at which event occurs */
  char *mesg;                   /* event description */
  char *note;                   /* note attached to event */
};

/* Generic pointer data type for appointments and events. */
union aptev_ptr {
  struct apoint *apt;
  struct event *ev;
  struct recur_apoint *rapt;
  struct recur_event *rev;
};

/* Generic item description (to hold appointments, events...). */
struct day_item {
  int type;                     /* (recursive or normal) event or appointment */
  long start;                   /* start time of the repetition occurrence */
  union aptev_ptr item;         /* pointer to the actual item */
  int appt_pos;                 /* real position in recurrent list */
};

/* Available view for the calendar panel. */
enum {
  CAL_MONTH_VIEW,
  CAL_WEEK_VIEW,
  CAL_VIEWS
};

struct notify_app {
  long time;
  int got_app;
  char *txt;
  char state;
  pthread_mutex_t mutex;
};

struct io_file {
  FILE *fd;
  char name[BUFSIZ];
};

/* Available keys. */
enum key {
  KEY_GENERIC_CANCEL,
  KEY_GENERIC_SELECT,
  KEY_GENERIC_CREDITS,
  KEY_GENERIC_HELP,
  KEY_GENERIC_QUIT,
  KEY_GENERIC_SAVE,
  KEY_GENERIC_CUT,
  KEY_GENERIC_PASTE,
  KEY_GENERIC_CHANGE_VIEW,
  KEY_GENERIC_IMPORT,
  KEY_GENERIC_EXPORT,
  KEY_GENERIC_GOTO,
  KEY_GENERIC_OTHER_CMD,
  KEY_GENERIC_CONFIG_MENU,
  KEY_GENERIC_REDRAW,
  KEY_GENERIC_ADD_APPT,
  KEY_GENERIC_ADD_TODO,
  KEY_GENERIC_PREV_DAY,
  KEY_GENERIC_NEXT_DAY,
  KEY_GENERIC_PREV_WEEK,
  KEY_GENERIC_NEXT_WEEK,
  KEY_GENERIC_PREV_MONTH,
  KEY_GENERIC_NEXT_MONTH,
  KEY_GENERIC_PREV_YEAR,
  KEY_GENERIC_NEXT_YEAR,
  KEY_GENERIC_SCROLL_DOWN,
  KEY_GENERIC_SCROLL_UP,
  KEY_GENERIC_GOTO_TODAY,

  KEY_MOVE_RIGHT,
  KEY_MOVE_LEFT,
  KEY_MOVE_DOWN,
  KEY_MOVE_UP,
  KEY_START_OF_WEEK,
  KEY_END_OF_WEEK,
  KEY_ADD_ITEM,
  KEY_DEL_ITEM,
  KEY_EDIT_ITEM,
  KEY_VIEW_ITEM,
  KEY_PIPE_ITEM,
  KEY_FLAG_ITEM,
  KEY_REPEAT_ITEM,
  KEY_EDIT_NOTE,
  KEY_VIEW_NOTE,
  KEY_RAISE_PRIORITY,
  KEY_LOWER_PRIORITY,

  NBKEYS,
  KEY_UNDEF
};

/* To describe a key binding. */
struct binding {
  char *label;
  enum key action;
};

enum win {
  CAL,
  APP,
  TOD,
  NOT,
  STA,
  NBWINS
};

#define FLAG_CAL (1 << CAL)
#define FLAG_APP (1 << APP)
#define FLAG_TOD (1 << TOD)
#define FLAG_NOT (1 << NOT)
#define FLAG_STA (1 << STA)
#define FLAG_ALL ((1 << NBWINS) - 1)

enum ui_mode {
  UI_CURSES,
  UI_CMDLINE,
  UI_MODES
};

/* Generic window structure. */
struct window {
  WINDOW *p;                    /* pointer to window */
  unsigned w;                   /* width */
  unsigned h;                   /* height */
  int x;                        /* x position */
  int y;                        /* y position */
};

/* Generic scrolling window structure. */
struct scrollwin {
  struct window win;
  struct window pad;
  unsigned first_visible_line;
  unsigned total_lines;
  const char *label;
};

/* Pad structure to handle scrolling. */
struct pad {
  int width;
  int length;
  int first_onscreen;           /* first line to be displayed inside window */
  WINDOW *ptrwin;               /* pointer to the pad window */
};

/* Notification bar definition. */
struct nbar {
  unsigned show;                /* display or hide the notify-bar */
  int cntdwn;                   /* warn when time left before next app
                                   becomes lesser than cntdwn */
  char datefmt[BUFSIZ];         /* format for displaying date */
  char timefmt[BUFSIZ];         /* format for displaying time */
  char cmd[BUFSIZ];             /* notification command */
  const char *shell;            /* user shell to launch notif. cmd */
  unsigned notify_all;          /* notify all appointments */
  pthread_mutex_t mutex;
};

/* Available types of items. */
enum item_type {
  RECUR_EVNT = 1,
  EVNT,
  RECUR_APPT,
  APPT,
  MAX_TYPES = APPT
};

/* Flags used to adapt processing when erasing an item. */
enum eraseflg {
  ERASE_DONT_FORCE,
  ERASE_FORCE,
  ERASE_FORCE_ONLY_NOTE,
  ERASE_CUT
};

/* Return codes for the getstring() function. */
enum getstr {
  GETSTRING_VALID,
  GETSTRING_ESC,                /* user pressed escape to cancel editing. */
  GETSTRING_RET                 /* return was pressed without entering any text. */
};

/* Week days. */
enum wday {
  SUNDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  WDAYS
};

/* Possible movements inside calendar. */
enum move {
  DAY_PREV,
  DAY_NEXT,
  WEEK_PREV,
  WEEK_NEXT,
  WEEK_START,
  WEEK_END,
  MONTH_PREV,
  MONTH_NEXT,
  YEAR_PREV,
  YEAR_NEXT
};

/* Available color pairs. */
enum {
  COLR_RED = 1,
  COLR_GREEN,
  COLR_YELLOW,
  COLR_BLUE,
  COLR_MAGENTA,
  COLR_CYAN,
  COLR_DEFAULT,
  COLR_HIGH,
  COLR_CUSTOM
};

/* Available import types. */
enum import_type {
  IO_IMPORT_ICAL,
  IO_IMPORT_NBTYPES
};

/* Available export types. */
enum export_type {
  IO_EXPORT_ICAL,
  IO_EXPORT_PCAL,
  IO_EXPORT_NBTYPES
};

/* To customize the display when saving data. */
enum save_display {
  IO_SAVE_DISPLAY_BAR,
  IO_SAVE_DISPLAY_MARK,
  IO_SAVE_DISPLAY_NONE
};

/* apoint.c */
extern llist_ts_t alist_p;
void apoint_free_bkp(void);
void apoint_llist_init(void);
void apoint_llist_free(void);
void apoint_hilt_set(int);
void apoint_hilt_decrease(int);
void apoint_hilt_increase(int);
int apoint_hilt(void);
struct apoint *apoint_new(char *, char *, long, long, char);
int apoint_cut(unsigned *, unsigned *);
void apoint_paste(unsigned *, unsigned *, int);
unsigned apoint_inday(struct apoint *, long *);
void apoint_sec2str(struct apoint *, long, char *, char *);
void apoint_write(struct apoint *, FILE *);
struct apoint *apoint_scan(FILE *, struct tm, struct tm, char, char *);
void apoint_delete(struct apoint *, enum eraseflg);
void apoint_scroll_pad_down(int, int);
void apoint_scroll_pad_up(int);
struct notify_app *apoint_check_next(struct notify_app *, long);
void apoint_switch_notify(struct apoint *);
void apoint_update_panel(int);
void apoint_paste_item(void);

/* args.c */
int parse_args(int, char **);

/* calendar.c */
void calendar_view_next(void);
void calendar_view_prev(void);
void calendar_set_view(int);
int calendar_get_view(void);
void calendar_start_date_thread(void);
void calendar_stop_date_thread(void);
void calendar_set_current_date(void);
void calendar_set_first_day_of_week(enum wday);
void calendar_change_first_day_of_week(void);
unsigned calendar_week_begins_on_monday(void);
void calendar_store_current_date(struct date *);
void calendar_init_slctd_day(void);
struct date *calendar_get_slctd_day(void);
long calendar_get_slctd_day_sec(void);
void calendar_update_panel(struct window *);
void calendar_goto_today(void);
void calendar_change_day(int);
void calendar_move(enum move, int);
long calendar_start_of_year(void);
long calendar_end_of_year(void);
const char *calendar_get_pom(time_t);

/* config.c */

void config_load(void);
unsigned config_save(void);

/* custom.c */
void custom_init_attr(void);
void custom_apply_attr(WINDOW *, int);
void custom_remove_attr(WINDOW *, int);
void custom_config_bar(void);
void custom_layout_config(void);
void custom_sidebar_config(void);
void custom_color_config(void);
void custom_color_theme_name(char *);
void custom_confwin_init(struct window *, const char *);
void custom_set_swsiz(struct scrollwin *);
void custom_general_config(void);
void custom_keys_config(void);
void custom_config_main(void);

/* day.c */
void day_free_list(void);
char *day_item_get_mesg(struct day_item *);
char *day_item_get_note(struct day_item *);
long day_item_get_duration(struct day_item *);
int day_item_get_state(struct day_item *);
int day_store_items(long, unsigned *, unsigned *, regex_t *);
struct day_items_nb *day_process_storage(struct date *, unsigned,
                                         struct day_items_nb *);
void day_write_pad(long, int, int, int);
void day_write_stdout(long, const char *, const char *, const char *,
                      const char *);
void day_popup_item(struct day_item *);
int day_check_if_item(struct date);
unsigned day_chk_busy_slices(struct date, int, int *);
int day_cut_item(long, int);
int day_paste_item(long, int);
struct day_item *day_get_item(int);
int day_item_nb(long, int, int);
void day_edit_note(struct day_item *, const char *);
void day_view_note(struct day_item *, const char *);
void day_item_switch_notify(struct day_item *);

/* dmon.c */
void dmon_start(int);
void dmon_stop(void);

/* event.c */
extern llist_t eventlist;
void event_free_bkp(void);
void event_llist_init(void);
void event_llist_free(void);
struct event *event_new(char *, char *, long, int);
unsigned event_inday(struct event *, long *);
void event_write(struct event *, FILE *);
struct event *event_scan(FILE *, struct tm, int, char *);
void event_delete(struct event *, enum eraseflg);
void event_paste_item(void);

/* help.c */
void help_wins_init(struct scrollwin *, int, int, int, int);
void help_screen(void);

/* getstring.c */
enum getstr getstring(WINDOW *, char *, int, int, int);
int updatestring(WINDOW *, char **, int, int);

/* ical.c */
void ical_import_data(FILE *, FILE *, unsigned *, unsigned *, unsigned *,
                      unsigned *, unsigned *);
void ical_export_data(FILE *);

/* interaction.c */
void interact_day_item_add(void);
void interact_day_item_delete(unsigned *, unsigned *);
void interact_day_item_edit(void);
void interact_day_item_pipe(void);
void interact_day_item_repeat(void);
void interact_todo_add(void);
void interact_todo_delete(void);
void interact_todo_edit(void);
void interact_todo_pipe(void);

/* io.c */
unsigned io_fprintln(const char *, const char *, ...);
void io_init(const char *, const char *);
void io_extract_data(char *, const char *, int);
unsigned io_save_apts(void);
unsigned io_save_todo(void);
unsigned io_save_keys(void);
void io_save_cal(enum save_display);
void io_load_app(void);
void io_load_todo(void);
void io_load_keys(const char *);
void io_check_dir(char *, int *);
unsigned io_file_exist(char *);
void io_check_file(char *, int *);
int io_check_data_files(void);
void io_startup_screen(int);
void io_export_data(enum export_type);
void io_export_bar(void);
void io_import_data(enum import_type, const char *);
struct io_file *io_log_init(void);
void io_log_print(struct io_file *, int, const char *);
void io_log_display(struct io_file *, const char *, const char *);
void io_log_free(struct io_file *);
void io_start_psave_thread(void);
void io_stop_psave_thread(void);
void io_set_lock(void);
unsigned io_dump_pid(char *);
unsigned io_get_pid(char *);
int io_file_is_empty(char *);
int io_file_cp(const char *, const char *);

/* keys.c */
void keys_init(void);
void keys_free(void);
void keys_dump_defaults(char *);
const char *keys_get_label(enum key);
enum key keys_get_action(int);
enum key keys_getch(WINDOW * win, int *);
int keys_assign_binding(int, enum key);
void keys_remove_binding(int, enum key);
int keys_str2int(const char *);
const char *keys_int2str(int);
int keys_action_count_keys(enum key);
const char *keys_action_firstkey(enum key);
const char *keys_action_nkey(enum key, int);
char *keys_action_allkeys(enum key);
void keys_display_bindings_bar(WINDOW *, struct binding *[], int, int,
                               int, struct binding *);
void keys_popup_info(enum key);
void keys_save_bindings(FILE *);
int keys_check_missing_bindings(void);
void keys_fill_missing(void);

/* mem.c */
void *xmalloc(size_t);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t, size_t);
char *xstrdup(const char *);
void xfree(void *);

#ifdef CALCURSE_MEMORY_DEBUG

#define mem_malloc(s)         dbg_malloc ((s), __FILE_POS__)
#define mem_calloc(n, s)      dbg_calloc ((n), (s), __FILE_POS__)
#define mem_realloc(p, n, s)  dbg_realloc ((p), (n), (s), __FILE_POS__)
#define mem_strdup(s)         dbg_strdup ((s), __FILE_POS__)
#define mem_free(p)           dbg_free ((p), __FILE_POS__)

void *dbg_malloc(size_t, const char *);
void *dbg_calloc(size_t, size_t, const char *);
void *dbg_realloc(void *, size_t, size_t, const char *);
char *dbg_strdup(const char *, const char *);
void dbg_free(void *, const char *);
void mem_stats(void);

#else /* MEMORY DEBUG disabled */

#define mem_malloc(s)         xmalloc ((s))
#define mem_calloc(n, s)      xcalloc ((n), (s))
#define mem_realloc(p, n, s)  xrealloc ((p), (n), (s))
#define mem_strdup(s)         xstrdup ((s))
#define mem_free(p)           xfree ((p))
#define mem_stats()

#endif /* CALCURSE_MEMORY_DEBUG */

/* note.c */
char *generate_note(const char *);
void edit_note(char **, const char *);
void view_note(const char *, const char *);
void erase_note(char **);
void note_read(char *, FILE *);
void note_gc(void);

/* notify.c */
int notify_time_left(void);
unsigned notify_needs_reminder(void);
void notify_update_app(long, char, char *);
int notify_bar(void);
void notify_init_vars(void);
void notify_init_bar(void);
void notify_free_app(void);
void notify_start_main_thread(void);
void notify_stop_main_thread(void);
void notify_reinit_bar(void);
unsigned notify_launch_cmd(void);
void notify_update_bar(void);
unsigned notify_get_next(struct notify_app *);
unsigned notify_get_next_bkgd(void);
char *notify_app_txt(void);
void notify_check_next_app(int);
void notify_check_added(char *, long, char);
void notify_check_repeated(struct recur_apoint *);
int notify_same_item(long);
int notify_same_recur_item(struct recur_apoint *);
void notify_config_bar(void);

/* pcal.c */
void pcal_export_data(FILE *);

/* recur.c */
extern llist_ts_t recur_alist_p;
extern llist_t recur_elist;
void recur_event_free_bkp(void);
void recur_apoint_free_bkp(void);
void recur_apoint_llist_init(void);
void recur_apoint_llist_free(void);
void recur_event_llist_free(void);
struct recur_apoint *recur_apoint_new(char *, char *, long, long, char,
                                      int, int, long, llist_t *);
struct recur_event *recur_event_new(char *, char *, long, int, int, int,
                                    long, llist_t *);
char recur_def2char(enum recur_type);
int recur_char2def(char);
struct recur_apoint *recur_apoint_scan(FILE *, struct tm, struct tm,
                                       char, int, struct tm, char *,
                                       llist_t *, char);
struct recur_event *recur_event_scan(FILE *, struct tm, int, char,
                                     int, struct tm, char *, llist_t *);
void recur_apoint_write(struct recur_apoint *, FILE *);
void recur_event_write(struct recur_event *, FILE *);
void recur_save_data(FILE *);
unsigned recur_item_find_occurrence(long, long, llist_t *, int,
                                    int, long, long, unsigned *);
unsigned recur_apoint_find_occurrence(struct recur_apoint *, long, unsigned *);
unsigned recur_event_find_occurrence(struct recur_event *, long, unsigned *);
unsigned recur_item_inday(long, long, llist_t *, int, int, long, long);
unsigned recur_apoint_inday(struct recur_apoint *, long *);
unsigned recur_event_inday(struct recur_event *, long *);
void recur_event_erase(struct recur_event *, long, unsigned, enum eraseflg);
void recur_apoint_erase(struct recur_apoint *, long, unsigned, enum eraseflg);
void recur_exc_scan(llist_t *, FILE *);
struct notify_app *recur_apoint_check_next(struct notify_app *, long, long);
void recur_apoint_switch_notify(struct recur_apoint *);
void recur_event_paste_item(void);
void recur_apoint_paste_item(void);

/* sigs.c */
void sigs_init(void);
unsigned sigs_set_hdlr(int, void (*)(int));

/* todo.c */
extern llist_t todolist;
struct todo *todo_get_item(int);
void todo_hilt_set(int);
void todo_hilt_decrease(int);
void todo_hilt_increase(int);
int todo_hilt(void);
int todo_nb(void);
void todo_set_nb(int);
void todo_set_first(int);
void todo_first_increase(int);
void todo_first_decrease(int);
int todo_hilt_pos(void);
char *todo_saved_mesg(void);
struct todo *todo_add(char *, int, char *);
void todo_write(struct todo *, FILE *);
void todo_delete_note_bynum(unsigned);
void todo_delete(struct todo *);
void todo_flag(void);
void todo_chg_priority(int);
void todo_update_panel(int);
void todo_edit_note(const char *);
void todo_view_note(const char *);
void todo_init_list(void);
void todo_free_list(void);

/* utf8.c */
int utf8_width(char *);
int utf8_strwidth(char *);

/* utils.c */
void exit_calcurse(int) __attribute__ ((__noreturn__));
void free_user_data(void);
void fatalbox(const char *);
void warnbox(const char *);
void status_mesg(const char *, const char *);
int status_ask_choice(const char *, const char[], int);
int status_ask_bool(const char *);
int status_ask_simplechoice(const char *, const char *[], int);
void erase_window_part(WINDOW *, int, int, int, int);
WINDOW *popup(int, int, int, int, const char *, const char *, int);
void print_in_middle(WINDOW *, int, int, int, const char *);
int is_all_digit(const char *);
long get_item_time(long);
int get_item_hour(long);
int get_item_min(long);
long date2sec(struct date, unsigned, unsigned);
char *date_sec2date_str(long, const char *);
void date_sec2date_fmt(long, const char *, char *);
long date_sec_change(long, int, int);
long update_time_in_date(long, unsigned, unsigned);
long get_sec_date(struct date);
long min2sec(unsigned);
void draw_scrollbar(WINDOW *, int, int, int, int, int, unsigned);
void item_in_popup(const char *, const char *, const char *, const char *);
long get_today(void);
long now(void);
char *nowstr(void);
long mystrtol(const char *);
void print_bool_option_incolor(WINDOW *, unsigned, int, int);
const char *get_tempdir(void);
char *new_tempfile(const char *, int);
int parse_date(const char *, enum datefmt, int *, int *, int *, struct date *);
int parse_time(const char *, unsigned *, unsigned *);
int parse_duration(const char *, unsigned *);
void str_toupper(char *);
void file_close(FILE *, const char *);
void psleep(unsigned);
int fork_exec(int *, int *, const char *, const char *const *);
int shell_exec(int *, int *, const char *, const char *const *);
int child_wait(int *, int *, int);
void press_any_key(void);
void print_apoint(const char *, long, struct apoint *);
void print_event(const char *, long, struct event *);
void print_recur_apoint(const char *, long, unsigned, struct recur_apoint *);
void print_recur_event(const char *, long, struct recur_event *);
void print_todo(const char *, struct todo *);

/* vars.c */
extern int col, row;
extern int resize;
extern unsigned colorize;
extern int foreground, background;
extern enum ui_mode ui_mode;
extern int read_only;
extern const char *datefmt_str[DATE_FORMATS];
extern int days[12];
extern const char *monthnames[12];
extern const char *daynames[8];
extern char path_dir[BUFSIZ];
extern char path_todo[BUFSIZ];
extern char path_apts[BUFSIZ];
extern char path_conf[BUFSIZ];
extern char path_keys[BUFSIZ];
extern char path_notes[BUFSIZ];
extern char path_cpid[BUFSIZ];
extern char path_dpid[BUFSIZ];
extern char path_dmon_log[BUFSIZ];
extern struct conf conf;
extern struct pad apad;
extern struct nbar nbar;
extern struct dmon_conf dmon;
void vars_init(void);

/* wins.c */
extern struct window win[NBWINS];
int wins_refresh(void);
int wins_wrefresh(WINDOW *);
int wins_doupdate(void);
int wins_layout(void);
void wins_set_layout(int);
unsigned wins_sbar_width(void);
unsigned wins_sbar_wperc(void);
void wins_set_sbar_width(unsigned);
void wins_sbar_winc(void);
void wins_sbar_wdec(void);
void wins_slctd_init(void);
enum win wins_slctd(void);
void wins_slctd_set(enum win);
void wins_slctd_next(void);
void wins_init(void);
void wins_scrollwin_init(struct scrollwin *);
void wins_scrollwin_delete(struct scrollwin *);
void wins_scrollwin_display(struct scrollwin *);
void wins_scrollwin_up(struct scrollwin *, int);
void wins_scrollwin_down(struct scrollwin *, int);
void wins_reinit(void);
void wins_reinit_panels(void);
void wins_show(WINDOW *, const char *);
void wins_get_config(void);
void wins_update_border(int);
void wins_update_panels(int);
void wins_update(int);
void wins_reset(void);
void wins_prepare_external(void);
void wins_unprepare_external(void);
void wins_launch_external(const char *, const char *);
void wins_status_bar(void);
void wins_erase_status_bar(void);
void wins_other_status_page(int);
void wins_reset_status_page(void);

#endif /* CALCURSE_H */
