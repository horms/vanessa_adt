/**********************************************************************
 * hash.c                                                  January 2002
 * Simon Horman                                      horms@verge.net.au
 *
 * Hash to put your flims in.
 * Copyright (C) 2002-2008  Simon Horman <horms@verge.net.au>
 * 
 * The primitive type for the array is void *. Thus, providing your
 * own duplicate_primitive, destroy_primitive, match_primative,
 * display_primitive, length_primitive and hash_primitive, functions
 * will allow you to use the vanessa_hash API to have a
 * hash containing any primitive
 *
 * Includes macros, excluding hash_primitive,  required to create an
 * list of strings or integers.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 **********************************************************************/


#include "vanessa_adt.h"

#include <stdlib.h>

struct vanessa_hash_t_struct {
	vanessa_list_t **bucket;
	size_t         nobucket;
	void           (*e_destroy) (void *e);
	void           *(*e_duplicate) (void *e);
	void           (*e_display) (char *s, void *e);
	size_t         (*e_length) (void *e);
	int            (*e_match) (void *e, void *key);
	size_t         (*e_hash) (void *e);
};


/**********************************************************************
 * vanessa_hash_create
 * Create a new, empty hash
 * pre: nobucket: number of buckets in the hash
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
 *      element_hash:      Pointer to a function that return the hash
 *                         bucket index, a number >= 0 && < nobucket,
 *                         for an element.
 *
 * post: hash structure is alocated, and values initialised to NULL
 *       note that the hash buckets are linked lists and are created
 *       on demand.
 * return: pointer to hash
 *         NULL on error
 **********************************************************************/

vanessa_hash_t *vanessa_hash_create(size_t nobucket, 
		void (*element_destroy) (void *e),
		void *(*element_duplicate) (void *e),
		int (*element_match) (void *e, void *key),
		void (*element_display) (char *s, void *e),
		size_t(*element_length) (void *e),
		size_t (*element_hash) (void *e))
{
	vanessa_hash_t *h;

	h = (vanessa_hash_t *)malloc(sizeof(vanessa_hash_t));
	if(h == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return(NULL);
	}

	h->bucket = (vanessa_list_t **)malloc(sizeof(void *) * nobucket);
	if(h->bucket == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("mallocc");
		free(h);
		return(NULL);
	}

	bzero(h->bucket, nobucket*sizeof(void *));

	h->nobucket = nobucket;
	h->e_destroy = element_destroy;
	h->e_duplicate = element_duplicate;
	h->e_display = element_display;
	h->e_length = element_length;
	h->e_match = element_match;
	h->e_hash = element_hash;

	return(h);
}


/**********************************************************************
 * vanessa_hash_destroy
 * Destroy a hash and all the data contained in the hash
 * pre: h: hash
 * post: all elements of h are destroyed
 **********************************************************************/

void vanessa_hash_destroy(vanessa_hash_t *h) 
{
	ssize_t i;
	
	if(h == NULL) {
		return;
	}

	for(i = 0 ; i < h->nobucket ; i++) {
		if(h->bucket[i] != NULL) {
			vanessa_list_destroy(h->bucket[i]);
		}
	}

	free(h);
}


/**********************************************************************
 * vanessa_hash_length
 * Find the length of an ASCII representation of a dynamic array.
 * Not including a terminating '\0'.
 * pre: h: hash to find the length of
 * post: If h is NULL or there are no elements in h then the 
 *          length is 0
 *       If element_length, as passed to vanessa_hash_create, 
 *          is NULL, then 0 is returned. 
 *       Else the cumulative lenth of the elemements as per the 
 *          element_length, plus one character per element for a 
 *          delimiter between elements.
 *          The trailing '\0' is not counted.
 *          It is up to the user to free this buffer.  
 * return: Cumulative length of the elements.
 *         0 if a is NULL or there are no elements in a or if
 *            element_length passed to vanessa_hash_create is NULL.
 **********************************************************************/


