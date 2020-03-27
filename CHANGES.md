Release Notes
=============

Version 4.6.0 (2020-03-27)
--------------------------

- New features:

    * XDG base directory support: While using ~/.calcurse/ is still supported
      for backwards compatibility, we recommend putting your configuration
      files in ~/.config/calcurse/ and your data files in
      ~/.local/share/calcurse/ instead (implemented by Nitroretro).

    * Configurable text for empty days (implemented by Lars Henriksen).

    * Support RET to set the todo item priority to 0 (implemented by Issam
      Maghni).

    * Various improvements in iCal imports (implemented by Lars Henriksen).

- New calcurse-caldav features:

    * Documentation for Yahoo's Calendar Server.

- Bug fixes:

    * Escape necessary characters in SUMMARY on export (implemented by Kelvin
      Jackson).

    * Update notification bar properly when certain recurrent appointments are
      deleted (implemented by Lars Henriksen).

    * Fix save of interactively imported data (implemented by Lars Henriksen).

Version 4.5.1 (2019-10-18)
--------------------------

- Bug fixes:

    * Restore ability to edit recurrent items without exceptions (reported and
      fixed by Lars Henriksen).

    * Fix monthly and yearly recurrence algorithms for frequencies greater than
      one (reported and fixed by Lars Henriksen).

    * Fix DST issues (reported by Lars Henriksen and others, fixed by Lars
      Henriksen).

Version 4.5.0 (2019-06-17)
--------------------------

- Compatibility notes:

    * The --conf command line option has been renamed to --confdir and the
      --directory option has been renamed to --datadir.

- New features:

    * Multiple days support: Preview appointments for multiple days without
      having to navigate in the calendar panel (implemented by Lars Henriksen).
      There are five new configuration variables: one that turns the feature
      on/off and four that affect the number of lines used by each day and with
      it the number of days on display.

    * Experimental vdir support: The calcurse distribution tarball now includes
      an experimental script called calcurse-vdir that can be used as an
      alternative to calurse-caldav (implemented by vxid). For details, please
      refer to contrib/vdir/README.md in the calcurse source tree.

    * Support for editing recurrence exceptions (implemented by Lars
      Henriksen).

- New calcurse-caldav features:

    * Custom data directory support (implemented by vxid).

    * A new Troubleshooting section in the calcurse-caldav README.md (added by
      Randy Ramos).

- Bug fixes:

    * Various fixes in the notification system (implemented by Lars Henriksen).

    * Fix for a calendar redraw issue (implemented by Lars Henriksen).

    * Fix a potential deadlock (implemented by Lars Henriksen).

    * Use ICALDATETIMEFMT for EXDATE in iCal exports (implemented by Henrik
      Grimler).

    * Fix for the compile-time AsciiDoc check (implemented by Brandon Munger).

Version 4.4.0 (2019-02-21)
--------------------------

- Compatibility notes:

    * The -F option is superseded by -P which can be used to *remove* certain
      entries from the data file (instead of keeping them). Please consult the
      man page for details.

- New features:

    * Major overhaul of the input routines in interactive mode (implemented by
      Lars Henriksen).

    * Support for separate data and configuration file directories (partly
      implemented by Quentin Hibon).

    * A --filter-invert option which inverts the other filter options
      (implemented by Lars Henriksen).

    * Support for negative date ranges when using the -d command line option.

- New calcurse-caldav features:

    * The SyncFilter configuration option which allows for specifying the types
      of items synced from/to the server (implemented by Satvik Sharma).

    * Support for a CALCURSE_CALDAV_PASSWORD environment variable which can be
      used to pass a password securely from another program (implemented by
      Randy Ramos).

    * HTTP support (implemented by Dino Macri).

    * Authorization removed from debug logs by default.

