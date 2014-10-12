/* 
*   MemPool 内存池
*   减少内存碎片，提高分配内存效率，内存复用
*   
*   2013-04-14 By 宝爷
*
*/
#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <set>
#include <map>
#include <list>

namespace KxServer {

typedef std::map<unsigned int, std::list<void*>*>	    MemML;
typedef std::map<unsigned int, std::set<void*>*>		MemMS;

class CMemPool
{
public:
	CMemPool(void);
	virtual ~CMemPool(void);

	// 分配大小为size的内存,实际分配内存大小可能会更大
	void* MemAlocate(unsigned int size);

	// 回收内存,将MemPool分配的内存回收,回收的内存大小为size,成功返回0
	int MemRecycle(void* mem, unsigned int size);

	//显示内存池当前状态
	void MemDumpInfo();

private:
	
    //适配要分配的内存块大小
	inline unsigned int MemFitSize(unsigned int size);

	//获取一次性分配的数据数量
	inline unsigned int MemFitCounts(unsigned int size);

	//扩展内存池
	int MemExtend(unsigned int size, std::list<void*>* plist, std::set<void*>* pset);

	//扩展新内存池
	int MemExtendNewSize(unsigned int size);

	//释放指定大小的内存
	int MemReleaseWithSize(unsigned int size);

	//需要释放多少块内存
	unsigned int MemRelsaseCount(unsigned int size, unsigned int freecount, unsigned int stubcount);

	//根据当前水位等状态自动检测是否释放内存
	int MemAutoRelease(unsigned size, std::list<void*>* plist, std::set<void*>* pset);


private:

	MemML			m_Free;				                    //空闲内存块
	MemMS			m_Stub;				                    //内存块存根

	unsigned int	m_AlocatedSize;		                    //已分配大小
	unsigned int	m_WaterMark;		                    //标记水位
	unsigned int	m_MinAlocateSize;	                    //分配最小内存大小为 1 << m_MinAlocateSize

	static const unsigned int MAX_WATER_MARK = 1 << 30;		//最大水位
	static const unsigned int MAX_POOL_SIZE = 1 << 31;		//内存池最大容量 

	static const unsigned int MEM_BASE_COUNT = 32;			//每次分配数量
	static const unsigned int MEM_SIZE_MIN = 1 << 20;		//内存标量——小	1M
	static const unsigned int MEM_SIZE_MID = 1 << 26;		//内存标量——中	64M
	static const unsigned int MEM_SIZE_BIG = 1 << 27;		//内存标量——大 128M

};

class CMemManager
{
public:

    static CMemManager* GetInstance();
    
    static void Destroy();

    // 分配大小为size的内存,实际分配内存大小可能会更大
    void* MemAlocate(unsigned int size);

    // 回收内存,将MemPool分配的内存回收,回收的内存大小为size,成功返回0
    int MemRecycle(void* mem, unsigned int size);

    //显示内存池当前状态
    void MemDumpInfo();

private:

    CMemManager();

    ~CMemManager();

private:

    CMemPool* m_MemPool;

    static CMemManager* m_Instance;
};

inline void* MemMgrAlocate(unsigned int size)
{
    return CMemManager::GetInstance()->MemAlocate(size);
}

inline int MemMgrRecycle(void* mem, unsigned int size)
{
    return CMemManager::GetInstance()->MemRecycle(mem, size);
}

}

#endif