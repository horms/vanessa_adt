/**********************************************************************
 * vanessa_queue.c                                         October 1999
 * Horms                                             horms@vergenet.net
 *
 * Data independent queue 
 *
 * Useless Note: This code was origionally written as a queue of a
 *               fixed data type on some steps at St. Marys Public
 *               School, NSW, Australia in June 1997.
 * vanessa_adt
 * Library of Abstract Data Types
 * Copyright (C) 1999-2002  Horms
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

struct vanessa_queue_member_t_struct {
        void *value;
        struct vanessa_queue_member_t_struct *prev;
        struct vanessa_queue_member_t_struct *next;
};

struct vanessa_queue_t_struct {
        vanessa_queue_member_t *first;
        vanessa_queue_member_t *last;
        void (*e_destroy) (const void *);
        int size;
};



/**********************************************************************
 * vanessa_queue_member_free
 * Free a vanessa_queue member
 * pre: vanessa_queue_member: element to free
 * post: vanessa_queue_member is freed
 * return: none
 **********************************************************************/

static void vanessa_queue_member_free(vanessa_queue_member_t * mem,
				      void (*e_destroy) (const void *)
    )
{
	if (mem == NULL) {
		return;
	}

	if (mem->value != NULL && e_destroy != NULL) {
		e_destroy(&(mem->value));
	}
	free(mem);
}


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


vanessa_queue_t *vanessa_queue_create(void (*e_destroy) (const void *))
{
	vanessa_queue_t *q;

	if ((q =
	     (vanessa_queue_t *) malloc(sizeof(vanessa_queue_t))) ==
	    NULL) {
		VANESSA_LOGGER_DEBUG("malloc");
		return (NULL);
	}

	q->first = NULL;
	q->last = NULL;
	q->e_destroy = e_destroy;
	q->size = 0;

	return q;
}


/**********************************************************************
 * vanessa_queue_push
 * push an element onto the beginning of a vanessa_queue
 * pre: q: vanessa_queue
 *      value: element to push onto the vanessa_queue
 * post: element is added to the queue
 * return: vanessa_queue with element added
 *       NULL on error. On error, where possible the vanessa_queue is 
 *       destroyed.
 **********************************************************************/

vanessa_queue_t *vanessa_queue_push(vanessa_queue_t * q, void *value)
{
	vanessa_queue_member_t *new;

	if (q == NULL) {
		return (NULL);
	}

	if ((new =
	     (vanessa_queue_member_t *)
	     malloc(sizeof(vanessa_queue_member_t))) == NULL) {
		VANESSA_LOGGER_DEBUG("malloc");
		vanessa_queue_destroy(q);
		return (NULL);
	}

	/*Put in payload */
	new->value = value;

	/*Update vanessa_queue pointers */
	new->next = q->first;
	new->prev = NULL;
	if (q->first == NULL) {
		q->last = new;
	} else {
		q->first->prev = new;
	}
	q->first = new;

	/*Increment vanessa_queue size */
	q->size++;

	return q;
}


/**********************************************************************
 * vanessa_queue_pop 
 * Pop an element off the end of a vanessa_queue
 * pre: q: vanessa_queue to pop the element off
 *      value: element removed from the vanessa_queue is assigned to 
 *             *value
 * post: elelemt is removed from queue
 * return: vanessa_queue with element removed
 *         NULL on error
 * Note: popping an empty vanessa_queue results in NULL being returned
 **********************************************************************/

vanessa_queue_t *vanessa_queue_pop(vanessa_queue_t * q, void **value)
{
	vanessa_queue_member_t *old;

	if (q == NULL || q->last == NULL) {
		return (NULL);
	}

	/*Grab payload */
	*value = q->last->value;

	/*Fix pointers */
	old = q->last;
	if ((q->last = old->prev) == NULL) {
		q->first = NULL;
	} else {
		q->last->next = NULL;
	}

	/*Decrement vanessa_queue size */
	q->size--;

	/* Free queue member, but not value */
	vanessa_queue_member_free(old, NULL);

	return q;
}


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

void *vanessa_queue_peek_last(vanessa_queue_t * q)
{
	return((q == NULL || q->last == NULL)?NULL:q->last->value);
}


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

void *vanessa_queue_peek_first(vanessa_queue_t * q)
{
	return((q==NULL || q->first==NULL)?NULL:q->first->value);
}



/**********************************************************************
 * vanessa_queue_destroy
 * Destroy a vanessa_queue, destroying each element present in the
 * vanessa_queue first
 * pre: q: vanessa_queue to destroy
 *      element
 * post: vanessa_queue and all elements in the vanessa_queue are 
 *       destroyed.
 * return: none
 **********************************************************************/

void vanessa_queue_destroy(vanessa_queue_t * q)
{
	vanessa_queue_member_t *old_first;

	if (q == NULL) {
		return;
	}

	while (q->first != NULL) {
		old_first = q->first;
		q->first = old_first->next;
		vanessa_queue_member_free(old_first, q->e_destroy);
	}

	free(q);
}


/**********************************************************************
 * vanessa_queue_length
 * Return the number of elements in the vanessa_queue
 * pre: q: vanessa_queue to find the number of elements in
 * post: none
 * return: number of elements in the vanessa_queue
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_queue_length(vanessa_queue_t * q)
{
	return(q == NULL?-1:q->size);
}
