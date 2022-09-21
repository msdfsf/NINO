#pragma once

#include "Render.h"
#include "Color.h"
#include "StringAlignment.h"

#include "font.h"

#pragma comment(lib, "Msimg32.lib")
// #include "Config.h"

#include <cstdint>
#include <cstdio>
#include <math.h> 

namespace Render {

	HWND hWnd;

	static const uint32_t ALPHA_MASK	= 0xFF000000;
	static const uint32_t RED_MASK		= 0x00FF0000;
	static const uint32_t BLUE_MASK		= 0x000000FF;
	static const uint32_t GREEN_MASK	= 0x0000FF00;
	static const uint32_t RED_BLUE_MASK = 0x00FF00FF;

	static const unsigned int RED_SHIFT		= 2 * 8;
	static const unsigned int GREEN_SHIFT	= 1 * 8;
	static const unsigned int BLUE_SHIFT	= 0;

	int renderWidth;
	int renderHeight;

	const int32_t DEFAULT_COLOR = Color::WHITE;
	int32_t color = DEFAULT_COLOR;

	int32_t* pixels;

	int renderAlloc(int width, int height);

	int loadBitmap(char* fileName);

	uint32_t alphaBlendPixels(uint32_t pxlDst, uint32_t pxlSrc);

	BITMAPINFOHEADER BITMAP_INFO_HEADER = {

		sizeof(BITMAPINFO::bmiHeader),
		0,
		0,
		1,
		32,
		BI_RGB

	};

	BITMAPINFO BITMAP_INFO = {

		BITMAP_INFO_HEADER

	};

	int init(HWND hWnd, int width, int height) {

		if (renderAlloc(width, height)) {
			return 1;
		}

		Render::hWnd = hWnd;

		clear();

		return 0;

	}

	int renderAlloc(int width, int height) {

		pixels = (int32_t*)VirtualAlloc(
			0,
			width * height * sizeof(int32_t),
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		);
		if (pixels == NULL) return 1;

		BITMAP_INFO.bmiHeader.biWidth = width;
		BITMAP_INFO.bmiHeader.biHeight = -height;

		renderWidth = width;
		renderHeight = height;

		return 0;

	}

	void clear() {

		for (int i = 0; i < renderHeight; i++) {

			int offset = i * renderWidth;
			for (int j = 0; j < renderWidth; j++) {
				pixels[offset + j] = DEFAULT_COLOR;
			}

		}

	}

	void render(HDC hdc) {

		StretchDIBits(
			hdc,
			0,
			0,
			renderWidth,
			renderHeight,
			0,
			0,
			renderWidth,
			renderHeight,
			pixels,
			&BITMAP_INFO,
			DIB_RGB_COLORS,
			SRCCOPY
		);

	}

	void render(HDC hdc, int width, int height) {

		int x = width;
		int y = height;

		if (renderHeight * width > renderWidth * height) {

			width = (renderWidth * height) / renderHeight;
			x = (x - width) / 2;
			y = 0;

			BitBlt(hdc, 0, 0, x, height, 0, 0, 0, BLACKNESS);
			BitBlt(hdc, x + width, 0, x, height, 0, 0, 0, BLACKNESS);

		} else {

			height = (renderHeight * width) / renderWidth;
			y = (y - height) / 2;
			x = 0;

			BitBlt(hdc, 0, 0, width, y, 0, 0, 0, BLACKNESS);
			BitBlt(hdc, 0, y + height, width, y, 0, 0, 0, BLACKNESS);
		
		}

		StretchDIBits(
			hdc,
			x,
			y,
			width,
			height,
			0,
			0,
			renderWidth,
			renderHeight,
			pixels,
			&BITMAP_INFO,
			DIB_RGB_COLORS,
			SRCCOPY
		);

	}

	void redraw() {

		RedrawWindow(
			hWnd,
			NULL,
			NULL,
			RDW_INVALIDATE | RDW_UPDATENOW
		);
	
	}

	void redraw(HWND hwnd) {


	}

