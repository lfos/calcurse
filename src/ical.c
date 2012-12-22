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

#include <strings.h>
#include <sys/types.h>

#include "calcurse.h"

#define ICALDATEFMT      "%Y%m%d"
#define ICALDATETIMEFMT  "%Y%m%dT%H%M%S"

typedef enum {
  ICAL_VEVENT,
  ICAL_VTODO,
  ICAL_TYPES
} ical_types_e;

typedef enum {
  UNDEFINED,
  APPOINTMENT,
  EVENT
} ical_vevent_e;

typedef struct {
  enum recur_type type;
  int freq;
  long until;
  unsigned count;
} ical_rpt_t;

static void ical_export_header(FILE *);
static void ical_export_recur_events(FILE *);
static void ical_export_events(FILE *);
static void ical_export_recur_apoints(FILE *);
static void ical_export_apoints(FILE *);
static void ical_export_todo(FILE *);
static void ical_export_footer(FILE *);

static const char *ical_recur_type[RECUR_TYPES] =
    { "", "DAILY", "WEEKLY", "MONTHLY", "YEARLY" };

/* iCal alarm notification. */
static void ical_export_valarm(FILE * stream)
{
  fputs("BEGIN:VALARM\n", stream);
  pthread_mutex_lock(&nbar.mutex);
  fprintf(stream, "TRIGGER:-P%dS\n", nbar.cntdwn);
  pthread_mutex_unlock(&nbar.mutex);
  fputs("ACTION:DISPLAY\n", stream);
  fputs("END:VALARM\n", stream);
}

/* Export header. */
static void ical_export_header(FILE * stream)
{
  fputs("BEGIN:VCALENDAR\n", stream);
  fprintf(stream, "PRODID:-//calcurse//NONSGML v%s//EN\n", VERSION);
  fputs("VERSION:2.0\n", stream);
}

/* Export footer. */
static void ical_export_footer(FILE * stream)
{
  fputs("END:VCALENDAR\n", stream);
}

/* Export recurrent events. */
static void ical_export_recur_events(FILE * stream)
{
  llist_item_t *i, *j;
  char ical_date[BUFSIZ];

  LLIST_FOREACH(&recur_elist, i) {
    struct recur_event *rev = LLIST_GET_DATA(i);
    date_sec2date_fmt(rev->day, ICALDATEFMT, ical_date);
    fputs("BEGIN:VEVENT\n", stream);
    fprintf(stream, "DTSTART:%s\n", ical_date);
    fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
            ical_recur_type[rev->rpt->type], rev->rpt->freq);

    if (rev->rpt->until != 0) {
      date_sec2date_fmt(rev->rpt->until, ICALDATEFMT, ical_date);
      fprintf(stream, ";UNTIL=%s\n", ical_date);
    } else
      fputc('\n', stream);

    if (LLIST_FIRST(&rev->exc)) {
      fputs("EXDATE:", stream);
      LLIST_FOREACH(&rev->exc, j) {
        struct excp *exc = LLIST_GET_DATA(j);
        date_sec2date_fmt(exc->st, ICALDATEFMT, ical_date);
        fprintf(stream, "%s", ical_date);
        if (LLIST_NEXT(j))
          fputc(',', stream);
        else
          fputc('\n', stream);
      }
    }

    fprintf(stream, "SUMMARY:%s\n", rev->mesg);
    fputs("END:VEVENT\n", stream);
  }
}

/* Export events. */
static void ical_export_events(FILE * stream)
{
  llist_item_t *i;
  char ical_date[BUFSIZ];

  LLIST_FOREACH(&eventlist, i) {
    struct event *ev = LLIST_TS_GET_DATA(i);
    date_sec2date_fmt(ev->day, ICALDATEFMT, ical_date);
    fputs("BEGIN:VEVENT\n", stream);
    fprintf(stream, "DTSTART:%s\n", ical_date);
    fprintf(stream, "SUMMARY:%s\n", ev->mesg);
    fputs("END:VEVENT\n", stream);
  }
}

