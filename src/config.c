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

/* Available configuration variables. */
enum config_var {
  CUSTOM_CONF_AUTOSAVE,
  CUSTOM_CONF_AUTOGC,
  CUSTOM_CONF_PERIODICSAVE,
  CUSTOM_CONF_CONFIRMQUIT,
  CUSTOM_CONF_CONFIRMDELETE,
  CUSTOM_CONF_SKIPSYSTEMDIALOGS,
  CUSTOM_CONF_SKIPPROGRESSBAR,
  CUSTOM_CONF_CALENDAR_DEFAULTVIEW,
  CUSTOM_CONF_WEEKBEGINSONMONDAY,
  CUSTOM_CONF_COLORTHEME,
  CUSTOM_CONF_LAYOUT,
  CUSTOM_CONF_SBAR_WIDTH,
  CUSTOM_CONF_NOTIFYBARSHOW,
  CUSTOM_CONF_NOTIFYBARDATE,
  CUSTOM_CONF_NOTIFYBARCLOCK,
  CUSTOM_CONF_NOTIFYBARWARNING,
  CUSTOM_CONF_NOTIFYBARCOMMAND,
  CUSTOM_CONF_NOTIFYALL,
  CUSTOM_CONF_OUTPUTDATEFMT,
  CUSTOM_CONF_INPUTDATEFMT,
  CUSTOM_CONF_DMON_ENABLE,
  CUSTOM_CONF_DMON_LOG,
  CUSTOM_CONF_INVALID
};

struct config_varname {
  enum config_var var;
  const char *name;
};

static struct config_varname config_varmap[] =
{
  { CUSTOM_CONF_AUTOSAVE, "auto_save" },
  { CUSTOM_CONF_AUTOGC, "auto_gc" },
  { CUSTOM_CONF_PERIODICSAVE, "periodic_save" },
  { CUSTOM_CONF_CONFIRMQUIT, "confirm_quit" },
  { CUSTOM_CONF_CONFIRMDELETE, "confirm_delete" },
  { CUSTOM_CONF_SKIPSYSTEMDIALOGS, "skip_system_dialogs" },
  { CUSTOM_CONF_SKIPPROGRESSBAR, "skip_progress_bar" },
  { CUSTOM_CONF_CALENDAR_DEFAULTVIEW, "calendar_default_view" },
  { CUSTOM_CONF_WEEKBEGINSONMONDAY, "week_begins_on_monday" },
  { CUSTOM_CONF_COLORTHEME, "color-theme" },
  { CUSTOM_CONF_LAYOUT, "layout" },
  { CUSTOM_CONF_SBAR_WIDTH, "side-bar_width" },
  { CUSTOM_CONF_NOTIFYBARSHOW, "notify-bar_show" },
  { CUSTOM_CONF_NOTIFYBARDATE, "notify-bar_date" },
  { CUSTOM_CONF_NOTIFYBARCLOCK, "notify-bar_clock" },
  { CUSTOM_CONF_NOTIFYBARWARNING, "notify-bar_warning" },
  { CUSTOM_CONF_NOTIFYBARCOMMAND, "notify-bar_command" },
  { CUSTOM_CONF_NOTIFYALL, "notify-all" },
  { CUSTOM_CONF_OUTPUTDATEFMT, "output_datefmt" },
  { CUSTOM_CONF_INPUTDATEFMT, "input_datefmt" },
  { CUSTOM_CONF_DMON_ENABLE, "notify-daemon_enable" },
  { CUSTOM_CONF_DMON_LOG, "notify-daemon_log" }
};

static int
config_parse_bool (unsigned *dest, char *val)
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
config_parse_unsigned (unsigned *dest, char *val)
{
  if (is_all_digit (val))
    *dest = atoi (val);
  else
    return 0;

  return 1;
}

static int
config_parse_int (int *dest, char *val)
{
  if ((*val == '+' || *val == '-' || isdigit (*val)) && is_all_digit (val + 1))
    *dest = atoi (val);
  else
    return 0;

  return 1;
}

/*
 * Load user color theme from file.
 * Need to handle calcurse versions prior to 1.8, where colors where handled
 * differently (number between 1 and 8).
 */
static int
config_parse_color (char *val)
{
#define AWAITED_COLORS	2

  int i, len, color_num;
  char c[AWAITED_COLORS][BUFSIZ];
  int colr[AWAITED_COLORS];

  len = strlen (val);
  if (len > 1)
    {
      /* New version configuration */
      if (sscanf (val, "%s on %s", c[0], c[1]) != AWAITED_COLORS)
        return 0;

      for (i = 0; i < AWAITED_COLORS; i++)
        {
          if (!strncmp (c[i], "black", 5))
            colr[i] = COLOR_BLACK;
          else if (!strncmp (c[i], "red", 3))
            colr[i] = COLOR_RED;
          else if (!strncmp (c[i], "green", 5))
            colr[i] = COLOR_GREEN;
          else if (!strncmp (c[i], "yellow", 6))
            colr[i] = COLOR_YELLOW;
          else if (!strncmp (c[i], "blue", 4))
            colr[i] = COLOR_BLUE;
          else if (!strncmp (c[i], "magenta", 7))
            colr[i] = COLOR_MAGENTA;
          else if (!strncmp (c[i], "cyan", 4))
            colr[i] = COLOR_CYAN;
          else if (!strncmp (c[i], "white", 5))
            colr[i] = COLOR_WHITE;
          else if (!strncmp (c[i], "default", 7))
            colr[i] = background;
          else
            return 0;
        }
      init_pair (COLR_CUSTOM, colr[0], colr[1]);
    }
  else if (len == 1)
    {
      /* Old version configuration */
      if (isdigit (*val))
        color_num = atoi (val);
      else
        return 0;

      switch (color_num)
        {
        case 0:
          colorize = 0;
          break;
        case 1:
          init_pair (COLR_CUSTOM, COLOR_RED, background);
          break;
        case 2:
          init_pair (COLR_CUSTOM, COLOR_GREEN, background);
          break;
        case 3:
          init_pair (COLR_CUSTOM, COLOR_BLUE, background);
          break;
        case 4:
          init_pair (COLR_CUSTOM, COLOR_CYAN, background);
          break;
        case 5:
          init_pair (COLR_CUSTOM, COLOR_YELLOW, background);
          break;
        case 6:
          init_pair (COLR_CUSTOM, COLOR_BLACK, COLR_GREEN);
          break;
        case 7:
          init_pair (COLR_CUSTOM, COLOR_BLACK, COLR_YELLOW);
          break;
        case 8:
          init_pair (COLR_CUSTOM, COLOR_RED, COLR_BLUE);
          break;
        default:
          return 0;
        }
    }
  else
    return 0;

  return 1;
}

