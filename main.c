#include "mem_pool.h"

void dump_memory(int *addr, int size_in_bytes)
{
	int i;
	int size_in_words = size_in_bytes/4;

	cout << "Dumping memory ..." << endl;
	for(i=0; i<size_in_words; i++)
	{
		if(i==0 || (i*4 == (size_in_bytes-4)))
		{
			cout << "*(" << addr << " + " << i*4 <<") = " << std::hex << *(addr + i) << endl;
		}
	}
}

#define TEST_OBJ_SIZE (1*1024)

int test_1 ()
{
	int i;
	mem_pool_status_t status;

	/* Create pool of TEST_OBJ_SIZE byte size */
	MemPool *mem_pool = mem_pool->getInstance();

	status = mem_pool->create(TEST_OBJ_SIZE);
	if(status != MEM_POOL_SUCCESS) {
		cout << endl << "pool create failed for OBJ size = " << TEST_OBJ_SIZE \
			<< ", error = " << status << endl;
		return 1;
	}
	cout << "pool created for OBJ size = " << TEST_OBJ_SIZE << endl;

	int *addr = (int *)mem_pool->allocate(TEST_OBJ_SIZE);
	if(addr == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr = " << addr << endl;

	memset(addr, 0xAB, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr, TEST_OBJ_SIZE);

	int *addr1 = (int *)mem_pool->allocate(TEST_OBJ_SIZE);
	if(addr1 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr1 = " << addr1 << endl;

	memset(addr1, 0xCD, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr1, TEST_OBJ_SIZE);

	int *addr2 = (int *)mem_pool->allocate(TEST_OBJ_SIZE);
	if(addr2 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr2 = " << addr2 << endl;

	memset(addr2, 0xEF, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr2, TEST_OBJ_SIZE);

	status = mem_pool->deallocate(addr1);
	if(status != MEM_POOL_SUCCESS) {
		cout << "deallocation failed for address = "<< addr1 <<", error = " << status << endl;
		return 1;
	}
	cout << "deallocation successm addr1 = " << addr1 << endl;

	dump_memory(addr, TEST_OBJ_SIZE);
	dump_memory(addr1, TEST_OBJ_SIZE);
	dump_memory(addr2, TEST_OBJ_SIZE);

	addr1 = (int *)mem_pool->allocate(TEST_OBJ_SIZE);
	if(addr1 == NULL) {
		cout << "allocation unsuccessful" << endl;
		return 1;
	}
	cout << "allocation success, addr1 = " << addr1 << endl;

	memset(addr1, 0xCD, TEST_OBJ_SIZE*sizeof(char));

	dump_memory(addr1, TEST_OBJ_SIZE);

	cout << "destroy status : " << mem_pool->destroy(TEST_OBJ_SIZE) << endl;

	cout << "Test complete" << endl;

	return 0;
}

int test_2(void)
{
	mem_pool_status_t status;
	int obj_size = 16;
	int obj_num;
	int pool_idx;

	/* Create pool of TEST_OBJ_SIZE byte size */
	MemPool *mem_pool = mem_pool->getInstance();

	cout << "Creating pools from 16bytes to 1MB" << endl;
	for(pool_idx=4; pool_idx<(MEM_POOL_MAX_CNT-12); pool_idx++) {
		/* Create entry for all pools */
		status = mem_pool->create(obj_size);
		if(status != MEM_POOL_SUCCESS) {
			cout << "create : Test failed" << " error : " << status <<  endl;
			return 1;
		}
		cout << "create success for " << obj_size << " bytes" << endl;
		obj_size <<= 1;
	}

	cout << "Allocating for 1000 objects for each of the pools" << endl;
	obj_size = 16;
	/* allocate now in all the memory pools available */
	for(pool_idx=4; pool_idx<(MEM_POOL_MAX_CNT-12); pool_idx++) {
		for(obj_num=0; obj_num<MEM_POOL_MAX_OBJECTS; obj_num++) {
			mem_pool->allocate(obj_size);
		}
		obj_size <<= 1;
	}

	cout << "Destroying pools from 16bytes to 1MB" << endl;
	/* destroy all memory pools */
	obj_size = 16;
	for(pool_idx=4; pool_idx<(MEM_POOL_MAX_CNT-12); pool_idx++) {
		mem_pool->destroy(obj_size);
		obj_size <<= 1;
	}

	cout << "Test complete" << endl;
}

int main()
{
	//return test_1();
	return test_2();
}
