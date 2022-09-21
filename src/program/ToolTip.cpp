#pragma once

#include "ToolTip.h"
#include "Render.h"
#include "Color.h"
#include "Main.h"

const int BUFFER_LENGTH = 32;

ToolTip::ToolTip() {

	visible = 0;
	fontSize = 10;
	displayOrder = DisplayOrder::ABOVE_ALL;
	
	window->addControl(this);

	setPadding(4);
	setBorderWidth(1);
	setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
	borderColor = MAIN_BACK_COLOR;

	text = (char*) malloc(sizeof(char) * (BUFFER_LENGTH + 1));
	if (text == NULL) return;
	text[0] = '\0';

	textLength = BUFFER_LENGTH;

}

void ToolTip::fillBuffer(char* text) {

	char ch = text[0];
	int i = 0;
	while (ch != '\0' && i < BUFFER_LENGTH) {
		
		this->text[i] = ch;

		i++;
		ch = text[i];
	
	}
	this->text[i] = '\0';
	this->textLength = i;

}

void ToolTip::setDesireCoords(int x, int y) {

	this->x = x - paddingLeft;
	this->y = y - paddingTop;

}

void ToolTip::draw() {

	if (!visible) return;

	width = fontSize * textLength + paddingLeft + paddingRight + borderLeftWidth + borderRightWidth;
	height = fontSize + paddingBottom + paddingTop + borderTopWidth + borderBottomWidth;

	Render::color = color;
	Render::fillRect(x + 4, y + 4, width, height);

	Render::color = backgroundColor;
	Render::fillRect(x, y, width, height);

	Render::color = borderColor;
	Render::drawRect(
		x, y, width, height,
		borderLeftWidth, borderRightWidth,
		borderTopWidth, borderBottomWidth
	);

	Render::color = color;
	Render::drawString(
		text, 
		textLength, 
		StringAlignment::CENTER, 
		fontSize, 
		x, 
		y, 
		width, 
		height
	);

}
