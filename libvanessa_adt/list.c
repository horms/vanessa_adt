/**********************************************************************
 * list.c                                                  January 2002
 * Horms                                             horms@vergenet.net
 *
 * Linked List to store all your flims in.
 *
 * The primitive type for the array is void *. Thus, providing your own
 * duplicate_primitive, destroy_primitive, match_primative,
 * display_primitive and length_primitive functions will allow you to use
 * the vanessa_dynamic_array API to have a dynamic array containing any
 * primitive
 *
 * Includes macros required to create an array of strings or integers.

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
 * 02111-1307  USA
 **********************************************************************/

#include "vanessa_adt.h"
#include "logger.h"
#include <stdlib.h>

#define DEFAULT_NORECENT 7

struct vanessa_list_elem_struct {
	struct vanessa_list_elem_struct *next;
	struct vanessa_list_elem_struct *prev;
	void *value;
};


/**********************************************************************
 * vanessa_list_elem_assign
 * Assign values to a list element
 * pre: e: alocated pointer to elelemt to seed
 *      prev: previous element in list
 *      next: next element in list
 *      value: value to store in element
 * post: values are assigned to e
 * return: pointer to e
 *         NULL if e was NULL
 **********************************************************************/

static
vanessa_list_elem_t *vanessa_list_elem_assign(vanessa_list_elem_t * e,
					      vanessa_list_elem_t * prev,
					      vanessa_list_elem_t * next,
					      void * value)
{
	if (e == NULL) {
		return (NULL);
	}
	e->next = next;
	e->prev = prev;
	e->value = value;

	return (e);
}


/**********************************************************************
 * vanessa_list_elem_unassign
 * Assign NULL values to a list element.
 * As the name suggets this can be used to unasign values, possibly
 * so you can destroy the list element without freeing its contents.
 * pre: e: alocated pointer to elelemt to seed
 * post: values are assigned to e
 * return: pointer to e
 *         NULL if e was NULL
 **********************************************************************/

static
vanessa_list_elem_t *vanessa_list_elem_unassign(vanessa_list_elem_t * e)
{
	return (vanessa_list_elem_assign(e, NULL, NULL, NULL));
}


/**********************************************************************
 * vanessa_list_elem_destroy
 * Destroy a list element
 * pre: e: pointer to elelemt to destroy
 * post: e and values in e are dstroyed
 *       nothing if e is NULL
 **********************************************************************/

static
void vanessa_list_elem_destroy(vanessa_list_elem_t * e, 
		void (*destroy_value) (void *))
{
	if (e == NULL) {
		return;
	}
	destroy_value(e->value);
	free(e);
}


/**********************************************************************
 * vanessa_list_elem_remove
 * remove an element from a list
 * pre: l: list to remove elment from
 *      e: element to remove
 * post: element is removed from list
 *       If this was the last element of the list then l->recent is
 *       set to the previous (now last) elemet of the list.
 *       Otherwise l->recent is set to the next element in the list
 *       No change is made to the list if l or e are NULL
 * return: l
 *         NULL if l or e are NULL
 **********************************************************************/

static
vanessa_list_t *vanessa_list_elem_remove(vanessa_list_t * l,
					 vanessa_list_elem_t * e)
{
	size_t i;

	if (l == NULL || e == NULL) {
		return (NULL);
	}
	for (i = 0; i < l->norecent; i++) {
		if (*(l->recent + i) == e) {
			*(l->recent + i) = NULL;
		}
	}

	if(l->norecent > 0) {
		l->recent_offset = (l->recent_offset + 1) % l->norecent;
	}
	if (e->prev != NULL) {
		e->prev->next = e->next;
		*(l->recent + l->recent_offset) = e->prev;
	}
	if (e->next != NULL) {
		e->next->prev = e->prev;
		*(l->recent + l->recent_offset) = e->next;
	}
	if (l->first == e) {
		l->first = e->next;
	}
	if (l->last == e) {
		l->last = e->prev;
	}

	return (l);
}


/**********************************************************************
 * vanessa_list_elem_create
 * Create a new element, and seed it
 * pre: prev: previous element in list
 *      next: next element in list
 *      value: value to store
 * post: e is initialised and values are seeded
 * return: pointer to e
 *         NULL on error
 **********************************************************************/

static
vanessa_list_elem_t *vanessa_list_elem_create(vanessa_list_elem_t * prev,
					      vanessa_list_elem_t * next,
					      void * value)
{
	vanessa_list_elem_t *e;

	e = (vanessa_list_elem_t *) malloc(sizeof(vanessa_list_elem_t));
	if (e == NULL) {
		VANESSA_ADT_DEBUG_ERRNO("malloc");
		return (NULL);
	}
	e = vanessa_list_elem_assign(e, prev, next, value);

	return (e);
}


/**********************************************************************
 * vanessa_list_elem_create_empty
 * Create a new element, and seed it with NULL values
 * pre: none
 * post: e is initialised and values are seeded with NULL
 * return: pointer to e
 *         NULL on error
 **********************************************************************/

