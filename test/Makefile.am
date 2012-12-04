AUTOMAKE_OPTIONS = foreign

TESTS = \
	true-001.sh \
	run-test-001.sh \
	run-test-002.sh \
	todo-001.sh \
	todo-002.sh \
	todo-003.sh \
	day-001.sh \
	day-002.sh \
	day-003.sh \
	range-001.sh \
	range-002.sh \
	range-003.sh \
	appointment-001.sh \
	next-001.sh \
	search-001.sh \
	bug-002.sh \
	recur-001.sh \
	recur-002.sh \
	recur-003.sh \
	recur-004.sh \
	recur-005.sh

TESTS_ENVIRONMENT = \
	CALCURSE='$(top_builddir)/src/calcurse' \
	DATA_DIR='$(top_srcdir)/test/data/'

AM_CFLAGS = -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L

check_PROGRAMS = run-test
check_SCRIPTS = $(TESTS)

run_test_SOURCES = run-test.c

EXTRA_DIST = \
	$(TESTS) \
	data/apts \
	data/apts-bug-002 \
	data/apts-recur \
	data/conf \
	data/todo
