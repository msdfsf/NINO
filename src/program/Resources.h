#pragma once

#include "Render.h"
#include <cstdint>

namespace Resources {

	extern Render::BitmapEx* const settingsIcon;
	extern Render::BitmapEx* const settingsIconInv;
	extern Render::BitmapEx* const presetsIcon;
	extern Render::BitmapEx* const presetsIconInv;
	extern Render::BitmapEx* const pluginsIcon;
	extern Render::BitmapEx* const pluginsIconInv;

	extern Render::BitmapEx* const checkMark;
	extern Render::BitmapEx* const checkSquare;
	
	extern Render::BitmapEx* const powerButtonOn;
	extern Render::BitmapEx* const powerButtonOff;
	
	extern Render::BitmapEx* const plus;
	extern Render::BitmapEx* const plusInv;
	extern Render::BitmapEx* const minus;
	extern Render::BitmapEx* const minusInv;

	extern Render::BitmapEx* const upTriangle;
	extern Render::BitmapEx* const upTriangleInv;
	extern Render::BitmapEx* const downTriangle;
	extern Render::BitmapEx* const downTriangleInv;

	extern Render::BitmapEx* const closeCross;

	extern Render::BitmapEx* const pluginKnob;
	extern Render::BitmapEx* const pluginInnerKnob;

	int load();

}