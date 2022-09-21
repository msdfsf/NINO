#pragma once

#include "Control.h"

class Input : public Control {

	public:

		static const int BUFFER_SIZE = 250;
		char buffer[BUFFER_SIZE];

		int textCursor; // if cursor is visible
		int textCursorColor;
		int textCursorWidth;

		using Control::Control;

		Input();

		virtual void draw();
		void drawCursor();

		void input(char ch);
		void erase();
		void eraseAll();

	private:

};