size_t vanessa_hash_length(vanessa_hash_t *h) 
{
	int i;
	size_t len = 0;

	if(h == NULL) {
		return(0);
	}

	for(i = 0 ; i < h->nobucket ; i++) {
		if(h->bucket[i] != NULL) {
			len += vanessa_list_length(h->bucket[i]);
			len++;
		}
	}

	if(len) {
		len--;
	}

	return(len);
}


/**********************************************************************
 * vanessa_hash_display
 * Make an ASCII representation of the hash
 * pre: h: hash to display
 *      delimiter: character to place between elements of the hash
 * post: If h is NULL or there are no elements in h then nothing is 
 *         done
 *       If element_display or element_length, as passed to
 *          vanessa_hash_create, are NULL, then an empty string 
 *          ("") is returned.  
 *       Else a character buffer is allocated and an ASCII 
 *          representation of each hashed element, as determined by 
 *          element_display, separated by delimiter is placed in the 
 *          '\0' terminated buffer that is returned. 
 *          It is up to the user to free this buffer.  
 * return: Allocated buffer as above 
 *         NULL on error, 
 *         NULL h or empty h
 **********************************************************************/

char *vanessa_hash_display(vanessa_hash_t *h, const char delimiter) 
{
	int i;
	char *str;
	char *bucket_str;
	size_t len;

	if(h == NULL) {
		return(NULL);
	}

	len = vanessa_hash_length(h) + 1;

	str = (char *)malloc(len);
	if(str == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return(NULL);
	}


	bzero(str, len);
	for(i = 0 ; i < h->nobucket ; i++) {
		if(h->bucket[i] == NULL) {
			continue;
		}
		if(*str != '\0') {
			*(str+strlen(str)) = ',';
		}
		bucket_str = vanessa_list_display(h->bucket[i], delimiter);
		if(bucket_str == NULL) {
			VANESSA_LOGGER_DEBUG("vanessa_list_length");
			return(NULL);
		}
		strcat(str, bucket_str);
		free(bucket_str);
	}

	return(str);
}


/**********************************************************************
 * vanessa_hash_get_element
 * Get the number of elements stored in the hash
 * pre: h: hash to count the elements of
 * post: none
 * return: Number of elements stored in the hash
 *         0 if h is NULL or empty
 **********************************************************************/

size_t vanessa_hash_get_count(vanessa_hash_t *h) 
{
	int i;
	size_t count = 0;

	if(h == NULL) {
		return(0);
	}
		
	for(i = 0 ; i < h->nobucket ; i++) {
		if(h->bucket[i] != NULL) {
			count += vanessa_list_get_count(h->bucket[i]);
		}
	}

	return(count);
}


/**********************************************************************
 * __vanessa_hash_get_bucket
 * Get the bucket for a given valye
 * pre: h: hash to get the hash bucket from
 *      value: value to hash and get the hash bucket of
 *      hash_key: used to return the hash key for value
 * post: hash_key is seeded with the hashed value
 * return: hash bucket for value
 *         NULL on error
 **********************************************************************/

static vanessa_list_t *__vanessa_hash_get_bucket(vanessa_hash_t *h,
						void *value,
						size_t *hash_key) 
{
	if(h == NULL || value == NULL || h->e_hash == NULL) {
		return(NULL);
	}

	*hash_key = h->e_hash(value);

	if(*hash_key >= h->nobucket) {
		VANESSA_LOGGER_DEBUG_UNSAFE(
				"hash value too large: %d >= %d", *hash_key,
				h->nobucket);
		abort();
		return(NULL);
	}

	return(h->bucket[*hash_key]);
}


/**********************************************************************
 * vanessa_hash_get_element
 * Retrieve an element from the hash by value
 * passed to vanessa_list_create
 * pre: h: hash to search
 *      value: value to match
 * post: none
 * return: element if found
 *         NULL if h or value is NULL or if the element is in the hash
 **********************************************************************/

