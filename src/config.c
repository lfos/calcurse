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

#include "calcurse.h"

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

  if (!strcmp(key, "calendar_default_view")) {
    calendar_set_view (atoi (value));
    return 1;
  }

  if (!strcmp(key, "week_begins_on_monday")) {
    unsigned tmp;
    if (config_parse_bool (&tmp, value)) {
      if (tmp)
        calendar_set_first_day_of_week (MONDAY);
      else
        calendar_set_first_day_of_week (SUNDAY);
      return 1;
    }
    else
      return 0;
  }

  if (!strcmp(key, "color-theme"))
    {
      int color1, color2;
      if (!config_parse_color_pair (&color1, &color2, value))
        return 0;
      init_pair (COLR_CUSTOM, color1, color2);
      return 1;
    }

  if (!strcmp(key, "layout")) {
    wins_set_layout (atoi (value));
    return 1;
  }

  if (!strcmp(key, "side-bar_width")) {
    wins_set_sbar_width (atoi (value));
    return 1;
  }

  if (!strcmp(key, "notify-bar_show"))
    return config_parse_bool (&nbar.show, value);

  if (!strcmp(key, "notify-bar_date")) {
    strncpy (nbar.datefmt, value, strlen (value) + 1);
    return 1;
  }

  if (!strcmp(key, "notify-bar_clock")) {
    strncpy (nbar.timefmt, value, strlen (value) + 1);
    return 1;
  }

  if (!strcmp(key, "notify-bar_warning"))
    return config_parse_int (&nbar.cntdwn, value);

  if (!strcmp(key, "notify-bar_command")) {
    strncpy (nbar.cmd, value, strlen (value) + 1);
    return 1;
  }

  if (!strcmp(key, "notify-all"))
    return config_parse_bool(&nbar.notify_all, value);

  if (!strcmp(key, "output_datefmt")) {
    if (value[0] != '\0')
      strncpy (conf.output_datefmt, value, strlen (value) + 1);
    return 1;
  }

  if (!strcmp(key, "input_datefmt")) {
    if (config_parse_int (&conf.input_datefmt, value)) {
      if (conf.input_datefmt <= 0 || conf.input_datefmt >= DATE_FORMATS)
        conf.input_datefmt = 1;
      return 1;
    }
    else
      return 0;
  }

  if (!strcmp(key, "notify-daemon_enable"))
    return config_parse_bool (&dmon.enable, value);

  if (!strcmp(key, "notify-daemon_log"))
    return config_parse_bool (&dmon.log, value);

  return -1;
}

/* Load the user configuration. */
void
config_load (void)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open config file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char buf[BUFSIZ], e_conf[BUFSIZ];
  char *key, *value;
  int result;

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
        continue;

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

      result = config_set_conf (key, value);
      if (result < 0)
        EXIT (_("configuration variable unknown: \"%s\""), key);
        /* NOTREACHED */
      else if (result == 0)
        EXIT (_("wrong configuration variable format for \"%s\""), key);
        /* NOTREACHED */
    }
  file_close (data_file, __FILE_POS__);
  pthread_mutex_unlock (&nbar.mutex);
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


