#include "mem_pool.h"

void dump_memory(char *addr, int size_in_bytes)
{
	cout << "Dumping memory ..." << endl;
	cout << "At : " << (void *)(addr + 0) << " = " << *addr << endl;
	cout << "At : " << (void *)(addr + size_in_bytes - 1) << " = " << *(addr + size_in_bytes - 1) << endl;
}


int test_1 (int obj_size)
{
	int i;
	mem_pool_status_t status;

	/* Create pool of obj_size byte size */
	MemPool *mem_pool = mem_pool->getInstance();

	status = mem_pool->create(obj_size);
	if(status != MEM_POOL_SUCCESS) {
		cout << endl << "pool create failed for OBJ size = " << obj_size \
			<< ", error = " << status << endl;
		return 1;
	}
	cout << "pool created for OBJ size = " << obj_size << endl;

	char *addr = (char *)mem_pool->allocate(obj_size, true);
	if(addr == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr = " << (void *)addr << endl;

	memset(addr, 'A', obj_size*sizeof(char));

	dump_memory(addr, obj_size);

	char *addr1 = (char *)mem_pool->allocate(obj_size, true);
	if(addr1 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr1 = " << (void *)addr1 << endl;

	memset(addr1, 'B', obj_size*sizeof(char));

	dump_memory(addr1, obj_size);

	char *addr2 = (char *)mem_pool->allocate(obj_size, true);
	if(addr2 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr2 = " << (void *)addr2 << endl;

	memset(addr2, 'C', obj_size*sizeof(char));

	dump_memory(addr2, obj_size);

	status = mem_pool->deallocate(addr1);
	if(status != MEM_POOL_SUCCESS) {
		cout << "deallocation failed for address = "<< (void *)addr1 <<", error = " << status << endl;
		return 1;
	}
	cout << "deallocation success, addr1 = " << (void *)addr1 << endl;

	dump_memory(addr, obj_size);
	dump_memory(addr1, obj_size);
	dump_memory(addr2, obj_size);

	addr1 = (char *)mem_pool->allocate(obj_size, true);
	if(addr1 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr1 = " << (void *)addr1 << endl;

	memset(addr1, 'D', obj_size*sizeof(char));

	dump_memory(addr1, obj_size);

	cout << "destroy status : " << mem_pool->destroy(obj_size) << endl;

	cout << "Test complete" << endl;

	return 0;
}

int test_2(void)
{
	mem_pool_status_t status;
	int obj_size = 1;
	int obj_num;
	int pool_idx;

	/* Create pool of obj_size byte size */
	MemPool *mem_pool = mem_pool->getInstance();

	cout << "Creating pools upto 1MB" << endl;
	for(pool_idx=MEM_POOL_MIN_VALID_INDEX; pool_idx<=(MEM_POOL_MAX_CNT-12); pool_idx++) {
		/* Create entry for all pools */
		status = mem_pool->create(obj_size);
		if(status != MEM_POOL_SUCCESS) {
			cout << "create : Test failed" << " error : " << status <<  endl;
			return 1;
		}
		cout << "create success for " << obj_size << " bytes" << endl;
		obj_size <<= 1;
	}

	cout << "Allocating for "<< MEM_POOL_MAX_OBJECTS << " objects for each of the pools" << endl;
	obj_size = 1;
	/* allocate now in all the memory pools available */
	for(pool_idx=MEM_POOL_MIN_VALID_INDEX; pool_idx<=(MEM_POOL_MAX_CNT-12); pool_idx++) {
		for(obj_num=0; obj_num<MEM_POOL_MAX_OBJECTS; obj_num++) {
			mem_pool->allocate(obj_size, false);
		}
		obj_size <<= 1;
	}

	cout << "Destroying pools" << endl;
	/* destroy all memory pools */
	obj_size = 1;
	for(pool_idx=MEM_POOL_MIN_VALID_INDEX; pool_idx<=(MEM_POOL_MAX_CNT-12); pool_idx++) {
		mem_pool->destroy(obj_size);
		obj_size <<= 1;
	}

	cout << "Test complete" << endl;
}

int main()
{
	int obj_size = 1;
	for(obj_size=1; obj_size<=1*1024*1024; obj_size <<= 1)
	{
		test_1(obj_size);
		cout << endl;
	}
	return test_2();
}
