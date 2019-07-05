#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

/* macros */
#define MEM_POOL_MAX_CNT		(32)
#define MEM_POOL_VALID_OBJECT_MASK	(0xFFFFFFFF)
#define MEM_POOL_MIN_VALID_INDEX	(0x0)
#define MEM_POOL_MAX_OBJECTS		(1000)
#define MEM_POOL_EMPTY	 		(NULL)
#define MEM_POOL_OBJECT_EMPTY		(NULL)

#ifndef NULL
#define NULL				(0)
#endif

/* typedef's */
typedef struct
{
	int              pool_obj_size;
	char             *pool_addr;
	char             *pool_obj_addr[MEM_POOL_MAX_OBJECTS];
}mem_pool_t;

typedef enum
{
	MEM_POOL_SUCCESS 		   = 0,
	MEM_POOL_ERROR_INVALID_ARGUMENT    = -1,
	MEM_POOL_ERROR_INVALID_OBJ_SIZE    = -2,
	MEM_POOL_ERROR_NOT_ENOUGH_MEMORY   = -3,
	MEM_POOL_ERROR_DOES_NOT_EXIST      = -4,
	MEM_POOL_ERROR_UNKNOWN		   = -5
}mem_pool_status_t;

/* Class template */
class MemPool
{
	private:
		static MemPool *instance;
		char           *mem_pools[MEM_POOL_MAX_CNT];
		MemPool()
		{
			int i;
			for(i=0; i<MEM_POOL_MAX_CNT;i++)
			{
				mem_pools[i] = MEM_POOL_EMPTY;
			}
		}

		mem_pool_status_t validate_obj_size(int obj_size);
		int               get_mem_pool_idx (int obj_size);
		mem_pool_status_t add_pool_entry   (int obj_size);

	public:
		static MemPool *getInstance() {
			if(instance != NULL)
			{
				return instance;
			}
			instance = new MemPool();
			return instance;
		}
		mem_pool_status_t create    (int  obj_size);
		void              *allocate (int  obj_size);
		mem_pool_status_t deallocate(char *address);
		mem_pool_status_t destroy   (int  obj_size);
};

#endif
