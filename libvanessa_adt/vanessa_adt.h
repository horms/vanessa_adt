/***********************************************************************
 * vanessa_adt.h                                           December 1999
 * Horms                                              horms@vergenet.net
 *
 * Abstract data types
 *
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2000  Horms
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

#ifndef _VANESSA_ADT_H
#define _VANESSA_ADT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <vanessa_logger.h>


typedef unsigned int vanessa_adt_flag_t;

/**********************************************************************
 * Data independent queue
 **********************************************************************/

typedef struct vanessa_queue_member_t_struct vanessa_queue_member_t;

typedef struct vanessa_queue_t_struct vanessa_queue_t;


/**********************************************************************
 * vanessa_queue_create
 * Create a new, empty vanessa_queue
 * pre: e_destroy: pointer to a function to destroy elements of the queue
 *                 If null, then elements will not be freed on calls
 *                 to vanessa_queue_destroy or on errors
 * post: memory is allocated for queue and values are initialised
 * return: new, empty vanessa_queue
 *         NULL on error
 **********************************************************************/

vanessa_queue_t *vanessa_queue_create(void (*e_destroy) (const void *));


/**********************************************************************
 * vanessa_queue_push
 * push an element onto the beginning of a vanessa_queue
 * pre: q: vanessa_queue
 *      value: element to push onto the vanessa_queue
 * post: element is added to queue
 * return: vanessa_queue with element added
 *         NULL on error. On error, where possible the vanessa_queue is 
 *         destroyed.
 **********************************************************************/

vanessa_queue_t *vanessa_queue_push(vanessa_queue_t * q, void *value);


/**********************************************************************
 * vanessa_queue_pop 
 * Pop an element off the end of a vanessa_queue
 * pre: q: vanessa_queue to pop the element off
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: element is removed from queue
 * return: vanessa_queue with element removed
 *         NULL on error
 * Note: popping an empty vanessa_queue results in NULL being returned
 **********************************************************************/

vanessa_queue_t *vanessa_queue_pop(vanessa_queue_t * q, void **value);


/**********************************************************************
 * vanessa_queue_peek_last 
 * Retrieve the last element from a vanessa_queue without removing it 
 * from the vanessa_queue
 * pre: q: vanessa_queue to peek at
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: none
 * return: pointer to an element from the vanessa_queue
 *         NULL on error
 * Note: peeking at an empty vanessa_queue results in NULL being returned
 **********************************************************************/

void *vanessa_queue_peek_last(vanessa_queue_t * q);

#define vanessa_queue_peek vanessa_queue_peek_last


/**********************************************************************
 * vanessa_queue_peek_first
 * Retrieve the first element from a vanessa_queue without removing it 
 * from the * vanessa_queue
 * pre: q: vanessa_queue to peek at
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: none
 * return: pointer to an element from the vanessa_queue
 *         NULL on error
 * Note: peeking at an empty vanessa_queue results in NULL being returned
 **********************************************************************/

void *vanessa_queue_peek_first(vanessa_queue_t * q);


/**********************************************************************
 * vanessa_queue_destroy
 * Destroy a vanessa_queue, destroying each element present in the
 * vanessa_queue first
 * pre: q: vanessa_queue to destroy
 *      element
 * post: vanessa_queue and all elements in the vanessa_queue are destroyed
 * return: none
 **********************************************************************/

void vanessa_queue_destroy(vanessa_queue_t * q);


/**********************************************************************
 * vanessa_queue_length
 * Return the number of elements in the vanessa_queue
 * pre: q: vanessa_queue to find the number of elements in
 * post: none
 * return: number of elements in the vanessa_queue
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_queue_length(vanessa_queue_t * q);


/**********************************************************************
 * Dynamic array, to store all your flims in. 
 *
 * The primitive type for the array is void *. Thus, providing your own
 * duplicate_primitive, destroy_primitive, display_primitive and
 * length_primitive functions will allow you to use the
 * vanessa_dynamic_array API to have a dynamic array containing any
 * primitive
 *
 * Includes macros required to create an array of strings or integers.
 **********************************************************************/

/*
 * Default blocking size for dynamic array
 * can be overridden when array is created
 */
#define VANESSA_DEFAULT_DYNAMIC_ARRAY_BLOCK_SIZE (size_t)7


