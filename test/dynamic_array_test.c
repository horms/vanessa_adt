/**********************************************************************
 * dynamic_array_test.c                                  September 2000
 * Horms                                             horms@verge.net.au
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2008  Horms
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
	vanessa_dynamic_array_t *a;
	char *str;
	int i;

	/* 
	 * Open logger to filehandle stderr
	 */
	vl = vanessa_logger_openlog_filehandle(stderr,
					       "dynamic_array_test",
					       LOG_DEBUG, 0);
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
	vanessa_logger_set(vl);


	/*
	 * Create a dynamic array
	 */
	printf("Creating Dynamic Array\n");
	if ((a = vanessa_dynamic_array_create(0,
					      VANESSA_DESTROY_INT,
					      VANESSA_DUPLICATE_INT,
					      VANESSA_DISPLAY_INT,
					      VANESSA_LENGTH_INT)) ==
	    NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_dynamic_array_create");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error creating dynamic array. Exiting.");
		exit(-1);
	}

	/*
	 * Insert some elements
	 */
	printf("Inserting Elements into Dynamic Array\n");
	for (i = 0; i < 8; i++) {
		if ((vanessa_dynamic_array_add_element(a, &i)) ==
		    NULL) {
			vanessa_logger_log(vl, LOG_ERR,
					   "main: vanessa_dynamic_array_add_element");
			vanessa_logger_log(vl, LOG_ERR,
					   "Fatal error creating adding element. Exiting.");
			exit(-1);
		}
	}

	/* 
	 * Display the contents
	 */
	printf("Displaying contents of Dynamic Array\n");
	if ((str = vanessa_dynamic_array_display(a, ',')) == NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_dynamic_array_display");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error displaying dynamic array. Exiting.");
		exit(-1);
	}
	printf("%s\n", str);
	free(str);

	/*
	 * Reverse the dynamic array
	 */
	printf("Reversing the contents of Dynamic Array\n");
	vanessa_dynamic_array_reverse(a);


	/* 
	 * Display the contents again
	 */
	printf("Displaying contents of Dynamic Array\n");
	if ((str = vanessa_dynamic_array_display(a, ',')) == NULL) {
		vanessa_logger_log(vl, LOG_DEBUG,
				   "main: vanessa_dynamic_array_display 2");
		vanessa_logger_log(vl, LOG_ERR,
				   "Fatal error displaying dynamic array. Exiting.");
		exit(-1);
	}
	printf("%s\n", str);
	free(str);

	/* 
	 * Clean Up
	 */
	printf("Cleaning Up\n");
	vanessa_dynamic_array_destroy(a);
	vanessa_adt_logger_unset();
	vanessa_logger_closelog(vl);

	exit(0);
}
