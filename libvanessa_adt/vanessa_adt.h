/***********************************************************************
 * vanessa_adt.h                                           December 1999
 * Horms                                              horms@vergenet.net
 *
 * Abstract data types
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

#ifndef VANESSA_ADT_FLIM
#define VANESSA_ADT_FLIM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <vanessa_logger.h>


typedef unsigned int vanessa_adt_flag_t;

/**********************************************************************
 * Data independant queue
 **********************************************************************/

typedef struct __vanessa_queue_member_t_struct {
  void *value;
  struct __vanessa_queue_member_t_struct *prev;
  struct __vanessa_queue_member_t_struct *next;
} vanessa_queue_member_t;

typedef struct {
  vanessa_queue_member_t *first;
  vanessa_queue_member_t *last;
  void (*e_destroy)(const void *);
  int size;
} vanessa_queue_t;
  

/**********************************************************************
 * vanessa_queue_create
 * Create a new, empty vanessa_queue
 * pre: e_destroy: pointer to a function to destroy elements of the queue
 *                 If null, then elements will not be freed on calls
 *                 to vanessa_queue_destroy or on errors
 * post: memoryn is allocated for queue and values are initialised
 * return: new, empty vanessa_queue
 *         NULL on error
 **********************************************************************/

vanessa_queue_t *vanessa_queue_create(void (*e_destroy)(const void *));


/**********************************************************************
 * vanessa_queue_push
 * push an element onto the begining of a vanessa_queue
 * pre: q: vanessa_queue
 *      value: elemnt to push onto the vanessa_queue
 * post: element is added to queue
 * return: vanessa_queue with element added
 *         NULL on error. On error, where possible the vanessa_queue is 
 *         destroyed.
 **********************************************************************/

vanessa_queue_t *vanessa_queue_push(vanessa_queue_t *q, void *value);


/**********************************************************************
 * vanessa_queue_pop 
 * Pop an element off the end of a vanessa_queue
 * pre: q: vanessa_queue to pop the element off
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: element is removed from queue
 * return: vanessa_queue with element removed
 *         NULL on error
 * Note: popping an emty vanessa_queue results in NULL being returned
 **********************************************************************/

vanessa_queue_t *vanessa_queue_pop(vanessa_queue_t *q, void **value);


/**********************************************************************
 * vanessa_queue_peek_last 
 * Retreive the last element from a vanessa_queue without removing it 
 * from the vanessa_queue
 * pre: q: vanessa_queue to peek at
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: none
 * return: pointer to an element from the vanessa_queue
 *         NULL on error
 * Note: peeking at an emty vanessa_queue results in NULL being returned
 **********************************************************************/

#define vanessa_queue_peek_last(_q) \
  ((_q)==NULL || (_q)->last==NULL)?NULL:(_q)->last->value

#define vanessa_queue_peek vanessa_queue_peek_last


/**********************************************************************
 * vanessa_queue_peek_first
 * Retreive the first element from a vanessa_queue without removing it 
 * from the * vanessa_queue
 * pre: q: vanessa_queue to peek at
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: none
 * return: pointer to an element from the vanessa_queue
 *         NULL on error
 * Note: peeking at an emty vanessa_queue results in NULL being returned
 **********************************************************************/

#define vanessa_queue_peek_first(_q) \
  ((_q)==NULL || (_q)->first==NULL)?(NULL):(_q)->first->value


/**********************************************************************
 * vanessa_queue_destroy
 * Destroy a vanessa_queue, destroying each element present in the
 * vanessa_queue first
 * pre: q: vanessa_queue to destroy
 *      element
 * post: vanessa_queue and all elements in the vanessa_queue are destroyed
 * return: none
 **********************************************************************/

void vanessa_queue_destroy(vanessa_queue_t *q);


/**********************************************************************
 * vanessa_queue_length
 * Return the number of elements in the vanessa_queue
 * pre: q: vanessa_queue to find the number of elements in
 * post: none
 * return: number of elements in the vanessa_queue
 *         -1 on error
 **********************************************************************/

#define vanessa_queue_length(_q)  ((_q!=NULL)?(_q)->size:-1)



/**********************************************************************
 * Dynamic array, to store all your flims in
 * Includes macros required to create an array of strings but as the
 * primitive type for the array is void * providing your own
 * duplicate_primitive, destroy_primitive, display_primitive and
 * length_primitive functions will allow you to use the
 * vanessa_dynamic_array API to have a dynamic array containing any
 * primitive
 **********************************************************************/

/*
 * Default blocking size for dynamic array
 * can be overriden when array is created
 */
