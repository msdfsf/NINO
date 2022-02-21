#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Main.h"

#include "SelectMenu.h"
#include "Render.h"
#include "Color.h"
#include "Cursor.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>

void scrollWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void mouseOverWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void linguisticSort(SelectMenu::Item** array, int left, int right);
void strMerge(SelectMenu::Item** array, int left, int right, int pivot);
int itemcmp(SelectMenu::Item* itemA, SelectMenu::Item* itemB);

SelectMenu::SelectMenu() {

	itemPadding = 10;
	itemHeight = 0;

	itemDelimiterHeight = 2;
	itemDelimiterWidth = 100;

	this->eMouseScroll = &scrollWrapper;
	this->eMouseMove = &mouseOverWrapper;

	this->items = NULL;

}

SelectMenu::SelectMenu(char** items, int itemCount) : SelectMenu() {

	insertItems(items, itemCount);

}

void SelectMenu::setItemHeight() {
	
	const double marginCoef = 0.5;
	const int margin = this->fontSize * marginCoef;

	itemHeight = this->fontSize + 2 * margin;

	const int height = this->height - this->borderTopWidth - this->borderBottomWidth;
	maxScrollOffsetY = (this->itemHeight + this->itemDelimiterHeight) * this->itemCount - height - this->itemDelimiterHeight;
	maxScrollOffsetY = maxScrollOffsetY < 0 ? 0 : maxScrollOffsetY;

}

void SelectMenu::draw() {

	if (!visible) return;

	Control::draw();

	if (items == NULL || itemCount < 1) return;

	const int startY = this->y + this->borderTopWidth + this->paddingTop;

	const int width = this->width - this->borderLeftWidth - this->borderRightWidth - this->paddingLeft - this->paddingRight;
	const int height = this->height - this->borderTopWidth - this->borderBottomWidth - this->paddingTop - this->paddingBottom;

	const int step = itemHeight + itemDelimiterHeight;
	
	const int startIdx = scrollOffsetY / step;
	int idx = startIdx;

	int x = this->x + this->borderLeftWidth + this->paddingLeft;
	int y = startY - (scrollOffsetY % step);

	int frontColor = this->color;
	int backColor = this->backgroundColor;

	if (hoverIdx == idx) {

		frontColor = this->hoverFrontColor;
		backColor = this->hoverBackColor;

		Render::color = backColor;
		Render::fillRect(x, startY, width, itemHeight + y - startY);

	} else {

		frontColor = this->color;
		backColor = this->backgroundColor;

	}

	Render::color = frontColor;
	Render::drawString(
		this->items[idx]->text,
		this->items[idx]->textLen,
		this->textAlignment,
		backColor,
		this->fontSize,
		itemPadding + x,
		y,
		width - itemPadding,
		itemHeight,
		0,
		startY - y,
		width - itemPadding,
		itemHeight 
	);

	if (itemCount < 2) return;

	const int delY = y + itemHeight;

	Render::color = Color::WHITE;
	if (startY > delY) {
		Render::fillRect(x, startY, width, itemDelimiterHeight - (startY - delY));
	} else {
		Render::fillRect(x, delY, width, itemDelimiterHeight);
	}

	idx++;
	y += step;

	for (y; y < startY + height - step && idx < itemCount - 1; y += step) {
	
		if (hoverIdx == idx) {

			frontColor = this->hoverFrontColor;
			backColor = this->hoverBackColor;

			Render::color = backColor;
			Render::fillRect(x, y, width, itemHeight);

		} else {

			frontColor = this->color;
			backColor = this->backgroundColor;

		}

		Render::color = frontColor;
		Render::drawString(
			this->items[idx]->text,
			this->items[idx]->textLen,
			this->textAlignment,
			backColor,
			this->fontSize,
			itemPadding + x,
			y,
			width - itemPadding,
			itemHeight
		);

		idx++;

		Render::color = Color::WHITE;
		Render::fillRect(x, y + itemHeight, width, itemDelimiterHeight);

	}

	const int remainingY = this->y + this->height - this->borderBottomWidth - y;

	if (hoverIdx == idx) {

		frontColor = this->hoverFrontColor;
		backColor = this->hoverBackColor;

		Render::color = backColor;
		Render::fillRect(x, y, width, (remainingY < itemHeight) ? remainingY : itemHeight);

	} else {

		frontColor = this->color;
		backColor = this->backgroundColor;

	}

	Render::color = frontColor;
	Render::drawString(
		this->items[idx]->text,
		this->items[idx]->textLen,
		this->textAlignment,
		backColor,
		this->fontSize,
		itemPadding + x,
		y,
		width - itemPadding,
		itemHeight,
		0,
		0,
		width - itemPadding,
		remainingY
	);

	if (idx < itemCount - 1) {
		Render::color = Color::WHITE;
		Render::fillRect(x, y + itemHeight, width, (remainingY - itemHeight));
	}

}

void scrollWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	SelectMenu* src = (SelectMenu*) source;
	src->scroll(paramA, paramB);

}

void SelectMenu::scroll(CTRL_PARAM paramA, CTRL_PARAM paramB) {

	int wheelDistance = paramB / WHEEL_DELTA;

	const int minOffset = 0;
	const int maxOffset = maxScrollOffsetY;

	int pixelsToScroll = wheelDistance * Control::SCROLL_STEP;
	scrollOffsetY -= pixelsToScroll;
	
	if (scrollOffsetY < minOffset) {
		scrollOffsetY = minOffset;
	} else if (scrollOffsetY > maxOffset) {
		scrollOffsetY = maxOffset;
	};

	Render::redraw();


}

void mouseOverWrapper(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	SelectMenu* src = (SelectMenu*)source;
	src->mouseOver(paramA, paramB);

}

void SelectMenu::mouseOver(CTRL_PARAM paramA, CTRL_PARAM paramB) {

	const int realMouseX = GET_LOW_ORDER_WORD(paramA);
	const int realMouseY = GET_HIGH_ORDER_WORD(paramA);

	if (!isInBounds(realMouseX, realMouseY)) return;

	const int startY = y + borderTopWidth;
	const int startX = x + borderLeftWidth;

	const int mouseX = realMouseX - startX;
	const int mouseY = realMouseY - startY;
	
	const int step = itemHeight + itemDelimiterHeight;
	
	const int oldHoverIdx = hoverIdx;

	if (mouseY < 0 || mouseY > height) {
		hoverIdx = -1;
	} else if ((scrollOffsetY + mouseY) <= step * itemCount) {
		cursor = Cursor::POINTER;
		hoverIdx = (scrollOffsetY + mouseY) / step;
	} else {
		cursor = Cursor::DEFAULT;
		hoverIdx = -1;
	}
		//hoverIdx = (scrollOffsetY + mouseY) / step;//(scrollOffsetY + (scrollOffsetY % step) + mouseY) / step;//((scrollOffsetY % step) + mouseY) / step;

	if (oldHoverIdx != hoverIdx)
		Render::redraw();

}

int SelectMenu::getItemCount() {
	
	return itemCount;

}

int SelectMenu::getIdealHeight() {

	return itemCount * itemHeight + itemDelimiterHeight * (itemCount - 1) + borderBottomWidth + borderTopWidth;

}

int itemcmp(SelectMenu::Item* itemA, SelectMenu::Item* itemB) {
	
	int lenA = itemA->textLen;
	int lenB = itemB->textLen;

	char* strA = itemA->text;
	char* strB = itemB->text;

	int len = (lenA > lenB) ? lenB : lenA;
	for (int i = 0; i < len; i++) {

		if (strA[i] > strB[i]) return 2;
		else if (strA[i] < strB[i]) return 1;

	}

	if (lenA > lenB) return 2;
	else if (lenA < lenB) return 1;

	return 0;

}

void linguisticSort(SelectMenu::Item** array, int left, int right) {

	if (right <= left) return;

	int pivot = left + (right - left) / 2;
	linguisticSort(array, left, pivot);
	linguisticSort(array, pivot + 1, right);

	strMerge(array, left, right, pivot);

}

void strMerge(SelectMenu::Item** array, int left, int right, int pivot) {

	// copy left and right part to separate arrays
	int leftLen = pivot - left + 1;
	int rightLen = right - pivot;

	SelectMenu::Item** leftArr = (SelectMenu::Item**) malloc(sizeof(SelectMenu::Item*) * leftLen);
	if (leftArr == NULL) return; // have to handle it better, but ok for now

	SelectMenu::Item** rightArr = (SelectMenu::Item**) malloc(sizeof(SelectMenu::Item*) * rightLen);
	if (rightArr == NULL) return;

	// may rewrite to have first for to fill both arrays and then second to 
	// fill the remaining part of longer one
	for (int i = 0; i < leftLen; i++) {
		leftArr[i] = array[left + i];
	}

	for (int i = 0; i < rightLen; i++) {
		rightArr[i] = array[pivot + 1 + i];
	}

	int i = 0;
	int j = 0;
	int arrIdx = left;
	while (i < leftLen && j < rightLen) {

		if (itemcmp(leftArr[i], rightArr[j]) == 1) {
			// leftArr[i] is linguistically first

			array[arrIdx] = leftArr[i];
			i++;

		} else {
			// rightArr[j] is linguistically first

			array[arrIdx] = rightArr[j];
			j++;

		}

		arrIdx++;

	}

	if (i >= leftLen) {
		// rightArr may have something more

		for (j; j < rightLen; j++, arrIdx++) {
			array[arrIdx] = rightArr[j];
		}

	} else if (j >= rightLen) {
		// leftArr may have something more

		for (i; i < leftLen; i++, arrIdx++) {
			array[arrIdx] = leftArr[i];
		}

	}

	free(rightArr);
	free(leftArr);

}

