#pragma once

#include "Control.h"

class Window : public Control {

	private:

	public:

		using Control::Control;

		Window();

		virtual void draw();

};