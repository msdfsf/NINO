#pragma once

#include "Utils.h"

#include <windows.h>

namespace Utils {

	void showError(const char* message) {
		
		MessageBoxA(
			NULL,
			message,
			"ERROR (((",
			MB_OK
		);
	
	}

	void showMessage(const char* message) {

		MessageBoxA(
			NULL,
			message,
			"INFO Hmm...",
			MB_OK
		);

	}

	int trimLeft(char* str) {

		int i = 0;
		char ch = *str;
		while (ch != '\0') {

			const char chr = *str;
			if (chr > ' ' && chr < 0x7f) {
				return i;
			}
			
			ch = chr;
			str++;
			i++;

		}
	
	}

	int trimRight(char* str, const int strLen) {
		
		for (int i = strLen - 1; i >= 0; i--) {
			const char ch = str[i];
			if (ch > ' ' && ch < 0x7f) {
				return i;
			}
		}

	}

	// following page was used https://en.wikipedia.org/wiki/UTF-8
	// outByets has to be allocated
	// return value is size in bytes of resulting utf8 string
	int wc2utf8(wchar_t* str, const int strLen, char** outBytes) {

		char* out = *outBytes;
		int outLen = 0;

		for (int i = 0; i < strLen; i++) {

			const int ch = str[i];

			if (ch <= 0x7F) {
				// ASCII

				*out = ch;

				out++;
				outLen++;

			} else if (ch <= 0x7FF) {
				// two bytes

				*out = (ch >> 6) & 0x1F | 0xC0;
				*(out + 1) = ch & 0x3F | 0x80;

				out += 2;
				outLen += 2;

			} else {
				// three bytes

				*out = (ch >> 12) & 0xF | 0xE0;
				*(out + 1) = (ch >> 6) & 0x3F | 0x80;
				*(out + 2) = ch & 0x3F | 0x80;

				out += 3;
				outLen += 3;

			}

		}

		return outLen;

	}

}