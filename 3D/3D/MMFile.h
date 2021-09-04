#pragma once

#include <Windows.h>

//readonly
class MMFile
{
public:
	enum { maxPathLen = 32767 };

	MMFile();

	~MMFile();

	void Close();

	void* Read(const wchar_t* p, DWORD desiredAccess1 = GENERIC_READ, DWORD fileProtect = PAGE_READONLY, DWORD desiredAccess2 = FILE_MAP_READ);

	void* Ptr();

	DWORD fileLen = 0;
	DWORD fileLenHigh;

private:
	HANDLE mFile;
	HANDLE mFileMap;
	void* mPtr;

	wchar_t mFilePath[maxPathLen];
};

