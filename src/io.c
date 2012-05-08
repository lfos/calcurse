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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#include "calcurse.h"
#include "sha1.h"

typedef enum {
  PROGRESS_BAR_SAVE,
  PROGRESS_BAR_LOAD,
  PROGRESS_BAR_EXPORT
} progress_bar_t;

enum {
  PROGRESS_BAR_CONF,
  PROGRESS_BAR_TODO,
  PROGRESS_BAR_APTS,
  PROGRESS_BAR_KEYS
};

enum {
  PROGRESS_BAR_EXPORT_EVENTS,
  PROGRESS_BAR_EXPORT_APOINTS,
  PROGRESS_BAR_EXPORT_TODO
};

struct ht_keybindings_s {
  const char *label;
  enum key    key;
  HTABLE_ENTRY (ht_keybindings_s);
};

static void load_keys_ht_getkey (struct ht_keybindings_s *, const char **,
                                 int *);
static int load_keys_ht_compare (struct ht_keybindings_s *,
                                 struct ht_keybindings_s *);

#define HSIZE 256
HTABLE_HEAD (ht_keybindings, HSIZE, ht_keybindings_s);
HTABLE_PROTOTYPE (ht_keybindings, ht_keybindings_s)
HTABLE_GENERATE (ht_keybindings, ht_keybindings_s, load_keys_ht_getkey,
                 load_keys_ht_compare)

/* Draw a progress bar while saving, loading or exporting data. */
static void
progress_bar (progress_bar_t type, int progress)
{
#define NBFILES		4
#define NBEXPORTED      3
#define LABELENGTH      15
  int i, step, steps;
  const char *mesg_sav = _("Saving...");
  const char *mesg_load = _("Loading...");
  const char *mesg_export = _("Exporting...");
  const char *error_msg = _("Internal error while displaying progress bar");
  const char *barchar = "|";
  const char *file[NBFILES] = {
    "[    conf    ]",
    "[    todo    ]",
    "[    apts    ]",
    "[    keys    ]"
  };
  const char *data[NBEXPORTED] = {
    "[   events   ]",
    "[appointments]",
    "[    todo    ]"
  };
  int ipos =  LABELENGTH + 2;
  int epos[NBFILES];

  /* progress bar length init. */
  ipos = LABELENGTH + 2;
  steps = (type == PROGRESS_BAR_EXPORT) ? NBEXPORTED : NBFILES;
  step = floor (col / (steps + 1));
  for (i = 0; i < steps - 1; i++)
    epos[i] = (i + 2) * step;
  epos[steps - 1] = col - 2;

  switch (type)
    {
    case PROGRESS_BAR_SAVE:
      EXIT_IF (progress < 0 || progress > PROGRESS_BAR_KEYS, "%s", error_msg);
      status_mesg (mesg_sav, file[progress]);
      break;
    case PROGRESS_BAR_LOAD:
      EXIT_IF (progress < 0 || progress > PROGRESS_BAR_KEYS, "%s", error_msg);
      status_mesg (mesg_load, file[progress]);
      break;
    case PROGRESS_BAR_EXPORT:
      EXIT_IF (progress < 0
               || progress > PROGRESS_BAR_EXPORT_TODO, "%s", error_msg);
      status_mesg (mesg_export, data[progress]);
      break;
    }

  /* Draw the progress bar. */
  mvwprintw (win[STA].p, 1, ipos, barchar);
  mvwprintw (win[STA].p, 1, epos[steps - 1], barchar);
  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  for (i = ipos + 1; i < epos[progress]; i++)
    mvwaddch (win[STA].p, 1, i, ' ' | A_REVERSE);
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);
  wmove (win[STA].p, 0, 0);
  wins_wrefresh (win[STA].p);
#undef NBFILES
#undef NBEXPORTED
#undef LABELENGTH
}

