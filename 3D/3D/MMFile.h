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

	void* Read(const wchar_t* path);

	void* Ptr();

	DWORD fileLen = 0;
	DWORD fileLenHigh;

private:
	HANDLE mFile;
	HANDLE mFileMap;
	void* mPtr;

	wchar_t mFilePath[maxPathLen];
};