/* #defines to destroy and duplicate strings */
#define VANESSA_DESTROY_STR (void (*)(void *s))free
#define VANESSA_DUPLICATE_STR (void *(*)(void *s))strdup
#define VANESSA_DISPLAY_STR (void (*)(char *d, void *s))strcpy
#define VANESSA_LENGTH_STR (size_t (*)(void *s))strlen
#define VANESSA_MATCH_STR (size_t (*)(void *s))strcmp
#define VANESSA_SORT_STR VANESSA_MATCH_STR

/* Sort versions */
#define VANESSA_DESS VANESSA_DESTROY_STR
#define VANESSA_DUPS VANESSA_DUPLICATE_STR
#define VANESSA_DISS VANESSA_DISPLAY_STR
#define VANESSA_LENS VANESSA_LENGTH_STR
#define VANESSA_MATS VANESSA_MATCH_STR
#define VANESSA_SORS VANESSA_MATCH_STR

typedef struct vanessa_dynamic_array_t_struct vanessa_dynamic_array_t;


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
 *                         pointed to.
 *      element_duplicate: Pointer to a function to duplicate an element
 *                         Function should take a pointer to an element to
 *                         duplicate as the only element and return a copy
 *                         of the element Any memory allocation required
 *                         should be done by this function.
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
 * post: Dynamic array is allocated and initialised.
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_create(size_t block_size,
						      void
						      (*element_destroy)
						      (void *),
						      void
						      *(*element_duplicate)
						      (void *s),
						      void
						      (*element_display)
						      (char *, void *),
						      size_t(*element_size)
						      (void *)
    );


/**********************************************************************
 * vanessa_dynamic_array_destroy
 * Free an array an all the elements held within
 * pre: a: array to destroy
 * post: array is freed and element_destroy as passed to
 *             vanessa_dynamic_array_create is called for all elements of 
 *             the array.
 *       Nothing if a is NULL
 **********************************************************************/

void vanessa_dynamic_array_destroy(vanessa_dynamic_array_t * a);


/**********************************************************************
 * vanessa_dynamic_array_add_element
 * Add an element to a dynamic array
 * pre: a: dynamic array to add element to
 *      e: element to add
 * post: element in inserted in the first unused position in the array
 *       array size is increased by block_size, as passed to
 *       vanessa_dynamic_array_create,if there is insufficient room in 
 *       the array to add the element.
 *       Nothing is done if e is NULL
 * return: a on success
 *         NULL if a is NULL or an error occurs
 **********************************************************************/

vanessa_dynamic_array_t
    *vanessa_dynamic_array_add_element(vanessa_dynamic_array_t * a,
				       void *e);


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

vanessa_dynamic_array_t
    *vanessa_dynamic_array_delete_element(vanessa_dynamic_array_t * a,
					  const int index);


/**********************************************************************
 * vanessa_dynamic_array_duplicate
 * Duplicate a dynamic array
 * pre: a: Dynamic Array to duplicate
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t
    *vanessa_dynamic_array_duplicate(vanessa_dynamic_array_t * a);


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

size_t vanessa_dynamic_array_length(vanessa_dynamic_array_t * a);


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

char *vanessa_dynamic_array_display(vanessa_dynamic_array_t * a,
				    char delimiter);


/**********************************************************************
 * vanessa_dynamic_array_get_element
 * Get an element from an array
 * pre: a: array to retrieve element from
 *      elementno: index element in array to retrieve
 * post: no change is made to a
 * return: element requested
 *         NULL if element is beyond the number of elements in the array
 **********************************************************************/

void * vanessa_dynamic_array_get_element(vanessa_dynamic_array_t * a,
					 size_t elementno);


/**********************************************************************
 * vanessa_dynamic_array_get_count
 * Get the number of elements in the array
 * pre: array to find the number of elements in
 * return: number of elements in the array
 *         -1 if a is NULL
 **********************************************************************/

ssize_t vanessa_dynamic_array_get_count(vanessa_dynamic_array_t * a);


/**********************************************************************
 * vanessa_dynamic_array_get_vector
 * Get the array contained in the dynamic array
 * pre: array to find the vector of
 * return: vector
 *         NULL if a is NULL
 **********************************************************************/

void **vanessa_dynamic_array_get_vector(vanessa_dynamic_array_t * a);


