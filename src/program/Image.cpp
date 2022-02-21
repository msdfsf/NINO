#pragma once

#include "Image.h"
#include "Render.h"

void Image::draw() {
	
	// Control::draw();

	if (!visible) return;

	// decide if we need to render control
	const int bottomY = this->y + this->height;
	const int parentY = parent->y;
	const int parentHeight = parent->height;
	const int parentOverflowTop = parent->overflowTop;
	const int parentOverflowBottom = parent->overflowBottom;
	if (bottomY < parentY + parentOverflowTop || this->y > parentY + parentHeight - parentOverflowBottom) return;

	const int parentInBoxY = parent->getInBoxY();
	const int inBoxY = this->getInBoxY();
	const int inBoxHeight = getInBoxHeight();

	// top overflow
	int overflowY = 0;
	if (this->y < parentInBoxY + parentOverflowTop) {

		overflowY = parentInBoxY - this->y + parentOverflowTop;
		if (parentOverflowTop != 0) {
			overflowY -= (inBoxY - parentY);
			if (overflowY < 0) overflowY = 0;
		}
		this->overflowTop = overflowY;
	
	} else {
		this->overflowTop = 0;
	}

	// bottom overflow handling
	const int parentInboxHeight = parent->getInBoxHeight();
	const int parentBottomY = parentInBoxY + parent->getInBoxHeight() - parentOverflowBottom;

	int bottomCutHeight = 0;
	int bottomCutY = 0;
	if (bottomY > parentBottomY) {

		bottomCutHeight = bottomY - parentBottomY;
		bottomCutY = bottomY - bottomCutHeight;
		if (parentOverflowBottom != 0) {
			bottomCutHeight += ((inBoxY + inBoxHeight) - (parentY + parentHeight));
			if (bottomCutHeight < 0) bottomCutHeight = 0;
		}

		this->overflowBottom = bottomCutHeight;

	} else {
		this->overflowBottom = 0;
	}

	const int x = this->getInBoxX();
	const int y = this->getInBoxY() + overflowY;
	const int width = this->getInBoxWidth();
	const int height = inBoxHeight - overflowY - bottomCutHeight;

	const int imageBottomCut = bottomCutHeight * (this->imageHeight / (double)inBoxHeight);
	const int imageWidth = this->imageWidth;
	const int imageHeight = this->imageHeight - (overflowY) * (this->imageHeight / (double) inBoxHeight) - imageBottomCut;

	uint32_t* const imageData = this->imageData + imageBottomCut * imageWidth;

	switch (drawType) {

		case ImageDrawType::RAW:

			Render::drawBitmap(
				imageData,
				imageWidth,
				imageHeight,
				x,
				y,
				width,
				height
			);
			break;

		case ImageDrawType::ONE_COLOR:

			Render::drawBitmap(
				imageData,
				imageWidth,
				imageHeight,
				x,
				y,
				width,
				height,
				(selected) ? backgroundColor : color
			);
			break;
	
	}

}

void Image::setImage(Render::Bitmap* bitmap) {

	this->imageData = bitmap->pixels;
	this->imageWidth = bitmap->width;
	this->imageHeight = bitmap->height;

}