/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
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

#include "calcurse.h"

static int find_basedir(const char *locale_info[], unsigned n, char **basedir)
{
	int i;
	char *locale = NULL;
	int ret = 0;

	for (i = 0; i < n; i++) {
		if (!locale_info[i])
			continue;
		locale = mem_strdup(locale_info[i]);

		asprintf(basedir, "%s/%s", DOCDIR, locale);
		if (io_dir_exists(*basedir)) {
			ret = 1;
			goto cleanup;
		}

		strtok(locale, ".@");

		mem_free(*basedir);
		asprintf(basedir, "%s/%s", DOCDIR, locale);
		if (io_dir_exists(*basedir)) {
			ret = 1;
			goto cleanup;
		}

		strtok(locale, "_");

		mem_free(*basedir);
		asprintf(basedir, "%s/%s", DOCDIR, locale);
		if (io_dir_exists(*basedir)) {
			ret = 1;
			goto cleanup;
		}

		mem_free(*basedir);
		*basedir = NULL;
		mem_free(locale);
		locale = NULL;
	}

cleanup:
	if (locale)
		mem_free(locale);
	return ret;
}

int display_help(const char *topic)
{
	const char *locale_info[] = {
		getenv("LANGUAGE"),
		getenv("LC_ALL"),
		getenv("LC_MESSAGE"),
		getenv("LANG")
	};
	char *basedir;
	char *path;
	int ret = 0;

	if (!topic)
		topic = "intro";

	if (!find_basedir(locale_info, ARRAY_SIZE(locale_info), &basedir))
		asprintf(&basedir, "%s", DOCDIR);

	asprintf(&path, "%s/%s.txt", basedir, topic);

	if (!io_file_exists(path) && keys_str2int(topic) > 0 &&
	    keys_get_action(keys_str2int(topic)) > 0) {
		int ch = keys_str2int(topic);
		enum key action = keys_get_action(ch);
		topic = keys_get_label(action);
		mem_free(path);
		asprintf(&path, "%s/%s.txt", basedir, topic);
	}

	if (!io_file_exists(path)) {
		if (!strcmp(topic, "generic-credits"))
			topic = "credits";
		else if (!strcmp(topic, "generic-help"))
			topic = "intro";
		else if (!strcmp(topic, "generic-save"))
			topic = "save";
		else if (!strcmp(topic, "generic-reload"))
			topic = "reload";
		else if (!strcmp(topic, "generic-copy"))
			topic = "copy_paste";
		else if (!strcmp(topic, "generic-paste"))
			topic = "copy_paste";
		else if (!strcmp(topic, "generic-change-view"))
			topic = "tab";
		else if (!strcmp(topic, "generic-import"))
			topic = "import";
		else if (!strcmp(topic, "generic-export"))
			topic = "export";
		else if (!strcmp(topic, "generic-goto"))
			topic = "goto";
		else if (!strcmp(topic, "generic-other-cmd"))
			topic = "other";
		else if (!strcmp(topic, "generic-config-menu"))
			topic = "config";
		else if (!strcmp(topic, "generic-add-appt"))
			topic = "general";
		else if (!strcmp(topic, "generic-add-todo"))
			topic = "general";
		else if (!strcmp(topic, "generic-prev-day"))
			topic = "general";
		else if (!strcmp(topic, "generic-next-day"))
			topic = "general";
		else if (!strcmp(topic, "generic-prev-week"))
			topic = "general";
		else if (!strcmp(topic, "generic-next-week"))
			topic = "general";
		else if (!strcmp(topic, "generic-prev-month"))
			topic = "general";
		else if (!strcmp(topic, "generic-next-month"))
			topic = "general";
		else if (!strcmp(topic, "generic-prev-year"))
			topic = "general";
		else if (!strcmp(topic, "generic-next-year"))
			topic = "general";
		else if (!strcmp(topic, "generic-goto-today"))
			topic = "general";
		else if (!strcmp(topic, "move-right"))
			topic = "displacement";
		else if (!strcmp(topic, "move-left"))
			topic = "displacement";
		else if (!strcmp(topic, "move-down"))
			topic = "displacement";
		else if (!strcmp(topic, "move-up"))
			topic = "displacement";
		else if (!strcmp(topic, "start-of-week"))
			topic = "displacement";
		else if (!strcmp(topic, "end-of-week"))
			topic = "displacement";
		else if (!strcmp(topic, "add-item"))
			topic = "add";
		else if (!strcmp(topic, "del-item"))
			topic = "delete";
		else if (!strcmp(topic, "edit-item"))
			topic = "edit";
		else if (!strcmp(topic, "view-item"))
			topic = "view";
		else if (!strcmp(topic, "pipe-item"))
			topic = "pipe";
		else if (!strcmp(topic, "flag-item"))
			topic = "flag";
		else if (!strcmp(topic, "repeat"))
			topic = "repeat";
		else if (!strcmp(topic, "edit-note"))
			topic = "enote";
		else if (!strcmp(topic, "view-note"))
			topic = "vnote";
		else if (!strcmp(topic, "raise-priority"))
			topic = "priority";
		else if (!strcmp(topic, "lower-priority"))
			topic = "priority";
		mem_free(path);
		asprintf(&path, "%s/%s.txt", basedir, topic);
	}

	if (io_file_exists(path)) {
		const char *arg[] = { conf.pager, path, NULL };
		wins_launch_external(arg);
		ret = 1;
	}

	mem_free(basedir);
	mem_free(path);
	return ret;
}
