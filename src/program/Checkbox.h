#pragma once

#include "Control.h"
#include "Render.h"

class Checkbox : public Control {

	public:

		enum CheckType {
			CHECK,
			SQUARE
		};

		int checked;
		CheckType checkType;

		Render::BitmapEx* checkImg;

		using Control::Control;

		Checkbox();

		virtual void draw();

		void setCheckType(CheckType type);

	private:

};