/**********************************************************************
 * vanessa_dynamic_array.c                                November 1999
 * Horms                                             horms@verge.net.au
 *
 * Dynamic array, to store all your flims in. 
 *
 * The primitive type for the array is void *. Thus, providing your own
 * duplicate_primitive, destroy_primitive, display_primitive and
 * length_primitive functions will allow you to use the
 * vanessa_dynamic_array API to have a dynamic array containing any
 * primitive
 *
 * Includes macros required to create an array of strings or integers.
 *
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2008  Horms <horms@verge.net.au>
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


struct vanessa_dynamic_array_t_struct {
        void **vector;
        size_t count;
        size_t allocated_size;
        size_t block_size;
        void (*e_destroy) (void *);
        void *(*e_duplicate) (void *s);
        void (*e_display) (char *, void *);
        size_t(*e_length) (void *);
};



/**********************************************************************
 * vanessa_dynamic_array_create
 * Create a dynamic array
 * pre: block_size: blocking size to use.
 *                  DEFAULT_DYNAMIC_ARRAY_BLOCK_SIZE is used if 
 *                  block_size is 0.
 *                  Block size refers to how many elements are 
 *                  preallocated each time the array is grown.
 *      element_destroy:   Pointer to a function to destroy an element
 *                         Function should take an argument of a pointer
 *                         and free the memory allocated to the structure
 *                         pointed to. May be null in which case elements
 *                         will not be freed when dynamic array is freed.
 *                         Makes sense if element_duplicate is NULL.
 *      element_duplicate: Pointer to a function to duplicate an element
 *                         Function should take a pointer to an element to
 *                         duplicate as the only element and return a copy
 *                         of the element Any memory allocation required
 *                         should be done by this function. May be null
 *                         in which case elements are inserted into
 *                         dynamic array by reference rather than by being
 *                         copied.
 *      element_display:   Pointer to a function to display an element
 *                         Function should take a pointer to char and a
 *                         pointer to an element as arguments. An ASCII
 *                         representation of the element should be placed
 *                         in the character buffer given as the first
 *                         argument.  May be NULL in which case
 *                         vanessa_dynamic_array_display will return an empty
 *                         string ("");
 *      element_length:    Pointer to a function to find the length of an
 *                         ASCII representation of the element not
 *                         including the trailing '\0'. Used to
 *                         guard against buffer over runs when using
 *                         element_display. May be NULL, in which case
 *                         vanessa_dynamic_array_display will return an empty
 *                         string ("");
 * post: Dynamic array is allocated and initialised.
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_create(size_t block_size,
		void (*element_destroy) (void *), void *(*element_duplicate)
		(void *s), void (*element_display) (char *, void *),
		size_t (*element_length) (void *))
{
	vanessa_dynamic_array_t *a;

	a = (vanessa_dynamic_array_t *) malloc(sizeof(vanessa_dynamic_array_t));
	if (!a) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return (NULL);
	}

	a->vector = NULL;
	a->count = 0;
	a->allocated_size = 0;
	a->block_size =
	    block_size ? block_size :
	    VANESSA_DEFAULT_DYNAMIC_ARRAY_BLOCK_SIZE;

	a->e_destroy = element_destroy;
	a->e_duplicate = element_duplicate;
	a->e_display = element_display;
	a->e_length = element_length;

	return (a);
}


/**********************************************************************
 * vanessa_dynamic_array_destroy
 * Free an array an all the elements held within
 * pre: a: array to destroy
 * post: array is freed and element_destroy as passed to
 *             vanessa_dynamic_array_create is called for all elements of 
 *             the array.
 *       Nothing if a is NULL
 **********************************************************************/

void vanessa_dynamic_array_destroy(vanessa_dynamic_array_t * a)
{
	if (a == NULL)
		return;
	if (a->e_destroy != NULL) {
		while (a->count-- > 0) {
			a->e_destroy(*(a->vector + a->count));
		}
	}
	if (a->allocated_size > 0) {
		free(a->vector);
	}
	free(a);
}