- Bug fixes:

    * Fix for an appointment becoming an event when entering an invalid start
      time (implemented by Lars Henriksen).

    * Fix for the deletion of a multi-day recurrent appointment (implemented by
      Lars Henriksen).

    * Fixes for potential database inconsistencies caused by appointments which
      end before they begin (implemented by Lars Henriksen).

    * Several fixes dealing with DST (implemented by Lars Henriksen).

    * Fixes for date range queries (implemented by Lars Henriksen).

    * Fix for segmentation fault with recent ncurses implementations.

    * Fix for potential segmentation fault in mutex handling.

    * RFC5545-compliant DTSTART in iCal exports (patch by Quentin Hibon).

    * Fix for not closing the stream after exporting iCal items (patch by
      crvs).

    * Fix for priority parsing in iCal imports (patch by dott).

    * Various improvements to the change detection and save conflict resolution
      code (implemented by Lars Henriksen).

    * Several improvements to concurrency control (implemented by Lars
      Henriksen).

    * Various fixes to the scrollbar (implemented by Lars Henriksen).

    * Various potential buffer overflow fixes.

    * A more robust command line argument parser (implemented by Lars
      Henriksen).

    * Various improvements to the UTF-8 handling code (implemented by Lars
      Henriksen).

    * Several stability and portability fixes (partly provided by Mikolaj
      Kucharski).

    * Several stability fixes in the calcurse-caldav script.

Version 4.3.0 (2017-11-05)
--------------------------

- Compatibility notes:

    * Month names and abbreviated day names now honor the locale setting
      (suggested and implemented by Lars Henriksen).

    * The phase of moon feature was removed (suggested and implemented by Lars
      Henriksen).

- New features:

    * The selected day in the calendar now stands out (suggested and
      implemented by Lars Henriksen).

    * Both the position and the date format used for the heading in the
      appointments panel are configurable (suggested and implemented by Lars
      Henriksen).

    * Support for `CTRL-U` and `CTRL-G` in prompts (implemented by Quentin
      Hibon).

    * Support for using all keys recognized by ncurses, particularly UTF-8
      keys, in key bindings (partly implemented by Lars Henriksen).

    * Support for `CALCURSE_EDITOR`, `CALCURSE_PAGER` and `CALCURSE_MERGETOOL`
      environment variables to override the default editor, pager and merge
      tool.

    * Several improvements to the reload and save routines. The save routine
      now detects external changes to the data files and no longer blindly
      overwrites data which was edited externally. In case of external changes,
      different options are offered, one of them being the execution of the
      merge tool to merge external changes and changes done from the running
      calcurse instance.  The reload routine also no longer performs a reload
      if both the external data files and the local data are unchanged.

    * Several improvements to the CalDAV synchronization script, including
      OAuth2 support and instructions on how to synchronize calcurse with
      Google Calendar (requested and implemented by Randy Ramos).

    * Support for passing the password as a command-line argument to the CalDAV
      synchronization script (suggested by azarus and implemented by Randy
      Ramos).

    * A new utility, called `calcurse-dateutil`, which can be used to create
      "extended recurrent events" such as events occurring on the third
      Thursday of each month.

- Bug fixes:

    * Gracefully parse iCal parameters with colons (reported by Hakan Jerning).

    * Fixes to the notification feature affecting recurrent appointments
      (reported by rgc69).

    * Do not crash when changing colors (reported by Randy Ramos).

    * Build fix for NetBSD (reported by Thomas Klausner).

    * Avoid fatal error when using the help command (reported and fixed by Lars
      Henriksen).

    * Avoid empty warning box when key is already in use (reported and fixed by
      Lars Henriksen).

    * Several fixes to the calcurse-caldav synchronization script to allow for
      synchronizing with NextCloud servers (reported by Geoffroy Tremblay).

    * Proper support for punctual appointments starting at 00:00 (reported by
      Maximiliano Redigonda).

    * Proper ISO 8601 week numbering (reported and fixed by Lars Henriksen).

    * The delete key no longer acts as backspace (reported by Zekario and fixed
      by cryptolmath).

    * Proper parsing of date-time strings (reported and fix suggested by Lars
      Henriksen).

