#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "FileDriver.h"
#include "Utils.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <corecrt_malloc.h>

namespace FileDriver {

	int loadBitmap(char* fileName, uint32_t** bmpData, int* width, int* height) {

		FILE* file = fopen(fileName, "rb");
		if (file <= 0) return 1;

		// skip header till the data offset
		fseek(file, 10, SEEK_CUR);

		int offset;
		fread(&offset, sizeof(int32_t), 1, file);

		// skip to the width and height
		fseek(file, 4, SEEK_CUR);

		fread(width, sizeof(int32_t), 1, file);
		fread(height, sizeof(int32_t), 1, file);

		int pixelCount = ((*width) * (*height));

		*bmpData = (uint32_t*) malloc(sizeof(uint32_t) * pixelCount);
		if (*bmpData == NULL) {
			fclose(file);
			return 2;
		}


		// skip till data
		fseek(file, offset, SEEK_SET);

		fread(*bmpData, sizeof(uint32_t), pixelCount, file);

		fclose(file);

		return 0;

	}

	int countAllFilesInDir(wchar_t* dirname, wchar_t* ext) {

		HANDLE hFind;
		WIN32_FIND_DATA findFileData;

		wchar_t flFilter[16];
		wchar_t flName[256];

		wcscpy(flFilter, (wchar_t*)L"*.");
		wcscpy(flFilter + 2, ext);

		flName[0] = L'\0';
		wcscat(flName, dirname);
		wcscat(flName, flFilter);

		int count = 0;
		if ((hFind = FindFirstFileW(flName, &findFileData)) != INVALID_HANDLE_VALUE) {

			do {
				count++;
			} while (FindNextFileW(hFind, &findFileData));

			FindClose(hFind);

		}

		return count;

	}

	void getAllFilesInDir(wchar_t* dirname, wchar_t* ext, FILE*** pfiles, const int fileCount, const char* mode) {

		FILE** files = *pfiles;

		HANDLE hFind;
		WIN32_FIND_DATA findFileData;

		wchar_t flFilter[16];
		wchar_t flName[256 + 1];

		wcscpy(flFilter, (wchar_t*)L"*.");
		wcscpy(flFilter + 2, ext);

		*flName = L'\0';
		wcscat(flName, dirname);
		wcscat(flName, flFilter);

		if ((hFind = FindFirstFileW(flName, &findFileData)) != INVALID_HANDLE_VALUE) {

			int i = 0;
			do {

				wchar_t fullPathW[256];
				*fullPathW = L'\0';
				wcscat(fullPathW, dirname);
				wcscat(fullPathW + wcslen(fullPathW), findFileData.cFileName);

				char fullPath[3 * sizeof(fullPathW) / 2];
				char* tmp = fullPath;
				Utils::wc2utf8(fullPathW, sizeof(fullPathW) / 2, &tmp);

				files[i] = fopen((char*) fullPath, mode);

				i++;

			} while (i < fileCount && FindNextFileW(hFind, &findFileData));

			for (i; i < fileCount; i++) {
				files[i] = NULL;
			}

			FindClose(hFind);

		}

	}

	void getAllFilesInDir(wchar_t* dirname, wchar_t* ext, char*** pfiles, const int fileCount, const char* mode) {

		char** files = *pfiles;

		HANDLE hFind;
		WIN32_FIND_DATA findFileData;

		wchar_t flFilter[16];
		wchar_t flName[256 + 1];

		wcscpy(flFilter, (wchar_t*)L"*.");
		wcscpy(flFilter + 2, ext);

		*flName = L'\0';
		wcscat(flName, dirname);
		wcscat(flName, flFilter);

		if ((hFind = FindFirstFileW(flName, &findFileData)) != INVALID_HANDLE_VALUE) {

			int i = 0;
			do {
				
				const int flNameLen = wcslen(findFileData.cFileName);
				const int flNameUtf8MaxLen = 3 * sizeof(flName) / 2 - 3;
				const int flNameMinLen = (flNameLen > flNameUtf8MaxLen) ? flNameUtf8MaxLen : flNameLen;

				char flNameUtf8[flNameUtf8MaxLen];
				char* tmp = flNameUtf8;
				const int flNameLenUtf8 = Utils::wc2utf8(findFileData.cFileName, flNameMinLen, &tmp);

				tmp = (char*) malloc(flNameLenUtf8 * sizeof(char) + 1);
				if (tmp == NULL) {
					break;
				}

				flNameUtf8[flNameLenUtf8] = '\0';
				tmp[flNameLenUtf8] = '\0';
				strcpy(tmp, flNameUtf8);

				files[i] = tmp;

				i++;

			} while (i < fileCount && FindNextFileW(hFind, &findFileData));

			for (i; i < fileCount; i++) {
				files[i] = NULL;
			}

			FindClose(hFind);

		}

	}

	void goThroughAllFilesInDir(wchar_t* dirname, wchar_t* ext, void (processFile)(wchar_t* dirname, wchar_t* flname)) {

		HANDLE hFind;
		WIN32_FIND_DATA findFileData;

		wchar_t flFilter[16];
		wchar_t flName[256];

		wcscpy(flFilter, (wchar_t*)L"*.");
		wcscpy(flFilter + 2, ext);

		flName[0] = L'\0';
		wcscat(flName, dirname);
		wcscat(flName, flFilter);

		if ((hFind = FindFirstFile(flName, &findFileData)) != INVALID_HANDLE_VALUE) {

			do {

				processFile(dirname, findFileData.cFileName);

			} while (FindNextFile(hFind, &findFileData));

			FindClose(hFind);

		}
	
	}

}