/* Export recurrent appointments. */
static void ical_export_recur_apoints(FILE * stream)
{
  llist_item_t *i, *j;
  char ical_datetime[BUFSIZ];
  char ical_date[BUFSIZ];

  LLIST_TS_LOCK(&recur_alist_p);
  LLIST_TS_FOREACH(&recur_alist_p, i) {
    struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

    date_sec2date_fmt(rapt->start, ICALDATETIMEFMT, ical_datetime);
    fputs("BEGIN:VEVENT\n", stream);
    fprintf(stream, "DTSTART:%s\n", ical_datetime);
    fprintf(stream, "DURATION:PT0H0M%ldS\n", rapt->dur);
    fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
            ical_recur_type[rapt->rpt->type], rapt->rpt->freq);

    if (rapt->rpt->until != 0) {
      date_sec2date_fmt(rapt->rpt->until + HOURINSEC, ICALDATEFMT, ical_date);
      fprintf(stream, ";UNTIL=%s\n", ical_date);
    } else
      fputc('\n', stream);

    if (LLIST_FIRST(&rapt->exc)) {
      fputs("EXDATE:", stream);
      LLIST_FOREACH(&rapt->exc, j) {
        struct excp *exc = LLIST_GET_DATA(j);
        date_sec2date_fmt(exc->st, ICALDATEFMT, ical_date);
        fprintf(stream, "%s", ical_date);
        if (LLIST_NEXT(j))
          fputc(',', stream);
        else
          fputc('\n', stream);
      }
    }

    fprintf(stream, "SUMMARY:%s\n", rapt->mesg);
    if (rapt->state & APOINT_NOTIFY)
      ical_export_valarm(stream);
    fputs("END:VEVENT\n", stream);
  }
  LLIST_TS_UNLOCK(&recur_alist_p);
}

/* Export appointments. */
static void ical_export_apoints(FILE * stream)
{
  llist_item_t *i;
  char ical_datetime[BUFSIZ];

  LLIST_TS_LOCK(&alist_p);
  LLIST_TS_FOREACH(&alist_p, i) {
    struct apoint *apt = LLIST_TS_GET_DATA(i);
    date_sec2date_fmt(apt->start, ICALDATETIMEFMT, ical_datetime);
    fputs("BEGIN:VEVENT\n", stream);
    fprintf(stream, "DTSTART:%s\n", ical_datetime);
    fprintf(stream, "DURATION:P%ldDT%ldH%ldM%ldS\n",
            apt->dur / DAYINSEC,
            (apt->dur / HOURINSEC) % DAYINHOURS,
            (apt->dur / MININSEC) % HOURINMIN, apt->dur % MININSEC);
    fprintf(stream, "SUMMARY:%s\n", apt->mesg);
    if (apt->state & APOINT_NOTIFY)
      ical_export_valarm(stream);
    fputs("END:VEVENT\n", stream);
  }
  LLIST_TS_UNLOCK(&alist_p);
}

/* Export todo items. */
static void ical_export_todo(FILE * stream)
{
  llist_item_t *i;

  LLIST_FOREACH(&todolist, i) {
    struct todo *todo = LLIST_TS_GET_DATA(i);
    if (todo->id < 0)           /* completed items */
      continue;

    fputs("BEGIN:VTODO\n", stream);
    fprintf(stream, "PRIORITY:%d\n", todo->id);
    fprintf(stream, "SUMMARY:%s\n", todo->mesg);
    fputs("END:VTODO\n", stream);
  }
}

/* Print a header to describe import log report format. */
static void ical_log_init(FILE * log, int major, int minor)
{
  const char *header =
      "+-------------------------------------------------------------------+\n"
      "| Calcurse icalendar import log.                                    |\n"
      "|                                                                   |\n"
      "| Items imported from icalendar file, version %d.%d                   |\n"
      "| Some items could not be imported, they are described hereafter.   |\n"
      "| The log line format is as follows:                                |\n"
      "|                                                                   |\n"
      "|       TYPE [LINE]: DESCRIPTION                                    |\n"
      "|                                                                   |\n"
      "| where:                                                            |\n"
      "|  * TYPE represents the item type ('VEVENT' or 'VTODO')            |\n"
      "|  * LINE is the line in the input stream at which this item begins |\n"
      "|  * DESCRIPTION indicates why the item could not be imported       |\n"
      "+-------------------------------------------------------------------+\n\n";

  if (log)
    fprintf(log, header, major, minor);
}

/*
 * Used to build a report of the import process.
 * The icalendar item for which a problem occurs is mentioned (by giving its
 * first line inside the icalendar file), together with a message describing the
 * problem.
 */
static void ical_log(FILE * log, ical_types_e type, unsigned lineno, char *msg)
{
  const char *typestr[ICAL_TYPES] = { "VEVENT", "VTODO" };

  RETURN_IF(type < 0 || type >= ICAL_TYPES, _("unknown ical type"));
  if (log)
    fprintf(log, "%s [%d]: %s\n", typestr[type], lineno, msg);
}

