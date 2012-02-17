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
  if (!key)
    return -1;

  if (!strcmp(key, "auto_save"))
    return config_parse_bool (&conf.auto_save, value);

  if (!strcmp(key, "auto_gc"))
    return config_parse_bool (&conf.auto_gc, value);

  if (!strcmp(key, "periodic_save"))
    return config_parse_unsigned (&conf.periodic_save, value);

  if (!strcmp(key, "confirm_quit"))
    return config_parse_bool (&conf.confirm_quit, value);

  if (!strcmp(key, "confirm_delete"))
    return config_parse_bool (&conf.confirm_delete, value);

  if (!strcmp(key, "skip_system_dialogs"))
    return config_parse_bool (&conf.skip_system_dialogs, value);

  if (!strcmp(key, "skip_progress_bar"))
    return config_parse_bool (&conf.skip_progress_bar, value);

  if (!strcmp(key, "calendar_default_view"))
    return config_parse_calendar_view (NULL, value);

  if (!strcmp(key, "week_begins_on_monday"))
    return config_parse_first_day_of_week (NULL, value);

  if (!strcmp(key, "color-theme"))
    return config_parse_color_theme (NULL, value);

  if (!strcmp(key, "layout"))
    return config_parse_layout (NULL, value);

  if (!strcmp(key, "side-bar_width"))
    return config_parse_sidebar_width (NULL, value);

  if (!strcmp(key, "notify-bar_show"))
    return config_parse_bool (&nbar.show, value);

  if (!strcmp(key, "notify-bar_date"))
    return config_parse_str (nbar.datefmt, value);

  if (!strcmp(key, "notify-bar_clock"))
    return config_parse_str (nbar.timefmt, value);

  if (!strcmp(key, "notify-bar_warning"))
    return config_parse_int (&nbar.cntdwn, value);

  if (!strcmp(key, "notify-bar_command"))
    return config_parse_str (nbar.cmd, value);

  if (!strcmp(key, "notify-all"))
    return config_parse_bool(&nbar.notify_all, value);

  if (!strcmp(key, "output_datefmt"))
    return config_parse_output_datefmt (NULL, value);

  if (!strcmp(key, "input_datefmt"))
    return config_parse_input_datefmt (NULL, value);

  if (!strcmp(key, "notify-daemon_enable"))
    return config_parse_bool (&dmon.enable, value);

  if (!strcmp(key, "notify-daemon_log"))
    return config_parse_bool (&dmon.log, value);

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
config_serialize_conf (char *buf, const char *key)
{
  if (!key)
    return -1;

  if (!strcmp(key, "auto_save"))
    return config_serialize_bool (buf, &conf.auto_save);

  if (!strcmp(key, "auto_gc"))
    return config_serialize_bool (buf, &conf.auto_gc);

  if (!strcmp(key, "periodic_save"))
    return config_serialize_unsigned (buf, &conf.periodic_save);

  if (!strcmp(key, "confirm_quit"))
    return config_serialize_bool (buf, &conf.confirm_quit);

  if (!strcmp(key, "confirm_delete"))
    return config_serialize_bool (buf, &conf.confirm_delete);

  if (!strcmp(key, "skip_system_dialogs"))
    return config_serialize_bool (buf, &conf.skip_system_dialogs);

  if (!strcmp(key, "skip_progress_bar"))
    return config_serialize_bool (buf, &conf.skip_progress_bar);

  if (!strcmp(key, "calendar_default_view"))
    return config_serialize_calendar_view (buf, NULL);

  if (!strcmp(key, "week_begins_on_monday"))
    return config_serialize_first_day_of_week (buf, NULL);

  if (!strcmp(key, "color-theme"))
    return config_serialize_color_theme (buf, NULL);

  if (!strcmp(key, "layout"))
    return config_serialize_layout (buf, NULL);

  if (!strcmp(key, "side-bar_width"))
    return config_serialize_sidebar_width (buf, NULL);

  if (!strcmp(key, "notify-bar_show"))
    return config_serialize_bool (buf, &nbar.show);

  if (!strcmp(key, "notify-bar_date"))
    return config_serialize_str (buf, nbar.datefmt);

  if (!strcmp(key, "notify-bar_clock"))
    return config_serialize_str (buf, nbar.timefmt);

  if (!strcmp(key, "notify-bar_warning"))
    return config_serialize_int (buf, &nbar.cntdwn);

  if (!strcmp(key, "notify-bar_command"))
    return config_serialize_str (buf, nbar.cmd);

  if (!strcmp(key, "notify-all"))
    return config_serialize_bool (buf, &nbar.notify_all);

  if (!strcmp(key, "output_datefmt"))
    return config_serialize_output_datefmt (buf, NULL);

  if (!strcmp(key, "input_datefmt"))
    return config_serialize_input_datefmt (buf, NULL);

  if (!strcmp(key, "notify-daemon_enable"))
    return config_serialize_bool (buf, &dmon.enable);

  if (!strcmp(key, "notify-daemon_log"))
    return config_serialize_bool (buf, &dmon.log);

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
config_save_cb (const char *key, const char *value, void *fp)
{
  char buf[BUFSIZ];
  int result = config_serialize_conf (buf, key);

  if (result < 0)
    EXIT (_("configuration variable unknown: \"%s\""), key);
    /* NOTREACHED */
  else if (result == 0)
    EXIT (_("wrong configuration variable format for \"%s\""), key);
    /* NOTREACHED */

  fputs (key, (FILE *) fp);
  fputc ('=', (FILE *) fp);
  fputs (buf, (FILE *) fp);
  fputc ('\n', (FILE *) fp);

  return 1;
}

static int
config_save_junk_cb (const char *data, void *fp)
{
  fputs (data, (FILE *) fp);
  return 1;
}

/* Save the user configuration. */
unsigned
config_save (void)
{
  char tmppath[BUFSIZ];
  char *tmpext;
  FILE *fp_tmp;

  strncpy (tmppath, get_tempdir (), BUFSIZ);
  strncat (tmppath, "/" CONF_PATH_NAME ".", BUFSIZ);
  if ((tmpext = new_tempfile (tmppath, TMPEXTSIZ)) == NULL)
    return 0;
  strncat (tmppath, tmpext, BUFSIZ);
  mem_free (tmpext);

  fp_tmp = fopen (tmppath, "w");
  if (!fp_tmp)
    return 0;
  config_file_walk (config_save_cb, config_save_junk_cb, (void *) fp_tmp);
  file_close (fp_tmp, __FILE_POS__);

  if (io_file_cp (tmppath, path_conf))
    unlink (tmppath);

  return 1;
}
