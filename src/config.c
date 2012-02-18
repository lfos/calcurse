/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2011 calcurse Development Team <misc@calcurse.org>
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

#include <ctype.h>
#include <unistd.h>

#include "calcurse.h"

typedef int (*config_fn_parse_t) (void *, const char *);
typedef int (*config_fn_serialize_t) (char *, void *);

struct confvar {
  const char *key;
  config_fn_parse_t fn_parse;
  config_fn_serialize_t fn_serialize;
  void *target;
};

static int config_parse_bool (unsigned *, const char *);
static int config_serialize_bool (char *, unsigned *);
static int config_parse_int (int *, const char *);
static int config_serialize_int (char *, int *);
static int config_parse_unsigned (unsigned *, const char *);
static int config_serialize_unsigned (char *, unsigned *);
static int config_parse_str (char *, const char *);
static int config_serialize_str (char *, const char *);
static int config_parse_calendar_view (void *, const char *);
static int config_serialize_calendar_view (char *, void *);
static int config_parse_first_day_of_week (void *, const char *);
static int config_serialize_first_day_of_week (char *, void *);
static int config_parse_color_theme (void *, const char *);
static int config_serialize_color_theme (char *, void *);
static int config_parse_layout (void *, const char *);
static int config_serialize_layout (char *, void *);
static int config_parse_sidebar_width (void *, const char *);
static int config_serialize_sidebar_width (char *, void *);
static int config_parse_output_datefmt (void *, const char *);
static int config_serialize_output_datefmt (char *, void *);
static int config_parse_input_datefmt (void *, const char *);
static int config_serialize_input_datefmt (char *, void *);

#define CONFIG_HANDLER_BOOL(var) (config_fn_parse_t) config_parse_bool, \
  (config_fn_serialize_t) config_serialize_bool, &(var)
#define CONFIG_HANDLER_INT(var) (config_fn_parse_t) config_parse_int, \
  (config_fn_serialize_t) config_serialize_int, &(var)
#define CONFIG_HANDLER_UNSIGNED(var) (config_fn_parse_t) config_parse_unsigned, \
  (config_fn_serialize_t) config_serialize_unsigned, &(var)
#define CONFIG_HANDLER_STR(var) (config_fn_parse_t) config_parse_str, \
  (config_fn_serialize_t) config_serialize_str, &(var)

static const struct confvar confmap[] = {
  { "auto_save", CONFIG_HANDLER_BOOL (conf.auto_save) },
  { "auto_gc", CONFIG_HANDLER_BOOL (conf.auto_gc) },
  { "periodic_save", CONFIG_HANDLER_UNSIGNED (conf.periodic_save) },
  { "confirm_quit", CONFIG_HANDLER_BOOL (conf.confirm_quit) },
  { "confirm_delete", CONFIG_HANDLER_BOOL (conf.confirm_delete) },
  { "skip_system_dialogs", CONFIG_HANDLER_BOOL (conf.skip_system_dialogs) },
  { "skip_progress_bar", CONFIG_HANDLER_BOOL (conf.skip_progress_bar) },
  { "calendar_default_view", config_parse_calendar_view, config_serialize_calendar_view, NULL },
  { "week_begins_on_monday", config_parse_first_day_of_week, config_serialize_first_day_of_week, NULL },
  { "color-theme", config_parse_color_theme, config_serialize_color_theme, NULL },
  { "layout", config_parse_layout, config_serialize_layout, NULL },
  { "side-bar_width", config_parse_sidebar_width, config_serialize_sidebar_width, NULL },
  { "notify-bar_show", CONFIG_HANDLER_BOOL (nbar.show) },
  { "notify-bar_date", CONFIG_HANDLER_STR (nbar.datefmt) },
  { "notify-bar_clock", CONFIG_HANDLER_STR (nbar.timefmt) },
  { "notify-bar_warning", CONFIG_HANDLER_INT (nbar.cntdwn) },
  { "notify-bar_command", CONFIG_HANDLER_STR (nbar.cmd) },
  { "notify-all", CONFIG_HANDLER_BOOL (nbar.notify_all) },
  { "output_datefmt", config_parse_output_datefmt, config_serialize_output_datefmt, NULL },
  { "input_datefmt", config_parse_input_datefmt, config_serialize_input_datefmt, NULL },
  { "notify-daemon_enable", CONFIG_HANDLER_BOOL (dmon.enable) },
  { "notify-daemon_log", CONFIG_HANDLER_BOOL (dmon.log) }
};

