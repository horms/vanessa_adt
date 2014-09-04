/**********************************************************************
 * config_file.c                                          November 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Read in a config and parse it into command line arguments,
 * return this as a dynamic array
 *
 * This code was written in Amsterdam. Hooray!
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "vanessa_adt.h"
#include "unused.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define MAX_LINE_LENGTH 4096   /* Its hard-coded, but its also
				  bloody long, so deal with it */


/**********************************************************************
 * vanessa_config_file_read_fd
 * Reads a configuration file from an file descriptor
 * that has been opend for reading. 
 * pre: filename: file to read configuration from
 *      flags: logical or of VANESSA_CONFIG_FILE_MULTI_VALUE,
 *             VANESSA_CONFIG_FILE_X and VANESSA_CONFIG_FILE_BLANK.
 *             VANESSA_CONFIG_FILE_NONE for no flags.
 *             VANESSA_CONFIG_FILE_X and VANESSA_CONFIG_FILE_BLANK
 *             may not be used together.
 * post: The file is parsed according to the following rules.
 *       Escaping and quoting is intended to be analogous to 
 *       how a shell (bash) handles these.
 *       o Each line begins with a key, optionally followed
 *         by some whitespace and a value 
 *         If flag is VANESSA_CONFIG_FILE_MULTI_VALUE
 *         then there may be multiple white-space delimited values
 *         Otherwise everything after the key and delimiting whitespace
 *         is considerd as one value
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
 *       o If flag includes VANESSA_CONFIG_FILE_BLANK
 *           key is not prefixed
 *         If flag includes VANESSA_CONFIG_FILE_X
 *           key is prefixed with a "-"
 *         Otherwise
 *           If a key is a single letter it is prefixed by a "-"
 *           Else the key is prefixed with "--"
 *         Note if flag contains both VANESSA_CONFIG_FILE_BLANK and
 *         VANESSA_CONFIG_FILE_X then the behaviour is undefined
 *        * If flag is VANESSA_CONFIG_FILE_MULTI_VALUE
 *           a NULL entry is inserted after each line
 *          Otherwise
 *           a "" entry is inserted as the first element in the
 *           dynamic array. This is intended to be a dummy argv[0]
 * return: dynamic array containin elements
 *         NULL on error
 **********************************************************************/

static void remove_trailing_whitespace(char *buf)
{
	size_t len;

	if(!buf) {
		return;
	}

	len = strlen(buf);

	while(len) {
		if(*(buf+len-1) != ' ' && *(buf+len-1) != '\t') {
			break;
		}
		len--;
		*(buf+len) = '\0';
	}
}

#define ADD_TOKEN(_a, _t) \
	remove_trailing_whitespace(_t+last_escaped); \
	if((_a=vanessa_dynamic_array_add_element(_a, _t))==NULL){ \
		VANESSA_LOGGER_DEBUG("config_file_read: " \
				"vanessa_dynamic_array_add_element"); \
		close(fd); \
		return(NULL); \
	} \

#define BEGIN_KEY \
	last_escaped = 0; \
	if(!in_escape && !in_comment && !in_quote){ \
		if(added_key && (flag & \
				VANESSA_CONFIG_FILE_MULTI_VALUE)) { \
			ADD_TOKEN(a, NULL); \
		} \
		in_key=1; \
		added_key=0; \
	}

#define END_KEY \
	if(!in_escape && in_key && !in_quote){ \
		if(in_key && token_pos){ \
			*(token_buffer+token_pos+2)='\0'; \
			tmp_token_buffer = token_buffer; \
			if(flag & VANESSA_CONFIG_FILE_BLANK) { \
				tmp_token_buffer += 2; \
			} \
			else if(flag & VANESSA_CONFIG_FILE_X || \
					!*(tmp_token_buffer+3)) { \
				tmp_token_buffer++; \
			} \
			ADD_TOKEN(a, tmp_token_buffer); \
			tmp_token_buffer++; \
			added_key=1; \
		} \
		token_pos=0; \
		in_key=0; \
	}