/* Ask user for a file name to export data to. */
static FILE *
get_export_stream (enum export_type type)
{
  FILE *stream;
  int cancel;
  char *home, *stream_name;
  const char *question = _("Choose the file used to export calcurse data:");
  const char *wrong_name =
    _("The file cannot be accessed, please enter another file name.");
  const char *press_enter = _("Press [ENTER] to continue.");
  const char *file_ext[IO_EXPORT_NBTYPES] = {"ical", "txt"};

  stream = NULL;
  stream_name = (char *) mem_malloc (BUFSIZ);
  if ((home = getenv ("HOME")) != NULL)
    snprintf (stream_name, BUFSIZ, "%s/calcurse.%s", home, file_ext[type]);
  else
    snprintf (stream_name, BUFSIZ, "%s/calcurse.%s", get_tempdir (),
              file_ext[type]);

  while (stream == NULL)
    {
      status_mesg (question, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
        {
          mem_free (stream_name);
          return NULL;
        }
      stream = fopen (stream_name, "w");
      if (stream == NULL)
        {
          status_mesg (wrong_name, press_enter);
          wgetch (win[STA].p);
        }
    }
  mem_free (stream_name);

  return stream;
}

/* Append a line to a file. */
unsigned
io_fprintln (const char *fname, const char *fmt, ...)
{
  FILE *fp;
  va_list ap;
  char buf[BUFSIZ];
  int ret;

  fp = fopen (fname, "a");
  RETVAL_IF (!fp, 0, _("Failed to open \"%s\", - %s\n"),
             fname, strerror (errno));

  va_start (ap, fmt);
  ret = vsnprintf (buf, sizeof buf, fmt, ap);
  RETVAL_IF (ret < 0, 0, _("Failed to build message\n"));
  va_end (ap);

  ret = fprintf (fp, "%s", buf);
  RETVAL_IF (ret < 0, 0, _("Failed to print message \"%s\"\n"), buf);

  ret = fclose (fp);
  RETVAL_IF (ret != 0, 0, _("Failed to close \"%s\" - %s\n"),
             fname, strerror (errno));

  return 1;
}

/*
 * Initialization of data paths. The cfile argument is the variable
 * which contains the calendar file. If none is given, then the default
 * one (~/.calcurse/apts) is taken. If the one given does not exist, it
 * is created.
 * The datadir argument can be use to specify an alternative data root dir.
 */
void
io_init (const char *cfile, const char *datadir)
{
  FILE *data_file;
  const char *home;
  char apts_file[BUFSIZ] = "";
  int ch;

  if (datadir != NULL)
    {
      home = datadir;
      snprintf (path_dir, BUFSIZ, "%s", home);
      snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH_NAME, home);
      snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH_NAME, home);
      snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR_NAME, home);
      snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH_NAME, home);
      snprintf (path_keys, BUFSIZ, "%s/" KEYS_PATH_NAME, home);
      snprintf (path_cpid, BUFSIZ, "%s/" CPID_PATH_NAME, home);
      snprintf (path_dpid, BUFSIZ, "%s/" DPID_PATH_NAME, home);
      snprintf (path_dmon_log, BUFSIZ, "%s/" DLOG_PATH_NAME, home);
    }
  else
    {
      home = getenv ("HOME");
      if (home == NULL)
        {
          home = ".";
        }
      snprintf (path_dir, BUFSIZ, "%s/" DIR_NAME, home);
      snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH, home);
      snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH, home);
      snprintf (path_keys, BUFSIZ, "%s/" KEYS_PATH, home);
      snprintf (path_cpid, BUFSIZ, "%s/" CPID_PATH, home);
      snprintf (path_dpid, BUFSIZ, "%s/" DPID_PATH, home);
      snprintf (path_dmon_log, BUFSIZ, "%s/" DLOG_PATH, home);
      snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR, home);
      if (cfile == NULL)
        {
          snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH, home);
        }
      else
        {
          snprintf (apts_file, BUFSIZ, "%s", cfile);
          strncpy (path_apts, apts_file, BUFSIZ);
          /* check if the file exists, otherwise create it */
          data_file = fopen (path_apts, "r");
          if (data_file == NULL)
            {
              printf (_("%s does not exist, create it now [y or n] ? "),
                      path_apts);
              ch = getchar ();
              switch (ch)
                {
                case 'N':
                case 'n':
                  puts (_("aborting...\n"));
                  exit_calcurse (EXIT_FAILURE);
                  break;

                case 'Y':
                case 'y':
                  data_file = fopen (path_apts, "w");
                  if (data_file == NULL)
                    {
                      perror (path_apts);
                      exit_calcurse (EXIT_FAILURE);
                    }
                  else
                    {
                      printf (_("%s successfully created\n"), path_apts);
                      puts (_("starting interactive mode...\n"));
                    }
                  break;

                default:
                  puts (_("aborting...\n"));
                  exit_calcurse (EXIT_FAILURE);
                  break;
                }
            }
          file_close (data_file, __FILE_POS__);
        }
    }
}

void
io_extract_data (char *dst_data, const char *org, int len)
{
  int i;

  for (; *org == ' ' || *org == '\t'; org++);
  for (i = 0; i < len - 1; i++)
    {
      if (*org == '\n' || *org == '\0' || *org == '#')
        break;
      *dst_data++ = *org++;
    }
  *dst_data = '\0';
}

static void
display_mark (void)
{
  const int DISPLAY_TIME = 1;
  WINDOW *mwin;

  mwin = newwin (1, 2, 1, col - 3);

  custom_apply_attr (mwin, ATTR_HIGHEST);
  mvwprintw (mwin, 0, 0, "**");
  wins_wrefresh (mwin);
  sleep (DISPLAY_TIME);
  mvwprintw (mwin, 0, 0, "  ");
  wins_wrefresh (mwin);
  delwin (mwin);
  wins_doupdate ();
}

