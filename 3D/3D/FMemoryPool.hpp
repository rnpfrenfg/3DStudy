#pragma once

#include "include.h"

//interlocked, cache
template<class T>
class FMemoryPool
{
	FMemoryPool()
	{
		InitializeSRWLock(&srwLock);
	}

	~FMemoryPool()
	{
		CloseHandle(srwLock);
	}
	
	void ReadStart()
	{
		AcquireSRWLockShared(&srwLock);
	}
	void ReadEnd()
	{
		ReleaseSRWLockShared(&srwLock);
	}
	void WriteStart()
	{
		AcquireSRWLockExclusive(&srwLock);
	}
	void WriteEnd()
	{
		ReleaseSRWLockExclusive(&srwLock);
	}

	void Init(int maxLen)
	{
		arr = new T[maxLen];
		actives = 0;
		maxActive = maxLen;
	}

	T* New()
	{
		if (actives == maxActive)
			return nullptr;
		WriteStart();
		T* t = arr[actives];
		actives++;
		WriteEnd();
		return t;
	}

	void Delete(T* t)
	{

	}

	SRWLOCK srwLock;
	T* arr;
	int actives;
	int maxActive;

private:

};