#define BEGIN_VALUE \
	last_escaped = 0; \
	if(!in_key && !in_comment && !in_quote){ \
		in_value=1; \
	}

#define END_VALUE \
	if(!in_escape && in_value && !in_quote){ \
		if(in_value){ \
			*(token_buffer+token_pos+2)='\0'; \
			ADD_TOKEN(a, token_buffer+2) ; \
		} \
		token_pos=0; \
		in_value=0; \
	}

#define END_COMMENT \
	if(!in_escape){ \
		in_comment=0; \
	}

#define BEGIN_COMMENT \
	if(!in_escape && !in_quote){ \
		in_comment=1; \
	}

#define BEGIN_ESCAPE \
	in_escape=1;

#define END_ESCAPE \
	if(in_escape) { \
		in_escape=0; \
		last_escaped = token_pos+1; \
	}

#define SINGLE_QUOTE 1
#define DOUBLE_QUOTE 2


vanessa_dynamic_array_t *vanessa_config_file_read_fd(int fd, 
		vanessa_adt_flag_t flag)
{
	vanessa_dynamic_array_t *a;
	size_t token_pos;
	ssize_t nread;
	char token_buffer[MAX_LINE_LENGTH];
	char read_buffer[MAX_LINE_LENGTH];
	char *tmp_token_buffer;
	char c;
	size_t max_token_pos = MAX_LINE_LENGTH - 3;
	int read_pos;
	struct stat stat_buf;

	int in_escape = 0;
	int in_comment = 0;
	size_t last_escaped = 0;
	int skip_char = 0;
	int in_value = 0;
	int in_quote = 0;
	int in_key = 1;
	int added_key = 0;

	if(fstat(fd, &stat_buf) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("stat");
		return(NULL);
	}

	a = vanessa_dynamic_array_create(0, VANESSA_DESTROY_STR,
		      VANESSA_DUPLICATE_STR, VANESSA_DISPLAY_STR,
		      VANESSA_LENGTH_STR);
	if (!a) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_create");
		return (NULL);
	}

	/*insert a dummy argv[0] into the dynamic array */
	if(! (flag & VANESSA_CONFIG_FILE_MULTI_VALUE) ) {
		ADD_TOKEN(a, "");
	}

	*token_buffer = '-';
	*(token_buffer + 1) = '-';
	token_pos = 0;

	while (1) {
		if ((nread = read(fd, read_buffer, 
						MAX_LINE_LENGTH)) < 0) {
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
				if(flag & VANESSA_CONFIG_FILE_MULTI_VALUE) {
					END_VALUE;
				}
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
				if (in_escape || in_quote & SINGLE_QUOTE) {
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
				if (!in_escape && !in_comment) {
					if (in_quote & DOUBLE_QUOTE) {
						in_quote ^=
						    in_quote &
						    DOUBLE_QUOTE;
						last_escaped = token_pos;
						skip_char = 1;
					}
					else if (!(in_quote & SINGLE_QUOTE)) {
						in_quote |= DOUBLE_QUOTE;
						skip_char = 1;
					}
				}
				END_ESCAPE;
				break;
			case '\'':
				BEGIN_VALUE;
				if (!in_escape && !in_comment) {
					if (in_quote & SINGLE_QUOTE) {
						in_quote ^= SINGLE_QUOTE;
						last_escaped = token_pos;
						skip_char = 1;
					}
					else if (! (in_quote & DOUBLE_QUOTE)) {
						in_quote |= SINGLE_QUOTE;
						skip_char = 1;
					}
				}
				END_ESCAPE;
				break;
			default:
				BEGIN_VALUE;
				END_ESCAPE;
				break;
			}

			if (in_key | in_value && 
					c != '\n' && c != '\r' &&
					!in_escape && !skip_char && 
					token_pos < max_token_pos) {
				*(token_buffer + token_pos + 2) = c;
				token_pos++;
			}
			skip_char = 0;
		}
	}

	return (a);
}