struct config_save_status {
  FILE *fp;
  int done[sizeof (confmap) / sizeof (confmap[0])];
};

typedef int (*config_fn_walk_cb_t) (const char *, const char *, void *);
typedef int (*config_fn_walk_junk_cb_t) (const char *, void *);

static int
config_parse_bool (unsigned *dest, const char *val)
{
  if (strncmp (val, "yes", 4) == 0)
    *dest = 1;
  else if (strncmp (val, "no", 3) == 0)
    *dest = 0;
  else
    return 0;

  return 1;
}

static int
config_parse_unsigned (unsigned *dest, const char *val)
{
  if (is_all_digit (val))
    *dest = atoi (val);
  else
    return 0;

  return 1;
}

static int
config_parse_int (int *dest, const char *val)
{
  if ((*val == '+' || *val == '-' || isdigit (*val)) && is_all_digit (val + 1))
    *dest = atoi (val);
  else
    return 0;

  return 1;
}

static int
config_parse_str (char *dest, const char *val)
{
  strncpy (dest, val, BUFSIZ);
  return 1;
}

static int
config_parse_color (int *dest, const char *val)
{
  if (!strcmp (val, "black"))
    *dest = COLOR_BLACK;
  else if (!strcmp (val, "red"))
    *dest = COLOR_RED;
  else if (!strcmp (val, "green"))
    *dest = COLOR_GREEN;
  else if (!strcmp (val, "yellow"))
    *dest = COLOR_YELLOW;
  else if (!strcmp (val, "blue"))
    *dest = COLOR_BLUE;
  else if (!strcmp (val, "magenta"))
    *dest = COLOR_MAGENTA;
  else if (!strcmp (val, "cyan"))
    *dest = COLOR_CYAN;
  else if (!strcmp (val, "white"))
    *dest = COLOR_WHITE;
  else if (!strcmp (val, "default"))
    *dest = background;
  else
    return 0;

  return 1;
}

static int
config_parse_color_pair (int *dest1, int *dest2, const char *val)
{
  char s1[BUFSIZ], s2[BUFSIZ];

  if (sscanf (val, "%s on %s", s1, s2) != 2)
    return 0;

  return (config_parse_color (dest1, s1) && config_parse_color (dest2, s2));
}

static int
config_parse_calendar_view (void *dummy, const char *val)
{
  calendar_set_view (atoi (val));
  return 1;
}

static int
config_parse_first_day_of_week (void *dummy, const char *val)
{
  unsigned tmp;
  if (config_parse_bool (&tmp, val)) {
    if (tmp)
      calendar_set_first_day_of_week (MONDAY);
    else
      calendar_set_first_day_of_week (SUNDAY);
    return 1;
  }
  else
    return 0;
}

static int
config_parse_color_theme (void *dummy, const char *val)
{
  int color1, color2;
  if (!config_parse_color_pair (&color1, &color2, val))
    return 0;
  init_pair (COLR_CUSTOM, color1, color2);
  return 1;
}

static int
config_parse_layout (void *dummy, const char *val)
{
  wins_set_layout (atoi (val));
  return 1;
}

static int
config_parse_sidebar_width (void *dummy, const char *val)
{
  wins_set_sbar_width (atoi (val));
  return 1;
}

static int
config_parse_output_datefmt (void *dummy, const char *val)
{
  if (val[0] != '\0')
    return config_parse_str (conf.output_datefmt, val);
  return 1;
}

static int
config_parse_input_datefmt (void *dummy, const char *val)
{
  if (config_parse_int (&conf.input_datefmt, val)) {
    if (conf.input_datefmt <= 0 || conf.input_datefmt >= DATE_FORMATS)
      conf.input_datefmt = 1;
    return 1;
  }
  else
    return 0;
}