static void ical_store_todo(int priority, char *mesg, char *note)
{
  todo_add(mesg, priority, note);
  mem_free(mesg);
  erase_note(&note);
}

static void
ical_store_event(char *mesg, char *note, long day, long end, ical_rpt_t * rpt,
                 llist_t * exc)
{
  const int EVENTID = 1;

  if (rpt) {
    recur_event_new(mesg, note, day, EVENTID, rpt->type, rpt->freq,
                    rpt->until, exc);
    mem_free(rpt);
  } else if (end && end != day) {
    /* Here we have an event that spans over several days. */
    rpt = mem_malloc(sizeof(ical_rpt_t));
    rpt->type = RECUR_DAILY;
    rpt->freq = 1;
    rpt->count = 0;
    rpt->until = end;
    recur_event_new(mesg, note, day, EVENTID, rpt->type, rpt->freq,
                    rpt->until, exc);
    mem_free(rpt);
  } else {
    event_new(mesg, note, day, EVENTID);
  }
  mem_free(mesg);
  erase_note(&note);
}

static void
ical_store_apoint(char *mesg, char *note, long start, long dur,
                  ical_rpt_t * rpt, llist_t * exc, int has_alarm)
{
  char state = 0L;

  if (has_alarm)
    state |= APOINT_NOTIFY;
  if (rpt) {
    recur_apoint_new(mesg, note, start, dur, state, rpt->type, rpt->freq,
                     rpt->until, exc);
    mem_free(rpt);
  } else {
    apoint_new(mesg, note, start, dur, state);
  }
  mem_free(mesg);
  erase_note(&note);
}

/*
 * Returns an allocated string representing the string given in argument once
 * unformatted.
 *
 * Note:
 * Even if the RFC2445 recommends not to have more than 75 octets on one line of
 * text, I prefer not to restrict the parsing to this size, thus I use a buffer
 * of size BUFSIZ.
 *
 * Extract from RFC2445:
 * Lines of text SHOULD NOT be longer than 75 octets, excluding the line
 * break.
 */
static char *ical_unformat_line(char *line)
{
  char *p, uline[BUFSIZ];
  int len;

  if (strlen(line) >= BUFSIZ)
    return NULL;

  memset(uline, 0, BUFSIZ);
  for (len = 0, p = line; *p; p++) {
    switch (*p) {
    case '\\':
      switch (*(p + 1)) {
      case 'n':
        uline[len++] = '\n';
        p++;
        break;
      case 't':
        uline[len++] = '\t';
        p++;
        break;
      case ';':
      case ':':
      case ',':
        uline[len++] = *(p + 1);
        p++;
        break;
      default:
        uline[len++] = *p;
        break;
      }
      break;
    default:
      uline[len++] = *p;
      break;
    }
  }

  return mem_strdup(uline);
}

static void
ical_readline_init(FILE * fdi, char *buf, char *lstore, unsigned *ln)
{
  char *eol;

  *buf = *lstore = '\0';
  if (fgets(lstore, BUFSIZ, fdi)) {
    if ((eol = strchr(lstore, '\n')) != NULL)
      *eol = '\0';
    (*ln)++;
  }
}

static int ical_readline(FILE * fdi, char *buf, char *lstore, unsigned *ln)
{
  char *eol;

  strncpy(buf, lstore, BUFSIZ);
  (*ln)++;

  while (fgets(lstore, BUFSIZ, fdi) != NULL) {
    if ((eol = strchr(lstore, '\n')) != NULL)
      *eol = '\0';
    if (*lstore != SPACE && *lstore != TAB)
      break;
    strncat(buf, lstore + 1, BUFSIZ - strlen(buf) - 1);
    (*ln)++;
  }

  if (feof(fdi)) {
    *lstore = '\0';
    if (*buf == '\0')
      return 0;
  }

  return 1;
}

static int
ical_chk_header(FILE * fd, char *buf, char *lstore, unsigned *lineno,
                int *major, int *minor)
{
  const char icalheader[] = "BEGIN:VCALENDAR";

  if (!ical_readline(fd, buf, lstore, lineno))
    return 0;

  if (strncasecmp(buf, icalheader, sizeof(icalheader) - 1) != 0)
    return 0;

  while (!sscanf(buf, "VERSION:%d.%d", major, minor)) {
    if (!ical_readline(fd, buf, lstore, lineno))
      return 0;
  }

  return 1;
}

