#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Control.h"
#include "Color.h"
#include "Render.h"
#include "StringAlignment.h"
#include "Main.h"
#include "Utils.h"
#include "OverflowType.h"
#include "System.h"

#include "Window.h"

#include <typeinfo>

#include <cstddef>

Control* Control::window = new Window();
Control* Control::focusedControl = NULL;

std::list<Control*> Control::childrensAboveAll = std::list<Control*>();

Control::Control() {

	this->parent = NULL;
	this->childrens = NULL;
	this->childrensDrawLast = NULL;

	displayOrder = DisplayOrder::NORMAL;

	this->x = 0;
	this->y = 0;

	this->width = 0;
	this->height = 0;

	this->paddingTop = 0;
	this->paddingBottom = 0;
	this->paddingLeft = 0;
	this->paddingRight = 0;

	this->marginTop = 0;
	this->marginBottom = 0;
	this->marginLeft = 0;
	this->marginRight = 0;

	this->color = Color::WHITE;
	this->backgroundColor = Color::BLACK;

	this->actualColor = this->color;
	this->actualBackColor = this->backgroundColor;

	this->selectFrontColor = this->color;
	this->selectBackColor = this->backgroundColor;

	this->hoverFrontColor = Color::WHITE;
	this->hoverBackColor = Color::PURPLE;

	this->borderColor = Color::BLACK;
	this->borderLeftWidth = 0;
	this->borderRightWidth = 0;
	this->borderTopWidth = 0;
	this->borderBottomWidth = 0;

	this->innerBorderBevelWidth = 0;
	this->outerBorderBevelWidth = 0;

	this->borderTitleText = NULL;
	this->borderTitleLength = 0;
	this->borderTitlePadding = 0;
	this->borderTitleFontSize = 0;

	this->visible = 1;
	this->selected = 0;

	this->text = NULL;
	this->textLength = 0;
	this->fontSize = 0;
	this->textAlignment = StringAlignment::CENTER;

	this->draggable = 0;
	this->droppable = 0;

	this->focused = 0;

	this->passMouseEvents = 1;
	this->breakEventsChain = 0;

	this->trackAnyClick = 0;

	this->cursor = Cursor::NONE;

}

Control::Control(Control* parent) : Control() {

	this->parent = parent;

}

Control::Control(Control* parent, int x, int y, int width, int height) {

	this->parent = parent;

	this->setX(x);
	this->setY(y);

	this->setWidth(width);
	this->setHeight(height);

}

Control::Control(Control* parent, double x, double y, double width, double height) {

	this->parent = parent;

	this->setX(x);
	this->setY(y);

	this->setWidth(width);
	this->setHeight(height);

}