static pthread_mutex_t io_save_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Save the apts data file, which contains the
 * appointments first, and then the events.
 * Recursive items are written first.
 */
unsigned
io_save_apts (void)
{
  llist_item_t *i;
  FILE *fp;

  if (read_only)
    return 1;

  if ((fp = fopen (path_apts, "w")) == NULL)
    return 0;

  recur_save_data (fp);

  if (ui_mode == UI_CURSES)
    LLIST_TS_LOCK (&alist_p);
  LLIST_TS_FOREACH (&alist_p, i)
    {
      struct apoint *apt = LLIST_TS_GET_DATA (i);
      apoint_write (apt, fp);
    }
  if (ui_mode == UI_CURSES)
    LLIST_TS_UNLOCK (&alist_p);

  LLIST_FOREACH (&eventlist, i)
    {
      struct event *ev = LLIST_TS_GET_DATA (i);
      event_write (ev, fp);
    }
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save the todo data file. */
unsigned
io_save_todo (void)
{
  llist_item_t *i;
  FILE *fp;

  if (read_only)
    return 1;

  if ((fp = fopen (path_todo, "w")) == NULL)
    return 0;

  LLIST_FOREACH (&todolist, i)
    {
      struct todo *todo = LLIST_TS_GET_DATA (i);
      todo_write (todo, fp);
    }
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save user-defined keys */
unsigned
io_save_keys (void)
{
  FILE *fp;

  if (read_only)
    return 1;

  if ((fp = fopen (path_keys, "w")) == NULL)
    return 0;

  keys_save_bindings (fp);
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save the calendar data */
void
io_save_cal (enum save_display display)
{
  const char *access_pb = _("Problems accessing data file ...");
  const char *save_success = _("The data files were successfully saved");
  const char *enter = _("Press [ENTER] to continue");
  int show_bar;

  if (read_only)
    return;

  pthread_mutex_lock (&io_save_mutex);

  show_bar = 0;
  if (ui_mode == UI_CURSES && display == IO_SAVE_DISPLAY_BAR
      && conf.progress_bar)
    show_bar = 1;
  else if (ui_mode == UI_CURSES && display == IO_SAVE_DISPLAY_MARK)
    display_mark ();

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_CONF);
  if (!config_save ())
    ERROR_MSG ("%s", access_pb);

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_TODO);
  if (!io_save_todo ())
    ERROR_MSG ("%s", access_pb);

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_APTS);
  if (!io_save_apts ())
    ERROR_MSG ("%s", access_pb);

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_KEYS);
  if (!io_save_keys ())
    ERROR_MSG ("%s", access_pb);

  /* Print a message telling data were saved */
  if (ui_mode == UI_CURSES && conf.system_dialogs
      && display != IO_SAVE_DISPLAY_MARK)
    {
      status_mesg (save_success, enter);
      wgetch (win[STA].p);
    }

  pthread_mutex_unlock (&io_save_mutex);
}

/*
 * Check what type of data is written in the appointment file,
 * and then load either: a new appointment, a new event, or a new
 * recursive item (which can also be either an event or an appointment).
 */