/*
 * iCalendar date-time format is based on the ISO 8601 complete
 * representation. It should be something like : DATE 'T' TIME
 * where DATE is 'YYYYMMDD' and TIME is 'HHMMSS'.
 * The time and 'T' separator are optional (in the case of an day-long event).
 *
 * Optionnaly, if the type pointer is given, specify if it is an event
 * (no time is given, meaning it is an all-day event), or an appointment
 * (time is given).
 *
 * The timezone is not yet handled by calcurse.
 */
static long ical_datetime2long(char *datestr, ical_vevent_e * type)
{
  const int NOTFOUND = 0, FORMAT_DATE = 3, FORMAT_DATETIME = 5;
  struct date date;
  unsigned hour, min;
  long datelong;
  int format;

  format = sscanf(datestr, "%04u%02u%02uT%02u%02u",
                  &date.yyyy, &date.mm, &date.dd, &hour, &min);
  if (format == FORMAT_DATE) {
    if (type)
      *type = EVENT;
    datelong = date2sec(date, 0, 0);
  } else if (format == FORMAT_DATETIME) {
    if (type)
      *type = APPOINTMENT;
    datelong = date2sec(date, hour, min);
  } else {
    datelong = NOTFOUND;
  }
  return datelong;
}

static long ical_durtime2long(char *timestr)
{
  long timelong;
  char *p;

  if ((p = strchr(timestr, 'T')) == NULL)
    timelong = 0;
  else {
    int nbmatch;
    struct {
      unsigned hour, min, sec;
    } time;

    p++;
    memset(&time, 0, sizeof time);
    nbmatch = sscanf(p, "%uH%uM%uS", &time.hour, &time.min, &time.sec);
    if (nbmatch < 1 || nbmatch > 3)
      timelong = 0;
    else
      timelong = time.hour * HOURINSEC + time.min * MININSEC + time.sec;
  }
  return timelong;
}

/*
 * Extract from RFC2445:
 *
 * Value Name: DURATION
 *
 * Purpose: This value type is used to identify properties that contain
 * duration of time.
 *
 * Formal Definition: The value type is defined by the following
 * notation:
 *
 * dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)
 * dur-date   = dur-day [dur-time]
 * dur-time   = "T" (dur-hour / dur-minute / dur-second)
 * dur-week   = 1*DIGIT "W"
 * dur-hour   = 1*DIGIT "H" [dur-minute]
 * dur-minute = 1*DIGIT "M" [dur-second]
 * dur-second = 1*DIGIT "S"
 * dur-day    = 1*DIGIT "D"
 *
 * Example: A duration of 15 days, 5 hours and 20 seconds would be:
 * P15DT5H0M20S
 * A duration of 7 weeks would be:
 * P7W
 */
static long ical_dur2long(char *durstr)
{
  const int NOTFOUND = -1;
  long durlong;
  char *p;
  struct {
    unsigned week, day;
  } date;

  memset(&date, 0, sizeof date);
  if ((p = strchr(durstr, 'P')) == NULL)
    durlong = NOTFOUND;
  else {
    p++;
    if (*p == '-')
      return NOTFOUND;
    else if (*p == '+')
      p++;

    if (*p == 'T')              /* dur-time */
      durlong = ical_durtime2long(p);
    else if (strchr(p, 'W')) {  /* dur-week */
      if (sscanf(p, "%u", &date.week) == 1)
        durlong = date.week * WEEKINDAYS * DAYINSEC;
      else
        durlong = NOTFOUND;
    } else {
      if (strchr(p, 'D')) {     /* dur-date */
        if (sscanf(p, "%uD", &date.day) == 1) {
          durlong = date.day * DAYINSEC;
          durlong += ical_durtime2long(p);
        } else
          durlong = NOTFOUND;
      } else
        durlong = NOTFOUND;
    }
  }
  return durlong;
}

/*
 * Compute the vevent repetition end date from the repetition count.
 *
 * Extract from RFC2445:
 * The COUNT rule part defines the number of occurrences at which to
 * range-bound the recurrence. The "DTSTART" property value, if specified,
 * counts as the first occurrence.
 */
