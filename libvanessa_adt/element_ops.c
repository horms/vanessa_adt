/**********************************************************************
 * element_opts.c                                          January 2002
 * Horms                                             horms@vergenet.net
 *
 * Operations for use in conjunction with vanessa_dynamic_array_create
 * and vanessa_list_create.
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

#include "vanessa_adt.h"
#include "logger.h"


/**********************************************************************
 * vanessa_destroy_int
 * Dummy function to "destroy" an int.
 * DOES NOTHING
 * pre: i: int to "destroy"
 * post: none
 * return: none
 **********************************************************************/

void vanessa_destroy_int(int i)
{;
}


/**********************************************************************
 * vanessa_dup_int
 * Dummy function to "duplicate" an int.
 * DOES NOTHING
 * pre: i: int to "destroy"
 * post: none
 * return: i is returned cast to a void *
 **********************************************************************/

void *vanessa_dup_int(int i)
{
	return ((void *) i);
}


/**********************************************************************
 * vanessa_display_int
 * Display an int in ASCII as a decimal.
 * pre: d: buffer to display ASCII represetation of int to
 *      i: int to display
 * post: an ASCII representation of i is in d
 * return: none
 **********************************************************************/

void vanessa_display_int(char *d, int i)
{
	sprintf(d, "%d", i);
}


/**********************************************************************
 * vanessa_length_int
 * Return the length in bytes of an ASCII representation of the in as a
 * decimal.
 * pre: i: int to find the "length" of
 * post: none
 * return: length is returned
 **********************************************************************/

size_t vanessa_length_int(int i)
{
	int j = 1;

	if (i < 0) {
		j++;
		i = -i;
	}
	while (i > 10) {
		i /= 10;
		j++;
	}
	return (j);
}


/**********************************************************************
 * vanessa_match_int
 * Compare two integers. Return the difference. That is < 1 if a < b,
 * 0 if a == b and > 1 if a > b.
 * Analogous to strcmp(3)
 * pre: a: int to compare
 *      b: other int to compare
 * post: none
 * return: difference between a and b
 **********************************************************************/

int vanessa_match_int(int a, int b)
{
	if(a < b) {
		return(a - b);
	}
	return(b - a);
}
