#pragma once

#include "Control.h"
#include "ResizeType.h"

class Window : public Control {

	private:

	public:

		ResizeType::ResizeType resizeType;

		using Control::Control;

		Window();

		virtual void draw();

		void addEventCallback(int eventType, void (*fcn) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB), Control* source);
};
