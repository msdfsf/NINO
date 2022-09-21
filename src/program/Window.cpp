#pragma once

#include <vector>

#include "Window.h"
#include "Cursor.h"
#include "CallbackEvent.h"

typedef struct Callback {

	Control* target;
	void (*fcn) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);


} Callback;

std::vector<Callback*> callbacks;

void Window::addEventCallback(int eventType, void (*fcn) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB), Control* source) {
	
	switch (eventType) {

		case CallbackEvent::MOUSE_CLICK: {

			Callback* cb = (Callback*)malloc(sizeof(Callback));
			if (!cb) return;

			cb->target = (source) ? source : Control::window;
			cb->fcn = fcn;
			callbacks.push_back(cb);
			
			break;
		
		}
	
	}

}

void windowMouseClickCallback(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	for (int i = 0; i < callbacks.size(); i++) {
		const Callback cb = *callbacks[i];
		cb.fcn(cb.target, paramA, paramB);
	}

}

Window::Window() : Control() {

	this->eMouseClick = &windowMouseClickCallback;

	this->resizeType = ResizeType::SCALE;
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