	// what about < 0 check, huh?
	void drawLineX(int x, int y, int len) {
	
		const int maxX = (x + len < renderWidth) ? x + len : renderWidth - x;
		const int offset = y* renderWidth;
		for (int i = x; i < maxX; i++) {
			pixels[offset + i] = color;
		}

	}

	// what about < 0 check, huh?
	void drawLineY(int x, int y, int len) {
	
		const int maxY = (y + len < renderHeight) ? y + len : renderHeight - y;
		const int offset = x;
		for (int i = y; i < maxY; i++) {
			pixels[i * renderWidth + offset] = color;
		}

	}

	// optimize?
	void drawLine(int x1, int y1, int x2, int y2) {

		if (x1 >= renderWidth && x2 >= renderWidth) return;
		if (y1 >= renderHeight && y2 >= renderHeight) return;
		if (y1 < 0 && y2 < 0) return;

		if (x1 >= renderWidth) x1 = renderWidth - 1;
		if (x2 >= renderWidth) x2 = renderWidth - 1;

		if (x1 < 0) x1 = 0;
		if (x2 < 0) x2 = 0;

		if (y1 >= renderHeight) y1 = renderHeight - 1;
		if (y2 >= renderHeight) y2 = renderHeight - 1;

		if (y1 < 0) y1 = 0;
		if (y2 < 0) y2 = 0;

		int yDir;
		int xDir;
		int width;
		int height;

		if (x1 > x2) {
			xDir = -1;
			width = x1 - x2 + 1;
		} else {
			xDir = 1;
			width = x2 - x1 + 1;
		} if (y1 > y2) {
			yDir = -1;
			height = y1 - y2 + 1;
		} else {
			yDir = 1;
			height = y2 - y1 + 1;
		}

		int stepWidth = renderWidth;
		int stepHeight = 1;
		int screenWidth = renderWidth;
		int screenHeight = renderHeight;
		double step = width / (double) height;
		if (step < 1) {
			stepWidth = 1;
			stepHeight = renderWidth;
			screenWidth = renderHeight;
			screenHeight = renderWidth;

			int tmp = x1;
			x1 = y1;
			y1 = tmp;
			y2 = x2;

			tmp = xDir;
			xDir = yDir;
			yDir = tmp;

			step = 1 / step;
		}

		int rStep = (int) round(step);
		int offsetX = x1;
		double overstep = 0;

		int y = y1;
		while (y != y2 + yDir) {
			int offsetY = y * stepWidth;
			int x = offsetX;
			while (x != offsetX + xDir * rStep) {
				pixels[offsetY + x * stepHeight] = color;
				x = x + xDir;
			}
			offsetX = offsetX + xDir * rStep;
			overstep += rStep - step;
			rStep = (int) round(step - overstep);
			if (offsetX + rStep > screenWidth) return;
			y = y + yDir;
		}

	}

	// x and y are coords of top left corner
	void fillCircle(int x, int y, int radius) {
		
		x = x + radius;
		y = y + radius;

		const int d = 2 * radius;
		const int r2 = radius * radius;

		for (int i = -radius; i < radius; i++) {

			const int offset = (y + i) * renderWidth;
			for (int j = -radius; j < radius; j++) {
				
				if (i * i + j * j <= radius * radius) {
					pixels[offset + x + j] = color;
				}			
			
			}
		
		}
	
	}

	void fillCircle(int x, int y, int radius, int yTopOffset, int yBottomOffset) {

		const int centerX = x + radius;
		const int centerY = y + radius;

		const int d = 2 * radius;
		const int r2 = radius * radius;

		for (int i = -radius + yTopOffset; i < radius - yBottomOffset; i++) {

			const int offset = (centerY + i) * renderWidth;
			for (int j = -radius; j < radius; j++) {

				if (i * i + j * j <= radius * radius) {
					pixels[offset + centerX + j] = color;
				}

			}

		}

	}

	void drawRect(int x, int y, int width, int height, int borderSize) {

		fillRect(x, y, width, borderSize);
		fillRect(x, y, borderSize, height);
		fillRect(x + width - borderSize, y, borderSize, height);
		fillRect(x, y + height - borderSize, width, borderSize);

	}