#define VANESSA_DEFAULT_DYNAMIC_ARRAY_BLOCK_SIZE (size_t)7


/* #defines to destroy and dupilcate strings */
#define VANESSA_DESTROY_STR (void (*)(void *s))free
#define VANESSA_DUPLICATE_STR (void *(*)(void *s))strdup
#define VANESSA_DISPLAY_STR (void (*)(char *d, void *s))strcpy
#define VANESSA_LENGTH_STR (size_t (*)(void *s))strlen

/* Sort versions */
#define VANESSA_DESS VANESSA_DESTROY_STR
#define VANESSA_DUPS VANESSA_DUPLICATE_STR
#define VANESSA_DISS VANESSA_DISPLAY_STR
#define VANESSA_LENS VANESSA_LENGTH_STR

typedef struct {
  void   **vector;
  size_t count;
  size_t allocated_size;
  size_t block_size;
  void   (*e_destroy)(void *);
  void  *(*e_duplicate)(void *s);
  void   (*e_display)(char *, void *);
  size_t (*e_length)(void *);
} vanessa_dynamic_array_t;


/**********************************************************************
 * vanessa_dynamic_array_create
 * Create a dynamic array
 * pre: block_size: blocking size to use.
 *                  DEFAULT_DYNAMIC_ARRAY_BLOCK_SIZE is used if 
 *                  block_size is 0.
 *                  Block size refers to how many elements are 
 *                  prealocated each time the array is grown. 
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
 *                         pointer to an element as aruments. An ascii
 *                         representation of the element should be placed
 *                         in the character buffer given as the first
 *                         argument.  May be NULL in which case
 *                         vanessa_dynamic_array_display will return an empty
 *                         string ("\0");
 *      element_length:    Pointer to a function to find the length of an
 *                         ascii representation of the element. Used to
 *                         gaurd against buffer over runs when using
 *                         element_display. May be NULL, in which case
 *                         vanessa_dynamic_array_display will return an empty
 *                         string ("\0");
 * post: Dynamic array is allocated and intialised.
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_create(
  size_t block_size,
  void   (*element_destroy)(void *),
  void  *(*element_duplicate)(void *s),
  void   (*element_display)(char *, void *),
  size_t (*element_size)(void *)
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

void vanessa_dynamic_array_destroy(vanessa_dynamic_array_t *a);


/**********************************************************************
 * vanessa_dynamic_array_add_element
 * Add an element to a dynamic array
 * pre: a: dynamic array to add element to
 *      e: element to add
 * post: element in inserted in the first unused position in the array
 *       array size is incresaed by block_size, as passed to
 *       vanessa_dynamic_array_create,if there is insufficient room in 
 *       the array to add the element.
 *       Nothing is done if e is NULL
 * return: a on success
 *         NULL if a is NULL or an error occurs
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_add_element(
  vanessa_dynamic_array_t *a, 
  void *e
);


/**********************************************************************
 * vanessa_dynamic_array_delete_element
 * Delete an element from a dynamic array
 * pre: a: dynamic array to delete element from
 *      index: index of element to delete
 * post: Element is destroyed and removed from array. Subsequent
 *       elements in the array are shuffeled up to fill the gap.
 *       array size is decresaed by block_size, as passed to
 *       vanessa_dynamic_array_create,  if there number if used elements in
 *       the array falls below a block boundry.
 *       Nothing is done if e is NULL or index is not a valid element
 *       in the array.
 * return: a on success
 *         NULL if a is NULL, index is out of bounds or an error occurs
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_delete_element(
  vanessa_dynamic_array_t *a, 
  const int index
);


/**********************************************************************
 * vanessa_dynamic_array_duplicate
 * Duplicate a dynamic array
 * pre: a: Dynamic Array to duplicate
 * return: An empty dynamic array 
 *         NULL on error
 **********************************************************************/

vanessa_dynamic_array_t *vanessa_dynamic_array_duplicate(
  vanessa_dynamic_array_t *a
);


/**********************************************************************
 * vanessa_dynamic_array_display
 * Make an ASCII representation of a dynamic array.
 * pre: a: dynamic array to display
 *      delimiter: character to place between elements of the array
 * post: If a is NULL or there are no elements in a then nothing is done
 *       If element_display or element_length, as passed to
 *       vanessa_dynamic_array_create, are NULL, then an empty string
 *       ("") is returned.  Else a character buffer is alocated and
 *       and ASCII representation of of each array element, as
 *       determined by element_display passed to
 *       vanessa_dynamic_array_create, separated by delimiter is
 *       placed in the '\0' termintated buffer returned. It is up to
 *       the user to free this buffer.  
 * return: Allocated buffer as above 
 *         NULL on error, 
 *         NULL a or empty a
 **********************************************************************/