static long ical_compute_rpt_until(long start, ical_rpt_t * rpt)
{
  long until;

  switch (rpt->type) {
  case RECUR_DAILY:
    until = date_sec_change(start, 0, rpt->freq * (rpt->count - 1));
    break;
  case RECUR_WEEKLY:
    until = date_sec_change(start, 0,
                            rpt->freq * WEEKINDAYS * (rpt->count - 1));
    break;
  case RECUR_MONTHLY:
    until = date_sec_change(start, rpt->freq * (rpt->count - 1), 0);
    break;
  case RECUR_YEARLY:
    until = date_sec_change(start, rpt->freq * 12 * (rpt->count - 1), 0);
    break;
  default:
    until = 0;
    break;
    /* NOTREACHED */
  }
  return until;
}

/*
 * Read a recurrence rule from an iCalendar RRULE string.
 *
 * Value Name: RECUR
 *
 * Purpose: This value type is used to identify properties that contain
 * a recurrence rule specification.
 *
 * Formal Definition: The value type is defined by the following
 * notation:
 *
 * recur      = "FREQ"=freq *(
 *
 * ; either UNTIL or COUNT may appear in a 'recur',
 * ; but UNTIL and COUNT MUST NOT occur in the same 'recur'
 *
 * ( ";" "UNTIL" "=" enddate ) /
 * ( ";" "COUNT" "=" 1*DIGIT ) /
 *
 * ; the rest of these keywords are optional,
 * ; but MUST NOT occur more than
 * ; once
 *
 * ( ";" "INTERVAL" "=" 1*DIGIT )          /
 * ( ";" "BYSECOND" "=" byseclist )        /
 * ( ";" "BYMINUTE" "=" byminlist )        /
 * ( ";" "BYHOUR" "=" byhrlist )           /
 * ( ";" "BYDAY" "=" bywdaylist )          /
 * ( ";" "BYMONTHDAY" "=" bymodaylist )    /
 * ( ";" "BYYEARDAY" "=" byyrdaylist )     /
 * ( ";" "BYWEEKNO" "=" bywknolist )       /
 * ( ";" "BYMONTH" "=" bymolist )          /
 * ( ";" "BYSETPOS" "=" bysplist )         /
 * ( ";" "WKST" "=" weekday )              /
 * ( ";" x-name "=" text )
 * )
*/
static ical_rpt_t *ical_read_rrule(FILE * log, char *rrulestr,
                                   unsigned *noskipped, const int itemline)
{
  const char daily[] = "DAILY";
  const char weekly[] = "WEEKLY";
  const char monthly[] = "MONTHLY";
  const char yearly[] = "YEARLY";
  const char count[] = "COUNT=";
  const char interv[] = "INTERVAL=";
  unsigned interval;
  ical_rpt_t *rpt;
  char *p;

  rpt = NULL;
  if ((p = strchr(rrulestr, ':')) != NULL) {
    char freqstr[BUFSIZ];

    p++;
    rpt = mem_malloc(sizeof(ical_rpt_t));
    memset(rpt, 0, sizeof(ical_rpt_t));
    if (sscanf(p, "FREQ=%s", freqstr) != 1) {
      ical_log(log, ICAL_VEVENT, itemline,
               _("recurrence frequence not found."));
      (*noskipped)++;
      mem_free(rpt);
      return NULL;
    } else {
      if (strncmp(freqstr, daily, sizeof(daily) - 1) == 0)
        rpt->type = RECUR_DAILY;
      else if (strncmp(freqstr, weekly, sizeof(weekly) - 1) == 0)
        rpt->type = RECUR_WEEKLY;
      else if (strncmp(freqstr, monthly, sizeof(monthly) - 1) == 0)
        rpt->type = RECUR_MONTHLY;
      else if (strncmp(freqstr, yearly, sizeof(yearly) - 1) == 0)
        rpt->type = RECUR_YEARLY;
      else {
        ical_log(log, ICAL_VEVENT, itemline,
                 _("recurrence frequence not recognized."));
        (*noskipped)++;
        mem_free(rpt);
        return NULL;
      }
    }
    /*
       The UNTIL rule part defines a date-time value which bounds the
       recurrence rule in an inclusive manner.  If not present, and the
       COUNT rule part is also not present, the RRULE is considered to
       repeat forever.

       The COUNT rule part defines the number of occurrences at which to
       range-bound the recurrence.  The "DTSTART" property value, if
       specified, counts as the first occurrence.
     */
    if ((p = strstr(rrulestr, "UNTIL")) != NULL) {
      char *untilstr;

      untilstr = strchr(p, '=');
      rpt->until = ical_datetime2long(++untilstr, NULL);
    } else {
      unsigned cnt;
      char *countstr;

      if ((countstr = strstr(rrulestr, count)) != NULL) {
        countstr += sizeof(count) - 1;
        if (sscanf(countstr, "%u", &cnt) != 1) {
          rpt->until = 0;
          /* endless repetition */
        } else {
          rpt->count = cnt;
        }
      } else
        rpt->until = 0;
    }

    if ((p = strstr(rrulestr, interv)) != NULL) {
      p += sizeof(interv) - 1;
      if (sscanf(p, "%u", &interval) != 1) {
        rpt->freq = 1;
        /* default frequence if none specified */
      } else {
        rpt->freq = interval;
      }
    } else {
      rpt->freq = 1;
    }
  } else {
    ical_log(log, ICAL_VEVENT, itemline, _("recurrence rule malformed."));
    (*noskipped)++;
  }
  return rpt;
}

