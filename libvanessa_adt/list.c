/**********************************************************************
 * list.c                                                  January 2002
 * Horms                                             horms@verge.net.au
 *
 * Linked List to store all your flims in.
 *
 * The primitive type for the array is void *. Thus, providing your own
 * duplicate_primitive, destroy_primitive, match_primative,
 * display_primitive and length_primitive functions will allow you to use
 * the vanessa_list API to have a linked list containing any
 * primitive
 *
 * Includes macros required to create an list of strings or integers.
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2003  Horms
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

#include <stdlib.h>

#include "vanessa_adt.h"

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
		vanessa_list_elem_t * prev, vanessa_list_elem_t * next,
		void * value)
{
	if (!e) {
		return (NULL);
	}
	e->next = next;
	e->prev = prev;
	e->value = value;

	return (e);
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
	if (!e) {
		return;
	}
	if(destroy_value != NULL) {
		destroy_value(e->value);
	}
	free(e);
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
	vanessa_list_elem_t * next, void *value,
	void *(*element_duplicate) (void *e))
{
	vanessa_list_elem_t *e;
	void *new_value;

	e = (vanessa_list_elem_t *) malloc(sizeof(vanessa_list_elem_t));
	if (!e) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return (NULL);
	}

	if(value && element_duplicate) {
		new_value = element_duplicate(value);
		if(!new_value) {
			VANESSA_LOGGER_DEBUG("element_duplicate");
			free(e);
			return(NULL);
		}
	}
	else {
		new_value = value;
	}

	e = vanessa_list_elem_assign(e, prev, next, new_value);

	return (e);
}


/**********************************************************************
 * vanessa_list_create
 * Create a new, empty list
 * pre: norecent: number of elements for recent list.
 * 		  If VANESSA_LIST_REORDER then no recent list is used
 * 		  but elements are moved to the front of the list
 * 		  as they are retrieved using vanessa_list_get_element
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
 *      element_sort:      Pointer to a function that will compare
 *                         two elements and and b. Will return < 0 if a 
 *                         should be before b in the list > 0 if a should
 *                         be after b in the list. 0 if they are equal.
 *                         Used in vanessa_list_add_element
 *
 * post: list structure is alocated, and values initialised to NULL
 * return: pointer to list
 *         NULL on error
 **********************************************************************/