/**********************************************************************
 * vanessa_dynamic_array_add_element
 * Add an element to a dynamic array
 * pre: a: dynamic array to add element to
 *      e: element to add
 * post: element in inserted in the first unused position in the array
 *       array size is increased by block_size, as passed to
 *       vanessa_dynamic_array_create,  if there is insufficient room in the 
 *       array to add the element.
 *       Nothing is done if e is NULL
 * return: a on success
 *         NULL if a is NULL or an error occurs
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_add_element(
		vanessa_dynamic_array_t * a, void *e)
{
	/* Make sure arguments are sane */
	if (!a) {
		return (NULL);
	}

	/* Grow vector as required */
	if (a->count == a->allocated_size) {
		a->allocated_size += a->block_size;
		a->vector = (void **) realloc(a->vector, 
				a->allocated_size * sizeof(void *));
		if (!(a->vector)) {
			VANESSA_LOGGER_DEBUG_ERRNO("realloc");
			vanessa_dynamic_array_destroy(a);
			return (NULL);
		}
	}

	/* Duplicate element and add it to array */
	if (e && a->e_duplicate) {
		e = a->e_duplicate(e);
		if(!e) {
			VANESSA_LOGGER_DEBUG("a->e_duplicate");
			return (NULL);
		}
	}
	*(a->vector + a->count) = e;
	a->count++;

	return (a);
}


/**********************************************************************
 * vanessa_dynamic_array_delete_element
 * Delete an element from a dynamic array
 * pre: a: dynamic array to delete element from
 *      index: index of element to delete
 * post: Element is destroyed and removed from array. Subsequent
 *       elements in the array are shuffled up to fill the gap.
 *       array size is decreased by block_size, as passed to
 *       vanessa_dynamic_array_create,  if there number if used elements in
 *       the array falls below a block boundary.
 *       Nothing is done if e is NULL or index is not a valid element
 *       in the array.
 * return: a on success
 *         NULL if a is NULL, index is out of bounds or an error occurs
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_delete_element(
		vanessa_dynamic_array_t * a, const int index)
{
	int i = 0;

	/* Make sure parameters are sane */
	if (!a) {
		return (NULL);
	}
	if (i < 0 || i >= a->count) {
		return (NULL);
	}

	/* Destroy element to be removed */
	if (a->e_destroy && a->vector[index]) {
		a->e_destroy(a->vector[index]);
	}

	/* Shuffle elements up */
	for (i = index + 1; i < a->count; i++) {
		a->vector[i - 1] = a->vector[i];
	}
	a->count--;

	/* Shrink vector as required */
	if (a->count && a->count <= a->allocated_size - a->block_size) {
		a->allocated_size -= a->block_size;
		a->vector = (void **) realloc(a->vector, 
				a->allocated_size * sizeof(void *));
		if (!(a->vector)) {
			VANESSA_LOGGER_DEBUG_ERRNO("realloc");
			vanessa_dynamic_array_destroy(a);
			return (NULL);
		}
	}

	return (a);
}


/**********************************************************************
 * vanessa_dynamic_array_duplicate
 * Duplicate a dynamic array
 * pre: a: Dynamic Array to duplicate
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_duplicate(
		vanessa_dynamic_array_t * a)
{
	vanessa_dynamic_array_t *new_a;
	int i;

	extern int errno;

	new_a = vanessa_dynamic_array_create(a->block_size, a->e_destroy,
			a->e_duplicate, a->e_display, a->e_length);
	if(!new_a) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_create");
		return (NULL);
	}

	for (i = 0; i < a->count; i++) {
		if (!vanessa_dynamic_array_add_element(new_a, a->vector[i])) {
			VANESSA_LOGGER_DEBUG(
					"vanessa_dynamic_array_add_element");
			vanessa_dynamic_array_destroy(new_a);
			return (NULL);
		}
	}

	return (new_a);
}


/**********************************************************************
 * vanessa_dynamic_array_length
 * Find the length of an ASCII representation of a dynamic array.
 * Not including a terminating '\0'.
 * pre: a: dynamic array to find the length of
 * post: If a is NULL or there are no elements in a then the 
 *          length is 0
 *       If element_length, as passed to vanessa_dynamic_array_create, 
 *          is NULL, then 0 is returned.
 *       Else the cumulative lenth of the elemements as per the
 *           element_length function, plus one character per 
 *           element for a delimiter between elements.
 *           The trailing '\0' is not counted.
 *          It is up to the user to free this buffer.  
 * return: Cumulative length of the elements.
 *         0 if a is NULL or there are no elements in a or if
 *           element_length passed to vanessa_list_create is NULL.
 **********************************************************************/