static void ical_add_exc(llist_t * exc_head, long date)
{
  if (date != 0) {
    struct excp *exc = mem_malloc(sizeof(struct excp));
    exc->st = date;

    LLIST_ADD(exc_head, exc);
  }
}

/*
 * This property defines the list of date/time exceptions for a
 * recurring calendar component.
 */
static void
ical_read_exdate(llist_t * exc, FILE * log, char *exstr, unsigned *noskipped,
                 const int itemline)
{
  char *p, *q;
  long date;

  LLIST_INIT(exc);
  if ((p = strchr(exstr, ':')) != NULL) {
    p++;
    while ((q = strchr(p, ',')) != NULL) {
      char buf[BUFSIZ];
      const int buflen = q - p;

      strncpy(buf, p, buflen);
      buf[buflen] = '\0';
      date = ical_datetime2long(buf, NULL);
      ical_add_exc(exc, date);
      p = ++q;
    }
    date = ical_datetime2long(p, NULL);
    ical_add_exc(exc, date);
  } else {
    ical_log(log, ICAL_VEVENT, itemline,
             _("recurrence exception dates malformed."));
    (*noskipped)++;
  }
}

/* Return an allocated string containing the name of the newly created note. */
static char *ical_read_note(char *line, unsigned *noskipped,
                            ical_types_e item_type, const int itemline,
                            FILE * log)
{
  char *p, *notestr, *note;

  if ((p = strchr(line, ':')) != NULL) {
    p++;
    notestr = ical_unformat_line(p);
    if (notestr == NULL) {
      ical_log(log, item_type, itemline,
               _("could not get entire item description."));
      (*noskipped)++;
      return NULL;
    } else if (strlen(notestr) == 0) {
      mem_free(notestr);
      return NULL;
    } else {
      note = generate_note(notestr);
      mem_free(notestr);
      return note;
    }
  } else {
    ical_log(log, item_type, itemline, _("description malformed."));
    (*noskipped)++;
    return NULL;
  }
}

/* Returns an allocated string containing the ical item summary. */
static char *ical_read_summary(char *line)
{
  char *p, *summary;

  if ((p = strchr(line, ':')) != NULL) {
    p++;
    summary = ical_unformat_line(p);
    return summary;
  } else
    return NULL;
}

