#include "mem_pool.h"

/* Initialize instance to NULL */
MemPool *MemPool::instance = NULL;

/* Function    : validate_obj_size
 * Description : checks if object size is power of 2 and for minimum object size
 * Arguments   : object size
 * Return      : mem_pool_status_t
 */
mem_pool_status_t MemPool::validate_obj_size(int obj_size)
{
	/* Check if object size id power of 2 */
	if(obj_size & (obj_size-1))
	{
		return MEM_POOL_ERROR_INVALID_OBJ_SIZE;
	}

	/* Check for valid object size */
	if(obj_size & ~MEM_POOL_VALID_OBJECT_MASK)
	{
		return MEM_POOL_ERROR_INVALID_OBJ_SIZE; 
	}
	return MEM_POOL_SUCCESS;
}

/* Function    : get_mem_pool_idx
 * Description : determines the pool index associated for a given object size
 * Arguments   : object size
 * Return      : pool index
 */
int MemPool::get_mem_pool_idx(int obj_size)
{
	int idx = MEM_POOL_MIN_VALID_INDEX;
	int i   = (0x1 << idx);

	while(!(obj_size & i)) {
		i = i << 1;
		idx++;
	}
	
	return idx;
}

/* Function    : add_pool_entry
 * Description : allocates memory for pool structure and pool memory
 * Arguments   : object size
 * Return      : mem_pool_status_t
 */
mem_pool_status_t MemPool::add_pool_entry(int obj_size)
{
	mem_pool_t *new_pool;
	int        pool_idx;

	pool_idx = get_mem_pool_idx(obj_size);

	/* Check if pool entry already exists? */
	if(mem_pools[pool_idx] != MEM_POOL_EMPTY)
	
	{
		return MEM_POOL_SUCCESS;
	}

	/* Create pool */
	new_pool = (mem_pool_t *)malloc(sizeof(mem_pool_t));
	if(new_pool == NULL) {
		return MEM_POOL_ERROR_NOT_ENOUGH_MEMORY;
	}

	/* track the pool address */
	mem_pools[pool_idx] = (char *)new_pool;

	/* Allocate pool memory */
	new_pool->pool_addr = (char *)malloc(obj_size*MEM_POOL_MAX_OBJECTS);
	if(new_pool->pool_addr == NULL) {
		free(new_pool);
		mem_pools[pool_idx] = MEM_POOL_EMPTY;
		return MEM_POOL_ERROR_NOT_ENOUGH_MEMORY;
	}

#if (MEM_POOL_MAX_OBJECTS == 32)
	/* Init pool object slot tracker to zero */
	new_pool->pool_obj_slot = 0x0;
#else
	int slot;
	/* Init pool object address array to known value */	
	for(slot=0; slot<MEM_POOL_MAX_OBJECTS; slot++)
	{
		new_pool->pool_obj_addr[slot] = MEM_POOL_OBJECT_EMPTY;
	}
#endif

	/* record the object size in pool */
	new_pool->pool_obj_size = obj_size;

	return MEM_POOL_SUCCESS;
}


/* Function    : create
 * Description : creates a pool for a given object size
 * Arguments   : object size
 * Return      : mem_pool_status_t
 */
mem_pool_status_t MemPool::create(int obj_size)
{
	mem_pool_status_t status;

	/* Validate obj_size requested */
	status = validate_obj_size(obj_size);
	if(status != MEM_POOL_SUCCESS)
	{
		return status;
	}


	/* Create pool entry */
	return add_pool_entry(obj_size);
}


/* Function    : allocate
 * Description : allocate address for requested object size 
 * Arguments   : object size 
 * Return      : valid address otherwise NULL
 */