size_t vanessa_dynamic_array_length(vanessa_dynamic_array_t * a)
{
	void **a_current;
	void **a_top;
	size_t len = 0;

	if (!a || !(a->count) || !(a->e_length)) {
		return (0);
	}

	a_top = a->vector + a->count;
	len = a->count - 1;
	for (a_current = a->vector; a_current < a_top; a_current++) {
		if(*a_current) {
			len += a->e_length(*a_current);
		}
		len++;		/* Space for delimiter */
	}
	len--;			/* No space for trailing '\0' */

	return (len);
}


/**********************************************************************
 * vanessa_dynamic_array_display
 * Make an ASCII representation of a dynamic array.
 * pre: a: dynamic array to display
 *      delimiter: character to place between elements of the array
 * post: If a is NULL or there are no elements in a then nothing is 
 *          done
 *       If element_display or element_length, as passed to
 *          vanessa_dynamic_array_create, are NULL, then an empty 
 *          string ("") is returned.  
 *       Else a character buffer is allocated and an ASCII 
 *          representation of of each array element, as determined by 
 *          element_display, separated by delimiter is placed in the 
 *          '\0' terminated buffer that is returned. 
 *       It is up to the user to free this buffer.  
 * return: Allocated buffer as above 
 *         NULL on error, 
 *         NULL a or empty a
 **********************************************************************/

char *vanessa_dynamic_array_display(vanessa_dynamic_array_t * a,
				    char delimiter)
{
	void **a_current;
	void **a_top;
	char *buffer;
	char *buffer_current;
	size_t nochar;
	size_t len = 0;

	if (!a || !(a->count)) {
		return (NULL);
	}

	if (!(a->e_length) || !(a->e_display)) {
		return (strdup(""));
	}

	nochar = vanessa_dynamic_array_length(a) + 1;
	buffer = (char *) malloc(nochar);
	if (!buffer) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return (NULL);
	}

	a_top = a->vector + a->count;
	buffer_current = buffer;
	for (a_current = a->vector; a_current < a_top; a_current++) {
		if (*a_current && (len = a->e_length(*a_current))) {
			a->e_display(buffer_current, *a_current);
			buffer_current += len;
		}
		*buffer_current++ = delimiter;
	}

	if (buffer_current != buffer) {
		buffer_current--;
	}
	*buffer_current = '\0';

	return (buffer);
}


/**********************************************************************
 * vanessa_dynamic_array_get_element
 * Get an element from an array
 * pre: a: array to retrieve element from
 *      elementno: index element in array to retrieve
 * post: no change is made to a
 * return: element requested
 *         NULL if element is beyond the number of elements in the array
 *         N.B. element may actually be NULL. You can check if an
 *         overflow has occured using by comparing elemntno to the output
 *         of vanessa_dynamic_array_get_count()
 **********************************************************************/

void * vanessa_dynamic_array_get_element(vanessa_dynamic_array_t * a, 
					size_t elementno)
{
	if(!a || elementno > a->count) {
		return(NULL);
	}
	
	return(a->vector[elementno]);
}


/**********************************************************************
 * vanessa_dynamic_array_get_count
 * Get the number of elements in the array
 * pre: array to find the number of elements in
 * return: number of elements in the array
 *         -1 if a is NULL
 **********************************************************************/