char *vanessa_dynamic_array_display(vanessa_dynamic_array_t *a, char delimiter);


/**********************************************************************
 * vanessa_dynamic_array_length
 * Find the length of an ASCII representation of a dynamic array.
 * pre: a: dynamic array to find the length of
 * post: If a is NULL or there are no elements in a then the length i
 *       If element_display or element_length, as passed to
 *       vanessa_dynamic_array_create, are NULL, then an empty string
 *       ("\0") is returned.  Else a character buffer is alocated and
 *       and ASCII representation of of each array element, as
 *       determined by element_display passed to
 *       vanessa_dynamic_array_create, separated by delimiter is
 *       placed in the '\0' termintated buffer returned. It is up to
 *       the user to free this buffer.  
 * return: Cumulative length of the elements as per the element_length 
 *         function passed to vanessa_dynamic_array_create, plus once
 *         character per element for a delimiter betweeen elements.
 *         The trailing '\0' is not counted.  0 if a is NULL or there
 *         are no elements in a or if element_length passed to
 *         vanessa_dynamic_array_create is nULL.
 **********************************************************************/

size_t vanessa_dynamic_array_length(vanessa_dynamic_array_t *a);


/**********************************************************************
 * vanessa_dynamic_array_get_element
 * Get an element from an array
 * pre: a: array to retrieve element from
 *      elementno: index element in array to retrieve
 * post: no change is made to a
 * return: element requested
 *         NULL if element is beyond the number of elements in the arary
 **********************************************************************/

#define vanessa_dynamic_array_get_element(_a, _elementno) \
  ((void *)((_a==NULL||_elementno<0||_elementno>=(_a)->count)? \
	   NULL:*(((_a)->vector)+_elementno)))


/**********************************************************************
 * vanessa_dynamic_array_get_count
 * Get the number of elements in the array
 * pre: array to find the number of elements in
 * return: number of elements in the array
 *         -1 if a is NULL
 **********************************************************************/

#define vanessa_dynamic_array_get_count(_a) \
  ((_a==NULL)?-1:(_a)->count)
  

/**********************************************************************
 * vanessa_dynamic_array_get_vector
 * Get the array contained in the dynamic array
 * pre: array to find the vector of
 * return: vector
 *         NULL if a is NULL
 **********************************************************************/

#define vanessa_dynamic_array_get_vector(_a) \
  ((_a==NULL)?NULL:(_a)->vector)


/**********************************************************************
 * vanessa_dynamic_array_reverse
 * Reverse the order of the elements in a dynamic array
 * pre: a: dynamic array to reverse
 * post: Elements of a are in the reverse order
 * return: none
 **********************************************************************/

void vanessa_dynamic_array_reverse(vanessa_dynamic_array_t *a);


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

vanessa_dynamic_array_t *vanessa_dynamic_array_split_str(
  char *string, 
  const char delimiter
);


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

vanessa_dynamic_array_t *vanessa_dynamic_array_split_str_to_int(
  char *string,
  const char delimiter
);


/**********************************************************************
 * vanessa_dynamic_array_destroy_int
 * Dummy function to "destroy" an int.
 * DOES NOTHING
 * pre: i: int to "destroy"
 * post: none
 * return: none
 **********************************************************************/

void vanessa_dynamic_array_destroy_int(int i);


/**********************************************************************
 * vanessa_dynamic_array_dup_int
 * Dummy function to "duplicate" an int.
 * DOES NOTHING
 * pre: i: int to "destroy"
 * post: none
 * return: i is returned cast to a void *
 **********************************************************************/

void *vanessa_dynamic_array_dup_int(int i);


/**********************************************************************
 * vanessa_dynamic_array_display_int
 * Display an int in ascii as a decimal.
 * pre: d: buffer to display ascii represetation of int to
 *      i: int to display
 * post: an ascii representation of i is in d
 * return: none
 **********************************************************************/

void vanessa_dynamic_array_display_int(char *d, int i);


/**********************************************************************
 * vanessa_dynamic_array_length_int
 * Return the length in bytes of an ascii representation of the in as a
 * decimal.
 * DOES NOTHING
 * pre: i: int to find the "length" of
 * post: none
 * return: length is returned
 **********************************************************************/

size_t vanessa_dynamic_array_length_int(int i);


/* #defines to destroy and dupilcate integers */
#define VANESSA_DESTROY_INT \
	(void (*)(void *s))vanessa_dynamic_array_destroy_int
#define VANESSA_DUPLICATE_INT \
	(void *(*)(void *s))vanessa_dynamic_array_dup_int
