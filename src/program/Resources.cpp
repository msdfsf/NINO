#pragma once

#include "Resources.h"
#include "FileDriver.h"

namespace Resources {

#define RESOURCES_FOLDER_NAME "./Resources/"
#define BITMAP_FOLDER_NAME RESOURCES_FOLDER_NAME "Bitmaps/"

#define defBitmap(varname, flname) \
	Render::BitmapEx* const varname = new Render::BitmapEx{\
		NULL,\
		0,\
		0,\
		0,\
	};\
	const char varname##Name[] = BITMAP_FOLDER_NAME flname;

#define ldBitmap(varname) \
	if (FileDriver::loadBitmap(\
		(char*)varname##Name,\
		&(varname->pixels),\
		&(varname->width),\
		&(varname->height)\
	) != 0) goto exit;

	defBitmap(settingsIcon, "Settings.bmp");
	defBitmap(presetsIcon, "Presets.bmp");
	defBitmap(pluginsIcon, "Plugins.bmp");
	defBitmap(settingsIconInv, "SettingsInv.bmp");
	defBitmap(presetsIconInv, "PresetsInv.bmp");
	defBitmap(pluginsIconInv, "PluginsInv.bmp");

	defBitmap(checkMark, "CheckMark.bmp");
	defBitmap(checkSquare, "CheckSquare.bmp");
	
	defBitmap(powerButtonOn, "PowerButtonOn.bmp");
	defBitmap(powerButtonOff, "PowerButtonOff.bmp");
	
	defBitmap(plus, "Plus.bmp");
	defBitmap(plusInv, "PlusInv.bmp");
	defBitmap(minus, "Minus.bmp");
	defBitmap(minusInv, "MinusInv.bmp");

	defBitmap(upTriangle, "UpTriangle.bmp");
	defBitmap(upTriangleInv, "UpTriangleInv.bmp");
	defBitmap(downTriangle, "DownTriangle.bmp");
	defBitmap(downTriangleInv, "DownTriangleInv.bmp");

	defBitmap(closeCross, "CloseCross.bmp");

	defBitmap(pluginKnob, "PluginKnob.bmp");
	defBitmap(pluginInnerKnob, "PluginInnerKnob.bmp");

	int load() {

		int err = 0;

		ldBitmap(settingsIcon);
		ldBitmap(presetsIcon);
		ldBitmap(pluginsIcon);
		ldBitmap(settingsIconInv);
		ldBitmap(presetsIconInv);
		ldBitmap(pluginsIconInv);

		ldBitmap(checkMark);
		ldBitmap(checkSquare);

		ldBitmap(powerButtonOn);
		ldBitmap(powerButtonOff);
		
		ldBitmap(plus);
		ldBitmap(plusInv);
		ldBitmap(minus);
		ldBitmap(minusInv);
		
		ldBitmap(upTriangle);
		ldBitmap(upTriangleInv);
		ldBitmap(downTriangle);
		ldBitmap(downTriangleInv);

		ldBitmap(closeCross);

		ldBitmap(pluginKnob);
		ldBitmap(pluginInnerKnob);

	exit:
		return err;

	}
}