void
io_load_app (void)
{
  FILE *data_file;
  int c, is_appointment, is_event, is_recursive;
  struct tm start, end, until, *lt;
  llist_t exc;
  time_t t;
  int id = 0;
  int freq;
  char type, state = 0L;
  char note[MAX_NOTESIZ + 1], *notep;

  t = time (NULL);
  lt = localtime (&t);
  start = end = until = *lt;

  data_file = fopen (path_apts, "r");
  EXIT_IF (data_file == NULL, _("failed to open appointment file"));

  for (;;)
    {
      LLIST_INIT (&exc);
      is_appointment = is_event = is_recursive = 0;
      c = getc (data_file);
      if (c == EOF)
        break;
      ungetc (c, data_file);

      /* Read the date first: it is common to both events
       * and appointments.
       */
      if (fscanf (data_file, "%d / %d / %d ",
                  &start.tm_mon, &start.tm_mday, &start.tm_year) != 3)
        EXIT (_("syntax error in the item date"));

      /* Read the next character : if it is an '@' then we have
       * an appointment, else if it is an '[' we have en event.
       */
      c = getc (data_file);

      if (c == '@')
        is_appointment = 1;
      else if (c == '[')
        is_event = 1;
      else
        EXIT (_("no event nor appointment found"));

      /* Read the remaining informations. */
      if (is_appointment)
        {
          if (fscanf (data_file, " %d : %d -> %d / %d / %d @ %d : %d ",
                      &start.tm_hour, &start.tm_min,
                      &end.tm_mon, &end.tm_mday, &end.tm_year,
                      &end.tm_hour, &end.tm_min) != 7)
            EXIT (_("syntax error in item time or duration"));
        }
      else if (is_event)
        {
          if (fscanf (data_file, " %d ", &id) != 1 || getc (data_file) != ']')
            EXIT (_("syntax error in item identifier"));
          while ((c = getc (data_file)) == ' ');
          ungetc (c, data_file);
        }
      else
        {
          EXIT (_("wrong format in the appointment or event"));
          /* NOTREACHED */
        }

      /* Check if we have a recursive item. */
      c = getc (data_file);

      if (c == '{')
        {
          is_recursive = 1;
          if (fscanf (data_file, " %d%c ", &freq, &type) != 2)
            EXIT (_("syntax error in item repetition"));

          c = getc (data_file);
          if (c == '}')
            {			/* endless recurrent item */
              until.tm_year = 0;
              while ((c = getc (data_file)) == ' ');
              ungetc (c, data_file);
            }
          else if (c == '-' && getc (data_file) == '>')
            {
              if (fscanf (data_file, " %d / %d / %d ", &until.tm_mon,
                          &until.tm_mday, &until.tm_year) != 3)
                EXIT (_("syntax error in item repetition"));
              c = getc (data_file);
              if (c == '!')
                {
                  ungetc (c, data_file);
                  recur_exc_scan (&exc, data_file);
                  c = getc (data_file);
                }
              else if (c == '}')
                {
                  while ((c = getc (data_file)) == ' ');
                  ungetc (c, data_file);
                }
              else
                EXIT (_("syntax error in item repetition"));
            }
          else if (c == '!')
            {			/* endless item with exceptions */
              ungetc (c, data_file);
              recur_exc_scan (&exc, data_file);
              c = getc (data_file);
              until.tm_year = 0;
            }
          else
            {
              EXIT (_("wrong format in the appointment or event"));
              /* NOTREACHED */
            }
        }
      else
        ungetc (c, data_file);

      /* Check if a note is attached to the item. */
      c = getc (data_file);
      if (c == '>')
        {
          note_read (note, data_file);
          notep = note;
        }
      else
        {
          notep = NULL;
          ungetc (c, data_file);
        }

      /*
       * Last: read the item description and load it into its
       * corresponding linked list, depending on the item type.
       */
      if (is_appointment)
        {
          c = getc (data_file);
          if (c == '!')
            {
              state |= APOINT_NOTIFY;
              while ((c = getc (data_file)) == ' ');
              ungetc (c, data_file);
            }
          else if (c == '|')
            {
              state = 0L;
              while ((c = getc (data_file)) == ' ');
              ungetc (c, data_file);
            }
          else
            EXIT (_("syntax error in item repetition"));
          if (is_recursive)
            {
              recur_apoint_scan (data_file, start, end,
                                 type, freq, until, notep, &exc, state);
            }
          else
            {
              apoint_scan (data_file, start, end, state, notep);
            }
        }
      else if (is_event)
        {
          if (is_recursive)
            {
              recur_event_scan (data_file, start, id, type,
                                freq, until, notep, &exc);
            }
          else
            {
              event_scan (data_file, start, id, notep);
            }
        }
      else
        {
          EXIT (_("wrong format in the appointment or event"));
          /* NOTREACHED */
        }
    }
  file_close (data_file, __FILE_POS__);
}

/* Load the todo data */
void
io_load_todo (void)
{
  FILE *data_file;
  char *newline;
  int nb_tod = 0;
  int c, id;
  char buf[BUFSIZ], e_todo[BUFSIZ], note[MAX_NOTESIZ + 1];

  data_file = fopen (path_todo, "r");
  EXIT_IF (data_file == NULL, _("failed to open todo file"));

  for (;;)
    {
      c = getc (data_file);
      if (c == EOF)
        break;
      else if (c == '[')
        {			/* new style with id */
          if (fscanf (data_file, " %d ", &id) != 1 || getc (data_file) != ']')
            EXIT (_("syntax error in item identifier"));
          while ((c = getc (data_file)) == ' ');
          ungetc (c, data_file);
        }
      else
        {
          id = 9;
          ungetc (c, data_file);
        }
      /* Now read the attached note, if any. */
      c = getc (data_file);
      if (c == '>')
        note_read (note, data_file);
      else
        {
          note[0] = '\0';
          ungetc (c, data_file);
        }
      /* Then read todo description. */
      if (!fgets (buf, sizeof buf, data_file))
        buf[0] = '\0';
      newline = strchr (buf, '\n');
      if (newline)
        *newline = '\0';
      io_extract_data (e_todo, buf, sizeof buf);
      todo_add (e_todo, id, note);
      ++nb_tod;
    }
  file_close (data_file, __FILE_POS__);
  todo_set_nb (nb_tod);
}

