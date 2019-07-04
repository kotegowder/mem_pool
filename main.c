#include <iostream>
#include<cstring>
#include<stdio.h>
#include "system_pooling.h"

using namespace std;

void dump_memory(int *addr, int size_in_bytes)
{
	int i;
	int size_in_words = size_in_bytes/4;

	for(i=0; i<size_in_words; i++)
	{
		if(i==0 || (i*4 == (size_in_bytes-4)))
		{
			printf("\n*(%p + %d) = 0x%X", addr, i*4, *(addr + i)); 
		}
	}
}

#define TEST_OBJ_SIZE 1024

int main()
{
	int i;
	sys_pool_status_t status;

	/* Create pool of TEST_OBJ_SIZE byte size */
	SystemPool *sys_pool = sys_pool->getInstance();
	int pool_handle;

	status = sys_pool->create(TEST_OBJ_SIZE, &pool_handle);
	if(status != SYS_POOL_SUCCESS) {
		printf("\npool create failed for OBJ size = %d, error = %d\n", TEST_OBJ_SIZE, status);
		return 1;
	}
	printf("\npool created for OBJ size = %d", TEST_OBJ_SIZE);

	int *addr = (int *)sys_pool->allocate(pool_handle);
	printf("\nallocation success, addr = %p", addr);

	memset(addr, 0xAB, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr, TEST_OBJ_SIZE);

	int *addr1 = (int *)sys_pool->allocate(pool_handle);
	printf("\nallocation success, addr1 = %p", addr1);

	memset(addr1, 0xCD, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr1, TEST_OBJ_SIZE);

	int *addr2 = (int *)sys_pool->allocate(pool_handle);
	printf("\nallocation success, addr2 = %p", addr2);

	memset(addr2, 0xEF, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr2, TEST_OBJ_SIZE);

	status = sys_pool->deallocate(pool_handle, addr1);
	if(status != SYS_POOL_SUCCESS) {
		printf("\ndeallocation failed for pool handle = %d, address = %p, error = %d\n", pool_handle, addr1,  status);
		return 1;
	}
	printf("\ndeallocation success");

	dump_memory(addr, TEST_OBJ_SIZE);
	dump_memory(addr1, TEST_OBJ_SIZE);
	dump_memory(addr2, TEST_OBJ_SIZE);

	addr1 = (int *)sys_pool->allocate(pool_handle);
	printf("\nallocation success, addr1 = %p", addr1);

	memset(addr1, 0xCD, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr1, TEST_OBJ_SIZE);

	printf("\ndestroy status : %d", sys_pool->destroy(pool_handle));

	printf("\ndeallocate on a destroyed pool status = %d", sys_pool->deallocate(pool_handle, addr2));

	printf("\n");
	
	return 0;
}