static
vanessa_list_elem_t *vanessa_list_elem_create_empty(void)
{
	return (vanessa_list_elem_create(NULL, NULL, NULL));
}


/**********************************************************************
 * vanessa_list_create
 * Create a new, empty list
 * pre: norecent: number of elements for recent list.
 *                If negative DEFAULT_NORECENT is used
 *      element_destroy:   Pointer to a function to destroy an element
 *                         Function should take an argument of a pointer
 *                         and free the memory allocated to the structure
 *                         pointed to.
 *      element_duplicate: Pointer to a function to duplicate an element
 *                         Function should take a pointer to an element to
 *                         duplicate as the only element and return a copy
 *                         of the element Any memory allocation required
 *                         should be done by this function.
 *      element_match:     Pointer to a function to match an element
 *                         by a key.
 *      element_display:   Pointer to a function to display an element
 *                         Function should take a pointer to char and a
 *                         pointer to an element as arguments. An ASCII
 *                         representation of the element should be placed
 *                         in the character buffer given as the first
 *                         argument.  May be NULL in which case
 *                         vanessa_dynamic_array_display will return an empty
 *                         string ("\0");
 *      element_length:    Pointer to a function to find the length of an
 *                         ASCII representation of the element not
 *                         including the trailing '\0'. Used to guard
 *                         against buffer over runs when using
 *                         element_display. May be NULL, in which case
 *                         vanessa_dynamic_array_display will return an
 *                         empty string ("");
 *
 * post: list structure is alocated, and values initialised to NULL
 * return: pointer to list
 *         NULL on error
 **********************************************************************/

vanessa_list_t *vanessa_list_create(int norecent,
                                   void (*element_destroy) (void *e),
                                   void *(*element_duplicate) (void *e),
                                   void *(*element_match) (void *e, void *key),
                                   void (*element_display) (char *s, void *e),
                                   size_t(*element_size) (void *e))
{
	vanessa_list_t *l;
	size_t i;

	l = (vanessa_list_t *) malloc(sizeof(vanessa_list_t));
	if (l == NULL) {
		VANESSA_ADT_DEBUG_ERRNO("malloc");
		return (NULL);
	}

	l->norecent =
	    (size_t) ((norecent < 0) ? DEFAULT_NORECENT : norecent);
	if (norecent == 0) {
		l->recent = NULL;
	} 
	else {
		l->recent = (vanessa_list_elem_t **) 
			malloc(sizeof(vanessa_list_elem_t *) * l->norecent);
		if (l->recent == NULL) {
			VANESSA_ADT_DEBUG_ERRNO("malloc");
			free(l);
			return (NULL);
		}
		for (i = 0; i < l->norecent; i++) {
			*(l->recent + i) = NULL;
		}
	}

	l->recent_offset = 0;
	l->first = NULL;
	l->last = NULL;
	l->e_destroy = element_destroy;
	l->e_duplicate = element_duplicate;
	l->e_match = element_match;
	l->e_display = element_display;
	l->e_length = element_size;

	return (l);
}


/**********************************************************************
 * vanessa_list_destroy
 * Destroy a list and all the data contained in the list
 * pre: l: list
 * post: all elements of l are destroyed
 **********************************************************************/

void vanessa_list_destroy(vanessa_list_t * l)
{
	vanessa_list_elem_t *next;

	if (l == NULL) {
		return;
	}

	while (l->first != NULL) {
		next = l->first->next;
		l->e_destroy(l->first);
		l->first = next;
	}

	free(l);
	return;
}


/**********************************************************************
 * vanessa_list_length
 * Find the length of an ASCII representation of a dynamic array.
 * Not including a terminating '\0'.
 * pre: a: dynamic array to find the length of
 * post: If l is NULL or there are no elements in l then the length is 0
 *       If element_display or element_length, as passed to
 *       vanessa_list_create, are NULL, then 0 is returned.
 *       Else the cumulative lenth of the elemements as per the
 *       element_length function passed to vanessa_list_create, plus one
 *       character per element for a delimiter between elements.
 *       The trailing '\0' is not counted.
 *       It is up to the user to free this buffer.  
 * return: Cumulative length of the elements.
 *         0 if a is NULL or there are no elements in a or if
 *         element_length passed to vanessa_list_create is NULL.
 **********************************************************************/

size_t vanessa_list_length(vanessa_list_t * l) {
	vanessa_list_elem_t *e;
	size_t len = 0;

	if (l == NULL || (e = l->first) == NULL || l->e_length == NULL) {
		return (0);
	}

	len = 0;
	do {
		len += l->e_length(e->value);
		len++; /* Space for delimiter */
		e = e->next;
	} while (e != NULL);
	len--;                  /* No space for trailing '\0' */

	return (len);
}