#define VANESSA_DISPLAY_INT \
	(void (*)(char *d, void *s))vanessa_dynamic_array_display_int
#define VANESSA_LENGTH_INT \
	(size_t (*)(void *s))vanessa_dynamic_array_length_int

/* ... and shorter versions */
#define VANESSA_DESI VANESSA_DESTROY_INT
#define VANESSA_DUPI VANESSA_DUPLICATE_INT
#define VANESSA_DISI VANESSA_DISPLAY_INT
#define VANESSA_LENI VANESSA_LENGTH_INT


/**********************************************************************
 * Key value pair
 **********************************************************************/

typedef struct {
  void *key;
  void (*destroy_key)(void *); 
  void *(*dup_key)(void *);
  void *value;
  void (*destroy_value)(void *);
  void *(*dup_value)(void *);
} vanessa_key_value_t;


/**********************************************************************
 * vanessa_key_value_create
 * Create a new vanessa_key_value structure
 * pre: none
 * post: a new vanessa_key_value strcture is allocated and elements 
 *       are initialised
 * return: pointer to vanessa_key_value_t
 *         NULL on error
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_create(void);


/**********************************************************************
 * vanessa_key_value_destroy
 * Destory a vanessa_key_value and its contents
 * pre: kv: vanessa_key_value structure to destroy
 * post: kv and its contents are freed
 * return: none
 **********************************************************************/

void vanessa_key_value_destroy(vanessa_key_value_t *kv);


/**********************************************************************
 * vanessa_key_value_duplicate
 * Make a copy of a vanessa_key_value_structure
 * pre: kv: vanessa_key_value structure to duplicate
 * post: 
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_duplicate(vanessa_key_value_t *kv);


/**********************************************************************
 * vanessa_key_value_assign
 * Assign a key and value to a vanessa_key_value structure
 * pre: kv: key_valye_structure to assign key and value to
 *          key: key to assign
 *          key_detstory: pointer to a function to destroy key
 *                        Use NULL if you do not want the key destroyed
 *                        when the vanessa_key_value_destroy is called
 *          key_duplicate: pointer to a function to duplicate key
 *                         Use NULL if you want the key duplicated
 *                         using new_key=old_key
 *          value: value to assign
 *          value_detstory: pointer to a function to destroy value
 *                        Use NULL if you do not want the value destroyed
 *                        when the value_value_destroy is called
 *          value_duplicate: pointer to a function to duplicate value
 *                         Use NULL if you want the value duplicated
 *                         using new_value=old_value
 * post: key and value are assigned and destiry functions are registered
 * return: pointer to vanessa_key_value structure
 *         NULL if kv is NULL
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_assign(
  vanessa_key_value_t *kv,
  void *key,
  void (*destroy_key)(void *), 
  void *(*dup_key)(void *), 
  void *value,
  void (*destroy_value)(void *),
  void *(*dup_value)(void *)
);


/**********************************************************************
 * vanessa_key_value_unassign
 * Unassign valyes in vanessa_key_values structure
 * Useful if you want to destroy the vanessa_key_value structure
 * without freeing the contents
 * pre: kv: key_valye structure to unassign values of
 * post: All elements in vanessa_key_value structure are set to NULL
 * return: pointer to vanessa_key_value structure
 *         NULL if kv is NULL
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_unasign(vanessa_key_value_t *kv);


/**********************************************************************
 * vanessa_key_value_key_get_key
 * Get the key from a vanessa_key_value structure
 * pre: kv: key value structure to get the key of
 * return: key of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_key(vanessa_key_value_t *kv);


/**********************************************************************
 * vanessa_key_value_key_get_value
 * Get the value from a vanessa_key_value structure
 * pre: kv: key value structure to get the value of
 * return: value of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_value(vanessa_key_value_t *kv);


#define VANESSA_DESTROY_KV (void (*)(void *s))vanessa_key_value_destroy
#define VANESSA_DUPLICATE_KV (void *(*)(void *s))vanessa_key_value_duplicate


/**********************************************************************
 * Logging functionality
 **********************************************************************/

extern vanessa_logger_t *vanessa_adt_logger;

#define VANESSA_ADT_LOG(priority, fmt, args...) \
  vanessa_logger_log(vanessa_adt_logger, priority, fmt, ## args);

#define VANESSA_ADT_DEBUG(s) VANESSA_ADT_LOG(LOG_DEBUG, s);

#define VANESSA_ADT_DEBUG_ERRNO(s, e) \
  VANESSA_ADT_LOG(LOG_DEBUG, "%s: %e", s, strerror(e));



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

#endif