/**********************************************************************
 * vanessa_config_file_read
 * Read in a config file and put elements in a dynamic array
 * pre: filename: file to read configuration from
 *      flags: passed to vanessa_config_file_read_fd
 * post: File is opened read only
 *       File is checked, see vanessa_config_file_read for details
 *       File is closed
 * return: dynamic array containin elements
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_config_file_read(const char *filename, 
		vanessa_adt_flag_t flag)
{
	vanessa_dynamic_array_t *a;
	int fd;

	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		VANESSA_LOGGER_DEBUG_UNSAFE("open(%s): %s", filename,
					    strerror(errno));
		return (NULL);
	}

	a = vanessa_config_file_read_fd(fd, flag);
	if(!a) {
		VANESSA_LOGGER_DEBUG("vanessa_config_file_read");
		return (NULL);
	}

	close(fd);

	return(a);
}


/**********************************************************************
 * vanessa_mode_str
 * Make an asscii representation of a mode in "rwx" form.
 * e.g. -rwx------
 * pre: mode: mode to format
 *      mode_num: format to fill in, really just a string
 * post: mode_num is filled in
 * return: mode_num on success
 *         NULL on error (there are no errors)
 **********************************************************************/

vanessa_mode_str_t *vanessa_mode_str(mode_t mode, vanessa_mode_str_t *mode_str)
{
	memset(mode_str->mode_str, '-', 10);
	mode_str->mode_str[10] = '\0';

	switch(mode & S_IFMT) {
		case S_IFSOCK:
			mode_str->mode_str[0] = 's';
		case S_IFLNK:
			mode_str->mode_str[0] = 'l';
		case S_IFREG:
			mode_str->mode_str[0] = '-';
		case S_IFBLK:
			mode_str->mode_str[0] = 'b';
		case S_IFDIR:
			mode_str->mode_str[0] = 'd';
		case S_IFCHR:
			mode_str->mode_str[0] = 'c';
		case S_IFIFO:
			mode_str->mode_str[0] = 'p';
	}

	if(mode & S_IRUSR) {
		mode_str->mode_str[1] = 'r';
	}
	if(mode & S_IWUSR) {
		mode_str->mode_str[2] = 'w';
	}
	if(mode & S_IXUSR) {
		mode_str->mode_str[3] = 'x';
	}
	if(mode & S_ISUID ) {
		if(mode & S_IXUSR) {
			mode_str->mode_str[3] = 's';
		}
		else {
			mode_str->mode_str[3] = 'S';
		}
	}

	if(mode & S_IRGRP) {
		mode_str->mode_str[4] = 'r';
	}
	if(mode & S_IWGRP) {
		mode_str->mode_str[5] = 'w';
	}
	if(mode & S_IXGRP) {
		mode_str->mode_str[6] = 'x';
	}
	if(mode & S_ISGID ) {
		if(mode & S_IXGRP) {
			mode_str->mode_str[6] = 's';
		}
		else {
			mode_str->mode_str[6] = 'S';
		}
	}

	if(mode & S_IROTH) {
		mode_str->mode_str[7] = 'r';
	}
	if(mode & S_IWOTH) {
		mode_str->mode_str[8] = 'w';
	}
	if(mode & S_IXOTH) {
		mode_str->mode_str[9] = 'x';
	}
	if(mode & S_ISVTX ) {
		if(mode & S_IXOTH) {
			mode_str->mode_str[6] = 't';
		}
		else {
			mode_str->mode_str[6] = 'T';
		}
	}

	return(mode_str);
}


/**********************************************************************
 * vanessa_mode_num_str
 * Make an asscii representation of a mode in numerical form.
 * e.g. 0600
 * pre: mode: mode to format
 *      mode_num_str: format to fill in, really just a string
 * post: mode_num_str is filled in
 * return: mode_num_str on success
 *         NULL on error (there are no errors)
 **********************************************************************/