/**********************************************************************
 * vanessa_dynamic_array_reverse
 * Reverse the order of the elements in a dynamic array
 * pre: a: dynamic array to reverse
 * post: Elements of a are in the reverse order
 * return: none
 **********************************************************************/

void vanessa_dynamic_array_reverse(vanessa_dynamic_array_t * a);


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
							 const char
							 delimiter);


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

vanessa_dynamic_array_t *vanessa_dynamic_array_split_str_to_int(char
								*string,
								const char
								delimiter);


/**********************************************************************
 * vanessa_destroy_int
 * function to destroy an pointer to an int.
 * pre: i: int to "destroy"
 * post: none
 * return: none
 **********************************************************************/

void vanessa_destroy_int(int *i);


/**********************************************************************
 * vanessa_dup_int
 * function to duplicate pointer to an int.
 * pre: i: int to duplicate
 * post: new int is alocated and value of i is copied into it
 * return: pointer to new integer
 *         NULL on error
 **********************************************************************/

int *vanessa_dup_int(int *i);


/**********************************************************************
 * vanessa_display_int
 * Display an int in ASCII as a decimal.
 * pre: d: buffer to display ASCII represetation of int to
 *      i: pointer to int to display
 * post: an ASCII representation of i is in d
 * return: none
 **********************************************************************/

void vanessa_display_int(char *d, int *i);


/**********************************************************************
 * vanessa_length_int
 * Return the length in bytes of an ASCII representation of the in as a
 * decimal.
 * pre: i: pointer to int to find the "length" of
 * post: none
 * return: length is returned
 **********************************************************************/

size_t vanessa_length_int(int *i);


/**********************************************************************
 * vanessa_match_int
 * Compare two integers. Return the difference. That is < 1 if a < b,
 * 0 if a == b and > 1 if a > b.
 * Analogous to strcmp(3)
 * pre: a: pointer to int to compare
 *      b: pointer to other int to compare
 * post: none
 * return: difference between a and b
 **********************************************************************/

int vanessa_match_int(int *a, int *b);


/* #defines to destroy and duplicate integers */
#define VANESSA_DESTROY_INT \
	(void (*)(void *s))vanessa_destroy_int
#define VANESSA_DUPLICATE_INT \
	(void *(*)(void *s))vanessa_dup_int
#define VANESSA_DISPLAY_INT \
	(void (*)(char *d, void *s))vanessa_display_int
#define VANESSA_LENGTH_INT \
	(size_t (*)(void *s))vanessa_length_int
#define VANESSA_MATCH_INT \
	(int (*)(void *e, void *k))vanessa_match_int
#define VANESSA_SORT_INT VANESSA_MATCH_INT

/* ... and shorter versions */
#define VANESSA_DESI VANESSA_DESTROY_INT
#define VANESSA_DUPI VANESSA_DUPLICATE_INT
#define VANESSA_DISI VANESSA_DISPLAY_INT
#define VANESSA_LENI VANESSA_LENGTH_INT
#define VANESSA_MATI VANESSA_MATCH_INT
#define VANESSA_SORI VANESSA_MATCH_INT


/**********************************************************************
 * Key value pair
 **********************************************************************/

typedef struct vanessa_key_value_t_struct vanessa_key_value_t;


/**********************************************************************
 * vanessa_key_value_create
 * Create a new vanessa_key_value structure
 * pre: none
 * post: a new vanessa_key_value structure is allocated and elements 
 *       are initialised
 * return: pointer to vanessa_key_value_t
 *         NULL on error
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_create(void);


/**********************************************************************
 * vanessa_key_value_destroy
 * Destroy a vanessa_key_value and its contents
 * pre: kv: vanessa_key_value structure to destroy
 * post: kv and its contents are freed
 * return: none
 **********************************************************************/

void vanessa_key_value_destroy(vanessa_key_value_t * kv);


