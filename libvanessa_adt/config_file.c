/**********************************************************************
 * config_file.c                                          November 1999
 * Horms                                             horms@vergenet.net
 *
 * Read in a config and parse it into command line arguments,
 * return this as a dynamic array
 *
 * This code was written in Amsterdam. Hooray!
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2002  Horms
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "vanessa_adt.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define MAX_LINE_LENGTH 4096   /* Its hard-coded, but its also
				  bloody long, so deal with it */


/**********************************************************************
 * vanessa_config_file_read
 * Read in a config file and put elements in a dynamic array
 * pre: filename: file to read configuration from
 *      flags: unused
 * post: The file is parsed according to the following rules.
 *       Escaping and quoting is intended to be analogous to 
 *       how a shell (bash) handles these.
 *       o Each line begins with a key, optionally folled
 *         by some whitespace and a value 
 *       o Leading whitespace is ignored
 *       o Blank lines are ignored
 *       o Anything after a # (hash) on a line is ignored
 *       o If a \ precedes a new line then the lines will be concatenated
 *       o If a \ precedes any other character, including a # (hash)
 *         it will be treated as a literal
 *       o Anything inside single quotes (') will be treated as a litreal.
 *       o Anything other than a (') inside double quotes (") will be
 *         treated as a litreal.
 *       o Whitespace in keys must be escaped or quoted.
 *       o Whitespace in values need not be escaped or quoted.
 *       o If a key is a single letter it is prefixed by a "-"
 *         Else the key is prefixed with "--"
 * return: dynamic array containin elements
 *         NULL on error
 **********************************************************************/

#define ADD_TOKEN(_a, _t) \
	if((_a=vanessa_dynamic_array_add_element(_a, _t))==NULL){ \
		VANESSA_LOGGER_DEBUG("config_file_read: " \
				"vanessa_dynamic_array_add_element"); \
		close(fd); \
		return(NULL); \
	}

#define BEGIN_KEY \
	if(!in_escape && !in_comment && !in_quote){ \
		in_key=1; \
	} \

#define END_KEY \
	if(!in_escape && in_key && !in_quote){ \
		if(in_key && token_pos){ \
			*(token_buffer+token_pos+2)='\0'; \
			ADD_TOKEN(a, ((token_pos==1)? \
					token_buffer+1:token_buffer)) ; \
		} \
		token_pos=0; \
		in_key=0; \
	} \

#define BEGIN_VALUE \
	if(!in_key && !in_comment && !in_quote){ \
		in_value=1; \
	} \

#define END_VALUE \
	if(!in_escape && in_value && !in_quote){ \
		if(in_value){ \
			*(token_buffer+token_pos+2)='\0'; \
			ADD_TOKEN(a, token_buffer+2) ; \
		} \
		token_pos=0; \
		in_value=0; \
	} \

#define END_COMMENT \
	if(!in_escape){ \
		in_comment=0; \
	} \

#define BEGIN_COMMENT \
	if(!in_escape && !in_quote){ \
		in_comment=1; \
	} \

#define BEGIN_ESCAPE \
	in_escape=1;

#define END_ESCAPE \
	in_escape=0;

#define SINGLE_QUOTE 1
#define DOUBLE_QUOTE 2

vanessa_dynamic_array_t *vanessa_config_file_read(const char *filename, 
		vanessa_adt_flag_t flag)
{
	vanessa_dynamic_array_t *a;
	size_t token_pos;
	size_t nread;
	char token_buffer[MAX_LINE_LENGTH];
	char read_buffer[MAX_LINE_LENGTH];
	char c;
	int max_token_pos = MAX_LINE_LENGTH - 3;
	int read_pos;
	int fd;

	int in_escape = 0;
	int in_comment = 0;
	int skip_char = 0;
	int in_value = 0;
	int in_quote = 0;
	int in_key = 0;

	extern int errno;

	if (filename == NULL)
		return (NULL);
	if ((fd = open(filename, O_RDONLY)) < 0) {
		VANESSA_LOGGER_DEBUG_UNSAFE("open(%s): %s", filename,
					    strerror(errno));
		return (NULL);
	}

	if ((a = vanessa_dynamic_array_create(0,
					      VANESSA_DESTROY_STR,
					      VANESSA_DUPLICATE_STR,
					      VANESSA_DISPLAY_STR,
					      VANESSA_LENGTH_STR)) ==
	    NULL) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_create");
		return (NULL);
	}

	/*insert a dummy argv[0] into the dynamic array */
	ADD_TOKEN(a, "");

	*token_buffer = '-';
	*(token_buffer + 1) = '-';
	token_pos = 0;

	while (1) {
		if ((nread = read(fd, read_buffer, MAX_LINE_LENGTH)) < 0) {
			if (errno == EINTR) {
				continue;
			}
			VANESSA_LOGGER_DEBUG("read");
			vanessa_dynamic_array_destroy(a);
			close(fd);
			return (NULL);
		}
		if (nread == 0) {
			break;
		}

		for (read_pos = 0; read_pos < nread; read_pos++) {
			c = *(read_buffer + read_pos);

			switch (c) {
			case ' ':
			case '\t':
				END_KEY;
				if (in_escape) {
					BEGIN_VALUE;
				}
				END_ESCAPE;
				break;
			case '\n':
			case '\r':
				END_KEY;
				END_COMMENT;
				END_VALUE;
				BEGIN_KEY;
				END_ESCAPE;
				break;
			case '\\':
				if (in_escape || in_quote) {
					END_ESCAPE;
				} else {
					BEGIN_ESCAPE;
				}
				BEGIN_VALUE;
				break;
			case '#':
				BEGIN_COMMENT;
				END_KEY;
				END_VALUE;
				BEGIN_VALUE;
				END_ESCAPE;
				break;
			case '"':
				BEGIN_VALUE;
				if (!in_escape && !in_comment
				    && !(in_quote & SINGLE_QUOTE)) {
					if (in_quote & DOUBLE_QUOTE) {
						in_quote ^=
						    in_quote &
						    DOUBLE_QUOTE;
					} else {
						in_quote |= DOUBLE_QUOTE;
					}
					skip_char = 1;
				}
				END_ESCAPE;
				break;
			case '\'':
				BEGIN_VALUE;
				if (!in_escape && !in_comment) {
					if (in_quote & SINGLE_QUOTE) {
						in_quote ^= SINGLE_QUOTE;
					} else {
						in_quote |= SINGLE_QUOTE;
					}
					skip_char = 1;
				}
				END_ESCAPE;
				break;
			default:
				BEGIN_VALUE;
				END_ESCAPE;
				break;
			}

			if (in_key | in_value &&
			    c != '\n' &&
			    c != '\r' &&
			    !in_escape &&
			    !skip_char && token_pos < max_token_pos) {
				*(token_buffer + token_pos + 2) = c;
				token_pos++;
			}
			skip_char = 0;
		}
	}

	close(fd);
	return (a);
}
