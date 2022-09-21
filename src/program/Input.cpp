#pragma once

#include "Input.h"
#include "Render.h"
#include "Key.h";
#include "TickEventsDriver.h";
#include "Color.h"

void charInput(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void mouseClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
int timerCallback(void* ctrl);

Input::Input():Control() {

	text = buffer;
	textLength = 0;

	textCursor = 0;
	textCursorColor = Color::DEEP_SKY_BLUE_3;
	textCursorWidth = 4;

	eMouseClick = &mouseClick;
	eCharInput = &charInput;

	TickEventsDriver::Node* node = TickEventsDriver::add(this, &timerCallback, 10);
	node->type = TickEventsDriver::TET_RENDER;

}

void Input::draw() {

	Control::draw();
	if (focused) drawCursor();

}

void Input::input(char ch) {

	if (this->textLength + 1 >= BUFFER_SIZE) return;
	
	buffer[this->textLength] = ch;
	this->textLength++;

}

void Input::erase() {

	if (this->textLength <= 0) return;
	textLength--;

}

void Input::eraseAll() {

	textLength = 0;

}

void charInput(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Input* input = (Input*) source;

	const wchar_t ch = (wchar_t) paramA;
	if (ch == Key::BACKSPACE) {
		input->erase();
	} else {
		input->input(ch);
	}
	
	Render::redraw();

}

void mouseClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	source->focus();

}

int timerCallback(void* ctrl) {

	Input* const inp = (Input*) (ctrl);
	inp->textCursor = !inp->textCursor;

	return 0;

}

void Input::drawCursor() {

	const int realLen = this->fontSize * this->textLength;
	const int boxWidth = this->getInBoxWidth() - this->textCursorWidth;
	const int len = (realLen > boxWidth) ? this->fontSize * boxWidth / this->fontSize : realLen;

	const int x = this->getInBoxX() + len;
	const int y = this->getInBoxY();
	const int wd = this->textCursorWidth;
	const int hg = this->getInBoxHeight();

	Render::color = (this->textCursor) ? this->textCursorColor : this->actualBackColor;
	Render::fillRect(x, y, wd, hg);

}