void Control::draw() {

	if (!visible) return;

	// decide if we need to render control
	const int bottomY = this->y + this->height;
	const int parentY = parent->y;
	const int parentHeight = parent->height;
	const int parentOverflowTop = parent->overflowTop;
	const int parentOverflowBottom = parent->overflowBottom;
	if (overflowBehaviour != OverflowType::VISIBLE && (bottomY < parentY + parentOverflowTop || this->y > parentY + parentHeight - parentOverflowBottom)) return;

	const int color = this->actualColor;
	const int backColor = this->actualBackColor;

	const int parentInBoxY = parent->getInBoxY();

	int overflowTop = 0;
	int bottomCutHeight = 0;
	int bottomCutY = 0;
	//this->overflowTop = 0;
	//this->overflowBottom = 0;
	if (overflowBehaviour != OverflowType::VISIBLE) {

		// top overflow handling
		if (this->y < parentInBoxY + parentOverflowTop) {

			overflowTop = parentInBoxY - this->y + parentOverflowTop;
			if (parentOverflowTop != 0) {
				overflowTop -= (parentInBoxY - parentY);
			}
			this->overflowTop = overflowTop;

		}
		else {

			overflowTop = 0;
			this->overflowTop = 0;

		}

		// bottom overflow handling
		const int parentInboxHeight = parent->getInBoxHeight();
		const int parentBottomY = parentInBoxY + parent->getInBoxHeight() - parentOverflowBottom;

		if (bottomY > parentBottomY) {

			bottomCutHeight = bottomY - parentBottomY;
			bottomCutY = bottomY - bottomCutHeight;
			if (parentOverflowBottom != 0) {
				bottomCutHeight += ((parentInBoxY + parentInboxHeight) - (parentY + parentHeight));
				if (bottomCutHeight < 0) bottomCutHeight = 0;
			}

			overflowBottom = bottomCutHeight;

		}
		else {
			this->overflowBottom = 0;
		}

	}

	// borderBottomWidth < this->height - overflowTop - bottomCutY;
	const int visibleBorderBottomWidth = (borderBottomWidth < bottomCutY) ? 0 : borderBottomWidth - bottomCutY;

	// background
	Render::color = backColor;
	Render::fillRect(
		this->x + borderLeftWidth,
		y + overflowTop + borderTopWidth,
		this->width - borderLeftWidth - borderRightWidth,
		this->height - overflowTop - bottomCutHeight - visibleBorderBottomWidth - borderTopWidth
	);

	// borders
	Render::color = this->borderColor;

	if (outerBorderBevelWidth <= 0) {

		Render::drawRect(
			this->x,
			y + overflowTop,
			this->width,
			this->height - overflowTop - bottomCutHeight,
			borderLeftWidth,
			borderRightWidth,
			borderTopWidth - overflowTop,
			(borderBottomWidth < this->height - overflowTop - bottomCutY) ? borderBottomWidth : this->height - overflowTop - bottomCutY
		);

	}
	else {

		// not yet done
		// overflow needs to be done

		Render::drawRightTriangle(
			this->x,
			y + overflowTop,
			outerBorderBevelWidth,
			outerBorderBevelWidth - overflowTop,
			Render::CornerType::TOP_LEFT
		);

		Render::drawRightTriangle(
			x + this->width - outerBorderBevelWidth,
			y + overflowTop,
			outerBorderBevelWidth,
			outerBorderBevelWidth - overflowTop,
			Render::CornerType::TOP_RIGHT
		);

		Render::drawRightTriangle(
			x,
			y + overflowTop + this->height - outerBorderBevelWidth,
			outerBorderBevelWidth,
			outerBorderBevelWidth - overflowTop,
			Render::CornerType::BOTTOM_LEFT
		);

		Render::drawRightTriangle(
			x + this->width - outerBorderBevelWidth,
			y + overflowTop + this->height - outerBorderBevelWidth,
			outerBorderBevelWidth,
			outerBorderBevelWidth - overflowTop,
			Render::CornerType::BOTTOM_RIGHT
		);

		Render::fillRect(
			this->x + outerBorderBevelWidth, // maybe - 1
			y + overflowTop,
			this->width - 2 * outerBorderBevelWidth,
			borderTopWidth - overflowTop
		);

		Render::fillRect(
			this->x,
			y + outerBorderBevelWidth + overflowTop,
			borderLeftWidth,
			this->height - overflowTop - bottomCutHeight - 2 * outerBorderBevelWidth
		);

		Render::fillRect(
			this->x + this->width - borderRightWidth,
			y + outerBorderBevelWidth + overflowTop,
			borderRightWidth,
			this->height - overflowTop - bottomCutHeight - 2 * outerBorderBevelWidth
		);

		Render::fillRect(
			this->x + outerBorderBevelWidth,
			y + overflowTop + this->height - borderBottomWidth,
			this->width - 2 * outerBorderBevelWidth,
			borderBottomWidth - overflowTop - bottomCutHeight
		);

	}

	// text
	const int x = this->x + paddingLeft + borderTopWidth;
	const int y = this->y + paddingTop + borderTopWidth;

	const int width = this->width - paddingLeft - paddingRight - borderLeftWidth - borderRightWidth;
	const int height = this->height - paddingTop - paddingBottom - borderTopWidth - borderBottomWidth;

	Render::color = color;
	if (text != NULL) {
		Render::drawString(
			text,
			textLength,
			textAlignment,
			backColor,
			fontSize,
			x,
			y,
			width,
			height,
			0,
			overflowTop,
			width,
			height - overflowTop - bottomCutHeight
		);
	}

	// just fast implementation, does not work as expected and as has to
	if (borderTitleText != NULL) {

		const int textWidth = borderTitleFontSize * (1 + borderTitleLength);
		const int remWidth = this->width - borderTitlePadding;
		const int width = (textWidth > remWidth) ? remWidth : textWidth;

		Render::color = backColor;
		Render::fillRect(this->x + borderTitlePadding, this->y, borderTitleFontSize / 2, borderTopWidth);

		Render::color = borderColor;
		Render::drawString(
			borderTitleText,
			borderTitleLength,
			StringAlignment::CENTER,
			backColor,
			borderTitleFontSize,
			this->x + borderTitlePadding,
			this->y - (borderTitleFontSize - borderTopWidth) / 2,
			width,
			borderTitleFontSize
		);

		Render::color = backColor;
		Render::fillRect(this->x + borderTitlePadding + textWidth - borderTitleFontSize / 2, this->y, borderTitleFontSize / 2, borderTopWidth);

	}

	if (innerBorderBevelWidth > 0) {

		Render::color = borderColor;

		Render::drawRightTriangle(
			this->x + borderLeftWidth,
			this->y + overflowTop + borderTopWidth,
			innerBorderBevelWidth,
			innerBorderBevelWidth - overflowTop,
			Render::CornerType::BOTTOM_RIGHT
		);

		Render::drawRightTriangle(
			this->x + this->width - innerBorderBevelWidth - borderRightWidth,
			this->y + overflowTop + borderTopWidth,
			innerBorderBevelWidth,
			innerBorderBevelWidth - overflowTop,
			Render::CornerType::BOTTOM_LEFT
		);

		Render::drawRightTriangle(
			this->x + borderLeftWidth,
			this->y + overflowTop + this->height - innerBorderBevelWidth - borderBottomWidth,
			innerBorderBevelWidth,
			innerBorderBevelWidth - overflowTop,
			Render::CornerType::TOP_RIGHT
		);

		Render::drawRightTriangle(
			this->x + this->width - innerBorderBevelWidth - borderRightWidth,
			this->y + overflowTop + this->height - innerBorderBevelWidth - borderBottomWidth,
			innerBorderBevelWidth,
			innerBorderBevelWidth - overflowTop,
			Render::CornerType::TOP_LEFT
		);

	}

	// childrens
	Render::color = Render::DEFAULT_COLOR;

	int lastIdxDrawLast = 0;

	for (int i = 0; i < childrenCount; i++) {

		switch (childrens[i]->displayOrder) {

		case DisplayOrder::PARENT_LAST: {
			childrensDrawLast[lastIdxDrawLast] = childrens[i];
			lastIdxDrawLast++;
			continue;
		}

		case DisplayOrder::ABOVE_ALL: {
			childrensAboveAll.push_back(childrens[i]);
			continue;
		}

		}

		childrens[i]->draw();

	}

	for (int i = 0; i < lastIdxDrawLast; i++) {
		childrensDrawLast[i]->draw();
	}

}

