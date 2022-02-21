#pragma once

#include "Window.h"
#include "Cursor.h"

Window::Window() : Control() {

	this->cursor = Cursor::DEFAULT;

}

void Window::draw() {

	if (!visible) return;

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

	for (Control* const& ctrl : childrensAboveAll) {
		ctrl->draw();
	}

	childrensAboveAll.clear();

}