Version 4.2.2 (2017-01-14)
--------------------------

- Bug fixes:

    * Various fixes and improvements to calcurse-caldav.

Version 4.2.1 (2016-10-30)
--------------------------

- Bug fixes:

    * Do not crash when editing the repetition of an appointment and passing an
      empty repetition value (reported and fixed by Vlad Glagolev).

    * Avoid a crash when all todo items are hidden and an interactive action is
      performed (reported by Vlad Glagolev).

    *  Documentation for the --daemon command line option.

    *  Avoid starting the daemon more than once (reported by Vlad Glagolev).

    *  Improvements to scrolling within list boxes.

Version 4.2.0 (2016-10-12)
--------------------------

- New features:

    * In the calendar panel, days with non-recurrent items now have another
      color as days containing recurrent items only.

    * Automatically select new appointments/events after creation.

    * Support for (optional) dates when specifying the start/end time of items.
      Allows for easily moving items and adding appointments without switching
      the selected day beforehand.

- Bug fixes:

    * Encoding fixes in calcurse-caldav (reported by Raf Czlonka).

    * Avoid notification bar artifacts after leaving calcurse.

    * Fixes to the key binding pagination (reported by Kevin Wang).

    * Do not change selection unexpectedly after modifying items.

Version 4.1.0 (2016-04-12)
--------------------------

- New features:

    * Hook support: Run scripts before/after loading or saving items. This
      feature can be used to automatically make Git commits or synchronize with
      a server when the data files are written. Example hooks can be found in
      the calcurse source tree.

    * Experimental CalDAV support: The calcurse distribution tarball now
      includes an experimental script called calcurse-caldav that can be used
      to synchronize calcurse with your mobile devices or any other computer.
      For details, please refer to contrib/caldav/README in the calcurse source
      tree.

    * Support for a filter mode -F that is identical to -G but writes the
      filtered items back to the calcurse data files instead of stdout. Can be
      used to remove certain items from a script. Use with care!

    * New long format specifiers "raw" and "hash".

    * A --daemon command line option to start calcurse in daemon mode.

    * A --quiet command line option to disable system dialogs.

    * Support for todo items with an undefined priority.

    * Support for durations when specifying recurrence end dates.

    * Support for decimal values when specifying durations.

    * Support for UTC times when importing iCal events.

    * Support for sending notifications for all appointments (both flagged and
      unflagged items).

    * The "Add Item" key binding now works from the calendar panel.

    * Week numbers in the monthly view (requested by Hakan Jerning).

- Bug fixes:

    * Fixes to the slice computation (reported by Hakan Jerning).

    * Various fixes to the iCal import and export functionality.

Version 4.0.0 (2015-02-22)
--------------------------

- New features:

    * Support for reloading appointments via a key binding and via SIGUSR1
      (thanks to Tim Hentenaar for submitting a patch).

    * The compact mode and default panel options are no longer hidden.

    * A powerful set of new command line options. The new main operations in
      non-interactive mode are --grep and --query. There are filter switches to
      restrict the set of items that are read from the appointments file. All
      old command line options are still supported for backwards compatibility.

    * Support for shorthands such as "tomorrow" or "monday" as date specifiers.

    * Support for dates beyond 2038 on platforms with 64-bit time_t.

- Bug fixes:

    * Several fixes to the user interface.

    * Handle CRLF line endings in iCal files (reported by Hakan Jerning).

    * Gracefully handle all day events in iCal imports (reported by Jörn
      Tillmanns and by Hakan Jerning).

    * Retain comments in descriptions and configuration values (reported by
      Hakan Jerning).

    * Support all types of iCal durations (reported by Hakan Jerning).

Version 3.2.1 (2014-07-08)
--------------------------

- Bug fixes:

    * Load todo items on startup (reported by BARE Willy sprl).

    * Do not highlight items on inactive windows.

Version 3.2.0 (2014-07-08)
--------------------------