/* Save the user configuration. */
unsigned
config_save (void)
{
  char *config_txt =
    "#\n"
    "# Calcurse configuration file\n#\n"
    "# This file sets the configuration options used by Calcurse. These\n"
    "# options are usually set from within Calcurse. A line beginning with \n"
    "# a space or tab is considered to be a continuation of the previous "
    "line.\n"
    "# For a variable to be unset its value must be blank, followed by an\n"
    "# empty line. To set a variable to the empty string its value should be "
    "\"\".\n"
    "# Lines beginning with \"#\" are comments, and ignored by Calcurse.\n";
  char theme_name[BUFSIZ];
  FILE *fp;

  if ((fp = fopen (path_conf, "w")) == NULL)
    return 0;

  config_color_theme_name (theme_name);

  fprintf (fp, "%s\n", config_txt);

  fputs ("# If this option is set to yes, "
         "automatic save is done when quitting\n", fp);
  fputs ("auto_save=", fp);
  fprintf (fp, "%s\n", (conf.auto_save) ? "yes" : "no");

  fputs ("\n# If this option is set to yes, "
         "the GC is run automatically when quitting\n", fp);
  fputs ("auto_gc=", fp);
  fprintf (fp, "%s\n", (conf.auto_gc) ? "yes" : "no");

  fputs ("\n# If not null, perform automatic saves every "
         "'periodic_save' minutes\n", fp);
  fputs ("periodic_save=", fp);
  fprintf (fp, "%d\n", conf.periodic_save);

  fputs ("\n# If this option is set to yes, "
         "confirmation is required before quitting\n", fp);
  fputs ("confirm_quit=", fp);
  fprintf (fp, "%s\n", (conf.confirm_quit) ? "yes" : "no");

  fputs ("\n# If this option is set to yes, "
         "confirmation is required before deleting an event\n", fp);
  fputs ("confirm_delete=", fp);
  fprintf (fp, "%s\n", (conf.confirm_delete) ? "yes" : "no");

  fputs ("\n# If this option is set to yes, messages about loaded and "
         "saved data will not be displayed\n", fp);
  fputs ("skip_system_dialogs=", fp);
  fprintf (fp, "%s\n", (conf.skip_system_dialogs) ? "yes" : "no");

  fputs ("\n# If this option is set to yes, progress bar appearing "
         "when saving data will not be displayed\n", fp);
  fputs ("skip_progress_bar=", fp);
  fprintf (fp, "%s\n", (conf.skip_progress_bar) ? "yes" : "no");

  fputs ("\n# Default calendar view (0)monthly (1)weekly:\n", fp);
  fputs ("calendar_default_view=", fp);
  fprintf (fp, "%d\n", calendar_get_view ());

  fputs ("\n# If this option is set to yes, "
         "monday is the first day of the week, else it is sunday\n", fp);
  fputs ("week_begins_on_monday=", fp);
  fprintf (fp, "%s\n", (calendar_week_begins_on_monday ())? "yes" : "no");

  fputs ("\n# This is the color theme used for menus :\n", fp);
  fputs ("color-theme=", fp);
  fprintf (fp, "%s\n", theme_name);

  fputs ("\n# This is the layout of the calendar :\n", fp);
  fputs ("layout=", fp);
  fprintf (fp, "%d\n", wins_layout ());

  fputs ("\n# Width ( percentage, 0 being minimun width, fp) "
         "of the side bar :\n", fp);
  fputs ("side-bar_width=", fp);
  fprintf (fp, "%d\n", wins_sbar_wperc ());

  if (ui_mode == UI_CURSES)
    pthread_mutex_lock (&nbar.mutex);
  fputs ("\n# If this option is set to yes, "
         "notify-bar will be displayed :\n", fp);
  fputs ("notify-bar_show=", fp);
  fprintf (fp, "%s\n", (nbar.show) ? "yes" : "no");

  fputs ("\n# Format of the date to be displayed inside notify-bar :\n", fp);
  fputs ("notify-bar_date=", fp);
  fprintf (fp, "%s\n", nbar.datefmt);

  fputs ("\n# Format of the time to be displayed inside notify-bar :\n", fp);
  fputs ("notify-bar_clock=", fp);
  fprintf (fp, "%s\n", nbar.timefmt);

  fputs ("\n# Warn user if he has an appointment within next "
         "'notify-bar_warning' seconds :\n", fp);
  fputs ("notify-bar_warning=", fp);
  fprintf (fp, "%d\n", nbar.cntdwn);

  fputs ("\n# Command used to notify user of "
         "an upcoming appointment :\n", fp);
  fputs ("notify-bar_command=", fp);
  fprintf (fp, "%s\n", nbar.cmd);

  fputs ("\n# Notify all appointments instead of flagged ones only\n", fp);
  fputs ("notify-all=", fp);
  fprintf (fp, "%s\n", (nbar.notify_all) ? "yes" : "no");

  fputs ("\n# Format of the date to be displayed "
         "in non-interactive mode :\n", fp);
  fputs ("output_datefmt=", fp);
  fprintf (fp, "%s\n", conf.output_datefmt);

  fputs ("\n# Format to be used when entering a date "
         "(1)mm/dd/yyyy (2)dd/mm/yyyy (3)yyyy/mm/dd) "
         "(4)yyyy-mm-dd:\n", fp);
  fputs ("input_datefmt=", fp);
  fprintf (fp, "%d\n", conf.input_datefmt);

  if (ui_mode == UI_CURSES)
    pthread_mutex_unlock (&nbar.mutex);

  fputs ("\n# If this option is set to yes, "
         "calcurse will run in background to get notifications "
         "after exiting\n", fp);
  fputs ("notify-daemon_enable=", fp);
  fprintf (fp, "%s\n", dmon.enable ? "yes" : "no");

  fputs ("\n# If this option is set to yes, "
         "activity will be logged when running in background\n", fp);
  fputs ("notify-daemon_log=", fp);
  fprintf (fp, "%s\n", dmon.log ? "yes" : "no");

  file_close (fp, __FILE_POS__);

  return 1;
}
