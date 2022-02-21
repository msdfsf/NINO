#pragma once

#include "Control.h"
#include "SelectMenu.h"

class DropDownSelectMenu : public Control {

	public:

		DropDownSelectMenu();
		DropDownSelectMenu(char** items, int itemCount);

		virtual void draw();

		void insertItems(char** items, int itemCount);

		void selectOption(int idx);
		int getSelectedOption();

		void mouseClickCallback(CTRL_PARAM paramA, CTRL_PARAM paramB);

	private:

		int opened;

		int selected;

		SelectMenu* selectMenu;

};