void Control::addControl(Control* control) {

	control->id = childrenCount;
	childrenCount++;

	Control** ctrls = new Control * [childrenCount];

	int i;
	for (i = 0; i < childrenCount - 1; i++) {
		ctrls[i] = childrens[i];
	}

	control->parent = this;
	ctrls[i] = control;

	// delete[] childrens;
	delete[] childrens; // how this 'delete' works? hope its fine 
	childrens = ctrls;

	//delete[] childrensDrawLast;
	// free(childrensDrawLast);
	delete[] childrensDrawLast;
	childrensDrawLast = new Control * [childrenCount]; // error in relase some times
	/*
	childrensDrawLast = (Control**) malloc(childrenCount * sizeof(Control*));
	if (!childrenCount) {
		exit(2);
	}
	*/

}

void Control::pushControlFront(Control* control) {

	control->id = 0;
	childrenCount++;

	Control** ctrls = new Control * [childrenCount];

	int i;
	for (i = 1; i < childrenCount; i++) {
		ctrls[i] = childrens[i - 1];
		ctrls[i]->id++;
	}

	control->parent = this;
	ctrls[0] = control;

	delete[] childrens;
	childrens = ctrls;


	delete[] childrensDrawLast;
	childrensDrawLast = new Control * [childrenCount];

}

void Control::setX(int x) {

	this->x = (x >= 0) ? x : 0;

}

void Control::setX(double x) {

	if (x < 0) {
		x = 0;
	}
	else if (x > 1) {
		x = 1;
	}

	this->x = parent->x + parent->borderLeftWidth + parent->paddingLeft + (parent->width - parent->borderLeftWidth - parent->borderRightWidth - parent->paddingLeft - parent->paddingRight) * x;

}

void Control::setY(const int y) {

	// const int newY = (y >= 0) ? y : 0;

	const int deltaY = y - this->y;
	this->y = y;

	const int len = childrenCount;
	for (int i = 0; i < len; i++) {
		Control* const ctrl = childrens[i];
		ctrl->setY(ctrl->y + deltaY);
	}

}

void Control::setY(double y) {

	if (y < 0) {
		y = 0;
	}
	else if (y > 1) {
		y = 1;
	}

	this->y = parent->y + parent->borderTopWidth + parent->paddingTop + (parent->height - parent->borderTopWidth - parent->borderBottomWidth - parent->paddingTop - parent->paddingBottom) * y;

}

void Control::setWidth(int width) {

	this->width = (width >= 0) ? width : 0;

}