ssize_t vanessa_dynamic_array_get_count(vanessa_dynamic_array_t * a) 
{
	return(a==NULL?-1:a->count);
}


/**********************************************************************
 * vanessa_dynamic_array_get_vector
 * Get the array contained in the dynamic array
 * pre: array to find the vector of
 * return: vector
 *         NULL if a is NULL
 **********************************************************************/

void **vanessa_dynamic_array_get_vector(vanessa_dynamic_array_t * a)
{
	return(a==NULL?NULL:a->vector);
}


/**********************************************************************
 * vanessa_dynamic_array_reverse
 * Reverse the order of the elements in a dynamic array
 * pre: a: dynamic array to reverse
 * post: Elements of a are in the reverse order
 * return: none
 **********************************************************************/

void vanessa_dynamic_array_reverse(vanessa_dynamic_array_t * a)
{
	void **begin;
	void **end;
	void *tmp;

	if (a == NULL || a->count == 0) {
		return;
	}

	for (begin = a->vector, end = a->vector + a->count - 1;
	     begin < end; begin++, end--) {
		tmp = *begin;
		*begin = *end;
		*end = tmp;
	}
}


/**********************************************************************
 * vanessa_dynamic_array_split_str
 * Split a string into substrings on a delimiter
 * pre: str: string to split
 *      delimiter: character to split string on
 * post: string is split. 
 *       Note: The string is modified.
 * return: dynamic array containing sub_strings
 *         NULL on error
 *         string being NULL is an error state
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_split_str(char *string,
		const char delimiter)
{
	vanessa_dynamic_array_t *a;
	char *sub_string;

	if (string == NULL) {
		return (NULL);
	}
	a = vanessa_dynamic_array_create(0, VANESSA_DESTROY_STR,
			VANESSA_DUPLICATE_STR, VANESSA_DISPLAY_STR,
			VANESSA_LENGTH_STR);
	if(!a) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_create");
		return (NULL);
	}
	while ((sub_string = strchr(string, delimiter)) != NULL) {
		*sub_string = '\0';
		if (!vanessa_dynamic_array_add_element(a, string)) {
			VANESSA_LOGGER_DEBUG(
					"vanessa_dynamic_array_add_element 1");
			return (NULL);
		}
		string = sub_string + 1;
	}
	if (*string != '\0' && !vanessa_dynamic_array_add_element(a, string)) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_add_element 2");
		return (NULL);
	}
	return (a);
}


/**********************************************************************
 * vanessa_dynamic_array_split_str_to_int
 * Split a string into substrings on a delimiter
 *       The substrings are converted into the integers that they represent
 *       and the integers are stored.
 * pre: str: string to split
 *      delimiter: character to split string on
 * post: string is split. 
 *       Note: The string is modified.
 * return: dynamic array containing integers
 *         NULL on error
 *         string being NULL is an error state
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_split_str_to_int(char *string,
		const char delimiter)
{
	vanessa_dynamic_array_t *a;
	char *sub_string;
	int i;

	if (string == NULL) {
		return (NULL);
	}
	a = vanessa_dynamic_array_create(0, VANESSA_DESTROY_INT,
			VANESSA_DUPLICATE_INT, VANESSA_DISPLAY_INT,
			VANESSA_LENGTH_INT);
	if(!a) {
		VANESSA_LOGGER_DEBUG("vanessa_dynamic_array_create");
		return (NULL);
	}
	while ((sub_string = strchr(string, delimiter)) != NULL) {
		*sub_string = '\0';
		i = atoi(string);
		if (!vanessa_dynamic_array_add_element(a, (void *) i)) {
			VANESSA_LOGGER_DEBUG(
					"vanessa_dynamic_array_add_element");
			return (NULL);
		}
		string = sub_string + 1;
	}
	if (*string != '\0') {
		i = atoi(string);
		if (!vanessa_dynamic_array_add_element(a, (void *) i)) {
			VANESSA_LOGGER_DEBUG(
					"vanessa_dynamic_array_add_element");
			return (NULL);
		}
	}
	return (a);
}