vanessa_list_t *vanessa_list_create(int norecent,
		void (*element_destroy) (void *e),
                void *(*element_duplicate) (void *e),
                void (*element_display) (char *s, void *e),
                size_t(*element_size) (void *e),
                int (*element_match) (void *e, void *key),
                int (*element_sort) (void *a, void *b))
{
	vanessa_list_t *l;
	size_t i;

	l = (vanessa_list_t *) malloc(sizeof(vanessa_list_t));
	if (!l) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return (NULL);
	}

	if(norecent > 0 || norecent == VANESSA_LIST_REORDER) {
		l->norecent = norecent;
	}
	else {
		l->norecent = 0;
	}
	if (norecent < 1) {
		l->recent = NULL;
	} 
	else {
		l->recent = (vanessa_list_elem_t **) 
			malloc(sizeof(vanessa_list_elem_t *) * l->norecent);
		if (l->recent == NULL) {
			VANESSA_LOGGER_DEBUG_ERRNO("malloc");
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
	l->e_display = element_display;
	l->e_length = element_size;
	l->e_match = element_match;
	l->e_sort = element_sort;

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

	if(l->e_destroy != NULL) {
		while (l->first != NULL) {
			next = l->first->next;
			if(l->first->value) {
				l->e_destroy(l->first->value);
			}
			free(l->first);
			l->first = next;
		}
	}

	free(l);
	return;
}


/**********************************************************************
 * vanessa_list_length
 * Find the length of an ASCII representation of a dynamic array.
 * Not including a terminating '\0'.
 * pre: l: list to find the length of
 * post: If l is NULL or there are no elements in l then the 
 *          length is 0
 *       If element_length, as passed to vanessa_list_create, 
 *          is NULL, then 0 is returned.
 *       Else the cumulative lenth of the elemements as per the
 *          element_length function, plus one character per element 
 *          for a delimiter between elements.
 *          The trailing '\0' is not counted.
 *          It is up to the user to free this buffer.  
 * return: Cumulative length of the elements.
 *         0 if a is NULL or there are no elements in a or if
 *           element_length passed to vanessa_list_create is NULL.
 **********************************************************************/

size_t vanessa_list_length(vanessa_list_t * l) {
	vanessa_list_elem_t *e;
	size_t len = 0;

	if (l == NULL || (e = l->first) == NULL || l->e_length == NULL) {
		return (0);
	}

	len = 0;
	do {
		if(e->value) {
			len += l->e_length(e->value);
		}
		len++; /* Space for delimiter */
		e = e->next;
	} while (e);
	len--;                  /* No space for trailing '\0' */

	return (len);
}


/**********************************************************************
 * vanessa_list_display
 * Make an ASCII representation of the list
 * pre: l: list to display
 *      delimiter: character to place between elements of the list
 * post: If l is NULL or there are no elements in l then nothing is 
 *          done
 *       If element_display or element_length, as passed to
 *          vanessa_list_create, are NULL, then an empty string 
 *          ("") is returned.  
 *       Else a character buffer is allocated and an ASCII 
 *          representation of each list element, as determined by 
 *          element_display, separated by delimiter is placed in 
 *          the '\0' terminated buffer that is returned. 
 *          It is up to the user to free this buffer.  
 * return: Allocated buffer as above 
 *         NULL on error, 
 *         NULL l or empty l
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
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return(NULL);
	}

	buffer_current = buffer;
	do {
		if (e->value && (len = l->e_length(e->value))) {
			l->e_display(buffer_current, e->value);
			buffer_current += len;
		}
		*buffer_current++ = delimiter;
		e = e->next;
	} while (e);

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

int __vanessa_list_get_element_match(void *value, void *key) {
	return((value==key)?0:1);
}

static vanessa_list_elem_t *__vanessa_list_get_element(vanessa_list_t *l,
		void *key) {
	int i;
	vanessa_list_elem_t *e = NULL;
	int (*match)(void *value, void *key);

	if (l == NULL  || key == NULL) {
		return(NULL);
	}

	match=(l->e_match != NULL)?l->e_match:__vanessa_list_get_element_match;

	for(i=0 ; i < l->norecent ; i++) {
		e = *(l->recent + i);
		if(e != NULL && match(e->value, key) == 0) {
			return(e);
		}
	}

	for(e = l->first; e != NULL ; e = e->next ) {
		if(match(e->value, key) == 0) {
			break;
		}
	}

	if(e && l->norecent == VANESSA_LIST_REORDER && l->first != e) {
		if(e->prev) {
			e->prev->next = e->next;
		}
		if(e->next) {
			e->next->prev = e->prev;
		}
		l->first->prev = e;
		e->next = l->first;
		e->prev = NULL;
		l->first = e;
		if(l->last == e) {
			l->last = e->next;
		}
	}
	return(e);
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
 *       if element_sort passed to vanessa_list_create is non-NULL
 *       then the element will be inserted in order. Otherwise
 *       the element will be inserted at the begining of the list.
 * return: NULL if l is NULL
 *         l, unchanged if value is null
 **********************************************************************/

vanessa_list_t *vanessa_list_add_element(vanessa_list_t * l, void *value)
{
	vanessa_list_elem_t *e;
	vanessa_list_elem_t *prev;

	if(l == NULL) {
		return(NULL);
	}

	if(l->e_sort == NULL) {
		prev = l->first;
	}
	else {
		for(prev = l->last ; prev != NULL ; prev = prev->prev) {
			if(l->e_sort(value, prev->value) >= 0) {
				break;
			}
		}
	}
	
	e = vanessa_list_elem_create(prev, (prev==NULL)?NULL:prev->next, 
			value, l->e_duplicate);
	if (e == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("vanessa_list_elem_create");
		vanessa_list_destroy(l);
		return (NULL);
	}

	if(prev != NULL) {
		if(prev->next != NULL) {
			prev->next->prev = e;
		}
		prev->next = e;
	}
	if(prev == l->last) {
		l->last = e;
	}
	if(prev == NULL) {
		e->next = l->first;
		if(l->first != NULL) {
			l->first->prev = e;
		}
		l->first = e;
	}

	if(l->norecent > 0) {
		l->recent_offset = (l->recent_offset + 1) % l->norecent;
		*(l->recent + l->recent_offset) = e;
	}

	return (l);
}


/**********************************************************************
 * vanessa_list_remove_element
 * Insert element into a list
 * pre: l: list to insert value into
 *      value: value to insert
 * post: value is removed from the list
 * return: NULL if l is NULL
 *         l, unchanged if value is null
 **********************************************************************/

static void __vanessa_list_remove_element(vanessa_list_t * l, 
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

	if(e->next != NULL) {
		e->next->prev = e->prev;
	}
	if(e->prev != NULL) {
		e->prev->next = e->next;
	}

	for(i=0 ; i<l->norecent ; i++) {
		if(l->recent[i] == e) {
			*(l->recent + i) = NULL;
		}
	}

	vanessa_list_elem_destroy(e, l->e_destroy);
}

void vanessa_list_remove_element(vanessa_list_t *l, void *key) {
	vanessa_list_elem_t *e;

	e = __vanessa_list_get_element(l, key);
	__vanessa_list_remove_element(l, e);
}


/**********************************************************************
 * vanessa_list_duplicate
 * Duplicate a list
 * pre: l: list to duplicate
 * post: list is duplicated
 * return: NULL if l is NULL or on error
 *         duplicated list 
 **********************************************************************/

vanessa_list_t *vanessa_list_duplicate(vanessa_list_t *l) {
	vanessa_list_t *new_list;
	vanessa_list_elem_t *e;
	
	if(l==NULL) {
		return(NULL);
	}

	new_list=vanessa_list_create(l->norecent, l->e_destroy,
			l->e_duplicate, l->e_display,
			l->e_length, l->e_match, l->e_sort);
	if(new_list == NULL) {
		VANESSA_LOGGER_DEBUG("vanessa_list_create");
		return(NULL);
	}


	for(e=l->last; e!=NULL; e=e->prev) {
		vanessa_list_add_element(new_list, e->value);
	}

	return(new_list);
}


/**********************************************************************
 * vanessa_list_iterate
 * Run a fucntion over each element in the list
 * pre: l: list run the function over
 *      action: function to run
 *              action should return < 0 if an error occurs,
 *              which indicates that processing will be stopped
 *      data: data passed to action
 * post: action is run with the value of each element as its first argumetn
 * return: 0 on success
 *         < 0 if action returns < 0
 **********************************************************************/

int vanessa_list_iterate(vanessa_list_t *l, int(* action)(void *e, void *data),
		void *data) {
	int status;
	vanessa_list_elem_t *e;

	if(l == NULL) {
		return(0);
	}

	for(e = l->first; e != NULL ; e = e->next) {
		if((status=action((e->value), data)) < 0) {
			return(status);
		}
	}

	return(0);
}
