/**********************************************************************
 * hash_test.c                                                July 2004
 * Simon Horman                                      horms@verge.net.au
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 **********************************************************************/

#include <vanessa_adt.h>
#include <vanessa_logger.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int reader(int fd[2])
{
	char *str;
	vanessa_dynamic_array_t *a;

	vanessa_logger_reopen(vanessa_logger_get());

	a = vanessa_config_file_read_fd(fd[0], 0);

	str = vanessa_dynamic_array_display(a, '\n');
	if (str < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_display");
		return -1;
	}
	printf ("Actual Result\n"
		"===begin===\n"
		"%s\n"
		"===end===\n\n", str);
	free(str);

	return 0;
}

static ssize_t write_str(int fd, const char *str)
{
	ssize_t n;
	ssize_t offset;
	ssize_t count;

	count = strlen(str);
	for (offset = 0; offset < count; offset += n) {
		n = write(fd, str, strlen(str));
		if (n < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("write");
			return n;
		}
		if (!n)
			return n;
	}

	return offset;
}

static int writer(int fd[2])
{

	printf ("Expected Result:\n"
		"===begin===\n"
		"\n"
		"--a=b\n"
		"--a=b\n"
		"--a=b\n"
		"--a=\\\\\n"
		"--a=\\\n"
		"--a=\'b\'\n"
		"--a=\"b\"\n"
		"--a=b\n"
		"--a=b\n"
		"--a=\'b\'\n"
		"--a=\"b\"\n"
		"===end===\n\n");

	write_str(fd[1], "a=b\n");
	write_str(fd[1], "a=\"b\"\n");
	write_str(fd[1], "a=\'b\'\n");
	write_str(fd[1], "a=\'\\\\\'\n");
	write_str(fd[1], "a=\"\\\\\"\n");
	write_str(fd[1], "a=\"\'b\'\"\n");
	write_str(fd[1], "a=\'\"b\"\'\n");
	write_str(fd[1], "a=\'b\'\n");
	write_str(fd[1], "a=\"b\"\n");
	write_str(fd[1], "a=\\\'b\\\'\n");
	write_str(fd[1], "a=\\\"b\\\"\n");
	return 0;
}


int main(int argc, char **argv)
{
	vanessa_logger_t *vl;
	pid_t child;
	int fd[2];

	/* 
	 * Open logger to filehandle stderr
	 */
	vl = vanessa_logger_openlog_filehandle(stderr,
					       "config_file_test",
					       LOG_DEBUG, 0);
	if (vl == NULL) {
		fprintf(stderr,
			"Error: vanessa_logger_openlog_filehandle\n");
		fprintf(stderr,
			"Fatal Error registering logger. Exiting.\n");
		return -1;
	}

	/*
	 * Set this as the logger for this programme
	 */
	vanessa_logger_set(vl);

	if (pipe(fd) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("pipe");
		return -1;
	}

#define NO_FORK
#ifdef NO_FORK
	if (writer(fd) < 0) {
		VANESSA_LOGGER_DEBUG("writer");
		return -1;
	}
	close (fd[1]);

	if (reader(fd) < 0) {
		VANESSA_LOGGER_DEBUG("reader");
		return -1;
	}
	close (fd[0]);
#else
	child = fork();
	if (child < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fork");
		return -1;
	}
	if (!child) {
		close (fd[1]);
		if (reader(fd) < 0) {
			VANESSA_LOGGER_DEBUG("reader");
		}
		close (fd[0]);
		exit(0);
	}

	if (writer(fd) < 0) {
		close (fd[0]);
		VANESSA_LOGGER_DEBUG("writer");
		close (fd[1]);
	}
	waitpid(child, NULL, 0);
#endif

	return 0;
}
