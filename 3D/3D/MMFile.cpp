#include "MMFile.h"

MMFile::MMFile()
{
	mFile = INVALID_HANDLE_VALUE;
	mFileMap = NULL;
	mPtr = NULL;
}

MMFile::~MMFile()
{
	Close();
}

void* MMFile::Read(const wchar_t* p)
{
	if (mPtr != NULL)
	{
		Close();
	}

	ExpandEnvironmentStrings(p, mFilePath, _countof(mFilePath));

	mFile = CreateFile(mFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (mFile == INVALID_HANDLE_VALUE)
		return NULL;

	fileLen = GetFileSize(mFile, &fileLenHigh);

	mFileMap = CreateFileMapping(mFile, NULL, PAGE_READONLY, fileLenHigh, fileLen, NULL);
	if (mFileMap == NULL)
	{
		Close();
		return NULL;
	}

	mPtr = (wchar_t*)MapViewOfFile(mFileMap, FILE_MAP_READ, 0, 0, 0);
	if (mPtr == NULL)
	{
		Close();
	}

	return mPtr;
}

void* MMFile::Ptr()
{
	return mPtr;
}

void MMFile::Close()
{
	CloseHandle(mFile);
	CloseHandle(mFileMap);
	UnmapViewOfFile(mPtr);

	mFile = INVALID_HANDLE_VALUE;
	mFileMap = NULL;
	mPtr = NULL;
}