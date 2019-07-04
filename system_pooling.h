
/* macros */
#define SYS_POOL_MAX_OBJECTS		(1000)
#define SYS_POOL_MARKER 		(0x1234)
#define SYS_POOL_OBJECT_SIZE_MASK	(0xFFFF)
#define SYS_POOL_OBJECT_EMPTY		(NULL)
#define SYS_POOL_32BIT_ALIGNED_MASK	(0x3)

#ifndef NULL
#define NULL				(0)
#endif

/* typedef's */
typedef int pool_handle_t;

struct system_pool
{
	pool_handle_t      pool_handle;
	unsigned int       pool_obj_size;
	int                *pool;
	int                *pool_obj_addr[SYS_POOL_MAX_OBJECTS];
	struct system_pool *next;

};

typedef enum
{
	SYS_POOL_SUCCESS 		   = 0,
	SYS_POOL_ERROR_INVALID_ARGUMENT    = -1,
	SYS_POOL_ERROR_INVALID_OBJ_SIZE    = -2,
	SYS_POOL_ERROR_DOES_NOT_EXIST      = -3,
	SYS_POOL_ERROR_UNKNOWN		   = -4
}sys_pool_status_t;

/* Class template */
class SystemPool
{
	private:
		static SystemPool *instance;
		struct system_pool *system_pool_head;
		SystemPool():system_pool_head(NULL)
		{
		}

		sys_pool_status_t  push_to_pool_list    (int pool_obj_size, pool_handle_t *pool_handle);
		struct system_pool *get_from_pool_list  (pool_handle_t pool_handle);
		sys_pool_status_t  remove_from_pool_list(pool_handle_t pool_handle);

	public:
		static SystemPool *getInstance() {
			if(instance != NULL)
			{
				return instance;
			}
			instance = new SystemPool();
			return instance;
		}
		sys_pool_status_t create(int object_size, pool_handle_t *pool_handle);
		void              *allocate (pool_handle_t pool_handle);
		sys_pool_status_t deallocate(pool_handle_t pool_handle, int *address);
		sys_pool_status_t destroy   (pool_handle_t pool_handle);
};