vanessa_mode_num_str_t *vanessa_mode_num_str(mode_t mode, 
		vanessa_mode_num_str_t *mode_num_str)
{
	mode_t mode_num;

	mode_num = 0;
	memset(mode_num_str->mode_str, 0, 5);

	/* NB: Numeric mode representations usually don't include
	 * the file type bits, so we won't either */

	if(mode & S_ISUID) {
		mode_num |= 0x4000;
	}
	if(mode & S_ISGID) {
		mode_num |= 0x2000;
	}
	if(mode & S_ISVTX) {
		mode_num |= 0x1000;
	}

	if(mode & S_IRUSR) {
		mode_num |= 0x0400;
	}
	if(mode & S_IWUSR) {
		mode_num |= 0x0200;
	}
	if(mode & S_IXUSR) {
		mode_num |= 0x0100;
	}

	if(mode & S_IRGRP) {
		mode_num |= 0x0040;
	}
	if(mode & S_IWGRP) {
		mode_num |= 0x0020;
	}
	if(mode & S_IXGRP) {
		mode_num |= 0x0010;
	}

	if(mode & S_IROTH) {
		mode_num |= 0x0004;
	}
	if(mode & S_IWOTH) {
		mode_num |= 0x0002;
	}
	if(mode & S_IXOTH) {
		mode_num |= 0x0010;
	}

	snprintf(mode_num_str->mode_str, 5, "%04x", mode_num);

	return(mode_num_str);
}


/**********************************************************************
 * vanessa_config_file_check_permission_fd
 * Check the permissions, ownership and mode of a file
 * Intended for use on files whose permissions need
 * to be strictly enforced for some reason.
 * pre: fd: Open file discriptor to file to check
 *      uid: desired uid
 *      gid: desired gid
 *      mode: desired mode
 *      flag: logical or of:
 *              VANESSA_CONFIG_FILE_CHECK_UID  to check uid
 *              VANESSA_CONFIG_FILE_CHECK_GID  to check gid
 *              VANESSA_CONFIG_FILE_CHECK_MODE to check mode
 *              VANESSA_CONFIG_FILE_CHECK_FILE to check that it is
 *                                             a regular file
 *              VANESSA_CONFIG_FILE_CHECK_ALL  all of the above
 * post: checks are performed
 * return: 0 if the file pases the checks
 *         -1 otherwise
 **********************************************************************/

