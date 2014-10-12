#include <iostream>
#include <vector>
#include <algorithm>
#include "KxServer.h"

using namespace std;
using namespace KxServer;

int main()
{
	CMemPool* pool = new CMemPool();

	pool->MemDumpInfo();

	void* buff1 = pool->MemAlocate(64);
	void* buff2 = pool->MemAlocate(63);
	void* buff3 = pool->MemAlocate(32);
	void* buff4 = pool->MemAlocate(31);

	pool->MemDumpInfo();

	pool->MemRecycle(buff1, 64);
	pool->MemRecycle(buff2, 64);
	pool->MemRecycle(buff3, 64);
	pool->MemRecycle(buff4, 64);

	pool->MemDumpInfo();

	int ret;
	cin >> ret;
	return ret;
}