/* Set a configuration variable. */
static int
config_set_conf (enum config_var var, char *val)
{
  unsigned tmp;

  switch (var)
    {
    case CUSTOM_CONF_AUTOSAVE:
      return config_parse_bool (&conf.auto_save, val);
      break;
    case CUSTOM_CONF_AUTOGC:
      return config_parse_bool (&conf.auto_gc, val);
      break;
    case CUSTOM_CONF_PERIODICSAVE:
      return config_parse_unsigned (&conf.periodic_save, val);
      break;
    case CUSTOM_CONF_CONFIRMQUIT:
      return config_parse_bool (&conf.confirm_quit, val);
      break;
    case CUSTOM_CONF_CONFIRMDELETE:
      return config_parse_bool (&conf.confirm_delete, val);
      break;
    case CUSTOM_CONF_SKIPSYSTEMDIALOGS:
      return config_parse_bool (&conf.skip_system_dialogs, val);
      break;
    case CUSTOM_CONF_SKIPPROGRESSBAR:
      return config_parse_bool (&conf.skip_progress_bar, val);
      break;
    case CUSTOM_CONF_CALENDAR_DEFAULTVIEW:
      calendar_set_view (atoi (val));
      break;
    case CUSTOM_CONF_WEEKBEGINSONMONDAY:
      return config_parse_bool (&tmp, val);
      if (tmp)
        calendar_set_first_day_of_week (MONDAY);
      else
        calendar_set_first_day_of_week (SUNDAY);
      break;
    case CUSTOM_CONF_COLORTHEME:
      return config_parse_color (val);
      break;
    case CUSTOM_CONF_LAYOUT:
      wins_set_layout (atoi (val));
      break;
    case CUSTOM_CONF_SBAR_WIDTH:
      wins_set_sbar_width (atoi (val));
      break;
    case CUSTOM_CONF_NOTIFYBARSHOW:
      return config_parse_bool (&nbar.show, val);
      break;
    case CUSTOM_CONF_NOTIFYBARDATE:
      strncpy (nbar.datefmt, val, strlen (val) + 1);
      break;
    case CUSTOM_CONF_NOTIFYBARCLOCK:
      strncpy (nbar.timefmt, val, strlen (val) + 1);
      break;
    case CUSTOM_CONF_NOTIFYBARWARNING:
      return config_parse_int (&nbar.cntdwn, val);
      break;
    case CUSTOM_CONF_NOTIFYBARCOMMAND:
      strncpy (nbar.cmd, val, strlen (val) + 1);
      break;
    case CUSTOM_CONF_NOTIFYALL:
      return config_parse_bool(&nbar.notify_all, val);
      break;
    case CUSTOM_CONF_OUTPUTDATEFMT:
      if (val[0] != '\0')
        strncpy (conf.output_datefmt, val, strlen (val) + 1);
      break;
    case CUSTOM_CONF_INPUTDATEFMT:
      return config_parse_int (&conf.input_datefmt, val);
      if (conf.input_datefmt <= 0 || conf.input_datefmt >= DATE_FORMATS)
        conf.input_datefmt = 1;
      break;
    case CUSTOM_CONF_DMON_ENABLE:
      return config_parse_bool (&dmon.enable, val);
      break;
    case CUSTOM_CONF_DMON_LOG:
      return config_parse_bool (&dmon.log, val);
      break;
    default:
      return 0;
      break;
    }

  return 1;
}

/* Load the user configuration. */
void
config_load (void)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open config file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char buf[BUFSIZ], e_conf[BUFSIZ];
  int i;
  char *name;
  enum config_var var;
  char *val;

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

      name = e_conf;
      val = strchr (e_conf, '=');
      if (val)
        {
          *val = '\0';
          val++;
        }

      var = CUSTOM_CONF_INVALID;
      for (i = 0; i < sizeof (config_varmap) / sizeof (struct config_varname); i++)
        {
          if (strncmp (name, config_varmap[i].name, BUFSIZ) == 0)
            {
              var = config_varmap[i].var;
              break;
            }
        }

      if (var == CUSTOM_CONF_INVALID)
        {
          EXIT (_("configuration variable unknown: \"%s\""), name);
          /* NOTREACHED */
        }

      if (val && (*val == '\0' || *val == '\n'))
        {
          /* Backward compatibility mode. */
          if (fgets (buf, sizeof buf, data_file) == NULL)
            break;
          io_extract_data (e_conf, buf, sizeof buf);
          val = e_conf;
        }

      if (!val || !config_set_conf (var, val))
        {
          EXIT (_("wrong configuration variable format for \"%s\""), name);
          /* NOTREACHED */
        }
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
