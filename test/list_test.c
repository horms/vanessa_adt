/**********************************************************************
 * list_test.c                                             January 2002
 * Horms                                             horms@vergenet.net
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 2000  Horms
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


/**********************************************************************
 * Muriel the main function
 **********************************************************************/

int main(int argc, char **argv)
{
	vanessa_logger_t *vl;
	vanessa_list_t *l;
	char *str;
	int i;

	/* 
	 * Open logger to filehandle stderr
	 */
	vl = vanessa_logger_openlog_filehandle(stderr,
					       "list_test", LOG_DEBUG, 0);
	if (vl == NULL) {
		fprintf(stderr,
			"Error: vanessa_logger_openlog_filehandle\n");
		fprintf(stderr,
			"Fatal Error registering logger. Exiting.\n");
		exit(-1);
	}

	/*
	 * Set this as the logger for this programme
	 */
	vanessa_adt_logger_set(vl);


	/*
	 * Create a dynamic array
	 */
	printf("Creating List\n");
	if ((l = vanessa_list_create(-1, VANESSA_DESTROY_INT,
				     VANESSA_DUPLICATE_INT,
				     VANESSA_MATCH_INT,
				     VANESSA_DISPLAY_INT,
				     VANESSA_LENGTH_INT)) == NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_list_create");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error creating dynamic array. Exiting.");
		exit(-1);
	}

	/*
	 * Insert some elements
	 */
	printf("Inserting Elements into List\n");
	for (i = 0; i < 8; i++) {
		if ((vanessa_list_add_element(l, (void *) i)) == NULL) {
			vanessa_logger_log(vl, LOG_ERR,
					   "main: vanessa_list_add_element");
			vanessa_logger_log(vl, LOG_ERR,
					   "Fatal error creating adding element. Exiting.");
			exit(-1);
		}
	}

	/* 
	 * Display the contents
	 */
	printf("Displaying contents of List\n");
	if ((str = vanessa_list_display(l, ',')) == NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_list_display");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error displaying dynamic array. Exiting.");
		exit(-1);
	}
	printf("%s\n", str);
	free(str);

	/*
	 * Delete an element
	 */
	printf("Deleting the evil element \"6\"\n");
	vanessa_list_remove_element(l, (void *)6);

	/* 
	 * Display the contents
	 */
	printf("Displaying contents of List\n");
	if ((str = vanessa_list_display(l, ',')) == NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_list_display");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error displaying dynamic array. Exiting.");
		exit(-1);
	}
	printf("%s\n", str);
	free(str);

	/*
	 * Find an element
	 */
	printf("Finding element \"5\"\n");
	i = (int)vanessa_list_get_element(l, (void *)5);
	if( i != 5 ) {
		vanessa_logger_log(vl, LOG_DEBUG,
				"main: vanessa_get_element");
		vanessa_logger_log(vl, LOG_ERR,
				"Fatal error retrieving element. Exiting.");
		exit(-1);
	}
	printf("%d\n", i);

	/*
	 * Counting the Elements
	 */
	printf("Counting the elements\n");
	i = (int)vanessa_list_get_count(l);
	printf("%d\n", i);

	/* 
	 * Clean Up
	 */
	printf("Cleaning Up\n");
	vanessa_list_destroy(l);
	vanessa_adt_logger_unset();
	vanessa_logger_closelog(vl);

	exit(0);
}
