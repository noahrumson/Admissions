// Noah Rubin

#ifndef SMALL_TYPE_ALLOCATOR_H_INCLUDED
#define SMALL_TYPE_ALLOCATOR_H_INCLUDED

#include <deque>
#include <iostream>

#include "defines.h"


template<class T>
class PoolAllocator
{
public:
	PoolAllocator();

	~PoolAllocator();

	PoolAllocator(const PoolAllocator& copy) = delete;

	PoolAllocator(PoolAllocator&& move) = delete;

	PoolAllocator& operator=(const PoolAllocator& copy) = delete;

	PoolAllocator& operator=(PoolAllocator&& move) = delete;

	void* Allocate();

	void Destroy(void* ptr);

private:
	static constexpr int PAGE_SIZE = 0x0400;	// 1024 = 2^10

	void Grow();

	std::deque<void*> m_available;
	std::deque<char*> m_pages;

	int allocs = 0;
	int deallocs = 0;
};


template<class T>
inline PoolAllocator<T>::PoolAllocator() :
	m_available(),
	m_pages()
{
	Grow();
}


template<class T>
inline PoolAllocator<T>::~PoolAllocator()
{
	for (char* page : m_pages) {
		delete[] page;
	}
}


template<class T>
inline void* PoolAllocator<T>::Allocate()
{
	if (m_available.size() == 0) {
		Grow();
	}
	void* ret = m_available.front();
	m_available.pop_front();
	++allocs;
	return ret;
}


template<class T>
inline void PoolAllocator<T>::Destroy(void* ptr)
{
	m_available.push_front(ptr);
	++deallocs;
}


template<class T>
inline void PoolAllocator<T>::Grow()
{
	char* buf = new char[PAGE_SIZE];
	int usableSize = PAGE_SIZE - PAGE_SIZE % sizeof(T);
	for (char* ptr = buf; ptr < buf + usableSize; ptr += sizeof(T)) {
		m_available.push_back(ptr);
	}
	m_pages.push_back(buf);
}


#endif // !SMALL_TYPE_ALLOCATOR_H_INCLUDED