void * MemPool::allocate(int obj_size, bool initialize_zero)
{
	mem_pool_t        *pool;
	int               pool_idx;
	int               slot;
	bool              is_pool_have_space = false;
	
	/* Validate obj_size requested */
	if(MEM_POOL_SUCCESS != validate_obj_size(obj_size))
	{
		return NULL;
	}

	pool_idx = get_mem_pool_idx(obj_size);

	/* Check if there is a pool entry for a given object size? */
	if(mem_pools[pool_idx] == MEM_POOL_EMPTY)
	{
		cout << "create should be called before calling allocate" << endl;
		return NULL;
	}

	/*  Get to the pool handle */
	pool = (mem_pool_t *)mem_pools[pool_idx];

	/*  Look for empty slot */
	for(slot=0; slot<MEM_POOL_MAX_OBJECTS; slot++)
	{
#if (MEM_POOL_MAX_OBJECTS == 32)
		if(!(pool->pool_obj_slot & (1U << slot)))
#else
		if(pool->pool_obj_addr[slot] == MEM_POOL_OBJECT_EMPTY)
#endif
		{
			is_pool_have_space = true;
			break;
		}
	}

	if(!is_pool_have_space)
	{
		cout << "No space left to allocate for object, maximum limit reached!" << endl;
		return NULL;
	}

	char *obj_addr  = (pool->pool_addr + (slot*pool->pool_obj_size));
	if(initialize_zero)
	{
		/* zero out the previously allocated memory for a given object */
		memset(obj_addr, 0x0, pool->pool_obj_size);
	}

#if (MEM_POOL_MAX_OBJECTS == 32)
	/* track the slot for new object */
	pool->pool_obj_slot |= (1U << slot);
	/* Return the address of new object */
	return ((void*)(obj_addr));
#else
	/* track the address of new object */
	pool->pool_obj_addr[slot] = obj_addr;

	/* Return the address of new object */
	return (void *)(pool->pool_obj_addr[slot]);
#endif
}


/* Function    : deallocate
 * Description : clean up memory assicated with object previously allocated
 * Arguments   : object address to be deallocated
 * Return      : mem_pool_status_t
 */
mem_pool_status_t MemPool::deallocate(char *address)
{
	mem_pool_t *pool;
	int        pool_idx;
	int        pool_bits;
	int        slot;
	bool       pool_present = false;

	if(address == NULL) {
		return MEM_POOL_ERROR_INVALID_ARGUMENT;
	}

	/* Determine the index to mem_pools from a given address */
	for(pool_idx=MEM_POOL_MIN_VALID_INDEX; pool_idx<MEM_POOL_MAX_CNT; pool_idx++)
	{
		/* Get pool handle */
		if(MEM_POOL_EMPTY == mem_pools[pool_idx]) {
			continue;
		}
		pool = (mem_pool_t *)mem_pools[pool_idx];
		if((address >= pool->pool_addr) &&
		   (address <= pool->pool_addr + (pool->pool_obj_size * MEM_POOL_MAX_OBJECTS)))
		{
			pool_present = true;
			break;
		}
	}

	if(!pool_present) {
		return MEM_POOL_ERROR_DOES_NOT_EXIST;
	}

	/* determine the slot number from given address */
	slot = ((address - pool->pool_addr)/pool->pool_obj_size);
#if (MEM_POOL_MAX_OBJECTS == 32)
	slot = (1U << slot);
	if(!(pool->pool_obj_slot & slot))
#else
	if(pool->pool_obj_addr[slot] != address)
#endif
	{
		return MEM_POOL_ERROR_INVALID_ARGUMENT;
	}

	/* clean up the record */
#if (MEM_POOL_MAX_OBJECTS == 32)
	pool->pool_obj_slot &= ~slot;
#else
	pool->pool_obj_addr[slot] = MEM_POOL_OBJECT_EMPTY;
#endif

	return MEM_POOL_SUCCESS;
}

/* Function    : destroy
 * Description : free pool structure and pool associated with it
 * Arguments   : obj_size mapping to one of the pool
 * Return      : mem_pool_status_t
 */
mem_pool_status_t MemPool::destroy(int obj_size)
{
	mem_pool_status_t status;
	mem_pool_t        *pool;
	int               pool_idx;
	int               slot;
	
	/* Validate obj_size requested */
	status = validate_obj_size(obj_size);
	if(status != MEM_POOL_SUCCESS)
	{
		return status;
	}

	pool_idx = get_mem_pool_idx(obj_size);

	/* Get pool handle */
	pool = (mem_pool_t *)mem_pools[pool_idx];

	if(pool == MEM_POOL_EMPTY)
	{
		return MEM_POOL_ERROR_DOES_NOT_EXIST;
	}

	mem_pools[pool_idx] = MEM_POOL_EMPTY;

	/* Deallocate memory */
	free(pool->pool_addr);
	free(pool);

	return MEM_POOL_SUCCESS;
}
