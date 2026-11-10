/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2023 calcurse Development Team <misc@calcurse.org>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

/*
 * Fork and execute an external process.
 *
 * If pfdin and/or pfdout point to a valid address, a pipe is created and the
 * appropriate file descriptors are written to pfdin/pfdout.
 */
static int
fork_exec(int *pfdin, int *pfdout, const char *path, char *const *arg)
{
	int pin[2], pout[2];
	int pid;

	if (pfdin && (pipe(pin) == -1))
		return 0;
	if (pfdout && (pipe(pout) == -1))
		return 0;

	if ((pid = fork()) == 0) {
		if (pfdout) {
			if (dup2(pout[0], STDIN_FILENO) < 0)
				_exit(127);
			close(pout[0]);
			close(pout[1]);
		}

		if (pfdin) {
			if (dup2(pin[1], STDOUT_FILENO) < 0)
				_exit(127);
			close(pin[0]);
			close(pin[1]);
		}

		execvp(path, arg);
		_exit(127);
	} else {
		if (pfdin)
			close(pin[1]);
		if (pfdout)
			close(pout[0]);

		if (pid > 0) {
			if (pfdin) {
				fcntl(pin[0], F_SETFD, FD_CLOEXEC);
				*pfdin = pin[0];
			}
			if (pfdout) {
				fcntl(pout[1], F_SETFD, FD_CLOEXEC);
				*pfdout = pout[1];
			}
		} else {
			if (pfdin)
				close(pin[0]);
			if (pfdout)
				close(pout[1]);
			return 0;
		}
	}
	return pid;
}

/* Wait for a child process to terminate. */
static int child_wait(int *pfdin, int *pfdout, int pid)
{
	int stat;

	if (pfdin)
		close(*pfdin);
	if (pfdout)
		close(*pfdout);

	waitpid(pid, &stat, 0);
	return stat;
}

/* Print error message and bail out. */
static void die(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	fprintf(stderr, "error: ");
	vfprintf(stderr, format, arg);
	va_end(arg);

	exit(1);
}

/* Read the remaining contents of a stream into a null-terminated buffer. */
static char *read_all(FILE *fp)
{
	size_t len = 0, size = BUFSIZ, n;
	char *buf = malloc(size);

	if (!buf)
		die("out of memory\n");
	while ((n = fread(buf + len, 1, size - len - 1, fp)) > 0) {
		len += n;
		if (size - len - 1 == 0) {
			size *= 2;
			if (!(buf = realloc(buf, size)))
				die("out of memory\n");
		}
	}
	buf[len] = '\0';

	return buf;
}

/* Return the number of the first line in which two strings differ. */
static int first_diff_line(const char *s1, const char *s2)
{
	int line = 1;

	for (; *s1 && *s1 == *s2; s1++, s2++) {
		if (*s1 == '\n')
			line++;
	}

	return line;
}

/* Print the status of a child process to stderr. */
static void print_status(const char *what, int stat)
{
	if (WIFEXITED(stat))
		fprintf(stderr, "%s: exit status %d\n", what,
			WEXITSTATUS(stat));
	else if (WIFSIGNALED(stat))
		fprintf(stderr, "%s: killed by signal %d\n", what,
			WTERMSIG(stat));
}

/* Print the outputs of a failed test to stderr. */
static void print_failure(const char *name, const char *expected,
			  const char *actual, int stat1, int stat2,
			  int expect_failure)
{
	fprintf(stderr, "\n=== %s failed", name);
	if (expect_failure) {
		fprintf(stderr, " (expected a failure, but test passed)\n");
		return;
	}
	fprintf(stderr, " ===\n");
	print_status("expected", stat1);
	print_status("actual", stat2);
	if (strcmp(expected, actual) != 0)
		fprintf(stderr, "outputs differ starting at line %d\n",
			first_diff_line(expected, actual));
	fprintf(stderr, "--- expected output:\n%s", expected);
	if (*expected && expected[strlen(expected) - 1] != '\n')
		fputc('\n', stderr);
	fprintf(stderr, "--- actual output:\n%s", actual);
	if (*actual && actual[strlen(actual) - 1] != '\n')
		fputc('\n', stderr);
	fprintf(stderr, "--- end of output\n");
}

/* Print usage message. */
static void usage(void)
{
	printf("usage: run-test [-h|--help] <test>...\n");
}

/* Run test with a specific name. */
static int run_test(const char *name, int expect_failure)
{
	char filename[BUFSIZ];
	char *arg1[3], *arg2[3];
	int pid1 = -1, pin1, pid2 = -1, pin2;
	FILE *fpin1 = NULL, *fpin2 = NULL;
	char *out1, *out2;
	int stat1, stat2;
	int ret = 1;

	if (snprintf(filename, BUFSIZ, "./%s", name) >= BUFSIZ)
		die("file name too long\n");

	if (access(filename, F_OK) != 0) {
		if (snprintf(filename, BUFSIZ, "./%s.sh", name) >= BUFSIZ)
			die("file name too long\n");

		if (access(filename, F_OK) != 0)
			die("test not found: %s\n", name);
	}

	if (access(filename, X_OK) != 0)
		die("script is not executable: %s\n", filename);

	arg1[0] = arg2[0] = filename;
	arg1[1] = "expected";
	arg2[1] = "actual";
	arg1[2] = arg2[2] = NULL;

	printf("Running %s...", name);

	if ((pid1 = fork_exec(&pin1, NULL, *arg1, arg1)) < 0)
		die("failed to execute %s: %s\n", filename,
		    strerror(errno));

	if ((pid2 = fork_exec(&pin2, NULL, *arg2, arg2)) < 0)
		die("failed to execute %s: %s\n", filename,
		    strerror(errno));

	fpin1 = fdopen(pin1, "r");
	fpin2 = fdopen(pin2, "r");
	if (!fpin1 || !fpin2)
		die("failed to read output of %s: %s\n", filename,
		    strerror(errno));

	out1 = read_all(fpin1);
	out2 = read_all(fpin2);
	if (strcmp(out1, out2) != 0)
		ret = 0;

	fclose(fpin1);
	fclose(fpin2);

	stat1 = child_wait(NULL, NULL, pid1);
	stat2 = child_wait(NULL, NULL, pid2);
	if (stat1 != 0 || stat2 != 0)
		ret = 0;

	if (expect_failure)
		ret = 1 - ret;

	if (ret == 1) {
		printf(" ok\n");
	} else {
		printf(" FAIL\n");
		fflush(stdout);
		print_failure(name, out1, out2, stat1, stat2, expect_failure);
	}

	free(out1);
	free(out2);

	return ret;
}

int main(int argc, char **argv)
{
	int i;

	if (!argv[1])
		die("no tests specified, bailing out\n");
	else if (strcmp(argv[1], "-h") == 0
		 || strcmp(argv[1], "--help") == 0) {
		usage();
		return 0;
	}

	for (i = 1; i < argc; i++) {
		if (*argv[i] == '!') {
			if (!run_test(argv[i] + 1, 1))
				return 1;
		} else {
			if (!run_test(argv[i], 0))
				return 1;
		}
	}

	return 0;
}
