#pragma once

#include "Checkbox.h"
#include "Resources.h"
#include "Render.h"
#include "Color.h"
#include "Cursor.h"

void mouseClickCallback(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

Checkbox::Checkbox() : Control() {

	setBorderWidth(2);
	borderColor = Color::WHITE;

	setCheckType(CheckType::CHECK);

	cursor = Cursor::POINTER;

	checked = 0;
	eMouseClick = &mouseClickCallback;


};

void mouseClickCallback(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Checkbox* const src = (Checkbox*) source;
	src->checked = !src->checked;

	Render::redraw();
	if (src->eChange) src->eChange(src, src->checked, NULL);

}

void Checkbox::draw() {
	
	this->width = this->height;

	Control::draw();

	if (!checked || !visible) return;

	const int x = this->x + this->borderLeftWidth;
	const int y = this->y + this->borderTopWidth;

	const int width = this->width - this->borderLeftWidth - this->borderRightWidth;
	const int height = this->height - this->borderBottomWidth - this->borderTopWidth;

	Render::drawBitmap(
		checkImg->pixels,
		checkImg->width,
		checkImg->height,
		x,
		y,
		width,
		height
	);

}

void Checkbox::setCheckType(CheckType type) {

	checkType = type;

	switch (type) {
		
		case CHECK: {
			checkImg = Resources::checkMark;
			break;
		}
		
		case SQUARE: {
			checkImg = Resources::checkSquare;
			break;
		}

		default: {
			checkImg = Resources::checkMark;
			break;
		}
	}

}

