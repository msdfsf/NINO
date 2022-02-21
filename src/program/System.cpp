#pragma once

#include "System.h"
#include "Cursor.h"

#include <windows.h>
#include "winuser.h"

namespace System {

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

}