void Control::setWidth(double width) {

	if (width < 0) {
		width = 0;
	}
	else if (width > 1) {
		width = 1;
	}

	this->width = (parent->width - parent->borderLeftWidth - parent->borderRightWidth - parent->paddingLeft - parent->paddingRight) * width;

}

void Control::setHeight(int height) {

	this->height = (height >= 0) ? height : 0;

}

void Control::setHeight(double height) {

	if (height < 0) {
		height = 0;
	}
	else if (height > 1) {
		height = 1;
	}

	this->height = (parent->height - parent->borderTopWidth - parent->borderBottomWidth - parent->paddingTop - parent->paddingBottom) * height;

}

void Control::setPadding(int width) {

	paddingLeft = width;
	paddingRight = width;
	paddingTop = width;
	paddingBottom = width;

}

void Control::setBorderWidth(int borderWidth) {

	this->borderLeftWidth = borderWidth;
	this->borderRightWidth = borderWidth;
	this->borderTopWidth = borderWidth;
	this->borderBottomWidth = borderWidth;

}

void Control::setBorderBevelWidth(int width) {

	innerBorderBevelWidth = width;
	outerBorderBevelWidth = width;

}

void Control::setColor(int color) {

	this->color = color;
	this->actualColor = color;

}

void Control::setBackColor(int color) {

	this->backgroundColor = color;
	this->actualBackColor = color;

}

void Control::setColor(int color, int backColor) {

	this->color = color;
	this->actualColor = color;

	this->backgroundColor = backColor;
	this->actualBackColor = backColor;

}

void Control::setText(char* text, const int textLen) {

	this->text = (char*)malloc((textLen + 1) * sizeof(char));
	if (this->text == NULL) return;

	for (int i = 0; i < textLen; i++) {
		this->text[i] = text[i];
	}
	this->text[textLen] = '\0';
	textLength = textLen;

}

void Control::setText(wchar_t* text, const int textLen) {

	char* utf8Buffer = (char*)malloc(3 * textLen * sizeof(char));
	if (utf8Buffer == NULL) return;

	const int realLen = Utils::wc2utf8(text, textLen, &utf8Buffer);
	char* tmp = (char*)realloc(utf8Buffer, realLen * sizeof(char));
	if (tmp == NULL) {
		free(utf8Buffer);
	}

	this->text = tmp;
	textLength = textLen;

}

void Control::setBorderTitle(char* text, const int len) {

	this->borderTitleText = (char*)malloc((len + 1) * sizeof(char));
	if (this->borderTitleText == NULL) return;

	for (int i = 0; i < len; i++) {
		this->borderTitleText[i] = text[i];
	}
	this->borderTitleText[len] = '\0';
	borderTitleLength = len;

}

void Control::setCursor(int cursor) {

	switch (cursor) {


	}

}

void Control::select(int redraw) {

	selected = 1;
	actualColor = selectFrontColor;
	actualBackColor = selectBackColor;

	if (redraw)
		Render::redraw();

}

void Control::unselect(int redraw) {

	selected = 0;
	actualColor = color;
	actualBackColor = backgroundColor;

	if (redraw)
		Render::redraw();

}

void Control::hide(int redraw) {

	visible = 0;

	if (redraw)
		Render::redraw();

}

void Control::show(int redraw) {

	visible = 1;

	if (redraw)
		Render::redraw();

}

void Control::toggleSelect() {

	if (selected) unselect();
	else select();

}

