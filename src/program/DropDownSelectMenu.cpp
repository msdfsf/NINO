#pragma once

#include "DropDownSelectMenu.h"
#include <Windows.h>
#include "Color.h"
#include "Render.h"
#include "Main.h"
#include "OverflowType.h"
#include "Cursor.h"

void selectMenuMouseClickCallback(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void mouseClickCallbackWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

DropDownSelectMenu::DropDownSelectMenu() {

	trackAnyClick = 1;
	opened = 0;
	selected = -1;
	
	cursor = Cursor::POINTER;

	selectMenu = new SelectMenu();
	window->pushControlFront(selectMenu);

	selectMenu->visible = 0;
	selectMenu->overflowBehaviour = OverflowType::VISIBLE;
	selectMenu->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
	selectMenu->hoverFrontColor = MAIN_BACK_COLOR;
	selectMenu->hoverBackColor = MAIN_HIGHLIGHT_COLOR;

}

DropDownSelectMenu::DropDownSelectMenu(char** items, int itemCount) {

	trackAnyClick = 1;
	opened = 0;
	selected = (itemCount > 0) ? 0 : -1;

	cursor = Cursor::POINTER;

	selectMenu = new SelectMenu(items, itemCount);
	window->pushControlFront(selectMenu);

	selectMenu->setBorderWidth(4);
	selectMenu->borderColor = MAIN_FRONT_COLOR;
	selectMenu->fontSize = 8;
	selectMenu->textAlignment = StringAlignment::LEFT;
	selectMenu->visible = 0;
	selectMenu->displayOrder = DisplayOrder::ABOVE_ALL;
	selectMenu->overflowBehaviour = OverflowType::VISIBLE;
	selectMenu->breakEventsChain = 0;
	selectMenu->passMouseEvents = 0;
	selectMenu->parent = this;
	selectMenu->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
	selectMenu->hoverFrontColor = MAIN_BACK_COLOR;
	selectMenu->hoverBackColor = MAIN_HIGHLIGHT_COLOR;

	selectMenu->setItemHeight();
	selectMenu->setHeight(selectMenu->getIdealHeight());

	selectMenu->eMouseClick = &selectMenuMouseClickCallback;
	eMouseClick = &mouseClickCallbackWrapper;

}

void DropDownSelectMenu::draw() {

	Control::draw();

	if (!opened) return;

	selectMenu->y = this->y + this->height;
	selectMenu->x = this->x;
	selectMenu->width = this->width;
	
	//selectMenu->setItemHeight();

	//const int idealHeight = selectMenu->getIdealHeight();
	//selectMenu->setHeight(idealHeight);

	//selectMenu->draw();

}

void DropDownSelectMenu::mouseClickCallback(CTRL_PARAM paramA, CTRL_PARAM paramB) {
	
	if (isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) {
		
		opened = !opened;
		selectMenu->visible = opened;
	
	} else if (!selectMenu->isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) {

		opened = 0;
		selectMenu->visible = 0;
	
	}

	Render::redraw();

}

void DropDownSelectMenu::insertItems(char** items, int itemCount) {

	selectMenu->insertItems(items, itemCount);
	if (itemCount > 0) {
		selected = 0;
		text = selectMenu->getItem(0, &textLength);
		//selectOption(0);
	}
	selectMenu->setItemHeight();
	selectMenu->setHeight(selectMenu->getIdealHeight());

}

int DropDownSelectMenu::getSelectedOption() {

	return selected;

}

void DropDownSelectMenu::selectOption(int idx) {

	opened = 0;
	selectMenu->visible = 0;

	char* newText;
	if (idx != selected) {
		
		int newTextLength = 0;
		char* newText = selectMenu->getItem(idx, &newTextLength);
		if (newText != NULL) {
			
			selected = idx;
			text = newText;
			textLength = newTextLength;

			if (eChange != NULL)
				eChange(this, idx, 0);

		}
	
	}

	Render::redraw();

}

void selectMenuMouseClickCallback(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	SelectMenu* const src = (SelectMenu*) source;
	DropDownSelectMenu* const menu = (DropDownSelectMenu*)src->parent;
	
	menu->selectOption(src->hoverIdx);

}

void mouseClickCallbackWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	DropDownSelectMenu* src = (DropDownSelectMenu*)source;
	src->mouseClickCallback(paramA, paramB);

}