static void
load_keys_ht_getkey (struct ht_keybindings_s *data, const char **key, int *len)
{
  *key = data->label;
  *len = strlen (data->label);
}

static int
load_keys_ht_compare (struct ht_keybindings_s *data1,
                      struct ht_keybindings_s *data2)
{
  const int KEYLEN = strlen (data1->label);

  if (strlen (data2->label) == KEYLEN
      && !memcmp (data1->label, data2->label, KEYLEN))
    return 0;
  else
    return 1;
}

/*
 * isblank(3) is protected by the __BSD_VISIBLE macro and this fails to be
 * visible in some specific cases. Thus replace it by the following is_blank()
 * function.
 */
static int is_blank (int c)
{
  return c == ' ' || c == '\t';
}

/*
 * Load user-definable keys from file.
 * A hash table is used to speed up loading process in avoiding string
 * comparisons.
 * A log file is also built in case some errors were found in the key
 * configuration file.
 */
void
io_load_keys (const char *pager)
{
  struct ht_keybindings_s keys[NBKEYS];
  FILE *keyfp;
  char buf[BUFSIZ];
  struct io_file *log;
  int i, skipped, loaded, line;
  const int MAX_ERRORS = 5;

  keys_init ();

  struct ht_keybindings ht_keys = HTABLE_INITIALIZER (&ht_keys);

  for (i = 0; i < NBKEYS; i++)
    {
      keys[i].key = (enum key)i;
      keys[i].label = keys_get_label ((enum key)i);
      HTABLE_INSERT (ht_keybindings, &ht_keys, &keys[i]);
    }

  keyfp = fopen (path_keys, "r");
  EXIT_IF (keyfp == NULL, _("failed to open key file"));

  log = io_log_init ();
  skipped = loaded = line = 0;
  while (fgets (buf, BUFSIZ, keyfp) != NULL)
    {
      char key_label[BUFSIZ], *p;
      struct ht_keybindings_s *ht_elm, ht_entry;
      const int AWAITED = 1;
      int assigned;

      line++;
      if (skipped > MAX_ERRORS)
        {
          const char *too_many =
            _("\nToo many errors while reading configuration file!\n"
              "Please backup your keys file, remove it from directory, "
              "and launch calcurse again.\n");

          io_log_print (log, line, too_many);
          break;
        }
      for (p = buf; is_blank ((int)*p); p++)
        ;
      if (p != buf)
        memmove (buf, p, strlen (p));
      if (buf[0] == '#' || buf[0] == '\n')
        continue;

      if (sscanf (buf, "%s", key_label) != AWAITED)
        {
          skipped++;
          io_log_print (log, line, _("Could not read key label"));
          continue;
        }
      ht_entry.label = key_label;
      p = buf + strlen (key_label) + 1;
      ht_elm = HTABLE_LOOKUP (ht_keybindings, &ht_keys, &ht_entry);
      if (!ht_elm)
        {
          skipped++;
          io_log_print (log, line, _("Key label not recognized"));
          continue;
        }
      assigned = 0;
      for (;;)
        {
          char key_ch[BUFSIZ], tmpbuf[BUFSIZ];

          while (*p == ' ')
            p++;
          (void)strncpy (tmpbuf, p, BUFSIZ);
          if (sscanf (tmpbuf, "%s", key_ch) == AWAITED)
            {
              int ch;

              if ((ch = keys_str2int (key_ch)) < 0)
                {
                  char unknown_key[BUFSIZ];

                  skipped++;
                  (void)snprintf (unknown_key, BUFSIZ,
                                  _("Error reading key: \"%s\""), key_ch);
                  io_log_print (log, line, unknown_key);
                }
              else
                {
                  int used;

                  used = keys_assign_binding (ch, ht_elm->key);
                  if (used)
                    {
                      char already_assigned[BUFSIZ];

                      skipped++;
                      (void)snprintf (already_assigned, BUFSIZ,
                                _("\"%s\" assigned multiple times!"), key_ch);
                      io_log_print (log, line, already_assigned);
                    }
                  else
                    assigned++;
                }
              p += strlen (key_ch) + 1;
            }
          else
            {
              if (assigned)
                loaded++;
              break;
            }
        }
    }
  file_close (keyfp, __FILE_POS__);
  file_close (log->fd, __FILE_POS__);
  if (skipped > 0)
    {
      const char *view_log =
        _("There were some errors when loading keys file, see log file ?");

      io_log_display (log, view_log, pager);
    }
  io_log_free (log);
  EXIT_IF (skipped > MAX_ERRORS,
           _("Too many errors while reading keys file, aborting..."));
  if (loaded < NBKEYS)
    keys_fill_missing ();
  if (keys_check_missing_bindings ())
    WARN_MSG (_("Some actions do not have any associated key bindings!"));
}

