#pragma once
#include "Control.h"

class ToolTip : public Control {

private:

public:

	ToolTip();

	virtual void draw();

	void computeInitialPosition();

	void fillBuffer(char* text);
	void setDesireCoords(int x, int y);

};
