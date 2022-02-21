#pragma once

#include "Control.h"
#include "Render.h"
#include "ImageDrawType.h"
#include <cstdint>

class Image : public Control {

private:

public:

	using Control::Control;

	virtual void draw();

	uint32_t* imageData;
	int imageWidth;
	int imageHeight;

	ImageDrawType::ImageDrawType drawType = ImageDrawType::RAW;

	void setImage(Render::Bitmap* bitmap);

};