static void
ical_read_event(FILE * fdi, FILE * log, unsigned *noevents, unsigned *noapoints,
                unsigned *noskipped, char *buf, char *lstore, unsigned *lineno)
{
  const int ITEMLINE = *lineno;
  const char endevent[] = "END:VEVENT";
  const char summary[] = "SUMMARY";
  const char dtstart[] = "DTSTART";
  const char dtend[] = "DTEND";
  const char duration[] = "DURATION";
  const char rrule[] = "RRULE";
  const char exdate[] = "EXDATE";
  const char alarm[] = "BEGIN:VALARM";
  const char endalarm[] = "END:VALARM";
  const char desc[] = "DESCRIPTION";
  ical_vevent_e vevent_type;
  char *p;
  struct {
    llist_t exc;
    ical_rpt_t *rpt;
    char *mesg, *note;
    long start, end, dur;
    int has_alarm;
  } vevent;
  int skip_alarm;

  vevent_type = UNDEFINED;
  memset(&vevent, 0, sizeof vevent);
  skip_alarm = 0;
  while (ical_readline(fdi, buf, lstore, lineno)) {
    if (skip_alarm) {
      /* Need to skip VALARM properties because some keywords could
         interfere, such as DURATION, SUMMARY,.. */
      if (strncasecmp(buf, endalarm, sizeof(endalarm) - 1) == 0)
        skip_alarm = 0;
      continue;
    }
    if (strncasecmp(buf, endevent, sizeof(endevent) - 1) == 0) {
      if (vevent.mesg) {
        if (vevent.rpt && vevent.rpt->count)
          vevent.rpt->until = ical_compute_rpt_until(vevent.start, vevent.rpt);

        switch (vevent_type) {
        case APPOINTMENT:
          if (vevent.start == 0) {
            ical_log(log, ICAL_VEVENT, ITEMLINE,
                     _("appointment has no start time."));
            goto cleanup;
          }
          if (vevent.dur == 0) {
            if (vevent.end == 0) {
              ical_log(log, ICAL_VEVENT, ITEMLINE,
                       _("could not compute duration " "(no end time)."));
              goto cleanup;
            } else if (vevent.start == vevent.end) {
              vevent_type = EVENT;
              vevent.end = 0L;
              ical_store_event(vevent.mesg, vevent.note,
                               vevent.start, vevent.end,
                               vevent.rpt, &vevent.exc);
              (*noevents)++;
              return;
            } else {
              vevent.dur = vevent.end - vevent.start;
              if (vevent.dur < 0) {
                ical_log(log, ICAL_VEVENT, ITEMLINE,
                         _("item has a negative duration."));
                goto cleanup;
              }
            }
          }
          ical_store_apoint(vevent.mesg, vevent.note, vevent.start,
                            vevent.dur, vevent.rpt, &vevent.exc,
                            vevent.has_alarm);
          (*noapoints)++;
          break;
        case EVENT:
          if (vevent.start == 0) {
            ical_log(log, ICAL_VEVENT, ITEMLINE,
                     _("event date is not defined."));
            goto cleanup;
          }
          ical_store_event(vevent.mesg, vevent.note, vevent.start,
                           vevent.end, vevent.rpt, &vevent.exc);
          (*noevents)++;
          break;
        case UNDEFINED:
          ical_log(log, ICAL_VEVENT, ITEMLINE,
                   _("item could not be identified."));
          goto cleanup;
          break;
        }
      } else {
        ical_log(log, ICAL_VEVENT, ITEMLINE,
                 _("could not retrieve item summary."));
        goto cleanup;
      }
      return;
    } else {
      if (strncasecmp(buf, dtstart, sizeof(dtstart) - 1) == 0) {
        if ((p = strchr(buf, ':')) != NULL)
          vevent.start = ical_datetime2long(++p, &vevent_type);
        if (!vevent.start) {
          ical_log(log, ICAL_VEVENT, ITEMLINE,
                   _("could not retrieve event start time."));
          goto cleanup;
        }
      } else if (strncasecmp(buf, dtend, sizeof(dtend) - 1) == 0) {
        if ((p = strchr(buf, ':')) != NULL)
          vevent.end = ical_datetime2long(++p, &vevent_type);
        if (!vevent.end) {
          ical_log(log, ICAL_VEVENT, ITEMLINE,
                   _("could not retrieve event end time."));
          goto cleanup;
        }
      } else if (strncasecmp(buf, duration, sizeof(duration) - 1) == 0) {
        if ((vevent.dur = ical_dur2long(buf)) <= 0) {
          ical_log(log, ICAL_VEVENT, ITEMLINE, _("item duration malformed."));
          goto cleanup;
        }
      } else if (strncasecmp(buf, rrule, sizeof(rrule) - 1) == 0) {
        vevent.rpt = ical_read_rrule(log, buf, noskipped, ITEMLINE);
      } else if (strncasecmp(buf, exdate, sizeof(exdate) - 1) == 0) {
        ical_read_exdate(&vevent.exc, log, buf, noskipped, ITEMLINE);
      } else if (strncasecmp(buf, summary, sizeof(summary) - 1) == 0) {
        vevent.mesg = ical_read_summary(buf);
      } else if (strncasecmp(buf, alarm, sizeof(alarm) - 1) == 0) {
        skip_alarm = 1;
        vevent.has_alarm = 1;
      } else if (strncasecmp(buf, desc, sizeof(desc) - 1) == 0) {
        vevent.note = ical_read_note(buf, noskipped, ICAL_VEVENT,
                                     ITEMLINE, log);
      }
    }
  }
  ical_log(log, ICAL_VEVENT, ITEMLINE,
           _("The ical file seems to be malformed. "
             "The end of item was not found."));

cleanup:

  if (vevent.note)
    mem_free(vevent.note);
  if (vevent.mesg)
    mem_free(vevent.mesg);
  if (vevent.rpt)
    mem_free(vevent.rpt);
  LLIST_FREE(&vevent.exc);
  (*noskipped)++;
}

