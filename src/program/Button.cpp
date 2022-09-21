#pragma once

#include "Button.h"
#include "Color.h"
#include "Cursor.h"

Button::Button() : Control() {
	
	this->selectFrontColor = Color::BLACK;
	this->selectBackColor = Color::WHITE;
	
	this->cursor = Cursor::POINTER;

}