/**********************************************************************
 * vanessa_key_value_duplicate
 * Make a copy of a vanessa_key_value_structure
 * pre: kv: vanessa_key_value structure to duplicate
 * post: 
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_duplicate(vanessa_key_value_t * kv);


/**********************************************************************
 * vanessa_key_value_assign
 * Assign a key and value to a vanessa_key_value structure
 * pre: kv: key_value_structure to assign key and value to
 *      key: key to assign
 *      key_destroy: pointer to a function to destroy key
 *                   Use NULL if you do not want the key destroyed
 *                   when the vanessa_key_value_destroy is called
 *      key_duplicate: pointer to a function to duplicate key
 *                     Use NULL if you want the key duplicated
 *                     using new_key=old_key
 *      value: value to assign
 *      value_destroy: pointer to a function to destroy value
 *                     Use NULL if you do not want the value destroyed
 *                     when the value_value_destroy is called
 *      value_duplicate: pointer to a function to duplicate value
 *                       Use NULL if you want the value duplicated
 *                       using new_value=old_value
 * post: key and value are assigned and destroy functions are registered
 * return: pointer to vanessa_key_value structure
 *         NULL if kv is NULL
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_assign(vanessa_key_value_t * kv,
					      void *key,
					      void (*destroy_key) (void *),
					      void *(*dup_key) (void *),
					      void *value,
					      void (*destroy_value) (void
								     *),
					      void *(*dup_value) (void *)
    );


/**********************************************************************
 * vanessa_key_value_unassign
 * Unassign values in vanessa_key_values structure
 * Useful if you want to destroy the vanessa_key_value structure
 * without freeing the contents
 * pre: kv: key_value structure to unassign values of
 * post: All elements in vanessa_key_value structure are set to NULL
 * return: pointer to vanessa_key_value structure
 *         NULL if kv is NULL
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_unassign(vanessa_key_value_t * kv);


/**********************************************************************
 * vanessa_key_value_key_get_key
 * Get the key from a vanessa_key_value structure
 * pre: kv: key value structure to get the key of
 * return: key of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_key(vanessa_key_value_t * kv);


/**********************************************************************
 * vanessa_key_value_key_get_value
 * Get the value from a vanessa_key_value structure
 * pre: kv: key value structure to get the value of
 * return: value of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_value(vanessa_key_value_t * kv);


#define VANESSA_DESTROY_KV (void (*)(void *s))vanessa_key_value_destroy
#define VANESSA_DUPLICATE_KV (void *(*)(void *s))vanessa_key_value_duplicate


/**********************************************************************
 * Logging functionality
 **********************************************************************/

extern vanessa_logger_t *vanessa_adt_logger;


/**********************************************************************
 * vanessa_adt_logger_set
 * set the logger function to use
 * No logging will take place if logger is set to NULL (default)
 * That is you _must_ call this function to enable logging.
 * pre: logger: pointer to a vanessa_logger
 * post: logger for vanessa_adt is set to logger
 * return: none
 **********************************************************************/

#define vanessa_adt_logger_set(_vl) vanessa_adt_logger=_vl


/**********************************************************************
 * vanessa_adt_logger_unset
 * set logger to NULL
 * That is no logging will take place
 * pre: none
 * post: logger is NULL
 * return: none
 **********************************************************************/

#define vanessa_adt_logger_unset() vanessa_adt_logger_set(NULL)


/**********************************************************************
 * Linked List to store all your flims in.
 *
 * The primitive type for the array is void *. Thus, providing your own
 * duplicate_primitive, destroy_primitive, match_primative,
 * display_primitive and length_primitive functions will allow you to use
 * the vanessa_list API to have a listcontaining any primitive
 *
 * Includes macros required to create an array of strings or integers.
 **********************************************************************/


typedef struct vanessa_list_elem_struct vanessa_list_elem_t;

typedef struct {
	vanessa_list_elem_t *first;
	vanessa_list_elem_t *last;
	vanessa_list_elem_t **recent;
	size_t norecent;
	size_t recent_offset;
	void (*e_destroy) (void *e);
	void *(*e_duplicate) (void *e);
	void (*e_display) (char *s, void *e);
	size_t(*e_length) (void *e);
	int (*e_match) (void *e, void *key);
	int (*e_sort) (void *a, void *b);
} vanessa_list_t;


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
 *      element_match:     Pointer to a function to match an element
 *                         by a key.
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
				   int (*element_sort) (void *a, void *b));


/**********************************************************************
 * vanessa_list_destroy
 * Destroy a list and all the data contained in the list
 * pre: l: list
 * post: all elements of l are destroyed
 **********************************************************************/

void vanessa_list_destroy(vanessa_list_t * l);


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

size_t vanessa_list_length(vanessa_list_t * l);


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

char *vanessa_list_display(vanessa_list_t * l, char delimiter);


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

