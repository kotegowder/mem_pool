#include "system_pooling.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

/* Initialize instance to NULL */
SystemPool *SystemPool::instance = NULL;

sys_pool_status_t SystemPool::push_to_pool_list(int pool_obj_size, pool_handle_t *pool_handle)
{
	struct system_pool **head_ref = &system_pool_head;
	sys_pool_status_t   status    = SYS_POOL_SUCCESS;
	int                 slot;

	/* allocate new node to track pool details */
	struct system_pool *new_pool = (struct system_pool *)malloc(sizeof(struct system_pool));

	/* ensure pool object size is multiple of 2 */
	if((pool_obj_size & SYS_POOL_32BIT_ALIGNED_MASK) != 0x0)
	{
		return SYS_POOL_ERROR_INVALID_OBJ_SIZE; 
	}

	/* Fill in initial details to the new pool */
	new_pool->pool_handle = SYS_POOL_MARKER | (pool_obj_size & SYS_POOL_OBJECT_SIZE_MASK);
	for(slot=0; slot<SYS_POOL_MAX_OBJECTS; slot++)
	{
		new_pool->pool_obj_addr[slot] = SYS_POOL_OBJECT_EMPTY; 
	}
	new_pool->pool          = (int *)malloc(SYS_POOL_MAX_OBJECTS*pool_obj_size);
	new_pool->pool_obj_size = pool_obj_size;


	*pool_handle = new_pool->pool_handle;

	/* Check if head is NULL */
	if(*head_ref == NULL)
	{
		*head_ref = new_pool;
		return status;
	}

	/* make next of new pool to point ot head */
	new_pool->next = *head_ref;

	/* make head to point to the new pool */
	*head_ref = new_pool;

	return status;
}

struct system_pool * SystemPool::get_from_pool_list(pool_handle_t pool_handle)
{
	struct system_pool *current = system_pool_head;

	if(current == NULL)
	{
		return NULL;
	}

	/* look for a given pool identifier in the system pool list */
	while(current != NULL)
	{
		if(current->pool_handle == pool_handle)
		{
			return current;
		}
		current = current->next;
	}

	/* call for a non-existent pool */
	return NULL;
}

sys_pool_status_t SystemPool::remove_from_pool_list(pool_handle_t pool_handle)
{
	struct system_pool **head_ref = &system_pool_head;
	struct system_pool *current   = *head_ref;
	struct system_pool *prev;

	if(*head_ref == NULL)
	{
		return SYS_POOL_ERROR_INVALID_ARGUMENT;
	}

	/* if head itself holds the pool identifier which is asked */
	if((current != NULL) && (current->pool_handle == pool_handle))
	{
		*head_ref = current->next;
		free(current);
		return SYS_POOL_SUCCESS;
	}

	/* search for the pool identifier */
	while((current != NULL) && (current->pool_handle != pool_handle))
	{
		prev    = current;
		current = current->next;
	}

	if(current == NULL)
	{
		return SYS_POOL_ERROR_DOES_NOT_EXIST;
	}

	/* unlink the pool from the list */
	prev->next = current->next;

	/* Free memory */
	free(current);
}

/* Function    : create
 * Description :
 * Arguments   :
 * Return      :
 */
sys_pool_status_t SystemPool::create(int object_size, pool_handle_t *pool_handle)
{
	struct system_pool *pool;

	/* Check if pool for given object_size already exists */
	pool = get_from_pool_list((SYS_POOL_MARKER | (object_size & SYS_POOL_OBJECT_SIZE_MASK)));
	if(pool != NULL)
	{
		*pool_handle = pool->pool_handle;
		return SYS_POOL_SUCCESS;
	}

	/* Add new pool details to the list */
	return push_to_pool_list(object_size, pool_handle);
}


/* Function    : allocate
 * Description : 
 * Arguments   :
 * Return      :
 */
void * SystemPool::allocate(pool_handle_t pool_handle)
{
	struct system_pool *pool;
	int                slot;
	bool               is_pool_have_space = false;

	/*  Get to the  pool from list based on pool_identifier */
	pool = get_from_pool_list(pool_handle); 
	if(pool == NULL)
	{
		printf("\nYou may have to create pool before calling allocate");
		return NULL;
	}

	/*  Look for empty slot */
	for(slot=0; slot<SYS_POOL_MAX_OBJECTS; slot++)
	{
		if(pool->pool_obj_addr[slot] == SYS_POOL_OBJECT_EMPTY)
		{
			is_pool_have_space = true;
			break;
		}
	}

	if(!is_pool_have_space)
	{
		return NULL;
	}

	/* track the address of new object */
	pool->pool_obj_addr[slot] = (pool->pool + ((slot*(pool->pool_obj_size))/4));

	printf("\npool_obj_addr = %p", pool->pool_obj_addr[slot]);

	/* Return the address of new object */
	return (void *)(pool->pool_obj_addr[slot]);
}


/* Function    : deallocate
 * Description : 
 * Arguments   :
 * Return      :
 */
sys_pool_status_t SystemPool::deallocate(pool_handle_t pool_handle, int *address)
{
	struct system_pool *pool;
	sys_pool_status_t  status = SYS_POOL_SUCCESS;
	int                slot;
	bool               is_address_valid = false;

	/* Cheeck for NULL address */
	if(address == NULL)
	{
		return SYS_POOL_ERROR_INVALID_ARGUMENT;
	}

	/* Get to the pool from list */
	pool = get_from_pool_list(pool_handle); 
	if(pool == NULL)
	{
		return SYS_POOL_ERROR_DOES_NOT_EXIST;
	}

	/* determine the slot number from given address */
	slot = (((address - pool->pool)*4)/pool->pool_obj_size);
	if(pool->pool_obj_addr[slot] != address)
	{
		return SYS_POOL_ERROR_INVALID_ARGUMENT;
	}

	/* zero out the previously allocated memory for a given object */
	printf("\npool_obj_size = %d", pool->pool_obj_size);
	memset(address, 0x0, pool->pool_obj_size);

	/* clean up the record */
	pool->pool_obj_addr[slot] = SYS_POOL_OBJECT_EMPTY;

	printf("\npool_obj_addr = %p", pool->pool_obj_addr[slot]);
	return status;
}

/* Function    : destroy
 * Description : 
 * Arguments   :
 * Return      :
 */
sys_pool_status_t SystemPool::destroy(pool_handle_t pool_handle)
{
	struct system_pool *pool;

	/*  Get to the  pool from list based on pool_identifier */
	pool = get_from_pool_list(pool_handle); 
	if(pool == NULL)
	{
		return SYS_POOL_ERROR_DOES_NOT_EXIST;
	}

	return remove_from_pool_list(pool_handle);
}
