#pragma once

#include "Control.h"

class SelectMenu : public Control {

	public:

		struct Item {

			int id;

			char* text;
			int textLen;

		};

		using Control::Control;

		SelectMenu();
		SelectMenu(char** items, int itemCount);

		virtual void draw();

		int itemPadding;
		int itemHeight;

		int itemDelimiterHeight;
		int itemDelimiterWidth;

		int hoverIdx = 0;

		int itemCount = 0;
		Item** items;

		int scrollOffsetY = 0;
		int maxScrollOffsetY = 0;
		int lastScrollOffsetY = 0;

		void freeItems();

		void setItemHeight();

		int getItemCount();

		int getIdealHeight();

		int getPrimeIndex(); // naming as allways

		void addItem(char* item);
		void addItem(char* item, const int textLen);
		void addItems(char** items, int itemCount);
		void insertItems(char** items, int itemCount);
		char* getItem(int idx, int* len);

		void scroll(CTRL_PARAM paramA, CTRL_PARAM paramB);
		void mouseOver(CTRL_PARAM paramA, CTRL_PARAM paramB);

		~SelectMenu() {};

private:

};