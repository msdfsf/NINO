#pragma once

#include "Plugin.h"
#include "Render.h"
#include "Resources.h"
#include "ControlEvent.h"
#include "Main.h"
#include "Control.h"
#include "ToolTip.h";
#include "Color.h";
#include "AudioDriver.h"
#include "TimedEventsDriver.h"

#include "TheSimpleTheBadAndTheUglyDSPLibrary.h"

#include <math.h>
#include <limits.h>

namespace Plugin {

	using namespace dsplib;

	const int TEXT_SIZE = 11;
	const int TEXT_SPACE = 5;

	const double CONTROL_SENS_COEF = 0.01;

	const int TARGET_FPS = 30;
	const int TIME_BETWEEN_FRAMES = 1000 / TARGET_FPS; // millis

	const double MAX_PLOT_FLOAT_VALUE = 1.3; // max amplitude of signal
	const double MAX_PLOT_DB_VALUE = 50; // means under zero
	const double MAX_PLOT_FREQ = 4000;

	const int DFT_MTX_LEN = DFT_MTX_MAX_LEN;
	fComplex DFT_MATRIX[DFT_MTX_LEN];

	int lastMouseX = 0;
	int lastMouseY = 0;

	PluginControl* activeKnob = NULL;
	
	ToolTip toolTip = ToolTip();

	int isInBounds(PluginControl* ctrl, int x, int y);