int vanessa_config_file_check_permission_fd(int fd, uid_t UNUSED(uid),
					    gid_t UNUSED(gid),
					    mode_t UNUSED(mode),
					    vanessa_adt_flag_t flag)
{
	vanessa_mode_str_t mode_a;
	vanessa_mode_str_t mode_b;
	vanessa_mode_num_str_t mode_num_a;
	vanessa_mode_num_str_t mode_num_b;
	struct stat stat_buf;
	struct passwd *pw_buf;
	struct group *gr_buf;
	char *str;
	char *estr;
	gid_t egid;
	uid_t euid;

	if(fstat(fd, &stat_buf) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fstat");
		return(-1);
	}

	if(flag & VANESSA_CONFIG_FILE_CHECK_FILE) {
		if(!S_ISREG(stat_buf.st_mode)) {
		 	VANESSA_LOGGER_DEBUG("not a regular file");
		 	return(-1);
		}
	}

	if(flag & VANESSA_CONFIG_FILE_CHECK_UID) {
		euid = geteuid();
		if(stat_buf.st_uid != euid) {
			pw_buf = getpwuid(euid);
			estr = (pw_buf && pw_buf->pw_name) ? 
				pw_buf->pw_name : "";
			estr = strdup(estr);
			if(!estr) {
				VANESSA_LOGGER_DEBUG_ERRNO("strdup");
				return(-1);
			}
			pw_buf = getpwuid(stat_buf.st_uid);
			str = (pw_buf && pw_buf->pw_name) ? 
				pw_buf->pw_name : "";
			VANESSA_LOGGER_DEBUG_UNSAFE("owned by %s (%d) " 
					"instead of %s (%d)",
					str, stat_buf.st_uid, estr, 
					euid);
			free(estr);
			return(-1);
		}
	}

	if(flag & VANESSA_CONFIG_FILE_CHECK_GID) {
		egid = getegid();
		if(stat_buf.st_gid != egid) {
			gr_buf = getgrgid(egid);
			estr = (gr_buf && gr_buf->gr_name) ? 
				gr_buf->gr_name : "";
			estr = strdup(estr);
			if(!estr) {
				VANESSA_LOGGER_DEBUG_ERRNO("strdup");
				return(-1);
			}
			gr_buf = getgrgid(stat_buf.st_gid);
			str = (gr_buf && gr_buf->gr_name) ? 
				gr_buf->gr_name : "";
			VANESSA_LOGGER_DEBUG_UNSAFE("group %s (%d) " 
					"instead of %s (%d)",
				str, stat_buf.st_gid, estr, egid);
			free(estr);
			return(-1);
		}
	}

	if(flag & VANESSA_CONFIG_FILE_CHECK_MODE) {
		if((stat_buf.st_mode&(~S_IFMT)) != (S_IRUSR|S_IWUSR)) {
			vanessa_mode_str(stat_buf.st_mode&(~S_IFMT), &mode_a);
			vanessa_mode_str((S_IRUSR|S_IWUSR), &mode_b);
			vanessa_mode_num_str(stat_buf.st_mode&(~S_IFMT), 
					&mode_num_a);
			vanessa_mode_num_str((S_IRUSR|S_IWUSR), &mode_num_b);
	 		VANESSA_LOGGER_DEBUG_UNSAFE(
				 	"mode %s (%s) instead of %s (%s)", 
				 	mode_num_a.mode_str, 
					mode_a.mode_str, mode_num_b.mode_str, 
					mode_b.mode_str);
		 	return(-1);
		}
	}

	return(0);
}


/**********************************************************************
 * vanessa_config_file_check_permission
 * Check the permissions, ownership and mode of a file
 * pre: filename: file to check
 *      see vanessa_config_file_check() for other arguments
 * post: File is opened read only
 *       File is checked, see vanessa_config_file_check() for details
 *       File is closed
 * return: 0 if file passes checks
 *         N.B: you must have permision to open the file for reading
 *         -1 on error
 **********************************************************************/

int vanessa_config_file_check_permission(const char *filename, 
		uid_t uid, gid_t gid, mode_t mode, vanessa_adt_flag_t flag)
{
	int status;
	int fd;

	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		return (-1);
	}

	status = vanessa_config_file_check_permission_fd(fd, uid, gid, mode, 
			flag);

	close(fd);

	return(status);
}


/**********************************************************************
 * vanessa_config_file_check_exits_fd
 * Check the that file exists and is a regular file or 
 * a symlink to a regular file.
 * pre: fd: Open file discriptor to file to check
 * post: checks are performed
 * return: 0 if the file pases the checks
 *         -1 otherwise
 **********************************************************************/

int vanessa_config_file_check_exits_fd(int fd)
{
	return(vanessa_config_file_check_permission_fd(fd, 0, 0, 0,
			VANESSA_CONFIG_FILE_CHECK_FILE));
}


/**********************************************************************
 * vanessa_config_file_check_exits
 * Check that a file exists and is a regular file or 
 * a symlink to a regular file.
 * pre: filename: file to check
 *      see vanessa_config_file_check_exits_fd() for other arguments
 * post: File is opened read only
 *       File is checked, see vanessa_config_file_check_exits_fd() for details
 *       File is closed
 * return: 0 if file passes checks
 *         N.B: you must have permision to open the file for reading
 *         -1 on error
 **********************************************************************/

int vanessa_config_file_check_exits(const char *filename)
{
	int status;
	int fd;

	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		return (-1);
	}

	status = vanessa_config_file_check_exits_fd(fd);

	close(fd);

	return(status);
}