void
io_check_dir (char *dir, int *missing)
{
  if (read_only)
    return;

  errno = 0;
  if (mkdir (dir, 0700) != 0)
    {
      if (errno != EEXIST)
        {
          fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"), dir,
                   strerror (errno));
          exit_calcurse (EXIT_FAILURE);
        }
    }
  else
    {
      if (missing)
        (*missing)++;
    }
}

unsigned
io_file_exist (char *file)
{
  FILE *fd;

  if (!file)
    return 0;

  if ((fd = fopen (file, "r")) == NULL)
    return 0;

  fclose (fd);

  return 1;
}

void
io_check_file (char *file, int *missing)
{
  if (read_only)
    return;

  errno = 0;
  if (!io_file_exist (file))
    {
      FILE *fd;

      if (missing)
        (*missing)++;
      if ((fd = fopen (file, "w")) == NULL)
        {
          fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"), file,
                   strerror (errno));
          exit_calcurse (EXIT_FAILURE);
        }
      file_close (fd, __FILE_POS__);
    }
}

/*
 * Checks if data files exist. If not, create them.
 * The following structure has to be created:
 *
 *	$HOME/.calcurse/
 *                 |
 *                 +--- notes/
 *                 |___ conf
 *                 |___ keys
 *                 |___ apts
 *                 |___ todo
 */
int
io_check_data_files (void)
{
  int missing, missing_keys;

  missing = missing_keys = 0;
  errno = 0;
  io_check_dir (path_dir, &missing);
  io_check_dir (path_notes, &missing);
  io_check_file (path_todo, &missing);
  io_check_file (path_apts, &missing);
  io_check_file (path_conf, &missing);
  io_check_file (path_keys, &missing_keys);
  if (missing_keys)
    {
      missing++;
      keys_dump_defaults (path_keys);
    }

  return missing;
}

/* Draw the startup screen */
void
io_startup_screen (int no_data_file)
{
  const char *enter = _("Press [ENTER] to continue");

  if (no_data_file)
    status_mesg (_("Data files found. Data will be loaded now."), enter);
  else
    status_mesg (_("Welcome to Calcurse. Missing data files were created."),
                 enter);

  wgetch (win[STA].p);
}

/* Export calcurse data. */
void
io_export_data (enum export_type type)
{
  FILE *stream;
  const char *success = _("The data were successfully exported");
  const char *enter = _("Press [ENTER] to continue");

  if (type < IO_EXPORT_ICAL || type >= IO_EXPORT_NBTYPES)
    EXIT (_("unknown export type"));

  stream = 0;
  switch (ui_mode)
    {
    case UI_CMDLINE:
      stream = stdout;
      break;
    case UI_CURSES:
      stream = get_export_stream (type);
      break;
    default:
      EXIT (_("wrong export mode"));
      /* NOTREACHED */
    }

  if (stream == NULL)
    return;

  if (type == IO_EXPORT_ICAL)
    ical_export_data (stream);
  else if (type == IO_EXPORT_PCAL)
    pcal_export_data (stream);

  if (conf.system_dialogs && ui_mode == UI_CURSES)
    {
      status_mesg (success, enter);
      wgetch (win[STA].p);
    }
}

/* Draws the export format selection bar */
void
io_export_bar (void)
{
  int smlspc, spc;

  smlspc = 2;
  spc = 15;

  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  mvwprintw (win[STA].p, 0, 2, "Q");
  mvwprintw (win[STA].p, 1, 2, "I");
  mvwprintw (win[STA].p, 0, 2 + spc, "P");
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);

  mvwprintw (win[STA].p, 0, 2 + smlspc, _("Exit"));
  mvwprintw (win[STA].p, 1, 2 + smlspc, _("Ical"));
  mvwprintw (win[STA].p, 0, 2 + spc + smlspc, _("Pcal"));

  wnoutrefresh (win[STA].p);
  wmove (win[STA].p, 0, 0);
  wins_doupdate ();
}