/**********************************************************************
 * vanessa_list_display
 * Make an ASCII representation of the list
 * pre: l: list to display
 *      delimiter: character to place between elements of the list
 * post: If l is NULL or there are no elements in l then nothing is done
 *       If element_display or element_length, as passed to
 *       vanessa_list_create, are NULL, then an empty string 
 *       ("") is returned.  Else a character buffer is allocated
 *       and * and ASCII representation of of each list, as
 *       determined by element_display passed to
 *       vanessa_list_create, separated by delimiter is placed in the 
 *       '\0' terminated buffer returned. It * is up to the user to free 
 *       this buffer.  
 * return: Allocated buffer as above 
 *         NULL on error, 
 *         NULL a or empty a
 **********************************************************************/

char *vanessa_list_display(vanessa_list_t * l, char delimiter) {
	vanessa_list_elem_t *e;
	size_t nochar;
	size_t len = 0;
	char *buffer_current;
	char *buffer;

	if (l==NULL || (e = l->first) == NULL) {
		return (NULL);
	}

	if (l->e_length==NULL || l->e_display == NULL) {
		return(strdup(""));
	}

	nochar = vanessa_list_length(l) + 1;
	if ((buffer = (char *) malloc(nochar)) == NULL){
		VANESSA_ADT_DEBUG_ERRNO("malloc");
		return(NULL);
	}

	buffer_current = buffer;
	do {
		if ((len = l->e_length(e->value))) {
			l->e_display(buffer_current, e->value);
			buffer_current += len;
		}
		*buffer_current++ = delimiter;
		e = e->next;
	} while (e != NULL);

	if (len) {
		buffer_current--;
	}
	*buffer_current = '\0';

	return (buffer);
}


/**********************************************************************
 * vanessa_list_get_element
 * Find an element in the list by key using the element_match function
 * passed to vanessa_list_create
 * pre: l: list to search
 *      key: key to match
 * post: none
 * return: value it is found
 *         NULL if l or key is NULL or if value matching key is not found
 **********************************************************************/

static vanessa_list_elem_t *__vanessa_list_get_element(vanessa_list_t *l,
		void *key) {
	int i;
	vanessa_list_elem_t *e;

	if (l == NULL || key == NULL) {
		return(NULL);
	}

	for(i=0 ; i<l->norecent ; i++) {
		e = *(l->recent + i);
		if(l->e_match(e->value, key) == 0) {
			return(e);
		}
	}

	for(e = l->first; e != NULL ; e = e->next ) {
		if(l->e_match(e->value, key) == 0) {
			return(e);
		}
	}

	return(NULL);
}

void *vanessa_list_get_element(vanessa_list_t *l, void *key) {
	vanessa_list_elem_t *e;

	e = __vanessa_list_get_element(l, key);

	if(e == NULL) {
		return(NULL);
	}

	return(e->value);
}


/**********************************************************************
 * vanessa_list_get_count
 * Count the number of elements in the list
 * pre: l: list to count
 * post: none
 * return: number of elements in the list
 *         0 if l is NULL
 **********************************************************************/

size_t vanessa_list_get_count(vanessa_list_t *l){
	size_t count = 0;
	vanessa_list_elem_t *e;

	if (l == NULL) {
		return(0);
	}

	for(e = l->first; e != NULL ; e = e->next ) {
		count++;
	}

	return(count);
}


/**********************************************************************
 * vanessa_list_add_element
 * Insert element into a list
 * pre: l: list to insert value into
 *      value: value to insert
 * post: value is inserted into the list
 * return: NULL if l is NULL
 *         l, unchanged if n is null
 **********************************************************************/

vanessa_list_t *vanessa_list_add_element(vanessa_list_t * l, void *value)
{
	vanessa_list_elem_t *e;

	if(l == NULL) {
		return(NULL);
	}

	if ((e = vanessa_list_elem_create(NULL, l->first, value)) == NULL) {
		VANESSA_ADT_DEBUG_ERRNO("vanessa_list_elem_create");
		vanessa_list_destroy(l);
		return (NULL);
	}

	l->first = e;
	if (l->first != NULL) {
		l->first->prev = e;
	}
	if (l->last == NULL) {
		l->last = e;
	}
	if(l->norecent > 0) {
		l->recent_offset = (l->recent_offset + 1) % l->norecent;
		*(l->recent + l->recent_offset) = e;
	}

	return (l);
}


/**********************************************************************
 * vanessa_list_delete_element
 * Insert element into a list
 * pre: l: list to insert value into
 *      value: value to insert
 * post: value is removed from the list
 * return: NULL if l is NULL
 *         l, unchanged if n is null
 **********************************************************************/

static void __vanessa_list_delete_element(vanessa_list_t * l, 
		vanessa_list_elem_t *e)
{
	int i;

	if(l == NULL || e == NULL) {
		return;
	}

	if (l->first == e) {
		l->first = e->next;
	}
	if (l->last == e) {
		l->last = e->prev;
	}

	for(i=0 ; i<l->norecent ; i++) {
		if(l->recent[i] == e) {
			*(l->recent + i) = NULL;
		}
	}

	vanessa_list_elem_destroy(e, l->e_destroy);
}

void vanessa_list_delete_element(vanessa_list_t *l, void *key) {
	vanessa_list_elem_t *e;

	e = __vanessa_list_get_element(l, key);
	__vanessa_list_delete_element(l, e);
}