	void drawRect(int x, int y, int width, int height, int leftBorder, int rightBorder, int topBorder, int bottomBorder) {

		fillRect(x, y, width, topBorder);
		fillRect(x, y, leftBorder, height);
		fillRect(x + width - rightBorder, y, rightBorder, height);
		fillRect(x, y + height - bottomBorder, width, bottomBorder);

	}

	void drawRect(int x, int y, int width, int height, int borderSize, int borderRadius) {

		int outerRadius = borderRadius;
		int innerRadius = borderRadius - borderSize;

		fillRect(x + borderRadius, y, width - 2 * borderRadius, borderSize);
		//fillRect(x, y + borderSize, borderSize, height - 2 * borderSize);
		//fillRect(x + width - borderSize, y + borderSize, borderSize, height - 2 * borderSize);
		//fillRect(x + borderSize, y + height - borderSize, width - 2 * borderSize, borderSize);

	}

	void drawRightTriangle(int x, int y, int width, int height, CornerType::CornerType type) {
		
		switch (type) {
			
			case CornerType::TOP_LEFT: {

				const int maxX = x + width;
				const int maxY = y;
				const int startY = y + height - 1;
				for (int j = startY; j >= maxY; j--) {
					const int offset = j * renderWidth;
					for (int i = x + (startY - j); i < maxX; i++) {
						pixels[offset + i] = color;
					}
				}

				break;
			}

			case CornerType::TOP_RIGHT: {

				const int maxX = x;
				const int maxY = y;
				const int startY = y + height - 1;
				for (int j = startY; j >= maxY; j--) {
					const int offset = j * renderWidth;
					for (int i = x + width - (startY - j) - 1; i >= maxX; i--) {
						pixels[offset + i] = color;
					}
				}

				break;
			}

			case CornerType::BOTTOM_LEFT: {

				const int maxX = x + width;
				const int maxY = y + height;
				for (int j = y; j < maxY; j++) {
					const int offset = j * renderWidth;
					for (int i = x + j - y; i < maxX; i++) {
						pixels[offset + i] = color;
					}
				}

				break;
			}

			case CornerType::BOTTOM_RIGHT: {

				const int maxX = x;
				const int maxY = y + height;
				for (int j = y; j < maxY; j++) {
					const int offset = j * renderWidth;
					for (int i = x + width - (j - y) - 1; i >= maxX; i--) {
						pixels[offset + i] = color;
					}
				}

				break;
			}
		
		}
	
	
	}

	void fillQuarterCircle() {

	}

#define RECT_FILL_CHECK \
	if (x < 0) {\
		wd += x;\
		x = 0;\
	} else if ((x) >= renderWidth) {\
		return;\
	}\
	\
	if (y < 0) {\
		hg += y;\
		y = 0;\
	} else if (y >= renderHeight) {\
		return;\
	}\
	\
	if (x + wd >= renderWidth) wd = renderWidth;\
	else wd += x;\
	\
	if (y + hg >= renderHeight) hg = renderHeight;\
	else hg += y

	void fillRect(int x, int y, int wd, int hg) {

		RECT_FILL_CHECK;

		for (y; y < hg; y++) {

			const int offset = y * renderWidth;
			for (int i = x; i < wd; i++) {
				pixels[offset + i] = color;
			}

		}

	}

	void fillRectChecker(int x, int y, int wd, int hg, int secondColor) {
	
		RECT_FILL_CHECK;

		for (y; y < hg; y++) {

			const int offset = y * renderWidth;
			
			int i = x;
			for (; i < wd - 1; i += 2) {
				pixels[offset + i] = color;
				pixels[offset + i + 1] = secondColor;
			}
			if (i < wd) pixels[offset + i] = color;

			int tmp = secondColor;
			secondColor = color;
			color = tmp;

		}
	
	}