	void clickEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void mouseDownEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void moveEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void mouseUpEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);

	void mouseUpEventStepKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);
	void moveEventStepKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB);

	void drawControl(PluginControl* ctrl);
	void initControl(PluginUIHandler* uihnd, PluginControl* ctrl);

	// [ LONG_LONG_LONG_LABEL ]--[ LABEL_2 ]-----[ LABEL ]
	// ---------[KNB]---------[KNB]---------[KNB]---------

	// ---------[KNB]---------[KNB]---------
	// ------[KNB]-----[KNB]-----[KNB]------

	// potential size = ps -> starts at width;
	// after each knob -> ps = (width - knbCount * knbSize) / (knbCount + 1) + knbCount


	// recalculates controls values, use to init new controls, or to recalculate properties
	void initDrawPlugin(IPlugin* plugin) {

		/*
		PluginUIHandler* uihnd = plugin->uihnd;
		PluginControl** controls = uihnd->controls;
		
		const int count = uihnd->controlCount - 1; // first is allways background
		int processedCount = 0;

		for (int i = 1; i <= count; i++) {
			initControl(uihnd, controls[i]);
		}

		const int padding = 10;
		const int rowSpace = 5;
		const int maxItemsPerLine = 5;

		const int startX = padding + uihnd->x;
		const int startY = padding + uihnd->y;

		const int width = uihnd->width - 2 * padding;
		const int height = uihnd->height - 2 * padding;

		int y = startY;

		int rowCount = 0;
		int totalHeight = 0;

		while (processedCount < count) {

			int x = startX;
			int freeWidth = width;

			int totalWidth = 0;
			int i = processedCount + 1;
			for (; i <= count && i <= processedCount + 1 + maxItemsPerLine; i++) {

				const int wd = controls[i]->width;
				totalWidth += wd;
				if (totalWidth > freeWidth) {
					totalWidth -= wd;
					break;
				}

				const int ctrlCount = i - processedCount;
				freeWidth = (width - ctrlCount * wd) / (ctrlCount + 1) + ctrlCount;

			}

			const int itemsInRow = i - processedCount - 1;
			if (itemsInRow <= 0) break;

			const int deltaX = (width - totalWidth) / (itemsInRow + 1);

			int maxHeight = 0;
			for (int i = processedCount + 1; i <= processedCount + itemsInRow; i++) {

				x += deltaX;
				controls[i]->x = x;
				x += controls[i]->width;

				controls[i]->y = y + TEXT_SIZE + TEXT_SPACE;

				const int hg = controls[i]->height + TEXT_SIZE + TEXT_SPACE;
				if (hg > maxHeight) maxHeight = hg;

			}

			y += maxHeight + rowSpace;

			processedCount += itemsInRow;
			totalHeight += maxHeight + rowSpace;

			rowCount++;
		
		}

		uihnd->height = totalHeight + 2 * padding - rowSpace;

		PluginControl* background = controls[0];
		background->x = uihnd->x;
		background->y = uihnd->y;
		background->width = uihnd->width;
		background->height = uihnd->height;
		*/
		PluginUIHandler* uihnd = plugin->uihnd;
		PluginControl** controls = uihnd->controls;
		
		const int count = uihnd->controlCount - 1; // first is allways background
		int processedCount = 0;

		for (int i = 1; i <= count; i++) {
			initControl(uihnd, controls[i]);
		}

		const int padding = 10;
		const int rowSpace = padding;
		const int maxItemsPerLine = 5;

		const int startX = padding + uihnd->x;
		const int startY = padding + uihnd->y;

		const int width = uihnd->width - 2 * padding;
		const int height = uihnd->height - 2 * padding;

		int y = startY;

		int rowCount = 0;
		int totalHeight = 0;

		while (processedCount < count) {

			int x = startX;
			int freeWidth = width;
			int totalCtrlWd = 0;

			int totalWidth = 0;
			int i = processedCount + 1;
			for (; i <= count && i <= processedCount + 1 + maxItemsPerLine; i++) {

				const int ctrlWd = controls[i]->width;
				const int labelWd = strlen(controls[i]->label) * TEXT_SIZE;
				const int wd = (ctrlWd > labelWd) ? ctrlWd : labelWd;

				totalWidth += wd;
				if (totalWidth > width) {
					totalWidth -= wd;
					break;
				}

				const int ctrlCount = i - processedCount;
				const int step = (width - totalCtrlWd - ctrlWd) / (ctrlCount + 1);
				int dist = 0;
				int ans = 1;
				for (int j = 1; j < ctrlCount; j++) {
					
					const int ctrlWdA = controls[j]->width;
					const int labelWdA = strlen(controls[j]->label) * TEXT_SIZE;
					const int wdA = (ctrlWdA > labelWdA) ? ctrlWdA : labelWdA;

					const int ctrlWdB = controls[j + 1]->width;
					const int labelWdB = strlen(controls[j + 1]->label) * TEXT_SIZE;
					const int wdB = (ctrlWdB > labelWdB) ? ctrlWdB : labelWdB;

					const int labelOverflowA = (labelWdA > ctrlWdA) ? (labelWdA - ctrlWdA) / 2 : 0;
					const int labelOverflowB = (labelWdB > ctrlWdB) ? (labelWdB - ctrlWdB) / 2 : 0;

					if (labelOverflowA + labelOverflowB > step) {
						ans = 0;
						break;
					}

					dist += step + wdA;

				}

				if (!ans) {
					totalWidth -= wd;
					break;
				}

				totalCtrlWd += ctrlWd;

			}

			const int itemsInRow = i - processedCount - 1;
			if (itemsInRow <= 0) break;

			const int deltaX = (width - totalCtrlWd) / (itemsInRow + 1);

			int maxHeight = 0;
			for (int i = processedCount + 1; i <= processedCount + itemsInRow; i++) {

				x += deltaX;
				controls[i]->x = x;
				x += controls[i]->width;

				controls[i]->y = y + TEXT_SIZE + TEXT_SPACE;

				const int hg = controls[i]->height + TEXT_SIZE + TEXT_SPACE;
				if (hg > maxHeight) maxHeight = hg;

			}

			y += maxHeight + rowSpace;

			processedCount += itemsInRow;
			totalHeight += maxHeight + rowSpace;

			rowCount++;
		
		}

		uihnd->height = totalHeight + 2 * padding - rowSpace;

		PluginControl* background = controls[0];
		background->x = uihnd->x;
		background->y = uihnd->y;
		background->width = uihnd->width;
		background->height = uihnd->height;
	
	}

	/*
	* void initDrawPlugin(IPlugin* plugin) {

		PluginUIHandler* uihnd = plugin->uihnd;
		PluginControl** controls = uihnd->controls;
		
		const int count = uihnd->controlCount - 1; // first is allways background
		int processedCount = 0;

		for (int i = 1; i <= count; i++) {
			initControl(uihnd, controls[i]);
		}

		const int padding = 10;
		const int rowSpace = 5;
		const int maxItemsPerLine = 5;

		const int startX = padding + uihnd->x;
		const int startY = padding + uihnd->y;

		const int width = uihnd->width - 2 * padding;
		const int height = uihnd->height - 2 * padding;

		int y = startY;

		int rowCount = 0;
		int totalHeight = 0;

		while (processedCount < count) {

			int x = startX;

			int totalWidth = 0;
			int i = processedCount + 1;
			for (; i <= count && i <= processedCount + 1 + maxItemsPerLine; i++) {

				const int wd = controls[i]->width;
				totalWidth += wd;
				if (totalWidth > width) {
					totalWidth -= wd;
					break;
				}

			}

			const int itemsInRow = i - processedCount - 1;
			if (itemsInRow <= 0) break;

			const int deltaX = (width - totalWidth) / (itemsInRow + 1);

			int maxHeight = 0;
			for (int i = processedCount + 1; i <= processedCount + itemsInRow; i++) {

				x += deltaX;
				controls[i]->x = x;
				x += controls[i]->width;

				controls[i]->y = y;

				const int hg = controls[i]->height;
				if (hg > maxHeight) maxHeight = hg;

			}

			y += maxHeight + rowSpace;

			processedCount += itemsInRow;
			totalHeight += maxHeight + rowSpace;

			rowCount++;
		
		}

		uihnd->height = totalHeight + 2 * padding - rowSpace;

		PluginControl* background = controls[0];
		background->x = uihnd->x;
		background->y = uihnd->y;
		background->width = uihnd->width;
		background->height = uihnd->height;
	
	}
	*/

	// refactor!!! remove copy paste!!!
	void drawPlugin(IPlugin* const plugin) {

		PluginUIHandler* uihnd = plugin->uihnd;
		if (!uihnd->visible) return;

		PluginControl** controls = uihnd->controls;
		const int controlCount = uihnd->controlCount;
		
		const int maxTopY = uihnd->maxTopY;
		const int maxBottomY = uihnd->maxBottomY;

		for (int i = 0; i < controlCount; i++) {

			PluginControl* ctrl = controls[i];

			const int x = ctrl->x;
			const int y = ctrl->y;

			const int width = ctrl->width;
			const int height = ctrl->height;

			if (y + height < maxTopY || y > maxBottomY) continue;

			const int overflowTop = (y < maxTopY) ? (maxTopY - y) : 0;
			const int overflowBottom = (y + height > maxBottomY) ? (y + height - maxBottomY) : 0;

			switch (ctrl->type) {

				case PCT_BACKGROUND : {

					const int fColor = Render::alphaBlendPixels(ctrl->color, 0x55FFFFFF);
					const int bColor = ctrl->backgroundColor; //0xFFEA638C;

					switch (ctrl->fillType) {

						case PFP_SOLID_COLOR: {
							Render::color = bColor;
							Render::fillRect(x, y + overflowTop, width, height - overflowTop - overflowBottom);
							break;
						}

						case PFP_CHECKERBOARD: {
							Render::color = bColor;
							Render::fillRectChecker(x, y + overflowTop, width, height - overflowTop - overflowBottom, fColor);
							break;
						}

						case PFP_DOTS: {

							Render::color = fColor;
							Render::fillRectDots(x, y + overflowTop, width, height - overflowTop - overflowBottom, bColor);
							break;
						}

						default: {
							Render::color = bColor;
							Render::fillRect(x, y + overflowTop, width, height - overflowTop - overflowBottom);
							break;
						}
					
					}

					break;

				}

				case PCT_KNOB : {

					const int textOverflowTop = (y - TEXT_SIZE - TEXT_SPACE < maxTopY) ? (maxTopY - y + TEXT_SIZE + TEXT_SPACE) : 0;
					const int textOverflowBottom = (y - TEXT_SIZE - TEXT_SPACE + height > maxBottomY) ? (y - TEXT_SIZE - TEXT_SPACE + height - maxBottomY) : 0;

					const double activeRange = 3 * M_PI_LONG / 2;
					const double deadRange = 2 * M_PI_LONG - activeRange;

					const double min = ctrl->MIN_VALUE;
					const double angle = (M_PI_LONG - deadRange / 2) - activeRange * (ctrl->value - min) / (ctrl->MAX_VALUE - min);

					const int imageWidth = Resources::pluginKnob->width;

					Render::drawBitmap(
						Resources::pluginKnob->pixels,// + imageWidth * overflowBottom,
						Resources::pluginKnob->width,
						Resources::pluginKnob->height, //- (overflowTop + overflowBottom) * (Resources::pluginKnob->height / (double) height),
						x,
						y,
						width,
						height,
						angle,
						0,
						overflowTop,
						0,
						height - overflowBottom
					);

					const int textLen = strlen(ctrl->label);
					const int textWidth = textLen * TEXT_SIZE;

					Render::color = ctrl->color;
					Render::drawString(
						ctrl->label,
						strlen(ctrl->label),
						StringAlignment::CENTER,
						TEXT_SIZE,
						x + width / 2 - textWidth / 2,
						y - TEXT_SIZE - TEXT_SPACE,
						textWidth,
						TEXT_SIZE,
						0,
						textOverflowTop,
						textWidth,
						TEXT_SIZE - textOverflowTop - textOverflowBottom
					);

					break;

				}

				case PCT_STEP_KNOB: {

					const int textOverflowTop = (y - TEXT_SIZE - TEXT_SPACE < maxTopY) ? (maxTopY - y + TEXT_SIZE + TEXT_SPACE) : 0;
					const int textOverflowBottom = (y - TEXT_SIZE - TEXT_SPACE + height > maxBottomY) ? (y - TEXT_SIZE - TEXT_SPACE + height - maxBottomY) : 0;

					const double activeRange = 3 * M_PI_LONG / 2;
					const double deadRange = 2 * M_PI_LONG - activeRange;

					const double max = ctrl->MAX_VALUE;
					const double min = ctrl->MIN_VALUE;
					const double angle = (M_PI_LONG - deadRange / 2) - activeRange * (ctrl->value - min) / (ctrl->MAX_VALUE - min);




					const double scaleMin = ctrl->minValue;
					const double scaleMax = ctrl->maxValue;
					const double scaleDelta = scaleMax - scaleMin;

					const double maxStep = round(scaleDelta / ctrl->step);

					const double phaseStep = activeRange / maxStep;
					double phase = (M_PI_LONG - deadRange / 2);

					const int radius = width / 2;
					const int centerX = x + width / 2;
					const int centerY = y + height / 2;
					
					Render::color = Color::WHITE_SMOKE;
					Render::fillCircle(x, y, radius, overflowTop, overflowBottom);

					for (int i = 0; i < maxStep + 1; i++, phase += phaseStep) {

						int endX = cos(phase) * radius;
						int endY = sin(phase) * radius;

						Render::color = Color::BLACK;

						if (phase > 7 * M_PI_LONG / 4 || phase < M_PI_LONG / 4) {
						
							Render::drawLine(centerX, centerY - 1, centerX + endX, centerY + endY - 1);
							Render::drawLine(centerX, centerY + 1, centerX + endX, centerY + endY + 1);
						
						} else if (phase >= M_PI_LONG / 4 && phase < 3 * M_PI_LONG / 4) {
						
							Render::drawLine(centerX - 1, centerY, centerX + endX - 1, centerY + endY);
							Render::drawLine(centerX + 1, centerY, centerX + endX + 1, centerY + endY);

						} else if (phase >= 3 * M_PI_LONG / 4 && phase < 5 * M_PI_LONG / 4) {
						
							Render::drawLine(centerX, centerY - 1, centerX + endX, centerY + endY - 1);
							Render::drawLine(centerX, centerY + 1, centerX + endX, centerY + endY + 1);

						} else {

							Render::drawLine(centerX - 1, centerY, centerX + endX - 1, centerY + endY);
							Render::drawLine(centerX + 1, centerY, centerX + endX + 1, centerY + endY);
						
						}

						Render::color = Color::WHITE;

						Render::drawLine(centerX, centerY, centerX + endX, centerY + endY);
					
					}






					const int imageWidth = Resources::pluginKnob->width;

					Render::drawBitmap(
						Resources::pluginInnerKnob->pixels,// + imageWidth * overflowBottom,
						Resources::pluginInnerKnob->width,
						Resources::pluginInnerKnob->height, //- (overflowTop + overflowBottom) * (Resources::pluginKnob->height / (double) height),
						x,
						y,
						width,
						height,
						angle,
						0,
						overflowTop,
						0,
						height - overflowBottom
					);





					const int textLen = strlen(ctrl->label);
					const int textWidth = textLen * TEXT_SIZE;

					Render::color = ctrl->color;
					Render::drawString(
						ctrl->label,
						strlen(ctrl->label),
						StringAlignment::CENTER,
						TEXT_SIZE,
						x + width / 2 - textWidth / 2,
						y - TEXT_SIZE - TEXT_SPACE,
						textWidth,
						TEXT_SIZE,
						0,
						textOverflowTop,
						textWidth,
						TEXT_SIZE - textOverflowTop - textOverflowBottom
					);

					break;

				}

				case PCT_SIGNAL_VIEWER : {

					const int textColor = uihnd->controls[0]->color;
					const int waveColor = ctrl->color;
					const int backColor = ctrl->backgroundColor;

					const int paddingX = 1;
					const int paddingY = 1;

					const int xLegendTextHeight = 8;
					const int xLegendYPadding = 4;
					const int xLegendHeight = xLegendTextHeight + xLegendYPadding;

					const int xLen = width - 2 * paddingX;
					const int yLen = height - 2 * paddingY - xLegendHeight;

					// draw borders / padding if needed
					const int borderHeight = height - overflowTop - overflowBottom - xLegendHeight;
					Render::color = textColor;
					Render::drawRect(
						x,
						y + overflowTop,
						width,
						borderHeight,
						paddingX,
						paddingX,
						paddingY - overflowTop,
						(borderHeight < 0) ? borderHeight + paddingY : paddingY//(paddingX < height - overflowTop - bottomCutY) ? paddingY : height - overflowTop - bottomCutY
					);

					// draw title
					const int titleOverflowTop = (y - TEXT_SIZE - TEXT_SPACE < maxTopY) ? (maxTopY - y + TEXT_SIZE + TEXT_SPACE) : 0;
					const int titleOverflowBottom = (y - TEXT_SPACE > maxBottomY) ? (y - TEXT_SPACE - maxBottomY) : 0;

					const int textLen = strlen(ctrl->label);
					const int textWidth = textLen * TEXT_SIZE;

					// Render::color = textColor;
					Render::drawString(
						ctrl->label,
						textLen,
						StringAlignment::CENTER,
						TEXT_SIZE,
						x + width / 2 - textWidth / 2,
						y - TEXT_SIZE - TEXT_SPACE,
						textWidth,
						TEXT_SIZE,
						0,
						titleOverflowTop,
						textWidth,
						TEXT_SIZE - titleOverflowTop - titleOverflowBottom
					);

					// draw legend
					const int legendOverflowTop = (y + height - xLegendHeight + xLegendYPadding < maxTopY) ? maxTopY - (y + height - xLegendHeight + xLegendYPadding) : 0;
					const int legendOverflowBottom = (y + height - xLegendHeight + xLegendYPadding + xLegendHeight > maxBottomY) ? (y + height - xLegendHeight + xLegendYPadding + xLegendHeight) - maxBottomY : 0;

					Render::drawString(
						(char*) "0s",
						2,
						StringAlignment::RIGHT,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,
						xLen,
						xLegendHeight,//y - TEXT_SIZE,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					Render::drawString(
						(char*)"-1s",
						3,
						StringAlignment::CENTER,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,//y + yLen + 4 * paddingY,
						xLen,
						xLegendHeight,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					Render::drawString(
						(char*)"-2s",
						3,
						StringAlignment::LEFT,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,
						xLen,
						xLegendHeight,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					// main draw
					PlotInfo* const plotInfo = (PlotInfo*) plugin->space;

					const int lenSamples = plotInfo->lenSamples;
					const int sampleRate = plotInfo->sampleRate;
					const int endIdx = plotInfo->endIdx;
					double* const buffer = plotInfo->dataBuffer;
					uint32_t* pixels = (uint32_t*) plotInfo->renderBuffer;

					const int observedInterval = 2 * 1000; // millis

					const int samplesToProcess = (sampleRate * TIME_BETWEEN_FRAMES) / 1000;
					const int pixelsToProcess = (xLen * TIME_BETWEEN_FRAMES) / observedInterval;

					const int middleY = y + paddingY + yLen / 2;

					// move previous data from cache
					for (int i = y + paddingY + overflowTop; i < y + paddingY + yLen - overflowBottom; i++) {

						const int offsetGlobal = i * Render::renderWidth;
						const int offsetLocal = (i - y - paddingY) * width;
						for (int j = x + paddingX + pixelsToProcess; j < x + paddingX + xLen; j++) {

							Render::pixels[offsetGlobal + j - pixelsToProcess] = pixels[offsetLocal + j - x - paddingX];

						}

					}

					// draw background
					for (int i = y + paddingY + overflowTop; i < y + paddingY + yLen - overflowBottom; i++) {

						const int offsetGlobal = i * Render::renderWidth;
						for (int j = x + paddingX + xLen - pixelsToProcess; j < x + paddingX + xLen; j++) {

							Render::pixels[offsetGlobal + j] = backColor;

						}

					}
					
					// draw base line
					if (middleY > overflowTop && middleY < maxBottomY) {
						Render::color = waveColor;
						Render::drawLineX(x + paddingX + xLen - pixelsToProcess, middleY, pixelsToProcess);
					}

					// pixels
					double xStep = samplesToProcess / (double) pixelsToProcess; // what if step is close to 0?
					double xDist = 0;
					double remSum = 0;

					const int maxHeight = yLen / 2;
					const double valueCoef = maxHeight / MAX_PLOT_FLOAT_VALUE;

					const int startLineX = x + xLen - pixelsToProcess;
					const int startIdxOffset = endIdx + 1 - samplesToProcess;

					if (xStep <= 1) {

						for (int i = 0; i < pixelsToProcess; i++) {

							const int offset = startIdxOffset + (int) floor(i * xStep);
							const int idx = ((offset < 0) ? lenSamples + offset : offset) % lenSamples;

							double yValue = fabs(buffer[idx]) * valueCoef; // maybe make abs standard in plugin definition, so it hasn to be called twice in some cases 

							if (yValue > maxHeight) yValue = maxHeight;
							const int lineY = middleY - yValue;

							Render::drawLineY(startLineX + i, lineY - overflowTop, 2 * yValue);

						}

					} else {

						int firstIdx = endIdx + 1 - samplesToProcess;
						if (firstIdx < 0) firstIdx = lenSamples + firstIdx;
						for (int i = 0; i < pixelsToProcess; i++) {

							const int offset = startIdxOffset + floor((i + 1) * xStep);
							const int nextIdx = (offset < 0) ? lenSamples + offset : offset;
							
							int len = 1;
							double sum = 0;
							double yValue = 0;
							if (nextIdx <= firstIdx) {
								// overflow

								for (int i = firstIdx; i < lenSamples; i++) {
									sum += fabs(buffer[i]);
								}

								for (int i = 0; i < nextIdx; i++) {
									sum += fabs(buffer[i]);
								}

								firstIdx = (nextIdx > lenSamples) ? 0 : nextIdx;

							} else {
								// ok

								for (int i = firstIdx; i < nextIdx; i++) {
									sum += fabs(buffer[i]);
								}

								len = nextIdx - firstIdx;
								firstIdx = (nextIdx > lenSamples) ? 0 : nextIdx;
							}

							yValue = sum * valueCoef / len;

							if (yValue > maxHeight) yValue = maxHeight;
							const int lineY = middleY - yValue;

							// overflow!!
							Render::drawLineY(startLineX + i, lineY, 2 * yValue);

						}

					}

					// cache data 
					for (int i = y + paddingY + overflowTop; i < y + paddingY + yLen - overflowBottom; i++) {

						const int offsetGlobal = i * Render::renderWidth;
						const int offsetLocal = (i - y - paddingY) * width;
						for (int j = x + paddingX; j < x + paddingX + xLen; j++) {

							pixels[offsetLocal + j - x - paddingX] = Render::pixels[offsetGlobal + j];

						}

					}

					break;

				}

				case PCT_FREQUENCY_VIEWER : {
				
					const int textColor = uihnd->controls[0]->color;
					const int waveColor = ctrl->color;
					const int backColor = ctrl->backgroundColor;

					const int paddingX = 1;
					const int paddingY = 1;

					const int xLegendTextHeight = 8;
					const int xLegendYPadding = 4;
					const int xLegendHeight = xLegendTextHeight + xLegendYPadding;

					const int xLen = width - 2 * paddingX;
					const int yLen = height - 2 * paddingY - xLegendHeight;

					// draw borders / padding if needed
					const int borderHeight = height - overflowTop - overflowBottom - xLegendHeight;
					Render::color = textColor;
					Render::drawRect(
						x,
						y + overflowTop,
						width,
						borderHeight,
						paddingX,
						paddingX,
						paddingY - overflowTop,
						(borderHeight < 0) ? borderHeight + paddingY : paddingY//(paddingX < height - overflowTop - bottomCutY) ? paddingY : height - overflowTop - bottomCutY
					);

					// draw title
					const int titleOverflowTop = (y - TEXT_SIZE - TEXT_SPACE < maxTopY) ? (maxTopY - y + TEXT_SIZE + TEXT_SPACE) : 0;
					const int titleOverflowBottom = (y - TEXT_SPACE > maxBottomY) ? (y - TEXT_SPACE - maxBottomY) : 0;

					const int textLen = strlen(ctrl->label);
					const int textWidth = textLen * TEXT_SIZE;

					// Render::color = textColor;
					Render::drawString(
						ctrl->label,
						textLen,
						StringAlignment::CENTER,
						TEXT_SIZE,
						x + width / 2 - textWidth / 2,
						y - TEXT_SIZE - TEXT_SPACE,
						textWidth,
						TEXT_SIZE,
						0,
						titleOverflowTop,
						textWidth,
						TEXT_SIZE - titleOverflowTop - titleOverflowBottom
					);

					// draw legend
					const int legendOverflowTop = (y + height - xLegendHeight + xLegendYPadding < maxTopY) ? maxTopY - (y + height - xLegendHeight + xLegendYPadding) : 0;
					const int legendOverflowBottom = (y + height - xLegendHeight + xLegendYPadding + xLegendHeight > maxBottomY) ? (y + height - xLegendHeight + xLegendYPadding + xLegendHeight) - maxBottomY : 0;

					Render::drawString(
						(char*)"4000Hz",
						6,
						StringAlignment::RIGHT,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,
						xLen,
						xLegendHeight,//y - TEXT_SIZE,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					Render::drawString(
						(char*)"2000hz",
						5,
						StringAlignment::CENTER,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,//y + yLen + 4 * paddingY,
						xLen,
						xLegendHeight,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					Render::drawString(
						(char*)"0hz",
						3,
						StringAlignment::LEFT,
						xLegendTextHeight,
						x + paddingX,
						y + height - xLegendHeight + xLegendYPadding,
						xLen,
						xLegendHeight,
						0,
						legendOverflowTop,
						xLen,
						xLegendHeight - legendOverflowTop - legendOverflowBottom
					);

					// main draw
					PlotInfo* const plotInfo = (PlotInfo*)plugin->space;

					const int lenSamples = plotInfo->lenSamples;
					const int sampleRate = plotInfo->sampleRate;
					const int endIdx = plotInfo->endIdx;
					double* const buffer = plotInfo->dataBuffer;

					const int samplesToFFT = lenSamples / 2;
					const int pixelsToProcess = xLen;

					const int topY = y + paddingY;
					const int middleY = y + paddingY + yLen / 2;
					const int bottomY = y + paddingY + yLen;

					// draw background
					for (int i = y + paddingY + overflowTop; i < y + paddingY + yLen - overflowBottom; i++) {

						const int offsetGlobal = i * Render::renderWidth;
						for (int j = x + paddingX + xLen - pixelsToProcess; j < x + paddingX + xLen; j++) {

							Render::pixels[offsetGlobal + j] = backColor;

						}

					}

					// computing fft
					double fftInBuffer[DFT_MTX_LEN];
					double fftOutBuffer[DFT_MTX_LEN];

					const int minLen = (samplesToFFT < DFT_MTX_LEN) ? samplesToFFT : DFT_MTX_LEN;
					int i = 0;
					for (; i < minLen; i++) {
						fftInBuffer[i] = (endIdx - i) >= 0 ? buffer[endIdx - i] : buffer[lenSamples - i];
					}

					for (; i < DFT_MTX_LEN; i++) {
						fftInBuffer[i] = 0;
					}

					fftFloat(fftInBuffer, fftOutBuffer, DFT_MATRIX, DFT_MTX_LEN);

					// pick only part we want to observe
					const int samplesToProcess = (sampleRate > MAX_PLOT_FREQ) ? floor(MAX_PLOT_FREQ / sampleRate * (DFT_MTX_LEN / 2)) : (DFT_MTX_LEN / 2);

					// convert to db
					fftOutBuffer[0] = fftOutBuffer[0];
					for (int i = 2; i < 2 * samplesToProcess; i += 2) {
						
						const double abs = sqrt(fftOutBuffer[i] * fftOutBuffer[i] + fftOutBuffer[i + 1] * fftOutBuffer[i + 1]);

						if (abs < 0.00001) {
							fftOutBuffer[i / 2] = 0;
						} else {
							fftOutBuffer[i / 2] = 20 * log10(abs / DFT_MTX_LEN) + MAX_PLOT_DB_VALUE;
						}

					}

					// pixels
					double xStep = samplesToProcess / (double) pixelsToProcess;

					const int maxHeight = yLen;
					const double valueCoef = maxHeight / (double) MAX_PLOT_DB_VALUE;

					const int startLineX = x + xLen - pixelsToProcess;

					if (xStep <= 1) {

						for (int i = 0; i < pixelsToProcess; i++) {

							double yValue = fftOutBuffer[(int)floor(i * xStep)] * valueCoef;

							if (yValue > maxHeight) yValue = maxHeight;
							else if (yValue < 0) continue;

							const int lineY = topY + (yLen - yValue);
							Render::drawLineY(startLineX + i, lineY - overflowTop, yValue);

						}

					} else {

						int firstIdx = 0;
						for (int i = 0; i < pixelsToProcess; i++) {

							const int nextIdx = floor((i + 1) * xStep);

							double sum = 0;
							for (int i = firstIdx; i < nextIdx; i++) {
								sum += fftOutBuffer[i];
							}

							const int len = nextIdx - firstIdx;
							firstIdx = (nextIdx > lenSamples) ? 0 : nextIdx;

							double yValue = sum * valueCoef / len;

							if (yValue > maxHeight) yValue = maxHeight;
							const int lineY = middleY - yValue;

							// overflow!!
							Render::drawLineY(startLineX + i, lineY, 2 * yValue);

						}

					}

					break;
				
				}

			}
			//drawControl(controls[i]);
		
		}

	}

	void drawSignalViewer(Control* ctrl) {

		Render::redraw();
	
	}

	void initControl(PluginUIHandler* uihnd, PluginControl* ctrl) {

		switch (ctrl->type) {

			case PCT_BACKGROUND : {

				ctrl->x = uihnd->x;
				ctrl->y = uihnd->y;
				ctrl->width = uihnd->width;

				break;

			}

			case PCT_KNOB : {

				ctrl->width = 32;
				ctrl->height = 32;

				ctrl->eMouseClick = &clickEventKnob;
				ctrl->eMouseMove = &moveEventKnob;
				ctrl->eMouseDown = &mouseDownEventKnob;
				ctrl->eMouseUp = &mouseUpEventKnob;

				break;

			}

			case PCT_STEP_KNOB: {

				ctrl->width = 32;
				ctrl->height = 32;

				ctrl->eMouseClick = &clickEventKnob;
				ctrl->eMouseMove = &moveEventStepKnob;
				ctrl->eMouseDown = &mouseDownEventKnob;
				ctrl->eMouseUp = &mouseUpEventStepKnob;

				break;

			}

			case PCT_SIGNAL_VIEWER: {

				const int padding = 10;

				ctrl->x = uihnd->x + padding;
				ctrl->y = uihnd->y + padding;
				ctrl->width = uihnd->width - padding * 2;
				ctrl->height = 100;

				PlotInfo* plotInfo = (PlotInfo*) ctrl->plugin->space;

				const int pixelCount = ctrl->width * ctrl->height;
				plotInfo->renderBuffer = malloc(sizeof(uint32_t) * pixelCount);
				if (plotInfo->renderBuffer == NULL) {
					break;
				}
				uint32_t* const pixels = (uint32_t*) plotInfo->renderBuffer;
				for (int i = 0; i < pixelCount; i++) {
					pixels[i] = 0xFF000000;
				}

				TimedEventsDriver::add(
					(Control*) ctrl,
					&drawSignalViewer,
					TIME_BETWEEN_FRAMES
				);

				break;
			
			}

			case PCT_FREQUENCY_VIEWER: {
				
				const int padding = 10;

				ctrl->x = uihnd->x + padding;
				ctrl->y = uihnd->y + padding;
				ctrl->width = uihnd->width - padding * 2;
				ctrl->height = 100;

				((PlotInfo*)ctrl->plugin->space)->renderBuffer = NULL;

				// not fancy
				getDFTMatrix(DFT_MATRIX, DFT_MTX_LEN);

				TimedEventsDriver::add(
					(Control*) ctrl,
					&drawSignalViewer,
					TIME_BETWEEN_FRAMES
				);

				break;
			
			}

		}

	}

	void processEvent (
		PluginControl** controls,
		const int controlCount,
		ControlEvent::ControlEvent controlEvent, 
		CTRL_PARAM paramA,
		CTRL_PARAM paramB
	) {

		for (int i = 0; i < controlCount; i++) {

			PluginControl* ctrl = controls[i];

			switch (controlEvent) {

				case ControlEvent::MOUSE_CLICK: {
					// paramA 
					//	- hight order word is y coord
					//	- low order word is x coord
					// paramB
					//	- unused

					if (ctrl->eMouseClick != NULL) {
						if (isInBounds(ctrl, GET_LOW_ORDER_WORD(paramA), GET_HIGH_ORDER_WORD(paramA))) {
							ctrl->eMouseClick(ctrl, paramA, paramB);
						}
					}

					break;

				}

				case ControlEvent::MOUSE_MOVE: {
					// paramA 
					//	- hight order word is y coord
					//	- low order word is x coord
					// paramB
					//	- unused

					if (ctrl->eMouseMove != NULL) {
						ctrl->eMouseMove(ctrl, paramA, paramB);
					}

					break;

				}

				case ControlEvent::MOUSE_DBL_CLICK: {
					// paramA 
					//	- hight order word is y coord
					//	- low order word is x coord
					// paramB
					//	- unused

					if (ctrl->eMouseDblClick != NULL) {
						ctrl->eMouseDblClick(ctrl, paramA, paramB);
					}

					break;

				}

				case ControlEvent::MOUSE_DOWN: {
					// paramA 
					//	- hight order word is y coord
					//	- low order word is x coord
					// paramB
					//	- unused

					if (ctrl->eMouseDown != NULL) {
						ctrl->eMouseDown(ctrl, paramA, paramB);
					}

					break;

				}

				case ControlEvent::MOUSE_UP: {
					// paramA 
					//	- hight order word is y coord
					//	- low order word is x coord
					// paramB
					//	- unused

					if (ctrl->eMouseUp != NULL) {
						ctrl->eMouseUp(ctrl, paramA, paramB);
					}

					break;

				}

				default: {

					break;

				}

			}

		}

	}

	void setY(PluginUIHandler* const uihnd, const int y) {

		const int deltaY = y - uihnd->y;
		uihnd->y = y;

		PluginControl** controls = uihnd->controls;
		const int len = uihnd->controlCount;
		for (int j = 0; j < len; j++) {
			controls[j]->y += deltaY;
		}

	}

	void scrollY(IPlugin** const plugins, const int pluginCount, const int value) {

		
		for (int i = 0; i < pluginCount; i++) {
			
			PluginUIHandler* uihnd = plugins[i]->uihnd;
			uihnd->y += value;

			PluginControl** controls = uihnd->controls;
			const int len = uihnd->controlCount;
			for (int j = 0; j < len; j++) {
				controls[j]->y += value;
			}

		}

	}

	void setValue(PluginControl* ctrl, double x) {

		if (x > ctrl->MAX_VALUE) {
			ctrl->value = ctrl->MAX_VALUE;
		} else if (x < ctrl->MIN_VALUE) {
			ctrl->value = ctrl->MIN_VALUE;
		} else {
			ctrl->value = x;
		}

	}

	int isInBounds(PluginControl* ctrl, int x, int y) {
		
		const int x1 = ctrl->x;
		const int y1 = ctrl->y;

		const int x2 = x1 + ctrl->width;
		const int y2 = y1 + ctrl->height;

		return (x >= x1 && x <= x2 && y >= y1 && y <= y2);

	}

	// events

	//
	// KNOB
	//
	
	void clickEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

		OutputDebugStringA("KNOB_CLICK\n");
	}

	void mouseDownEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

		const int x = GET_LOW_ORDER_WORD(paramA);
		const int y = GET_HIGH_ORDER_WORD(paramA);

		if (isInBounds(control, x, y)) {
			
			activeKnob = control;
			lastMouseX = GET_LOW_ORDER_WORD(paramA);
			
			char buffer[256];
			sprintf(buffer, "%.2f", control->value);

			toolTip.fillBuffer((char*) buffer);
			toolTip.setDesireCoords(x + 15, y + 15);
			toolTip.visible = 1;

		}
	
	}

	void mouseUpEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

			toolTip.visible = 0;
			activeKnob = NULL;
			Render::redraw();

	}

	void moveEventKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

		// has to be changed, MK_LBUTTON = 0x0001
		if (paramB != MK_LBUTTON || control != activeKnob) return;

		const int mouseX = GET_LOW_ORDER_WORD(paramA);
		const double mouseDelta = mouseX - lastMouseX;
		lastMouseX = mouseX;
		// mouseX - (control->x + control->width / 2);

		const double sens = control->sensitivity * CONTROL_SENS_COEF;

		const double max = control->MAX_VALUE;
		const double min = control->MIN_VALUE;
		const double delta = control->MAX_VALUE - control->MIN_VALUE;

		double value = control->value;
		setValue(control, value + mouseDelta * delta * sens);

		if (value == control->value) return;

		value = control->value;
		const double angle = M_PI_LONG * (value - min) / delta; // whats that? delete?

		char buffer[256];
		sprintf(buffer, "%.2f", control->value);
		toolTip.fillBuffer((char*) buffer);

		Render::redraw();

		// maybe add new thread, or pass this callback in signal processing thread, but lets keep it
		// paralel for now
		if (control->eChange != NULL) control->eChange(control, NULL, NULL);

	}




	//
	// STEP KNOB
	//

	void moveEventStepKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

		// has to be changed, MK_LBUTTON = 0x0001
		if (paramB != MK_LBUTTON || control != activeKnob) return;

		const int mouseX = GET_LOW_ORDER_WORD(paramA);
		const double mouseDelta = mouseX - lastMouseX;
		lastMouseX = mouseX;
		// mouseX - (control->x + control->width / 2);

		const double sens = control->sensitivity * CONTROL_SENS_COEF;

		const double max = control->MAX_VALUE;
		const double min = control->MIN_VALUE;
		const double delta = control->MAX_VALUE - control->MIN_VALUE;

		const double value = control->value;
		setValue(control, value + mouseDelta * delta * sens);

		if (value == control->value) return;

		char buffer[256];
		sprintf(buffer, "%.2f", control->value);
		toolTip.fillBuffer((char*)buffer);

		Render::redraw();

	}

	void mouseUpEventStepKnob(PluginControl* control, CTRL_PARAM paramA, CTRL_PARAM paramB) {

		const double max = control->MAX_VALUE;
		const double min = control->MIN_VALUE;
		const double delta = control->MAX_VALUE - control->MIN_VALUE;

		const double scaleMin = control->minValue;
		const double scaleMax = control->maxValue;
		const double scaleDelta = scaleMax - scaleMin;
		
		const double maxStep = round(scaleDelta / control->step);
		const double step = round((control->value - scaleMin) / control->step);

		double value = (step / maxStep) * scaleDelta;
		value = (value > max) ? max : value;
		value = (value < min) ? min : value;
		setValue(control, value);

		toolTip.visible = 0;
		activeKnob = NULL;
		Render::redraw();

		if (control->eChange != NULL) control->eChange(control, NULL, NULL);

	}

	int copyPlugin(IPlugin* const dest, IPlugin* const src) {
		
		dest->name = src->name;
		dest->init = src->init;
		dest->free = src->free;
		dest->process = src->process;
		dest->state = src->state;
		dest->space = src->space;

		dest->uihnd = (PluginUIHandler*) malloc(sizeof(PluginUIHandler));
		if (dest->uihnd == NULL) {
			return 1;
		}
		dest->uihnd->plugin = dest;

		return copyPluginUIHandelr(dest->uihnd, src->uihnd);
	
	}

	int copyPluginUIHandelr(PluginUIHandler* const dest, PluginUIHandler* const src) {

		dest->controlCount = src->controlCount;
		dest->height = src->height;
		dest->width = src->width;
		dest->maxBottomY = src->maxBottomY;
		dest->maxTopY = src->maxTopY;
		dest->visible = src->visible;
		dest->x = src->x;
		dest->y = src->y;

		const int ctrlsCount = dest->controlCount;

		dest->controls = (PluginControl**)malloc(sizeof(PluginControl*) * dest->controlCount);
		if (dest->controls == NULL) {
			return 1;
		}

		PluginControl** const srcCtrls = src->controls;
		PluginControl** const ctrls = dest->controls;
		for (int i = 0; i < ctrlsCount; i++) {

			ctrls[i] = (PluginControl*) malloc(sizeof(PluginControl));
			if (ctrls[i] == NULL) {
				
				for (int j = 0; j < i; j++) {
					free(ctrls[j]);
				}
				free(dest->controls);

				return 1;
			}
			ctrls[i]->plugin = dest->plugin;

			copyPluginControl(ctrls[i], srcCtrls[i]);

		}

		return 0;

	}

	void copyPluginControl(PluginControl* const dest, PluginControl* const src) {

		dest->backgroundColor = src->backgroundColor;
		dest->color = src->color;
		dest->fillType = src->fillType;
		dest->height = src->height;
		dest->label = src->label;
		dest->maxValue = src->maxValue;
		dest->MAX_VALUE = src->MAX_VALUE;
		dest->minValue = src->minValue;
		dest->MIN_VALUE = src->MIN_VALUE;
		dest->selected = src->selected;
		dest->sensitivity = src->sensitivity;
		dest->type = src->type;
		dest->step = src->step;
		dest->value = src->value;
		dest->width = src->width;
		dest->x = src->x;
		dest->y = src->y;

		dest->eChange = src->eChange;
		dest->eMouseClick = src->eMouseClick;
		dest->eMouseDblClick = src->eMouseDblClick;
		dest->eMouseDown = src->eMouseDown;
		dest->eMouseMove = src->eMouseMove;
		dest->eMouseUp = src->eMouseUp;

	}

}