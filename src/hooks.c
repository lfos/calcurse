/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
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

#include <stddef.h>
#include <sys/wait.h>

#include "calcurse.h"

int run_hook(const char *name)
{
	char *hook_path = NULL, *hook_cmd = NULL, *mesg;
	char const *arg[2];
	int pid, ret = -127;

	asprintf(&hook_path, "%s/%s", path_hooks, name);
	if (!io_file_exists(hook_path))
		goto cleanup;

	asprintf(&hook_cmd, "%s <&- >&- 2>&-", hook_path);
	arg[0] = hook_cmd;
	arg[1] = NULL;

	if ((pid = shell_exec(NULL, NULL, *arg, arg))) {
		ret = child_wait(NULL, NULL, pid);
		if (ret > 0 && WIFEXITED(ret)) {
			asprintf(&mesg, "%s hook: exit status %d",
				 name,
				 WEXITSTATUS(ret));
			que_ins(mesg, now(), 3);
			mem_free(mesg);
		} else if (ret != 0) {
			asprintf(&mesg, "%s hook: abnormal termination",
				 name);
			que_ins(mesg, now(), 4);
			mem_free(mesg);
		}
	}

cleanup:
	mem_free(hook_path);
	mem_free(hook_cmd);
	return ret;
}