	void fillRectDots(int x, int y, int wd, int hg, int secondColor) {

		RECT_FILL_CHECK;
		
		int lastState = 0;
		int state = 0;
		for (y; y < hg; y++) {

			const int offset = y * renderWidth;

			if (state == 0) {

				int i = x;
				for (; i < wd - 3; i += 4) {
					pixels[offset + i] = color;
					pixels[offset + i + 1] = secondColor;
					pixels[offset + i + 2] = secondColor;
					pixels[offset + i + 3] = secondColor;
				}

				if (i < wd - 2) {
					pixels[offset + i] = color;
					pixels[offset + i + 1] = secondColor;
					pixels[offset + i + 2] = secondColor;
				}
				else if (i < wd - 1) {
					pixels[offset + i] = color;
					pixels[offset + i + 1] = secondColor;
				}
				else if (i < wd) {
					pixels[offset + i] = color;
				}
			
				lastState = 0;
				state = 1;

			} else if (state == 1) {
				
				for (int i = x; i < wd; i++) {
					pixels[offset + i] = secondColor;
				}

				state = (lastState == 2) ? 0 : 2;
				lastState = 1;

			} else {
				
				int i = x;
				for (; i < wd - 3; i += 4) {
					pixels[offset + i] = secondColor;
					pixels[offset + i + 1] = secondColor;
					pixels[offset + i + 2] = color;
					pixels[offset + i + 3] = secondColor;
				}

				if (i < wd - 2) {
					pixels[offset + i] = secondColor;
					pixels[offset + i + 1] = secondColor;
					pixels[offset + i + 2] = color;
				}
				else if (i < wd - 1) {
					pixels[offset + i] = secondColor;
					pixels[offset + i + 1] = secondColor;
				}
				else if (i < wd) {
					pixels[offset + i] = color;
				}

				lastState = 2;
				state = 1;

			}

		}

	}

	void drawString(
		char* string,
		int length,
		StringAlignment::StringAlignment alignment,
		int size,
		int boundsX,
		int boundsY,
		int boundsWidth,
		int boundsHeight
	) {

		int dotedEnd = 0;
		if (size * length > boundsWidth) {

			length = boundsWidth / size - 2;

			if (length > -1) {
				dotedEnd = 1;
			}

		}

		int width = renderWidth;
		int height = renderHeight;

		int top;
		int left;

		switch (alignment) {

		case StringAlignment::CENTER: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX + (boundsWidth - length * size) / 2;
			break;

		}

		case StringAlignment::LEFT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX;

			break;

		}