static void
ical_read_todo(FILE * fdi, FILE * log, unsigned *notodos, unsigned *noskipped,
               char *buf, char *lstore, unsigned *lineno)
{
  const char endtodo[] = "END:VTODO";
  const char summary[] = "SUMMARY";
  const char alarm[] = "BEGIN:VALARM";
  const char endalarm[] = "END:VALARM";
  const char desc[] = "DESCRIPTION";
  const int LOWEST = 9;
  const int ITEMLINE = *lineno;
  struct {
    char *mesg, *note;
    int has_priority, priority;
  } vtodo;
  int skip_alarm;

  memset(&vtodo, 0, sizeof vtodo);
  skip_alarm = 0;
  while (ical_readline(fdi, buf, lstore, lineno)) {
    if (skip_alarm) {
      /* Need to skip VALARM properties because some keywords could
         interfere, such as DURATION, SUMMARY,.. */
      if (strncasecmp(buf, endalarm, sizeof(endalarm) - 1) == 0)
        skip_alarm = 0;
      continue;
    }
    if (strncasecmp(buf, endtodo, sizeof(endtodo) - 1) == 0) {
      if (!vtodo.has_priority)
        vtodo.priority = LOWEST;
      if (vtodo.mesg) {
        ical_store_todo(vtodo.priority, vtodo.mesg, vtodo.note);
        (*notodos)++;
      } else {
        ical_log(log, ICAL_VTODO, ITEMLINE,
                 _("could not retrieve item summary."));
        goto cleanup;
      }
      return;
    } else {
      int tmpint;

      if (strncasecmp(buf, "PRIORITY:", sizeof("PRIORITY:") - 1) == 0) {
        sscanf(buf, "%d", &tmpint);
        if (tmpint <= 9 && tmpint >= 1) {
          vtodo.priority = tmpint;
          vtodo.has_priority = 1;
        } else {
          ical_log(log, ICAL_VTODO, ITEMLINE,
                   _("item priority is not acceptable "
                     "(must be between 1 and 9)."));
          vtodo.priority = LOWEST;
        }
      } else if (strncasecmp(buf, summary, sizeof(summary) - 1) == 0) {
        vtodo.mesg = ical_read_summary(buf);
      } else if (strncasecmp(buf, alarm, sizeof(alarm) - 1) == 0) {
        skip_alarm = 1;
      } else if (strncasecmp(buf, desc, sizeof(desc) - 1) == 0) {
        vtodo.note = ical_read_note(buf, noskipped, ICAL_VTODO, ITEMLINE, log);
      }
    }
  }
  ical_log(log, ICAL_VTODO, ITEMLINE,
           _("The ical file seems to be malformed. "
             "The end of item was not found."));

cleanup:

  if (vtodo.note)
    mem_free(vtodo.note);
  if (vtodo.mesg)
    mem_free(vtodo.mesg);
  (*noskipped)++;
}

/* Import calcurse data. */
void
ical_import_data(FILE * stream, FILE * log, unsigned *events, unsigned *apoints,
                 unsigned *todos, unsigned *lines, unsigned *skipped)
{
  const char vevent[] = "BEGIN:VEVENT";
  const char vtodo[] = "BEGIN:VTODO";
  char buf[BUFSIZ], lstore[BUFSIZ];
  int major, minor;

  ical_readline_init(stream, buf, lstore, lines);
  RETURN_IF(!ical_chk_header(stream, buf, lstore, lines, &major, &minor),
            _("Warning: ical header malformed or wrong version number. "
              "Aborting..."));

  ical_log_init(log, major, minor);

  while (ical_readline(stream, buf, lstore, lines)) {
    (*lines)++;
    if (strncasecmp(buf, vevent, sizeof(vevent) - 1) == 0) {
      ical_read_event(stream, log, events, apoints, skipped, buf, lstore,
                      lines);
    } else if (strncasecmp(buf, vtodo, sizeof(vtodo) - 1) == 0) {
      ical_read_todo(stream, log, todos, skipped, buf, lstore, lines);
    }
  }
}

/* Export calcurse data. */
void ical_export_data(FILE * stream)
{
  ical_export_header(stream);
  ical_export_recur_events(stream);
  ical_export_events(stream);
  ical_export_recur_apoints(stream);
  ical_export_apoints(stream);
  ical_export_todo(stream);
  ical_export_footer(stream);
}
