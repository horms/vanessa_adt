/**********************************************************************
 * vanessa_key_value.c                                    December 1999
 * Horms                                             horms@verge.net.au
 *
 * Data independent queue 
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
 * 02111-1307 USA
 *
 **********************************************************************/

#include "vanessa_adt.h"


struct vanessa_key_value_t_struct {
	void *key;
	void (*destroy_key) (void *);
	void *(*dup_key) (void *);
	void *value;
	void (*destroy_value) (void *);
	void *(*dup_value) (void *);
};


/**********************************************************************
 * vanessa_key_value_create
 * Create a new vanessa_key_value structure
 * pre: none
 * post: a new vanessa_key_value strcture is allocated and elements are 
 *       initialised
 * return: pointer to vanessa_key_value_t
 *         NULL on error
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_create(void)
{
	vanessa_key_value_t *kv;

	if ((kv =
	     (vanessa_key_value_t *) malloc(sizeof(vanessa_key_value_t)))
	    == NULL) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return (NULL);
	}
	kv->key = NULL;
	kv->destroy_key = NULL;
	kv->dup_key = NULL;
	kv->value = NULL;
	kv->destroy_value = NULL;
	kv->dup_value = NULL;

	return (kv);
}


/**********************************************************************
 * vanessa_key_value_destroy
 * Destroy a vanessa_key_value and its contents
 * pre: kv: vanessa_key_value structure to destroy
 * post: kv and its contents are freed
 * return: none
 **********************************************************************/

void vanessa_key_value_destroy(vanessa_key_value_t * kv)
{
	if (kv != NULL && kv->destroy_key != NULL) {
		kv->destroy_key(kv->key);
	}
	if (kv != NULL && kv->destroy_value != NULL) {
		kv->destroy_value(kv->value);
	}
	free(kv);
}


/**********************************************************************
 * vanessa_key_value_duplicate
 * Make a copy of a vanessa_key_value_structure
 * pre: kv: vanessa_key_value structure to duplicate
 * post: 
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_duplicate(vanessa_key_value_t * kv)
{
	vanessa_key_value_t *cp;

	if (kv == NULL) {
		return (NULL);
	}
	if ((cp = vanessa_key_value_create()) == NULL) {
		return (NULL);
	}
	cp->key = (kv->dup_key == NULL) ? kv->key :
	    ((kv->key == NULL) ? NULL : kv->dup_key(kv->key));
	cp->destroy_key = kv->destroy_key;
	cp->dup_key = kv->dup_key;
	cp->value = (kv->dup_value == NULL) ? kv->value :
	    ((kv->value == NULL) ? NULL : kv->dup_value(kv->value));
	cp->destroy_value = kv->destroy_value;
	cp->dup_value = kv->dup_value;

	return (cp);
}


/**********************************************************************
 * vanessa_key_value_assign
 * Assign a key and value to a vanessa_key_value structure
 * pre: kv: key_value_structure to assign key and value to
 *      key: key to assign
 *      key_destroy: pointer to a function to destroy key
 *                    Use NULL if you do not want the key destroyed
 *                    when the vanessa_key_value_destroy is called
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
    )
{
	if (kv == NULL) {
		return (NULL);
	}
	kv->key = key;
	kv->destroy_key = destroy_key;
	kv->dup_key = dup_key;
	kv->value = value;
	kv->destroy_value = destroy_value;
	kv->dup_value = dup_value;
	return (kv);
}


/**********************************************************************
 * vanessa_key_value_unassign
 * Unassign values in vanessa_key_values structure
 * Useful if ou want to destroy the vanessa_key_value structure
 * without freeing the contents
 * pre: kv: key_value structure to unassign values of
 * post: All elements in vanessa_key_value structure are set to NULL
 * return: pointer to vanessa_key_value structure
 *         NULL if kv is NULL
 **********************************************************************/

vanessa_key_value_t *vanessa_key_value_unassign(vanessa_key_value_t * kv)
{
	if (kv == NULL) {
		return (NULL);
	}
	kv->key = NULL;
	kv->destroy_key = NULL;
	kv->dup_key = NULL;
	kv->value = NULL;
	kv->destroy_value = NULL;
	kv->dup_value = NULL;
	return (kv);
}


/**********************************************************************
 * vanessa_key_value_key_get_key
 * Get the key from a vanessa_key_value structure
 * pre: kv: key value structure to get the key of
 * return: key of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_key(vanessa_key_value_t * kv)
{
	if (kv == NULL) {
		return (NULL);
	}
	return (kv->key);
}


/**********************************************************************
 * vanessa_key_value_key_get_value
 * Get the value from a vanessa_key_value structure
 * pre: kv: key value structure to get the value of
 * return: value of kv
 *         NULL if kv is NULL
 **********************************************************************/

void *vanessa_key_value_get_value(vanessa_key_value_t * kv)
{
	if (kv == NULL) {
		return (NULL);
	}
	return (kv->value);
}