static FILE *
get_import_stream (enum export_type type)
{
  FILE *stream;
  char *stream_name;
  const char *ask_fname = _("Enter the file name to import data from:");
  const char *wrong_file =
    _("The file cannot be accessed, please enter another file name.");
  const char *press_enter = _("Press [ENTER] to continue.");
  int cancel;

  stream = NULL;
  stream_name = mem_malloc (BUFSIZ);
  memset (stream_name, 0, BUFSIZ);
  while (stream == NULL)
    {
      status_mesg (ask_fname, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
        {
          mem_free (stream_name);
          return NULL;
        }
      stream = fopen (stream_name, "r");
      if (stream == NULL)
        {
          status_mesg (wrong_file, press_enter);
          wgetch (win[STA].p);
        }
    }
  mem_free (stream_name);

  return stream;
}

/*
 * Import data from a given stream (either stdin in non-interactive mode, or the
 * user given file in interactive mode).
 * A temporary log file is created in /tmp to store the import process report,
 * and is cleared at the end.
 */
void
io_import_data (enum import_type type, const char *stream_name)
{
  const char *proc_report = _("Import process report: %04d lines read ");
  char stats_str[4][BUFSIZ];
  FILE *stream = NULL;
  struct io_file *log;
  struct {
    unsigned events, apoints, todos, lines, skipped;
  } stats;

  EXIT_IF (type < 0 || type >= IO_IMPORT_NBTYPES, _("unknown import type"));
  switch (ui_mode)
    {
    case UI_CMDLINE:
      stream = fopen (stream_name, "r");
      EXIT_IF (stream == NULL,
               _("FATAL ERROR: the input file cannot be accessed, "
                 "Aborting..."));
      break;
    case UI_CURSES:
      stream = get_import_stream (type);
      break;
    default:
      EXIT (_("FATAL ERROR: wrong import mode"));
      /* NOTREACHED */
    }

  if (stream == NULL)
    return;

  memset (&stats, 0, sizeof stats);

  log = io_log_init ();
  if (log == NULL)
    {
      if (stream != stdin)
        file_close (stream, __FILE_POS__);
      return;
    }

  if (type == IO_IMPORT_ICAL)
    ical_import_data (stream, log->fd, &stats.events, &stats.apoints,
                      &stats.todos, &stats.lines, &stats.skipped);

  if (stream != stdin)
    file_close (stream, __FILE_POS__);

  snprintf (stats_str[0], BUFSIZ,
            ngettext ("%d app", "%d apps", stats.apoints), stats.apoints);
  snprintf (stats_str[1], BUFSIZ,
            ngettext ("%d event", "%d events", stats.events), stats.events);
  snprintf (stats_str[2], BUFSIZ,
            ngettext ("%d todo", "%d todos", stats.todos), stats.todos);
  snprintf (stats_str[3], BUFSIZ, _("%d skipped"), stats.skipped);

  /* Update the number of todo items. */
  todo_set_nb (todo_nb () + stats.todos);

  if (ui_mode == UI_CURSES && conf.system_dialogs)
    {
      char read[BUFSIZ], stat[BUFSIZ];

      snprintf (read, BUFSIZ, proc_report, stats.lines);
      snprintf (stat, BUFSIZ, "%s / %s / %s / %s (%s)", stats_str[0],
                stats_str[1], stats_str[2], stats_str[3],
                _("Press [ENTER] to continue"));
      status_mesg (read, stat);
      wgetch (win[STA].p);
    }
  else if (ui_mode == UI_CMDLINE)
    {
      printf (proc_report, stats.lines);
      printf ("\n%s / %s / %s / %s\n", stats_str[0], stats_str[1],
              stats_str[2], stats_str[3]);
    }

  /* User has the choice to look at the log file if some items could not be
     imported.
  */
  file_close (log->fd, __FILE_POS__);
  if (stats.skipped > 0)
    {
      const char *view_log = _("Some items could not be imported, see log file ?");

      io_log_display (log, view_log, conf.pager);
    }
  io_log_free (log);
}

struct io_file *
io_log_init (void)
{
  char logprefix[BUFSIZ];
  char *logname;
  struct io_file *log;

  snprintf (logprefix, BUFSIZ, "%s/calcurse_log.", get_tempdir ());
  logname = new_tempfile (logprefix, TMPEXTSIZ);
  RETVAL_IF (logname == NULL, 0,
             _("Warning: could not create temporary log file, Aborting..."));
  log = mem_malloc (sizeof (struct io_file));
  RETVAL_IF (log == NULL, 0,
             _("Warning: could not open temporary log file, Aborting..."));
  snprintf (log->name, sizeof (log->name), "%s%s", logprefix, logname);
  mem_free (logname);
  log->fd = fopen (log->name, "w");
  if (log->fd == NULL)
    {
      ERROR_MSG (_("Warning: could not open temporary log file, Aborting..."));
      mem_free (log);
      return 0;
    }

  return log;
}

void
io_log_print (struct io_file *log, int line, const char *msg)
{
  if (log && log->fd)
    fprintf (log->fd, "line %d: %s\n", line, msg);
}

void
io_log_display (struct io_file *log, const char *msg, const char *pager)
{
  int ans;

  RETURN_IF (log == NULL, _("No log file to display!"));
  if (ui_mode == UI_CMDLINE)
    {
      printf ("\n%s [y/n] ", msg);
      ans = fgetc (stdin);
      if (ans == 'y')
        {
          const char *arg[] = { pager, log->name, NULL };
          int pid;

          if ((pid = fork_exec (NULL, NULL, pager, arg)))
            child_wait (NULL, NULL, pid);
        }
    }
  else
    {
      status_mesg_yesno (msg);
      do
        {
          ans = wgetch (win[STA].p);
          if (ans == 'y')
            {
              wins_launch_external (log->name, pager);
            }
        }
      while (ans != 'y' && ans != 'n');
      wins_erase_status_bar ();
    }
}

void
io_log_free (struct io_file *log)
{
  if (!log)
    return;
  EXIT_IF (unlink (log->name) != 0,
           _("Warning: could not erase temporary log file %s, Aborting..."),
           log->name);
  mem_free (log);
}

static pthread_t io_t_psave;

/* Thread used to periodically save data. */
static void *
io_psave_thread (void *arg)
{
  int delay;

  delay = conf.periodic_save;
  EXIT_IF (delay < 0, _("Invalid delay"));

  for (;;)
    {
      sleep (delay * MININSEC);
      io_save_cal (IO_SAVE_DISPLAY_MARK);
    }
}

/* Launch the thread which handles periodic saves. */
void
io_start_psave_thread (void)
{
  pthread_create (&io_t_psave, NULL, io_psave_thread, NULL);
}

/* Stop periodic data saves. */
void
io_stop_psave_thread (void)
{
  if (io_t_psave)
    {
      pthread_cancel (io_t_psave);
      pthread_join (io_t_psave, NULL);
    }
}

/*
 * This sets a lock file to prevent from having two different instances of
 * calcurse running.
 *
 * If the lock cannot be obtained, then warn the user and exit calcurse. Else,
 * create a .calcurse.pid file in the user defined directory, which will be
 * removed when calcurse exits.
 *
 * Note: When creating the lock file, the interactive mode is not initialized
 * yet.
 */
void
io_set_lock (void)
{
  FILE *lock = fopen (path_cpid, "r");
  int pid;

  if (lock != NULL)
    {
      /* If there is a lock file, check whether the process exists. */
      if (fscanf(lock, "%d", &pid) == 1)
        {
          fclose(lock);
          if (kill(pid, 0) != 0 && errno == ESRCH)
            lock = NULL;
        }
      else
        fclose(lock);
    }

  if (lock != NULL)
    {
      fprintf (stderr,
               _("\nWARNING: it seems that another calcurse instance is "
                 "already running.\n"
                 "If this is not the case, please remove the following "
                 "lock file: \n\"%s\"\n"
                 "and restart calcurse.\n"), path_cpid);
      exit (EXIT_FAILURE);
    }
  else
    {
      if (!io_dump_pid (path_cpid))
        EXIT (_("FATAL ERROR: could not create %s: %s\n"),
                         path_cpid, strerror (errno));
    }
}

/*
 * Create a new file and write the process pid inside (used to create a simple
 * lock for example). Overwrite already existing files.
 */
unsigned
io_dump_pid (char *file)
{
  pid_t pid;
  FILE *fp;

  if (!file)
    return 0;

  pid = getpid ();
  if (!(fp = fopen (file, "w"))
      || fprintf (fp, "%ld\n", (long)pid) < 0
      || fclose (fp) != 0)
    return 0;

  return 1;
}

/*
 * Return the pid number contained in a file previously created with
 * io_dump_pid ().
 * If no file was found, return 0.
 */
unsigned
io_get_pid (char *file)
{
  FILE *fp;
  unsigned pid;

  if (!file)
    return 0;

  if ((fp = fopen (file, "r")) == NULL)
    return 0;

  if (fscanf (fp, "%u", &pid) != 1)
    return 0;

  fclose (fp);

  return pid;
}

/*
 * Check whether a file is empty.
 */
int
io_file_is_empty (char *file)
{
  FILE *fp;

  if (file && (fp = fopen (file, "r")))
    {
      if ((fgetc (fp) == '\n' && fgetc (fp) == EOF) || feof (fp))
        {
          fclose (fp);
          return 1;
        }
      else
        {
          fclose (fp);
          return 0;
        }
    }

  return -1;
}

/*
 * Copy an existing file to a new location.
 */
int
io_file_cp (const char *src, const char *dst)
{
  FILE *fp_src, *fp_dst;
  char *buffer[BUFSIZ];
  unsigned int bytes_read;

  if (!(fp_src = fopen (src, "rb")))
    return 0;
  if (!(fp_dst = fopen (dst, "wb")))
    return 0;

  while (!feof (fp_src))
    {
      bytes_read = fread (buffer, 1, BUFSIZ, fp_src);
      if (bytes_read > 0)
        {
          if (fwrite (buffer, 1, bytes_read, fp_dst) != bytes_read)
            return 0;
        }
      else
        return 0;
    }

  fclose (fp_dst);
  fclose (fp_src);

  return 1;
}