		case StringAlignment::RIGHT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsWidth + boundsX - length * size;
			break;

		}

		default: {

			top = boundsY;
			left = boundsX;

		}

		}

	hereWeGoAgain:
		for (int i = 0; i < length; i++) {

			int ch = string[i];

			int x = 0;
			int y = top;
			int offset = CHAR_LENGTH * (ch - CHAR_OFFSET);
			for (int j = offset; j < offset + CHAR_LENGTH; j++) {

				if (x >= CHAR_SIZE) {
					x = 0;
					y--;
				}

				int xx = (x * size) / CHAR_SIZE;
				int yy = top + ((top - y) * size) / CHAR_SIZE;

				if (BMP_FONT[j]) {
					pixels[width * yy + left + xx] = color;
				}

				x++;

			}

			left += size;

		}

		if (dotedEnd) {

			string = (char*)"..";
			dotedEnd = 0;
			length = 2;

			goto hereWeGoAgain;

		}

	}

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
	) {

		int dotedEnd = 0;
		if (size * length > boundsWidth) {

			length = boundsWidth / size - 2;

			if (length > -1) {
				dotedEnd = 1;
			}

		}

		int width = renderWidth;
		int height = renderHeight;

		int top;
		int left;

		switch (alignment) {

		case StringAlignment::CENTER: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX + (boundsWidth - length * size) / 2;
			break;

		}

		case StringAlignment::LEFT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX;

			break;

		}

		case StringAlignment::RIGHT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsWidth + boundsX - length * size;
			break;

		}

		default: {

			top = boundsY;
			left = boundsX;

		}

		}

		hereWeGoAgain:
		for (int i = 0; i < length; i++) {

			int ch = string[i];

			int x = 0;
			int y = top;
			int offset = CHAR_LENGTH * (ch - CHAR_OFFSET);
			for (int j = offset; j < offset + CHAR_LENGTH; j++) {

				if (x >= CHAR_SIZE) {
					x = 0;
					y--;
				}

				int xx = (x * size) / CHAR_SIZE;
				int yy = top + ((top - y) * size) / CHAR_SIZE;
				pixels[width * yy + left + xx] = (BMP_FONT[j]) ? color : backColor;

				x++;

			}

			left += size;

		}

		if (dotedEnd) {

			string = (char*) "..";
			dotedEnd = 0;
			length = 2;
			
			goto hereWeGoAgain;
		
		}

	}

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
	) {

		int dotedEnd = 0;
		if (size * length > boundsWidth) {

			length = boundsWidth / size - 2;

			if (length > -1) {
				dotedEnd = 1;
			}

		}

		int width = renderWidth;
		int height = renderHeight;

		int top;
		int left;

		switch (alignment) {

		case StringAlignment::CENTER: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX + (boundsWidth - length * size) / 2;
			break;

		}

		case StringAlignment::LEFT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX;

			break;

		}

		case StringAlignment::RIGHT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsWidth + boundsX - length * size;
			break;

		}

		default: {

			top = boundsY;
			left = boundsX;

		}

		}

	hereWeGoAgain:
		for (int i = 0; i < length; i++) {

			int ch = string[i];

			int x = 0;
			int y = top;
			int offset = CHAR_LENGTH * (ch - CHAR_OFFSET);
			for (int j = offset; j < offset + CHAR_LENGTH; j++) {

				if (x >= CHAR_SIZE) {
					x = 0;
					y--;
				}

				int xx = (x * size) / CHAR_SIZE;
				int yy = top + ((top - y) * size) / CHAR_SIZE;
				pixels[width * yy + left + xx] = (BMP_FONT[j]) ? color : backColor;

				x++;

			}

			left += size;

		}

		if (dotedEnd) {

			string = (wchar_t*) L"..";
			dotedEnd = 0;
			length = 2;

			goto hereWeGoAgain;

		}

	}

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
	) {

		int dotedEnd = 0;
		if (size * length > cutWidth) {

			length = cutWidth / size - 2;

			if (length > -1) {
				dotedEnd = 1;
			}

		}

		int width = renderWidth;
		int height = renderHeight;

		int top;
		int left;

		switch (alignment) {

		case StringAlignment::CENTER: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX + (boundsWidth - length * size) / 2;
			break;

		}

		case StringAlignment::LEFT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsX;

			break;

		}

		case StringAlignment::RIGHT: {

			top = boundsY + (boundsHeight - size) / 2;
			left = boundsWidth + boundsX - length * size;
			break;

		}

		default: {

			top = boundsY;
			left = boundsX;

		}

		}

		int cutOffsetBottom = cutY + cutHeight - (top - boundsY);
		if (cutOffsetBottom < 0) cutOffsetBottom = CHAR_SIZE;
		else if (cutOffsetBottom > size) cutOffsetBottom = 0;
		else cutOffsetBottom = CHAR_SIZE - (CHAR_SIZE * cutOffsetBottom) / size; // n / 15


		int cutOffsetY = cutY - (top - boundsY);
		if (cutOffsetY < 0) cutOffsetY = 0;
		top += cutOffsetY;
		cutOffsetY = (CHAR_SIZE * cutOffsetY) / size;

	hereWeGoAgain:
		for (int i = 0; i < length; i++) {

			int ch = string[i];

			int x = 0;
			int y = top;
			int offset = CHAR_LENGTH * (ch - CHAR_OFFSET);

			const int from = offset + cutOffsetY * CHAR_SIZE;
			const int till = offset + CHAR_LENGTH - cutOffsetBottom * CHAR_SIZE;
			for (int j = from; j < till; j++) {

				if (x >= CHAR_SIZE) {
					x = 0;
					y--;
				}

				int xx = (x * size) / CHAR_SIZE;
				int yy = top + ((top - y) * size) / CHAR_SIZE;

				if (BMP_FONT[j]) {
					pixels[width * yy + left + xx] = color;
				}

				x++;

			}

			left += size;

		}

		if (dotedEnd) {

			string = (char*) "..";
			dotedEnd = 0;
			length = 2;

			goto hereWeGoAgain;

		}

	}

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
	) {

		int dotedEnd = 0;
		if (size * length > cutWidth) {

			length = cutWidth / size - 2;

			if (length > -1) {
				dotedEnd = 1;
			}

		}

		int width = renderWidth;
		int height = renderHeight;

		int top;
		int left;

		switch (alignment) {

			case StringAlignment::CENTER: {

				top = boundsY + (boundsHeight - size) / 2;
				left = boundsX + (boundsWidth - length * size) / 2;
				break;

			}

			case StringAlignment::LEFT: {

				top = boundsY + (boundsHeight - size) / 2;
				left = boundsX;

				break;

			}

			case StringAlignment::RIGHT: {

				top = boundsY + (boundsHeight - size) / 2;
				left = boundsWidth + boundsX - length * size;
				break;

			}

			default: {

				top = boundsY;
				left = boundsX;

			}

		}

		int cutOffsetBottom = cutY + cutHeight - (top - boundsY);
		if (cutOffsetBottom < 0) cutOffsetBottom = CHAR_SIZE;
		else if (cutOffsetBottom > size) cutOffsetBottom = 0;
		else cutOffsetBottom = CHAR_SIZE - (CHAR_SIZE * cutOffsetBottom) / size; // n / 15


		int cutOffsetY = cutY - (top - boundsY);
		if (cutOffsetY < 0) cutOffsetY = 0;
		top += cutOffsetY;
		cutOffsetY = (CHAR_SIZE * cutOffsetY) / size;
	
	hereWeGoAgain:
		for (int i = 0; i < length; i++) {

			int ch = string[i];

			int x = 0;
			int y = top;
			int offset = CHAR_LENGTH * (ch - CHAR_OFFSET);

			const int from = offset + cutOffsetY * CHAR_SIZE;
			const int till = offset + CHAR_LENGTH - cutOffsetBottom * CHAR_SIZE;
			for (int j = from; j < till; j++) {

				if (x >= CHAR_SIZE) {
					x = 0;
					y--;
				}

				int xx = (x * size) / CHAR_SIZE;
				int yy = top + ((top - y) * size) / CHAR_SIZE;
				pixels[width * yy + left + xx] = (BMP_FONT[j]) ? color : backColor;

				x++;

			}

			left += size;

		}

		if (dotedEnd) {

			string = (char*) "..";
			dotedEnd = 0;
			length = 2;

			goto hereWeGoAgain;

		}
	
	}

	void drawBitmap(uint32_t* bitmap, int bitmapWidth, int bitmapHeight, int x, int y) {
		
		int globalOffset = y * renderWidth + x;

		for (int i = 0; i < bitmapHeight; i++) {

			int offsetBitmap = i * bitmapWidth;
			int offsetPixels = globalOffset + i * renderWidth;
			for (int j = 0; j < bitmapWidth; j++) {

				pixels[offsetPixels + j] = alphaBlendPixels(
					pixels[offsetPixels + j], 
					bitmap[offsetBitmap + j]
				);
			
			}

		}
	
	}

	void drawBitmap(
		uint32_t* bitmap, 
		int bitmapWidth, 
		int bitmapHeight, 
		int x, 
		int y, 
		int width, 
		int height
	) {

		int globalOffset = y * renderWidth + x;

		for (int i = 0; i < height; i++) {

			const int bmpY = floor(bitmapHeight * ((double) (height - i - 1) / height));
			const int bmpOffset = bmpY * bitmapWidth;

			const int outY = i;
			const int outOffset = globalOffset + outY * renderWidth;

			for (int j = 0; j < width; j++) {

				const int bmpX = floor(bitmapWidth * ((double)j / width));
				const int outX = j;

				pixels[outOffset + outX] = alphaBlendPixels(
					pixels[outOffset + outX],
					bitmap[bmpOffset + bmpX]
				);

			}

		}

	}

	// meh for now
	void drawBitmap(
		uint32_t* bitmap,
		int bitmapWidth,
		int bitmapHeight,
		int x,
		int y,
		int width,
		int height,
		double angle
	) {

		//angle = 0;//3.14 / 2;
		
		const int bmpHfWidth = width / 2;
		const int bmpHfHeight = height / 2;

		const double cosa = cos(angle);
		const double sina = sin(angle);

		const double cosna = cos(-angle);
		const double sinna = sin(-angle);

		const int rx1 = round(cosa * (-bmpHfWidth) - sina * (bmpHfHeight));
		const int rx2 = round(cosa * (bmpHfWidth) - sina * (bmpHfHeight));
		const int rx3 = round(cosa * (bmpHfWidth) - sina * (-bmpHfHeight));
		const int rx4 = round(cosa * (-bmpHfWidth) - sina * (-bmpHfHeight));

		const int ry1 = round(sina * (-bmpHfWidth) + cosa * (bmpHfHeight));
		const int ry2 = round(sina * (bmpHfWidth) + cosa * (bmpHfHeight));
		const int ry3 = round(sina * (bmpHfWidth) + cosa * (-bmpHfHeight));
		const int ry4 = round(sina * (-bmpHfWidth) + cosa * (-bmpHfHeight));

		const int minX12 = (rx1 < rx2) ? rx1 : rx2;
		const int minX34 = (rx3 < rx4) ? rx3 : rx4;

		const int maxX12 = (rx1 > rx2) ? rx1 : rx2;
		const int maxX34 = (rx3 > rx4) ? rx3 : rx4;

		const int minX = (minX12 < minX34 ? minX12 : minX34);
		const int maxX = (maxX12 > maxX34 ? maxX12 : maxX34);

		const int minY12 = (ry1 < ry2) ? ry1 : ry2;
		const int minY34 = (ry3 < ry4) ? ry3 : ry4;

		const int maxY12 = (ry1 > ry2) ? ry1 : ry2;
		const int maxY34 = (ry3 > ry4) ? ry3 : ry4;

		const int minY = (minY12 < minY34 ? minY12 : minY34);
		const int maxY = (maxY12 > maxY34 ? maxY12 : maxY34);

		const int hfWidth = (maxX - minX) / 2;
		const int hfHeight = (maxY - minY) / 2;

		int globalOffset = (y + hfHeight + minY) * renderWidth + (x + hfWidth + minX);

		//color = Color::ANTIQUE_WHITE_2;
		//fillRect((x + bmpHfWidth + minX), (y + bmpHfHeight + minY), hfWidth * 2, hfHeight * 2);

		for (int i = minY; i < maxY + 1; i++) {

			const int outY = i;
			const int outOffset = globalOffset + (outY + bmpHfHeight) * renderWidth;

			for (int j = minX; j < maxX + 1; j++) {

				const int outX = j;

				int bmpX = round(cosna * outX + sinna * outY + bmpHfWidth);
				int bmpY = round(-sinna * outX + cosna * outY + bmpHfHeight);

				if (bmpX <= 0 || bmpX > width || bmpY <= 0 || bmpY > height) continue;

				const int bmpOffset = (height - bmpY) * bitmapWidth;

				pixels[outOffset + outX + bmpHfWidth] = alphaBlendPixels(
					pixels[outOffset + outX + bmpHfWidth],
					bitmap[bmpOffset + width - bmpX]
				);

			}

		}

	}

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
	) {

		//angle = 0;//3.14 / 2;

		const int bmpHfWidth = width / 2;
		const int bmpHfHeight = height / 2;

		const double cosa = cos(angle);
		const double sina = sin(angle);

		const double cosna = cos(-angle);
		const double sinna = sin(-angle);

		const int rx1 = round(cosa * (-bmpHfWidth) - sina * (bmpHfHeight));
		const int rx2 = round(cosa * (bmpHfWidth)-sina * (bmpHfHeight));
		const int rx3 = round(cosa * (bmpHfWidth)-sina * (-bmpHfHeight));
		const int rx4 = round(cosa * (-bmpHfWidth) - sina * (-bmpHfHeight));

		const int ry1 = round(sina * (-bmpHfWidth) + cosa * (bmpHfHeight));
		const int ry2 = round(sina * (bmpHfWidth)+cosa * (bmpHfHeight));
		const int ry3 = round(sina * (bmpHfWidth)+cosa * (-bmpHfHeight));
		const int ry4 = round(sina * (-bmpHfWidth) + cosa * (-bmpHfHeight));

		const int minX12 = (rx1 < rx2) ? rx1 : rx2;
		const int minX34 = (rx3 < rx4) ? rx3 : rx4;

		const int maxX12 = (rx1 > rx2) ? rx1 : rx2;
		const int maxX34 = (rx3 > rx4) ? rx3 : rx4;

		const int minX = (minX12 < minX34 ? minX12 : minX34);
		const int maxX = (maxX12 > maxX34 ? maxX12 : maxX34);

		const int minY12 = (ry1 < ry2) ? ry1 : ry2;
		const int minY34 = (ry3 < ry4) ? ry3 : ry4;

		const int maxY12 = (ry1 > ry2) ? ry1 : ry2;
		const int maxY34 = (ry3 > ry4) ? ry3 : ry4;

		const int minY = (minY12 < minY34 ? minY12 : minY34);
		const int maxY = (maxY12 > maxY34 ? maxY12 : maxY34);

		const int hfWidth = (maxX - minX) / 2;
		const int hfHeight = (maxY - minY) / 2;

		int globalOffset = (y + hfHeight + minY) * renderWidth + (x + hfWidth + minX);

		//color = Color::ANTIQUE_WHITE_2;
		//fillRect((x + bmpHfWidth + minX), (y + bmpHfHeight + minY), hfWidth * 2, hfHeight * 2);

		const int trueMinY = (minY < minCutY - bmpHfHeight) ? minCutY - bmpHfHeight : minY;
		const int trueMaxY = (maxY > maxCutY - bmpHfHeight) ? maxCutY - bmpHfHeight : maxY;
		for (int i = trueMinY; i < trueMaxY + 1; i++) {

			const int outY = i;
			const int outOffset = globalOffset + (outY + bmpHfHeight) * renderWidth;

			for (int j = minX; j < maxX + 1; j++) {

				const int outX = j;

				int bmpX = round(cosna * outX + sinna * outY + bmpHfWidth);
				int bmpY = round(-sinna * outX + cosna * outY + bmpHfHeight);

				if (bmpX <= 0 || bmpX > width || bmpY <= 0 || bmpY > height) continue;

				const int bmpOffset = (height - bmpY) * bitmapWidth;

				pixels[outOffset + outX + bmpHfWidth] = alphaBlendPixels(
					pixels[outOffset + outX + bmpHfWidth],
					bitmap[bmpOffset + width - bmpX]
				);

			}

		}

	}

	// if aplha canal is FF draws with given color
	void drawBitmap(
		uint32_t* bitmap,
		const int bitmapWidth,
		const int bitmapHeight,
		const int x,
		const int y,
		const int width,
		const int height,
		const int color
	) {

		int globalOffset = y * renderWidth + x;

		for (int i = 0; i < height; i++) {

			const int bmpY = floor(bitmapHeight * ((double)(height - i - 1) / height));
			const int bmpOffset = bmpY * bitmapWidth;

			const int outY = i;
			const int outOffset = globalOffset + outY * renderWidth;

			for (int j = 0; j < width; j++) {

				const int bmpX = floor(bitmapWidth * ((double)j / width));
				const int outX = j;

				if ((bitmap[bmpOffset + bmpX] >> 24) == 0xFF)
					pixels[outOffset + outX] = color;

			}

		}

	}

	uint32_t alphaBlendPixels(uint32_t pxlDst, uint32_t pxlSrc) {

		uint32_t alpha = (pxlSrc & ALPHA_MASK) >> 24;
		uint32_t nalpha = 255 - alpha;

		uint32_t rb = ( nalpha * (pxlDst & RED_BLUE_MASK) + alpha * (pxlSrc & RED_BLUE_MASK) ) >> 8;
		uint32_t g = ( nalpha * (pxlDst & GREEN_MASK) + alpha * (pxlSrc & GREEN_MASK) ) >> 8;

		return (rb & RED_BLUE_MASK) | (g & GREEN_MASK);

	}

}