void SelectMenu::freeItems() {

	SelectMenu::Item** items = this->items;
	const int count = this->itemCount;

	for (int i = 0; i < count; i++) {
		free(items[i]);
	}
	
	free(items);

}

void SelectMenu::insertItems(char** items, int itemCount) {

	freeItems();

	this->itemCount = itemCount;
	this->items = (SelectMenu::Item**) malloc(itemCount * sizeof(SelectMenu::Item*));
	if (this->items == NULL) return; // have to handele it better, but who cares

	for (int i = 0; i < itemCount; i++) {

		this->items[i] = (SelectMenu::Item*) malloc(sizeof(SelectMenu::Item));
		if (this->items[i] == NULL) return;

		this->items[i]->id = i;
		this->items[i]->text = (char*)items[i];
		this->items[i]->textLen = strlen(items[i]);

	}

	linguisticSort(this->items, 0, itemCount - 1);

}

void SelectMenu::addItems(char** items, int itemCount) {

	SelectMenu::Item** oldItems = this->items;
	const int oldItemCount = this->itemCount;

	this->itemCount += itemCount;
	this->items = (SelectMenu::Item**) realloc(this->items, this->itemCount * sizeof(SelectMenu::Item*));
	if (this->items == NULL) {

		this->itemCount = oldItemCount;
		this->items = oldItems;
		
		return;
	
	}

	const int count = this->itemCount;
	for (int i = oldItemCount; i < count; i++) {

		char* item = items[i - oldItemCount];
		SelectMenu::Item* thisItem = this->items[i];

		thisItem = (SelectMenu::Item*) malloc(sizeof(SelectMenu::Item));
		if (thisItem == NULL) {
			// maybe add realloc, but it could fail, so it will be to complicated, dunno...

			this->itemCount = i;
			return;

		}

		thisItem->id = i;
		thisItem->text = item;
		thisItem->textLen = strlen(item);

		this->items[i] = thisItem;

	}

	linguisticSort(this->items, 0, itemCount - 1);

	itemHeight = 0;

	itemDelimiterHeight = 2;
	itemDelimiterWidth = 100;

	//this->eMouseScroll = &pluginListScrollWrapper;
	//this->eMouseMove = &pluginListMouseOverWrapper;
}

void SelectMenu::addItem(char* item) {

	addItem(item, strlen(item));

	/*
	SelectMenu::Item** oldItems = this->items;
	const int oldItemCount = this->itemCount;

	this->itemCount++;
	this->items = (SelectMenu::Item**) realloc(this->items, this->itemCount * sizeof(SelectMenu::Item*));
	if (this->items == NULL) {

		this->itemCount = oldItemCount;
		this->items = oldItems;

		return;

	}

	SelectMenu::Item* thisItem = this->items[oldItemCount];

	thisItem = (SelectMenu::Item*) malloc(sizeof(SelectMenu::Item));
	if (thisItem == NULL) {

		this->itemCount--;
		return;

	}

	thisItem->id = oldItemCount;
	thisItem->text = item;
	thisItem->textLen = strlen(item);

	this->items[oldItemCount] = thisItem;

	linguisticSort(this->items, 0, itemCount - 1);

	itemHeight = 0;

	itemDelimiterHeight = 2;
	itemDelimiterWidth = 100;
	*/

}

int SelectMenu::getPrimeIndex() {
	return items[hoverIdx]->id;
}


void SelectMenu::addItem(char* item, const int textLen) {

	SelectMenu::Item** oldItems = this->items;
	const int oldItemCount = this->itemCount;

	this->itemCount++;
	this->items = (SelectMenu::Item**)realloc(this->items, this->itemCount * sizeof(SelectMenu::Item*));
	if (this->items == NULL) {

		this->itemCount = oldItemCount;
		this->items = oldItems;

		return;

	}

	SelectMenu::Item* thisItem = this->items[oldItemCount];

	thisItem = (SelectMenu::Item*)malloc(sizeof(SelectMenu::Item));
	if (thisItem == NULL) {

		this->itemCount--;
		return;

	}

	thisItem->id = oldItemCount;
	thisItem->text = item;
	thisItem->textLen = textLen;

	this->items[oldItemCount] = thisItem;

	linguisticSort(this->items, 0, itemCount - 1);

	itemHeight = 0;

	itemDelimiterHeight = 2;
	itemDelimiterWidth = 100;

}

char* SelectMenu::getItem(int idx, int* len) {

	if (idx >= itemCount || idx < 0) {
		*len = 0;
		return NULL;
	}

	*len = items[idx]->textLen;
	return items[idx]->text;

}