/* Set a configuration variable. */
static int
config_set_conf (const char *key, const char *value)
{
  int i;

  if (!key)
    return -1;

  for (i = 0; i < sizeof (confmap) / sizeof (confmap[0]); i++)
    {
      if (!strcmp (confmap[i].key, key))
        return confmap[i].fn_parse (confmap[i].target, value);
    }

  return -1;
}

static int
config_serialize_bool (char *dest, unsigned *val)
{
  if (*val)
    {
      dest[0] = 'y';
      dest[1] = 'e';
      dest[2] = 's';
      dest[3] = '\0';
    }
  else
    {
      dest[0] = 'n';
      dest[1] = 'o';
      dest[2] = '\0';
    }

  return 1;
}

static int
config_serialize_unsigned (char *dest, unsigned *val)
{
  snprintf (dest, BUFSIZ, "%u", *val);
  return 1;
}

static int
config_serialize_int (char *dest, int *val)
{
  snprintf (dest, BUFSIZ, "%d", *val);
  return 1;
}

static int
config_serialize_str (char *dest, const char *val)
{
  strncpy (dest, val, BUFSIZ);
  return 1;
}

/*
 * Return a string defining the color theme in the form:
 *       foreground color 'on' background color
 * in order to dump this data in the configuration file.
 * Color numbers follow the ncurses library definitions.
 * If ncurses library was compiled with --enable-ext-funcs,
 * then default color is -1.
 */
void
config_color_theme_name (char *theme_name)
{
#define MAXCOLORS		8
#define NBCOLORS		2
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1

  int i;
  short color[NBCOLORS];
  char *color_name[NBCOLORS];
  char *default_color = "default";
  char *name[MAXCOLORS] = {
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white"
  };

  if (!colorize)
    strncpy (theme_name, "0", BUFSIZ);
  else
    {
      pair_content (COLR_CUSTOM, &color[0], &color[1]);
      for (i = 0; i < NBCOLORS; i++)
        {
          if ((color[i] == DEFAULTCOLOR) || (color[i] == DEFAULTCOLOR_EXT))
            color_name[i] = default_color;
          else if (color[i] >= 0 && color[i] <= MAXCOLORS)
            color_name[i] = name[color[i]];
          else
            {
              EXIT (_("unknown color"));
              /* NOTREACHED */
            }
        }
      snprintf (theme_name, BUFSIZ, "%s on %s", color_name[0], color_name[1]);
    }
}

static int
config_serialize_calendar_view (char *buf, void *dummy)
{
  int tmp = calendar_get_view ();
  return config_serialize_int (buf, &tmp);
}

static int
config_serialize_first_day_of_week (char *buf, void *dummy)
{
  unsigned tmp = calendar_week_begins_on_monday ();
  return config_serialize_bool (buf, &tmp);
}

static int
config_serialize_color_theme (char *buf, void *dummy)
{
  config_color_theme_name (buf);
  return 1;
}

static int
config_serialize_layout (char *buf, void *dummy)
{
  int tmp = wins_layout ();
  return config_serialize_int (buf, &tmp);
}

static int
config_serialize_sidebar_width (char *buf, void *dummy)
{
  int tmp = wins_sbar_wperc ();
  return config_serialize_int (buf, &tmp);
}

static int
config_serialize_output_datefmt (char *buf, void *dummy)
{
  return config_serialize_str (buf, conf.output_datefmt);
}

static int
config_serialize_input_datefmt (char *buf, void *dummy)
{
  return config_serialize_int (buf, &conf.input_datefmt);
}

/* Serialize the value of a configuration variable. */
static int
config_serialize_conf (char *buf, const char *key,
                       struct config_save_status *status)
{
  int i;

  if (!key)
    return -1;

  for (i = 0; i < sizeof (confmap) / sizeof (confmap[0]); i++)
    {
      if (!strcmp (confmap[i].key, key))
        {
          if (confmap[i].fn_serialize (buf, confmap[i].target))
            {
              if (status)
                status->done[i] = 1;
              return 1;
            }
          else
            return 0;
        }
    }