void *vanessa_list_get_element(vanessa_list_t *l, void *key);


/**********************************************************************
 * vanessa_list_get_count
 * Count the number of elements in the list
 * pre: l: list to count
 * post: none
 * return: number of elements in the list
 *         0 if l is NULL
 **********************************************************************/

size_t vanessa_list_get_count(vanessa_list_t *l);


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
 *         l, unchanged if value is NULL
 **********************************************************************/

vanessa_list_t *vanessa_list_add_element(vanessa_list_t * l, void *value);


/**********************************************************************
 * vanessa_list_remove_element
 * Insert element into a list
 * pre: l: list to insert value into
 *      value: value to insert
 * post: value is removed from the list
 * return: NULL if l is NULL
 *         l, unchanged if value is null
 **********************************************************************/

void vanessa_list_remove_element(vanessa_list_t *l, void *key);


/**********************************************************************
 * vanessa_list_duplicate
 * Duplicate a list
 * pre: l: list to duplicate
 * post: list is duplicated
 * return: NULL if l is NULL or on error
 *         duplicated list 
 **********************************************************************/

vanessa_list_t *vanessa_list_duplicate(vanessa_list_t *l);

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
		void *data);



/**********************************************************************
 * Hash to put your flims in.
 * 
 * The primitive type for the array is void *. Thus, providing your
 * own duplicate_primitive, destroy_primitive, match_primative,
 * display_primitive, length_primitive and hash_primitive, functions
 * will allow you to use the vanessa_hash API to have a
 * hash containing any primitive
 *
 * Includes macros, excluding hash_primitive,  required to create an
 * list of strings or integers.
 **********************************************************************/


typedef struct vanessa_hash_t_struct vanessa_hash_t;


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
		                    size_t (*element_hash) (void *e));


/**********************************************************************
 * vanessa_hash_destroy
 * Destroy a hash and all the data contained in the hash
 * pre: h: hash
 * post: all elements of h are destroyed
 **********************************************************************/

void vanessa_hash_destroy(vanessa_hash_t *h);


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


size_t vanessa_hash_length(vanessa_hash_t *h) ;


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

char *vanessa_hash_display(vanessa_hash_t *h, const char delimiter);


/**********************************************************************
 * vanessa_hash_get_element
 * Get the number of elements stored in the hash
 * pre: h: hash to count the elements of
 * post: none
 * return: Number of elements stored in the hash
 *         0 if h is NULL or empty
 **********************************************************************/

size_t vanessa_hash_get_count(vanessa_hash_t *h) ;


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

void *vanessa_hash_get_element(vanessa_hash_t *h, void *value);


/**********************************************************************
 * vanessa_hash_add_element
 * Insert element into a hash
 * pre: h: hash to insert value into
 *      value: value to insert
 * post: value is inserted into the hash
 * return: NULL if h is NULL
 *         h, unchanged if value is NULL
 **********************************************************************/

vanessa_hash_t *vanessa_hash_add_element(vanessa_hash_t *h, void *value);


/**********************************************************************
 * vanessa_hash_remove_element
 * Insert element into a hash
 * pre: h: hash to insert value into
 *      value: value to insert
 * post: value is removed from the hash
 * return: NULL if h is NULL
 *         h, unchanged if value is null
 **********************************************************************/

vanessa_hash_t *vanessa_hash_remove_element(vanessa_hash_t *h, void *value);


/**********************************************************************
 * vanessa_hash_duplicate
 * Duplicate a hash
 * pre: h: hash to duplicate
 * post: hash is duplicated
 * return: NULL if h is NULL or on error
 *         duplicated hash 
 **********************************************************************/

vanessa_hash_t *vanessa_hash_duplicate(vanessa_hash_t *h);


/**********************************************************************
 * vanessa_hash_iterate
 * Run a fucntion over each element in the hash
 * pre: h: hash run the function over
 *      action: function to run
 *              action should return < 0 if an error occurs,
 *              which indicates that processing will be stopped
 *      data: data passed to action
 * post: action is run with the value of each element as its first argumetn
 * return: 0 on success
 *         < 0 if action returns < 0
 **********************************************************************/

int vanessa_hash_iterate(vanessa_hash_t *h, int(* action)(void *e, void *data),
		                void *data);

#endif /* _VANESSA_ADT_FLIM */