- New features:

    * Support for punctual appointments (appointments without an ending time).

    * A --limit option which allows for limiting the number of appointments
      returned (thanks to William Pettersson for submitting a patch).

    * Support for %(remaining) and %(duration) modifiers in format strings
      (thanks to William Pettersson for submitting a patch).

    * The online help system now uses the system pager (e.g. less(1)).

    * A new command prompt allows for browsing the help texts (type ":help" for
      more information).

    * Several general improvements to the user interface.

- Bug fixes:

    * Do not garble long notes (reported by Hakan Jerning).

    * Fix compilation under OS X (thanks to Jack Nagel for submitting a patch).

    * Do not break the appointments file when importing an iCal file that
      contains an item with a newline in the summary (reported by Jonathan
      McCrohan).

Version 3.1.4 (2013-02-09)
--------------------------

- Bug fixes:

    * Do not prompt for a todo after adding an appointment.

    * Close key binding window when reassigning the same key (thanks to Michael
      Smith for submitting a patch).

    * Update copyright ranges.

    * Do not ignore "--datarootdir" in the i18n Makefile.

Version 3.1.3 (2013-02-02)
--------------------------

- Bug fixes:

    * Complete the test-suite even if libfaketime is not present.

    * Add a workaround for broken libfaketime-based tests on 32-bit systems
      (fixes Debian bug #697013).

    * Do not update start time/duration with bogus values if the prompt is
      canceled in edit mode.

Version 3.1.2 (2012-12-16)
--------------------------

- Bug fixes:

    * Fix another corner case of the screen corruption bug (BUG#6).

    * Fix core dump when trying to edit a non-existent item.

    * Display correct welcome messages on startup.

Version 3.1.1 (2012-12-07)
--------------------------

- Bug fixes:

    * Fix another screen corruption bug.

    * Fix several compiler warnings.

Version 3.1.0 (2012-12-05)
--------------------------

- New features:

    * Vim-like copy/paste (FR#15). Use the delete key to cut items.

    * Support for entering times in 24 hour format ("2130" instead of "21:30",
      thanks to William Pettersson for submitting a patch).

    * Compact panel mode (FR#7). This can be enabled using the currently
      undocumented "appearance.compactpanels" configuration setting.

    * Configurable default view (FR#19). The default view can be changed using
      the currently undocumented "appearance.defaultpanel" configuration
      setting.

    * "-D" and "-c" can now be used simultaneously, whereby "-c" has precedence
      over "-D".

    * Cache monthly view to speed up browsing.

- Bug fixes:

    * Sort `calcurse -d` output by time (BUG#2, reported by Romeo Van Snick).

    * Fix a critical data corruption bug (BUG#7, BUG#8, reported by Baptiste
      Jonglez and Erik Saule).

    * Fix screen corruption (BUG#6, reported by Erik Saule and Antoine
      Jacoutot).

    * No longer show the calcurse screen in the editor/pager when the window is
      resized (BUG#9, reported by Michael Smith).

    * Calculate busy slices correctly if (recurrent) appointments with a
      duration of more than 24 hours are used.

    * Fix a core dump that occurred if the main window was too small.

- Translation:

    * Several translation updates.

Version 3.0.0 (2012-07-01)
--------------------------

- New features:

    * Full UTF-8 support.

    * Much more powerful formatting options for printing items in
      non-interactive mode. Format strings can be specified using
      "--format-apt", "--format-event", "--format-recur-apt" and
      "--format-recur-event".

    * Support for vim-style count prefixes for displacement keys.

    * Powerful duration strings: Allows using extended duration strings, such
      as "+3:10" or "+1d20h5m".

    * A feature that allows piping items to external commands.

    * New key bindings to jump to the previous/next month/year.

    * A new configuration file format. `calcurse-upgrade` can be used to
      convert existing configuration files.

    * Several performance improvements.

    * Notes are now stored using hash-based file names which results in lower
      disk space usage.

    * A test suite that can be used to test the core functionality of calcurse.

    * A "--read-only" command line option to discard all changes to data files.

- Bug fixes:

    * Do not hardcode paths to the default editor/pager. Rely on the "$PATH"
      variable instead.

    * Update the number of todo items when importing an iCal file to prevent
      some items from being inaccessible.

    * Fix a segmentation fault when importing iCal data (reported by Andraz
      Levstik).

    * Format the "DURATION" field properly on iCal export. Use
      days/hours/minutes/seconds instead of seconds (reported and fixed by
      Jerome Pinot).

    * Do not localize dates in pcal exports (reported by Baptiste Jonglez).

- Translation:

    * Portuguese translation (provided by Rafael Ferreira).

    * 100% complete French, German and Russian translations.

Version 2.9.2 (2011-09-08)
--------------------------

- Bugfixes:

    * Handle iCal line folding correctly.

    * Introduce a configure option to completely exclude the documentation
      subdirectory from the build process ("--disable-docs").

Version 2.9.1 (2011-08-03)
--------------------------

- Bugfixes:

    * Keep the order of events across restarts (reported by Alan).

    * Fix the build process when disabling documentation generation.

    * Avoid flickering on window resize.

    * Avoid a segfault when resizing the calcurse window.

    * Add missing documentation for the "backword-kill-word" line editing
      function.

    * Honor the "TMPDIR" environment variable instead of using hardcoded paths
      for temporary files (reported by Erik Saule).

    * Fix pager invocation when showing the log file during an import (reported
      by Andraz Levstik).

    * Accept resource parameters in iCal import. Accept additional parameters
      such as language parameters (reported by Andraz Levstik).

    * Sync the notification item after editing or pasting an appointment. This
      ensures the information in the notification bar as well as the countdown
      for the notification daemon are always up-to-date (reported by Andraz
      Levstik).

    * Fix recurrent appointment notification.

    * Fix flagging of regular appointments.

    * Fix sort order when using command line options to display upcoming
      appointments and events (reported by Erik Saule).

Version 2.9.0 (2011-05-29)
--------------------------

- New features:

    * Usage of short form dates such as "29/5/10" instead of "29/05/2010", "23"
      for the 23rd of the currently selected month and year or "3/1" for Mar 01
      (or Jan 03, depending on the date format) of the currently selected year.

    * "backword-kill-word" line editing function.

    * Automatically drop empty notes after editing.

    * Documentation and man pages now are in AsciiDoc format which is easier to
      maintain and can be translated to several formats such as HTML, PDF,
      PostScript, EPUB, DocBook and much more.

    * Manual and man pages contain updated links to our new website and mailing
      lists, as well as instructions on how to use Transifex.

    * Extensive code cleanups and improvements.

- Bugfixes:

    * Avoid a segfault when resizing the help window.

    * Remove the lock file if calcurse died (fixes Debian Bug #575772, thanks
      to Erik for submitting a patch).

    * Parse appointment end times correctly if they date back (reported by
      Aleksey Mechonoshin).

    * Fix some compiler warnings.

- Translation:

    * Russian translation provided by Aleksey Mechonoshin.

    * Several translation updates.

Version 2.8 (2010-05-29)
------------------------

- New features:

    * a weekly calendar view was added with the display of the week number and
      colored slices indicating appointment times

    * the side bar width can now be customized by the user

- Bugfixes:

    * wrong calculation of recurrent dates after a turn of year fixed (patch
      provided by Lukas Fleischer)

    * check for data directory availability added

    * fixed a possible segfault that could be triggered when calcurse screen
      became too small

    * INSTALL file is no longer missing from the distributed package

    * compilation issue related to memory functions definitions fixed

Version 2.7 (2009-08-22)
------------------------

- New features:

    * a daemon was implemented so that calcurse can now send reminders in
      background

    * new --status command line option to display information about calcurse
      running instances

- Bugfixes:

    * fixed a bug which prevented ical files from being imported

    * no more error when user's home directory does not exist

    * dates are now written properly again when using the '-r' flag

    * incorrect duration format fixed when exporting to ical

Version 2.6 (2009-07-11)
------------------------

- New features:

    * calcurse is now distributed under a 2-clause BSD-style license

    * todos can now be flagged as completed

    * support for regex-based searches added

    * locking mechanism implemented to prevent having two calcurse instances
      running at the same time

    * inside calendar panel, day names and selected date are now in the same
      colour as user's theme

- Bugfixes:

    * missing 'T' letter added in the DURATION field for ical export (reported
      by cuz)

    * ical events which spans over several days are now imported correctly
      (reported by Andreas Kalex)

    * fixed gcc's `format-security' issue (reported by Francois Boulogne)

    * no more freeze when changing color within the configuration screen on
      OpenBSD

    * fixed a memory leak caused by a wrong use of the structures related to
      the notification bar

    * todo items are not displayed twice if -d and -t flags are both given
      (reported by Timo Schmiade)

    * it is now possible to export data from a given appointment file even if
      the user does not have any home directory (reported by Ben Zanin)

    * prevent character deletion before the beginning of the string within the
      online editor (reported by Martin Rehak)

Version 2.5 (2009-01-25)
------------------------

- New features:

    * new option to periodically save data

    * cut and paste feature added, to move items from one day to another

    * support for iso date format (yyyy-mm-dd) added

    * new '--enable-memory-debug' configuration option to monitor memory usage

    * configuration scipt improved to be able to link against ncursesw if
      ncurses is not available

- Bugfixes:

    * fixed a two-years old bug (appeared in version 1.5) that made repeated
      items with exceptions load uncorrectly in some cases (thanks to Jan
      Smydke for reporting it)

    * fixed a bug related to user-configured keys that could be lost when using
      calcurse in non-interactive mode

Version 2.4 (2008-12-27)
------------------------

- New features:

    * key bindings are now user-definable

    * new layout configuration menu

- Bugfixes:

    * memory leak due to a wrong use of the pthread library fixed

    * fixed a possible freeze when deleting an appointment's note

    * exception dates now properly ignored when exporting data to pcal format

    * daylight saving time unwanted offset fixed

Version 2.3 (2008-10-15)
------------------------

- New feature:

    * ical import added

Version 2.2 (2008-08-28)
------------------------

- New features:

    * pcal export added, to be able to produce nice-looking PostScript output

    * '-s', '-r' and '-D' command line arguments added which allows to use an
      alternative data directory, and to be more flexible when specifying the
      range of dates to be considered when displaying appointments and events
      (thanks Erik for submiting the patch)

    * '^G', '0' and '$' keybindings added to ease movements in calendar

Version 2.1 (2008-05-17)
------------------------

- New features:

    * '--note' command line argument added which allows the display of note
      contents in non-interactive mode (patch submitted by Erik Saule)

    * It is now possible to configure date formats used in calcurse interactive
      and non-interactive modes (patch submitted by Tony)

- Bugfixes:

    * Debian Bug Report #469297

- Translation:

    * Italian manual provided by Leandro Noferini

Version 2.0 (2008-03-02)
------------------------

- New features:

    * Ability to attach notes to appointments, events and todos added

    * Call to an external editor/pager to edit/view notes implemented

    * Documentation improved, with the use of a css style sheet in html manuals

- Bugfixes:

    * Leap years are now properly handled

    * configure.ac updated to link against pthread and not lpthread

Version 1.9 (2007-10-23)
------------------------

- New features:

    * Moon phase calculation added

    * Automatic redraw is now performed when resizing terminal

    * Major code cleanup release, error and signal handling improved

- Bugfixes:

    * Current day is now automatically updated in the calendar panel

    * No more problem when trying to load a calendar from current directory

- Translation:

    * Dutch translation and manual provided by Jeremy Roon

Version 1.8 (2007-05-22)
------------------------

- New features:

    * The command launched to notify user of an upcoming appointment is now
      configurable, so that user can get warned by mail or by playing a tune
      for example

    * Color theme configuration menu was completely redesigned, with support
      for default terminal's color added

    * 'Export' command implemented, to be able to save calcurse data in
      iCalendar format. The '--export' command line argument was also
      implemented to be able to use this feature in non-interactive mode.

    * 'Flag Item' command implemented to mark appointments as 'important' so
      that user gets notified before they arrive

- Bugfixes:

    * Fixed a possible problem while editing an item description and using
      CTRL-D to delete last character

    * 01/01/1970 is not returned anymore when editing an item endless
      repetition

Version 1.7 (2007-01-20)
------------------------

- New features:

    * 'Edit Item' command implemented to be able to modify an already existing
      item

    * Long command-line options are now accepted

    * '-t' flag now takes a priority number as optional argument

    * Repeated items are now marked with an '*' to be recognizable from normal
      items

- Bugfixes:

    * When creating a recurrent item, the end-date is included again in the
      repetition

    * Date format corrected in 'Go To' command

Version 1.6 (2006-10-01)
------------------------

- New features:

    * Notification-bar implemented, which indicates current date and time, the
      calendar file in use and the next upcoming appointment together with the
      time left before it

    * '-n' flag added to get notified of the next appointment within upcoming
      24 hours

    * Support for todo priorities added

    * New screen layouts added to make the todo panel the largest one

    * General keybindings implemented, which apply whatever panel is selected

- Bugfixes:

    * The repeated end date can no longer be before the item start date

    * Fixed a possible conflict in the LOCALEDIR variable definition

- Translation:

    * spanish translation and manual provided by Jose Lopez

    * german translation and manual updated by Christoph M.

Version 1.5 (2006-08-26)
------------------------

- New features:

    * Support for recurrent events and appointments added

- Bugfixes:

    * Debian Bug Report #369550, #377543

    * fixed the compiler linking problem with libintl on systems which do not
      provide intl support within libc

- Translation:

    * german translation provided by Michael Schulz

    * english translation provided by Neil Williams

Version 1.4 (2006-05-15)
------------------------

- New features:

    * Support for i18n added

    * Support for non-color terminals added

    * Option added to choose which day is the first of the week (monday or
      sunday)

    * Documentation improved, with translated html manuals

- Bugfixes:

    * When confirmation is requested, it is now done by pressing 'y' or 'n'
      instead of 'yes' or 'no'

- Translation:

    * french translation

    * french and german manuals

Version 1.3 (2006-03-17)
------------------------

- New features:

    * Adding of all-day long events

    * Many GUI improvements: better scrolling (with the use of ncurses pad
      functions), scrollbars added, progress bar added

    * Appointment duration can now be entered either in minutes or in hh:mm
      format

- Bugfixes:

    * January 0 bug fixed

    * Current day is no longer highlighted in every year of the future and the
      past (thanks to Michael for reporting that bug)

    * Fixed compiler warnings (thanks to Uwe for reporting this)

    * Removed -lpanel link during compilation

    * Characters can now be erased with CTRL-H (to fix a problem reported by
      Brendan)

Version 1.2 (2005-11-26)
------------------------

- New features:

    * An option was added to skip system dialogs

    * Configure script was improved

- Bugfixes:

    * Ncurses library use improved: screen refreshing is faster, windows do not
      flicker anymore when updated, and memory footprint is much smaller

    * Changed abbreviation for 'Wednesday' from 'Wen' to 'Wed'

Version 1.1 (2005-10-29)
------------------------

- New features:

    * Command-line options which allows to display appointments and todo list
      without entering the interactive mode

    * Manpage and documentation updated

    * Configure script improved

- Bugfixes:

    * Debian Bug Report #335430 regarding the GoTo today function is now fixed

Version 1.0 (2005-10-08)
------------------------

- New features:

    * Calcurse now comes with a manpage

- Bugfixes:

    * Debian Bug Report #330869 regarding the October 0 which does not exist is
      now fixed

    * Default options "auto-save", "confirm-quit", and "confirm-delete" were
      set to 'yes'
