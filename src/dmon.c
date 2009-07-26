/*	$calcurse: dmon.c,v 1.3 2009/07/26 12:47:15 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2009 Frederic Culot <frederic@culot.org>
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
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <paths.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "utils.h"
#include "i18n.h"
#include "sigs.h"
#include "mem.h"
#include "io.h"
#include "custom.h"
#include "notify.h"
#include "dmon.h"

#define DMON_SLEEP_TIME  60

static void
dmon_sigs_hdlr (int sig)
{
  notify_free_app ();
  (void)io_fprintln (path_dmon_log, _("terminated at %s with signal %d\n"),
                     nowstr (), sig);

  if (unlink (path_dpid) != 0)
    EXIT (_("Could not remove daemon lock file: %s\n"), strerror (errno));
  
  exit (EXIT_SUCCESS);
}

static unsigned
daemonize (int status)
{
  int fd;
  
  /*
   * Operate in the background: Daemonizing.
   *
   * First need to fork in order to become a child of the init process,
   * once the father exits.
   */
  switch (fork ())
    {
    case -1:               /* fork error */
      ERROR_MSG (_("Could not fork: %s\n"), strerror (errno));
      return 0;
      break;
    case 0:                /* child */
      break;
    default:               /* parent */
      exit (status);
    }

  /*
   * Process independency.
   *
   * Obtain a new process group and session in order to get detached from the
   * controlling terminal.
   */
  if (setsid () == -1)
    {
      ERROR_MSG (_("Could not detach from the controlling terminal: %s\n"),
                 strerror (errno));
      return 0;
    }

  /*
   * Change working directory to root directory,
   * to prevent filesystem unmounts.
   */
  if (chdir ("/") == -1)
    {
      ERROR_MSG (_("Could not change working directory: %s\n"),
                 strerror (errno));
      return 0;
    }
      
  /* Redirect standard file descriptors to /dev/null. */
  if ((fd = open (_PATH_DEVNULL, O_RDWR, 0)) != -1)
    {
      (void)dup2 (fd, STDIN_FILENO);
      (void)dup2 (fd, STDOUT_FILENO);
      (void)dup2 (fd, STDERR_FILENO);
      if (fd > 2)
        (void)close (fd);
    }

  /* Write access for the owner only. */
  (void)umask (0022);

  if (!sigs_set_hdlr (SIGINT, dmon_sigs_hdlr)
      || !sigs_set_hdlr (SIGTERM, dmon_sigs_hdlr)
      || !sigs_set_hdlr (SIGALRM, dmon_sigs_hdlr)
      || !sigs_set_hdlr (SIGQUIT, dmon_sigs_hdlr)
      || !sigs_set_hdlr (SIGCHLD, SIG_IGN))
    return 0;

  return 1;
}

void
dmon_start (int parent_exit_status)
{
  conf_t conf;
  
  if (!daemonize (parent_exit_status))
    EXIT (_("Cannot daemonize, aborting\n"));

  if (!io_dump_pid (path_dpid))
    EXIT (_("Could not set lock file\n"));
  
  io_check_file (path_conf, (int *)0);
  custom_load_conf (&conf, 0);
  
  io_check_file (path_apts, (int *)0);
  io_load_app ();  
            
  for (;;)
    {
      struct notify_app_s next;

      (void)notify_get_next (&next);
      if (next.got_app)
        {
          int left;

          notify_update_app (next.time, next.state, next.txt);
          left = notify_time_left ();

          if (left < nbar.cntdwn)
            notify_launch_cmd ();
        }
      
      if (next.txt)
        mem_free (next.txt);

      (void)io_fprintln (path_dmon_log, _("sleeping at %s for %d seconds\n"),
                   nowstr (), DMON_SLEEP_TIME);
      psleep (DMON_SLEEP_TIME);
      (void)io_fprintln (path_dmon_log, _("awakened at %s\n"), nowstr ());
    }
}