int Control::processMessage(
	ControlEvent::ControlEvent controlEvent,
	CTRL_PARAM paramA,
	CTRL_PARAM paramB
) {

	if (!visible) return 0;//goto switchEnd;

	switch (controlEvent) {

	case ControlEvent::MOUSE_CLICK: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- unused

		if (eMouseClick != NULL) {
			if (!this->visible || !trackAnyClick && !isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) { break; }
			eMouseClick(this, paramA, paramB);
			if (!this->passMouseEvents) return PM_PREVENT_EVENT;
		}

		break;

	}

	case ControlEvent::MOUSE_SCROLL: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- scroll distance

		if (eMouseScroll != NULL) eMouseScroll(this, paramA, paramB);

		break;

	}

	case ControlEvent::MOUSE_MOVE: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- unused

		const int inBounds = isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA));
		if (inBounds) System::setCursor(cursor);
		mouseInBounds = inBounds;

		if (eMouseMove != NULL) eMouseMove(this, paramA, paramB);

		break;

	}

	case ControlEvent::MOUSE_DBL_CLICK: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- unused

		if (eMouseDblClick != NULL) {
			if (!this->visible || !isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) { break; }
			eMouseDblClick(this, paramA, paramB);
			if (!this->passMouseEvents) return PM_PREVENT_EVENT;
		}

		break;

	}

	case ControlEvent::MOUSE_UP: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- unused

		if (eMouseUp != NULL) eMouseUp(this, paramA, paramB);

		break;

	}

	case ControlEvent::MOUSE_DOWN: {
		// paramA 
		//	- hight order word is y coord
		//	- low order word is x coord
		// paramB
		//	- unused

		if (eMouseDown != NULL) eMouseDown(this, paramA, paramB);

		break;

	}

	case ControlEvent::DRAG_START: {

		if (draggable && eDragStart != NULL) eDragStart(this, paramA, paramB);

		break;

	}

	case ControlEvent::DRAG: {

		if (draggable && eDrag != NULL) eDrag(this, paramA, paramB);

		break;

	}

	case ControlEvent::DRAG_END: {

		if (draggable && eDragEnd != NULL) eDragEnd(this, paramA, paramB);

		break;

	}

	case ControlEvent::DROP: {

		if (droppable && eDrop != NULL) {
			if (!isInBounds(GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) { break; }
			eDrop(this, paramA, paramB);
		}

		break;

	}

	case ControlEvent::CHAR_INPUT: {

		if (focused) {
			eCharInput(this, paramA, paramB);
		}

		break;

	}

	default: {

		break;

	}

	}

	// have to come up with better name
switchEnd:

	for (int i = 0; i < childrenCount; i++) {

		if (childrens[i]->processMessage(controlEvent, paramA, paramB) == PM_PREVENT_EVENT) break;

	}

	return PM_OK;
	// window->pushFront(Control* ctrl)
	// 

}

void Control::getRelativeCoords(int* x, int* y) {

	*x = *x - this->x - this->borderLeftWidth;
	*y = *y - this->y - this->borderTopWidth;

}

void Control::getRelativeCoordsWithBorders(int* x, int* y) {

	*x = *x - this->x;
	*y = *y - this->y;

}

int Control::isInBounds(const int x, const int y) {

	return (this->x <= x && this->x + this->width >= x) &&
		(this->y <= y && this->y + this->height >= y);
}

int Control::getInBoxX() {

	return x + marginLeft + borderLeftWidth + paddingLeft;

}

int Control::getInBoxY() {

	return y + marginTop + borderTopWidth + paddingTop;

}

int Control::getInBoxWidth() {

	return width - marginLeft - marginRight - borderLeftWidth - borderRightWidth - paddingLeft - paddingRight;

}

int Control::getInBoxHeight() {

	return height - marginTop - marginBottom - borderTopWidth - borderBottomWidth - paddingTop - paddingBottom;

}

int Control::swapChildrens(const int idA, const int idB) {

	if (idA >= childrenCount || idB >= childrenCount || idA < 0 || idB < 0) return 1;

	Control* const tmp = childrens[idA];
	childrens[idA] = childrens[idB];
	childrens[idB] = tmp;

	childrens[idA]->id = idA;
	childrens[idB]->id = idB;

	return 0;

}

int Control::scrollY(const int value) {

	for (int i = 0; i < childrenCount; i++) {
		childrens[i]->y += value;
		childrens[i]->scrollY(value);
	}

	return 0;

}

int Control::scrollY(const int value, const int fromIdx) {

	for (int i = fromIdx; i < childrenCount; i++) {
		childrens[i]->y += value;
		childrens[i]->scrollY(value);
	}

	return 0;

}

void Control::focus() {

	if (focusedControl != NULL) {
		focusedControl->focused = 0;
	}

	this->focused = 1;
	focusedControl = this;

}

void deleteRecursive(Control* ctrl) {

	const int count = ctrl->childrenCount;
	for (int i = 0; i < count; i++) {
		deleteRecursive(ctrl->childrens[i]);
	}

	ctrl->parent->childrenCount--;
	delete ctrl;

}

Control::~Control() {

	/*
	const int count = childrenCount;
	for (int i = 0; i < count; i++) {

		Control* const ctrl = childrens[i];
		delete ctrl;

	}

	free(text);
	*/
	const int count = childrenCount;
	for (int i = 0; i < count; i++) {
		deleteRecursive(childrens[i]);
	}

	for (int i = this->id + 1; i < parent->childrenCount; i++) {
		parent->childrens[i]->id--;
		parent->childrens[i - 1] = parent->childrens[i];
	}

	parent->childrenCount--;
	//parent->childrens = NULL;

}
