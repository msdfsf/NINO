#pragma once
#include <cstdint>
#include <stdio.h>

namespace FileDriver {

	int loadBitmap(char* fileName, uint32_t** bmpData, int* width, int* height);

	int countAllFilesInDir(wchar_t* dirname, wchar_t* ext);
	void getAllFilesInDir(wchar_t* dirname, wchar_t* ext, FILE*** files, const int fileCount, const char* mode);
	void getAllFilesInDir(wchar_t* dirname, wchar_t* ext, char*** pfiles, const int fileCount, const char* mode);
	void goThroughAllFilesInDir(wchar_t* dirname, wchar_t* ext, void (processFile)(wchar_t* dirname, wchar_t* flname));

}