  return -1;
}

static void
config_file_walk (config_fn_walk_cb_t fn_cb,
                  config_fn_walk_junk_cb_t fn_junk_cb, void *data)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open config file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char buf[BUFSIZ], e_conf[BUFSIZ];
  char *key, *value;

  data_file = fopen (path_conf, "r");
  if (data_file == NULL)
    {
      status_mesg (mesg_line1, mesg_line2);
      wnoutrefresh (win[STA].p);
      wins_doupdate ();
      keys_getch (win[STA].p, NULL);
    }

  pthread_mutex_lock (&nbar.mutex);
  for (;;)
    {
      if (fgets (buf, sizeof buf, data_file) == NULL)
        break;
      io_extract_data (e_conf, buf, sizeof buf);

      if (*e_conf == '\0')
        {
          if (fn_junk_cb)
            fn_junk_cb (buf, data);
          continue;
        }

      key = e_conf;
      value = strchr (e_conf, '=');
      if (value)
        {
          *value = '\0';
          value++;
        }

      if (value && (*value == '\0' || *value == '\n'))
        {
          /* Backward compatibility mode. */
          if (fgets (buf, sizeof buf, data_file) == NULL)
            break;
          io_extract_data (e_conf, buf, sizeof buf);
          value = e_conf;
        }

      fn_cb (key, value, data);
    }
  file_close (data_file, __FILE_POS__);
  pthread_mutex_unlock (&nbar.mutex);
}

static int
config_load_cb (const char *key, const char *value, void *dummy)
{
  int result = config_set_conf (key, value);

  if (result < 0)
    EXIT (_("configuration variable unknown: \"%s\""), key);
    /* NOTREACHED */
  else if (result == 0)
    EXIT (_("wrong configuration variable format for \"%s\""), key);
    /* NOTREACHED */

  return 1;
}

/* Load the user configuration. */
void
config_load (void)
{
  config_file_walk (config_load_cb, NULL, NULL);
}

static int
config_save_cb (const char *key, const char *value, void *status)
{
  char buf[BUFSIZ];
  int result = config_serialize_conf (buf, key, (struct config_save_status *) status);

  if (result < 0)
    EXIT (_("configuration variable unknown: \"%s\""), key);
    /* NOTREACHED */
  else if (result == 0)
    EXIT (_("wrong configuration variable format for \"%s\""), key);
    /* NOTREACHED */

  fputs (key, ((struct config_save_status *) status)->fp);
  fputc ('=', ((struct config_save_status *) status)->fp);
  fputs (buf, ((struct config_save_status *) status)->fp);
  fputc ('\n', ((struct config_save_status *) status)->fp);

  return 1;
}

static int
config_save_junk_cb (const char *data, void *status)
{
  fputs (data, ((struct config_save_status *) status)->fp);
  return 1;
}

/* Save the user configuration. */
unsigned
config_save (void)
{
  char tmppath[BUFSIZ];
  char *tmpext;
  struct config_save_status status;
  int i;

  strncpy (tmppath, get_tempdir (), BUFSIZ);
  strncat (tmppath, "/" CONF_PATH_NAME ".", BUFSIZ - strlen (tmppath) - 1);
  if ((tmpext = new_tempfile (tmppath, TMPEXTSIZ)) == NULL)
    return 0;
  strncat (tmppath, tmpext, BUFSIZ - strlen (tmppath) - 1);
  mem_free (tmpext);

  status.fp = fopen (tmppath, "w");
  if (!status.fp)
    return 0;

  memset (status.done, 0, sizeof (status.done));

  config_file_walk (config_save_cb, config_save_junk_cb, (void *) &status);

  /* Set variables that were missing from the configuration file. */
  for (i = 0; i < sizeof (confmap) / sizeof (confmap[0]); i++)
    {
      if (!status.done[i])
        config_save_cb (confmap[i].key, NULL, &status);
    }

  file_close (status.fp, __FILE_POS__);

  if (io_file_cp (tmppath, path_conf))
    unlink (tmppath);

  return 1;
}
