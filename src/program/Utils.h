#pragma once

namespace Utils {

	void showError(const char* message);
	void showMessage(const char* message);

	int trimLeft(char* str);
	int trimRight(char* str, const int strLen);

	// following page was used https://en.wikipedia.org/wiki/UTF-8
	// outByets has to be allocated
	// return value is size in bytes of resulting utf8 string
	int wc2utf8(wchar_t* str, const int strLen, char** outBytes);

}