void *vanessa_hash_get_element(vanessa_hash_t *h, void *value) 
{
	void *result;
	size_t hash_key;


	if(__vanessa_hash_get_bucket(h, value, &hash_key) == NULL) {
		return(NULL);
	}

	result = vanessa_list_get_element(h->bucket[hash_key], value);

	return(result);
}


/**********************************************************************
 * vanessa_hash_add_element
 * Insert element into a hash
 * pre: h: hash to insert value into
 *      value: value to insert
 * post: value is inserted into the hash
 * return: NULL if h is NULL
 *         h, unchanged if value is NULL
 **********************************************************************/

vanessa_hash_t *vanessa_hash_add_element(vanessa_hash_t *h, void *value) 
{
	size_t hash_key;

	if(__vanessa_hash_get_bucket(h, value, &hash_key) == NULL) {
		h->bucket[hash_key] = vanessa_list_create(0,
				h->e_destroy, h->e_duplicate, h->e_display,
				h->e_length, h->e_match, NULL);
	}

	if(h->bucket[hash_key] == NULL) {
		VANESSA_LOGGER_DEBUG("vanessa_list_create");
		vanessa_hash_destroy(h);
		return(NULL);
	}

	if(vanessa_list_add_element(h->bucket[hash_key], value) == NULL) {
		VANESSA_LOGGER_DEBUG("vanessa_list_add_element");
		vanessa_hash_destroy(h);
		return(NULL);
	}

	return(h);
}


/**********************************************************************
 * vanessa_hash_remove_element
 * Insert element into a hash
 * pre: h: hash to insert value into
 *      value: value to insert
 * post: value is removed from the hash
 * return: NULL if h is NULL
 *         h, unchanged if value is null
 **********************************************************************/

vanessa_hash_t *vanessa_hash_remove_element(vanessa_hash_t *h, void *value) 
{
	size_t hash_key;

	if(__vanessa_hash_get_bucket(h, value, &hash_key) == NULL) {
		return(NULL);
	}

	vanessa_list_remove_element(h->bucket[hash_key], value);

	return(h);
}


/**********************************************************************
 * vanessa_hash_duplicate
 * Duplicate a hash
 * pre: h: hash to duplicate
 * post: hash is duplicated
 * return: NULL if h is NULL or on error
 *         duplicated hash 
 **********************************************************************/

vanessa_hash_t *vanessa_hash_duplicate(vanessa_hash_t *h) 
{
	int i;
	vanessa_hash_t *new_h;

	new_h = vanessa_hash_create(h->nobucket, h->e_destroy, h->e_duplicate,
			 h->e_match, h->e_display, h->e_length, h->e_hash);
	if(new_h == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return(NULL);
	}

	for(i = 0 ; i < h->nobucket ; i++ ){
		if(h->bucket[i] == NULL) {
			continue;
		}

		new_h->bucket[i]=vanessa_list_duplicate(h->bucket[i]);
		if(new_h->bucket[i] == NULL) {
			VANESSA_LOGGER_DEBUG("vanessa_list_duplicate");
			vanessa_hash_destroy(new_h);
			return(NULL);
		}
	}

	return(new_h);
}


/**********************************************************************
 * vanessa_hash_iterate
 * Run a fucntion over each element in the hash
 * pre: h: hash run the function over
 *      action: function to run
 *              action should return < 0 if an error occurs,
 *              which indicates that processing will be stopped
 *      data: data passed to action
 * post: action is run with the value of each element as its first argument
 * return: 0 on success
 *         < 0 if action returns < 0
 **********************************************************************/

int vanessa_hash_iterate(vanessa_hash_t *h, int(* action)(void *e, void *data),
		                void *data) 
{
	int i;
	int status;

	for(i = 0 ; i < h->nobucket ; i++ ){
		if(h->bucket[i] == NULL) {
			continue;
		}
		status = vanessa_list_iterate(h->bucket[i], action, data);
		if(status < 0) {
			return(status);
		}
	}

	return(0);
}
