#pragma once

// supposed to work as wrapper for system calls

#include "Cursor.h"

namespace System {

	extern void* windowHandler;

	void setCursor(int cursor);

	void setWindowSize(int width, int height);
	
	void blockResize();
	void allowResize();
	
	void blockMaximize();
	void allowMaximize();

}
