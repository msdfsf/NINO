#pragma once

#include "System.h"
#include "Cursor.h"

#include <windows.h>
#include "winuser.h"

// drawing stuff
#pragma comment(lib, "Msimg32.lib")

namespace System {

	void* windowHandler = NULL;

	const HCURSOR defaultCursor = LoadCursorW(0, IDC_ARROW);
	const HCURSOR pointerCursor = LoadCursorW(0, IDC_HAND);

	void setCursor(int cursor) {

		switch (cursor) {

			case Cursor::DEFAULT:
				SetCursor(defaultCursor);
				break;
			
			case Cursor::POINTER:
				SetCursor(pointerCursor);
				break;

		}

	}

	void setWindowSize(int width, int height) {
		
		RECT winRect;
		GetWindowRect((HWND) windowHandler, &winRect);

		winRect.right = winRect.left + width;
		winRect.bottom = winRect.top + height;

		AdjustWindowRectEx(&winRect, GetWindowLongA((HWND)windowHandler, GWL_STYLE), 0, WS_EX_CLIENTEDGE);

		SetWindowPos(
			(HWND)windowHandler,
			NULL,
			winRect.left,
			winRect.top,
			winRect.right - winRect.left,
			winRect.bottom - winRect.top,
			NULL
		);
	
	}
	
	void blockResize() {
		
		LONG style = GetWindowLongA((HWND)windowHandler, GWL_STYLE);
		SetWindowLongA(
			(HWND) windowHandler,
			GWL_STYLE,
			style &~ WS_SIZEBOX
		);

	}

	void allowResize() {

		LONG style = GetWindowLongA((HWND)windowHandler, GWL_STYLE);
		SetWindowLongA(
			(HWND)windowHandler,
			GWL_STYLE,
			style | WS_SIZEBOX
		);

	}
	
	void blockMaximize() {

		LONG style = GetWindowLongA((HWND)windowHandler, GWL_STYLE);
		SetWindowLongA(
			(HWND)windowHandler,
			GWL_STYLE,
			style &~ WS_MAXIMIZEBOX
		);

	}
	
	void allowMaximize() {
		
		LONG style = GetWindowLong((HWND) windowHandler, GWL_STYLE);
		SetWindowLong(
			(HWND)windowHandler,
			GWL_STYLE,
			style | WS_MAXIMIZEBOX
		);

	}

	void drawPixels() {

	}

}
