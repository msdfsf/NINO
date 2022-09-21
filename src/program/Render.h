#pragma once

#include <windows.h>
#include <cstdint>

#include "StringAlignment.h"

namespace Render {

	typedef struct Bitmap {

		int width;
		int height;

		uint32_t* pixels;

	} Bitmap;

	typedef struct BitmapEx {

		int width;
		int height;

		uint32_t* pixels;

		double angle;

	} BitmapEx;
	
	namespace CornerType {
		enum CornerType {

			TOP_LEFT,
			TOP_RIGHT,
			BOTTOM_LEFT,
			BOTTOM_RIGHT

		};
	}

	extern int renderWidth;
	extern int renderHeight;

	extern int32_t* pixels;

	extern const int32_t DEFAULT_COLOR;
	extern int32_t color;

	int init(HWND hWnd, int width, int height);

	void render(HDC hdc);
	void render(HDC hdc, int width, int height);

	void redraw();
	void redraw(HDC hdc);

	void drawLineX(int x, int y, int len);
	void drawLineY(int x, int y, int len);
	void drawLine(int x1, int y1, int x2, int y2);

	void fillCircle(int x, int y, int radius);
	void fillCircle(int x, int y, int radius, int maxY, int minY);

	void drawRect(int x, int y, int wd, int hg);
	void drawRect(int x, int y, int width, int height, int borderSize);
	void drawRect(int x, int y, int width, int height, int leftBorder, int rightBorder, int topBorder, int bottomBorder);
	void drawRect(int x, int y, int width, int height, int borderSize, int borderRadius);
	
	void drawRightTriangle(int x, int y, int width, int height, CornerType::CornerType type);

	void fillRect(int x, int y, int wd, int hg);
	void fillRectChecker(int x, int y, int wd, int hg, int secondColor);
	void fillRectDots(int x, int y, int wd, int hg, int secondColor);
	
	void drawString(
		char* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight
	);

	void drawString(
		char* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int backColor,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight
	);

	void drawString(
		wchar_t* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int backColor,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight
	);

	void drawString(
		char* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight,
		int cutX,
		int cutY,
		int cutWidth,
		int cutHeight
	);

	void drawString(
		char* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int backColor,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight,
		int cutX,
		int cutY,
		int cutWidth,
		int cutHeight
	);

	void drawBitmap(
		uint32_t* bitmap, 
		const int bitmapWidth, 
		const int bitmapHeight, 
		const int x, 
		const int y
	);

	void drawBitmap(
		uint32_t* bitmap,
		const int bitmapWidth,
		const int bitmapHeight,
		const int x,
		const int y,
		const int width,
		const int height
	);

	void drawBitmap(
		uint32_t* bitmap,
		const int bitmapWidth,
		const int bitmapHeight,
		const int x,
		const int y,
		const int width,
		const int height,
		const double angle
	);

	void drawBitmap(
		uint32_t* bitmap,
		int bitmapWidth,
		int bitmapHeight,
		int x,
		int y,
		int width,
		int height,
		double angle,
		int minCutX,
		int minCutY,
		int maxCutX,
		int maxCutY
	);

	void drawBitmap(
		uint32_t* bitmap,
		const int bitmapWidth,
		const int bitmapHeight,
		const int x,
		const int y,
		const int width,
		const int height,
		const int color
	);

	uint32_t alphaBlendPixels(uint32_t pxlDst, uint32_t pxlSrc);

	void clear();

}
