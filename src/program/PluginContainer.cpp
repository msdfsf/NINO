#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "PluginContainer.h"
#include "Plugin.h"
#include "Render.h"
#include "Image.h"
#include "Resources.h"
#include "Cursor.h"
#include "Main.h"
#include "AudioDriver.h"
#include "PluginState.h"
#include <limits.h>

void scroll(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void togglePluginOnOff(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void togglePluginUIVisibility(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void movePluginPlaceUp(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void movePluginPlaceDown(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void scroll(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	PluginContainer* src = (PluginContainer*) source;
	const int pluginCount = src->pluginCount;
	if (pluginCount <= 0) return;

	const int wheelDistance = paramB / WHEEL_DELTA;

	const int minOffset = 0;
	const int maxOffset = 1000;// maxScrollOffsetY;

	const int pixelsWeWantToScroll = wheelDistance * Control::SCROLL_STEP;
	const int maxTop = src->getInBoxY();
	const int maxBottom = maxTop + src->getInBoxHeight();
	const int firstElementTop = src->pluginsUI[0]->y;
	const int lastElementBottom = src->pluginsUI[pluginCount - 1]->y + src->pluginsUI[pluginCount - 1]->height;
	
	int pixelsToScroll = pixelsWeWantToScroll;
	if (pixelsWeWantToScroll < 0 && lastElementBottom + pixelsWeWantToScroll < maxBottom) {
		pixelsToScroll = (lastElementBottom > maxBottom) ? maxBottom - lastElementBottom : 0;
	} else if (pixelsWeWantToScroll >= 0 && firstElementTop + pixelsWeWantToScroll > maxTop) {
		pixelsToScroll = (firstElementTop < maxTop) ? maxTop - firstElementTop : 0;
	}

	src->scrollY(pixelsToScroll);

	for (int i = 0; i < pluginCount; i++) {

		PluginUIHandler* const uihnd = src->plugins[i]->uihnd;

		uihnd->maxTopY = maxTop;
		uihnd->maxBottomY = maxBottom;

		PluginControl** controls = uihnd->controls;
		const int len = uihnd->controlCount;
		for (int j = 0; j < len; j++) {
			controls[j]->y += pixelsToScroll;
		}

	}

	/*
	if (scrollOffsetY < minOffset) {
		scrollOffsetY = minOffset;
	}
	else if (scrollOffsetY > maxOffset) {
		scrollOffsetY = maxOffset;
	};
	*/

	Render::redraw();


}

void togglePluginOnOff(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*)source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int idx = container->id;
	IPlugin* const plugin = pluginContainer->plugins[idx];

	const int state = plugin->state;

	if (plugin->state == PluginState::OFF) {
		
		const int leftState = header->childrens[image->id + 2]->selected;
		const int rightState = header->childrens[image->id + 1]->selected;
		
		if (leftState && rightState) {
			plugin->state = PluginState::ON;
		} else {
			plugin->state = leftState ? PluginState::ON_LEFT : PluginState::ON_RIGHT;
		}
	
	} else {
		
		plugin->state = PluginState::OFF;
	
	}

	image->selected = !image->selected;

	Render::redraw();

}

void togglePluginLeftChannel(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*)source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int idx = container->id;
	IPlugin* const plugin = pluginContainer->plugins[idx];

	const int state = plugin->state;

	if (state == PluginState::ON_LEFT) return;

	if (state != PluginState::OFF) {
		
		plugin->state = (plugin->state == PluginState::ON_RIGHT) ? PluginState::ON : PluginState::ON_RIGHT;
		image->selected = !image->selected;

	} else {

		image->selected = !(image->selected && header->childrens[image->id - 1]->selected);

	}

	Render::redraw();

}

void togglePluginRightChannel(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*)source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int idx = container->id;
	IPlugin* const plugin = pluginContainer->plugins[idx];

	const int state = plugin->state;
	
	if (state == PluginState::ON_RIGHT) return;

	if (state != PluginState::OFF) {
		
		plugin->state = (plugin->state == PluginState::ON_LEFT) ? PluginState::ON : PluginState::ON_LEFT;
		image->selected = !image->selected;
	
	} else {

		image->selected = !(image->selected && header->childrens[image->id + 1]->selected);
	
	}

	Render::redraw();

}

void togglePluginUIVisibility(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
	
	Image* const image = (Image*) source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int idx = container->id;
	IPlugin** const plugins = pluginContainer->plugins;

	PluginUIHandler* const  uihnd = plugins[container->id]->uihnd;
	const int show = uihnd->visible = !uihnd->visible;

	int height;
	if (show) {
		height = uihnd->height + container->borderBottomWidth;
		container->height += height;
		image->setImage((Render::Bitmap*) Resources::minus);
	} else {
		height = - uihnd->height - container->borderBottomWidth;
		container->height += height;
		image->setImage((Render::Bitmap*) Resources::plus);
	}

	const int pluginCount = pluginContainer->childrenCount;
	if (idx < pluginContainer->childrenCount - 1) {
		pluginContainer->scrollY(height, idx + 1);
		Plugin::scrollY(plugins + idx + 1, pluginCount - idx - 1, height);
	}

	Render::redraw();

}

void movePluginPlaceUp(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*)source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int borderBottomWidth = pluginContainer->borderBottomWidth;

	const int idx = container->id;
	if (idx == 0) return;

	Control** const pluginsUI = pluginContainer->pluginsUI;

	{
		IPlugin** const plugins = pluginContainer->plugins;
		
		// exchange y coords
		PluginUIHandler* const uihndUpper = plugins[idx - 1]->uihnd;
		PluginUIHandler* const uihndBottom = plugins[idx]->uihnd;

		const int topY = uihndUpper->y;
		const int spaceBetween = uihndBottom->y - (topY + (uihndUpper->visible ? uihndUpper->height : -borderBottomWidth));

		// const int bottomY = topY + uihndBottom->height + spaceBetween;
		const int bottomY = topY + (uihndBottom->visible ? uihndBottom->height : -borderBottomWidth) + spaceBetween;

		Plugin::setY(uihndBottom, topY);
		Plugin::setY(uihndUpper, bottomY);

		// exchange pointers itself
		IPlugin* const tmp = plugins[idx - 1];
		plugins[idx - 1] = plugins[idx];
		plugins[idx] = tmp;


	}

	{
		Control** const pluginsUI = pluginContainer->pluginsUI;

		// recalculate y coords
		Control* const ctrlUpper = pluginsUI[idx - 1];
		Control* const ctrlBottom = pluginsUI[idx];

		const int topY = ctrlUpper->y;
		const int topBottomY = topY + ctrlBottom->height;

		const int bottomY = topBottomY + pluginContainer->rowSpace;

		ctrlBottom->setY(topY);
		ctrlUpper->setY(bottomY);

		// exchange pointers itself
		Control* const tmp = pluginsUI[idx - 1];
		pluginsUI[idx - 1] = pluginsUI[idx];
		pluginsUI[idx] = tmp;
	
	}

	pluginContainer->swapChildrens(idx, idx - 1);

	Render::redraw();

}

void movePluginPlaceDown(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*)source;
	Control* const header = (Control*)image->parent;
	Control* const container = (Control*)header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	const int borderBottomWidth = pluginContainer->borderBottomWidth;

	const int idx = container->id;
	if (idx >= pluginContainer->pluginCount - 1) return;

	{
		IPlugin** const plugins = pluginContainer->plugins;
		IPlugin** const samplePlugins = pluginContainer->samplePlugins;

		// recalculate y coords
		PluginUIHandler* const uihndUpper = plugins[idx]->uihnd;
		PluginUIHandler* const uihndBottom = plugins[idx + 1]->uihnd;

		const int topY = uihndUpper->y;
		const int spaceBetween = uihndBottom->y - (topY + (uihndUpper->visible ? uihndUpper->height : -borderBottomWidth));

		const int bottomY = topY + (uihndBottom->visible ? uihndBottom->height : -borderBottomWidth) + spaceBetween;

		Plugin::setY(uihndBottom, topY);
		Plugin::setY(uihndUpper, bottomY);

		// exchange pointers itself
		IPlugin* tmp = plugins[idx + 1];
		plugins[idx + 1] = plugins[idx];
		plugins[idx] = tmp;

		tmp = samplePlugins[idx + 1];
		samplePlugins[idx + 1] = samplePlugins[idx];
		samplePlugins[idx] = tmp;
	}

	{
		Control** const pluginsUI = pluginContainer->pluginsUI;

		// recalculate y coords
		Control* const ctrlUpper = pluginsUI[idx];
		Control* const ctrlBottom = pluginsUI[idx + 1];

		const int topY = ctrlUpper->y;
		const int topBottomY = topY + ctrlBottom->height;

		const int bottomY = topBottomY + pluginContainer->rowSpace;

		ctrlBottom->setY(topY);
		ctrlUpper->setY(bottomY);

		// exchange pointers itself
		Control* tmp = pluginsUI[idx + 1];
		pluginsUI[idx + 1] = pluginsUI[idx];
		pluginsUI[idx] = tmp;
	
	}

	pluginContainer->swapChildrens(idx, idx + 1);

	Render::redraw();

}


void removePlugin(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	Image* const image = (Image*) source;
	Control* const header = (Control*) image->parent;
	Control* const container = (Control*) header->parent;
	PluginContainer* const pluginContainer = (PluginContainer*)container->parent;

	IPlugin** const plugins = pluginContainer->plugins;
	IPlugin** const samplePlugins = pluginContainer->samplePlugins;

	const int idx = container->id;
	const int lastIdx = pluginContainer->pluginCount - 1;

	// scroll elements after deliting element
	const int pixelsToScroll = -(container->height + pluginContainer->rowSpace);
	if (idx < lastIdx) {
		pluginContainer->scrollY(pixelsToScroll, idx + 1);
		Plugin::scrollY(plugins + idx + 1, pluginContainer->pluginCount - idx - 1, pixelsToScroll);
	}

	// 'delete' element
	free(pluginContainer->pluginsUI[idx]->text);
	pluginContainer->pluginsUI[idx]->textLength = 0;
	pluginContainer->pluginsUI[idx]->visible = 0;

	plugins[idx]->uihnd->visible = 0;
	//if (plugins[idx]->free) plugins[idx]->free(plugins[idx]->space);
	AudioDriver::removePlugin(idx);
	pluginContainer->pluginCount--;

	// move all needed stuff one position to left and pass 'deleted' elements to the end
	//IPlugin* const tmpSamplePlugin = plugins[idx];
	//IPlugin* const tmpPlugin = plugins[idx];
	Control* const tmpUI = pluginContainer->pluginsUI[idx];
	Control* const tmpChild = pluginContainer->childrens[idx];
	for (int i = idx + 1; i <= lastIdx; i++) {
		//samplePlugins[i - 1] = samplePlugins[i];
		//plugins[i - 1] = plugins[i];
		pluginContainer->pluginsUI[i - 1] = pluginContainer->pluginsUI[i];
		pluginContainer->childrens[i - 1] = pluginContainer->childrens[i];
		pluginContainer->childrens[i - 1]->id = i - 1;
	}
	//samplePlugins[lastIdx] = tmpSamplePlugin;
	//plugins[lastIdx] = tmpPlugin;
	pluginContainer->pluginsUI[lastIdx] = tmpUI;
	pluginContainer->childrens[lastIdx] = tmpChild;
	pluginContainer->childrens[lastIdx]->id = lastIdx;

	//AudioDriver::removePlugin(idx);

	Render::redraw();

}


PluginContainer::PluginContainer() {

	//scrollOffsetY = 20;
	//eMouseClick = &mouseClick;
	eMouseScroll = &scroll;

}

PluginContainer::PluginContainer(IPlugin** plugins, int count) : PluginContainer() {

	this->plugins = plugins;
	this->pluginCount = count;

}

void PluginContainer::draw() {

	if (!visible) return;

	Control::draw();
	
	const int headerColor = MAIN_FRONT_COLOR;
	const int headerColorBack = MAIN_BACK_COLOR;

	const int headerHeight = 50;
	const int pluginHeight = 200;
	const int containerHeight = headerHeight + pluginHeight;
	const int borderSize = 4;

	const int x = this->x + this->borderLeftWidth + this->paddingLeft;
	int y = this->y + this->borderTopWidth + this->paddingTop;

	const int width = getInBoxWidth();

	const int len = pluginCount;
	for (int i = 0; i < len; i++) {

		IPlugin* const plugin = plugins[i];
		Plugin::drawPlugin(plugin);
	
	}

}

int PluginContainer::processMessage(ControlEvent::ControlEvent controlEvent, CTRL_PARAM paramA, CTRL_PARAM paramB) {

	const int len = pluginCount;
	for (int i = 0; i < pluginCount; i++) {

		PluginUIHandler* uihnd = plugins[i]->uihnd;

		Plugin::processEvent(
			uihnd->controls,
			uihnd->controlCount,
			controlEvent,
			paramA,
			paramB
		);

	}

	return Control::processMessage(controlEvent, paramA, paramB);

}

void PluginContainer::setPluginState(int pluginIdx, int state) {
	
	Control* const header = this->childrens[pluginIdx]->childrens[0];

	switch (state) {

		case PluginState::OFF:
			header->childrens[2]->selected = 0;
			break;

		case PluginState::ON:
			header->childrens[2]->selected = 1;
			break;

		case PluginState::ON_LEFT:
			header->childrens[3]->selected = 0;
			header->childrens[4]->selected = 1;
			break;

		case PluginState::ON_RIGHT:
			header->childrens[3]->selected = 1;
			header->childrens[4]->selected = 0;
			break;

	}

}

void PluginContainer::setPluginCount(int count) {

	if (count <= 0) return;

	const int borderWidth = 4;
	const int headerHeight = 30;
	const int width = this->getInBoxWidth();
	const int height = this->getInBoxHeight();
	const int y = this->getInBoxY();
	const int x = this->getInBoxX();

	if (maxPluginCount < count) {
		// we need to alloc more controls

		Control** tmp = (Control**) realloc(pluginsUI, count * sizeof(Control*));
		if (tmp == NULL) {
			return;
		} else {
			pluginsUI = tmp;
		}

		Control* container = NULL;
		int lastY = 0;
		if (pluginCount <= 0) {

			lastY = this->getInBoxY();
		
		} else {

			Control* pluginUI = pluginsUI[pluginCount - 1];
			lastY = pluginUI->y + pluginUI->height + rowSpace;

		}

		for (int i = maxPluginCount; i < count; i++) {

			// container
			Control* container = new Control();
			this->addControl(container);

			container->setX(0.0);
			container->setY(lastY);
			container->setWidth(1.0);
			container->setHeight(headerHeight + 2 * borderWidth);
			container->setBorderWidth(borderWidth);
			container->borderColor = MAIN_FRONT_COLOR;

			// header
			Control* header = new Control();
			container->addControl(header);

			header->setX(0.0);
			header->setY(0.0);
			header->setWidth(1.0);
			header->setHeight(headerHeight);
			header->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
			header->borderColor = MAIN_FRONT_COLOR;
			header->borderBottomWidth = borderWidth;
			header->paddingLeft = borderWidth;

			const int headerWidth = header->width;
			const int imgPadding = 1.7 * borderWidth;//2.5 * borderWidth; // top and bottom
			const int imgSpaceBetween = 2 * borderWidth;

			Image* closeButton = new Image();
			header->addControl(closeButton);

			closeButton->paddingTop = imgPadding;
			closeButton->paddingBottom = imgPadding;
			closeButton->setHeight(1.0);
			closeButton->width = closeButton->height - 2 * imgPadding + closeButton->paddingLeft + closeButton->paddingRight;
			closeButton->setX(0.0);
			closeButton->setY(0.0);
			closeButton->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
			closeButton->drawType = ImageDrawType::ONE_COLOR;
			closeButton->setImage((Render::Bitmap*)Resources::closeCross);
			closeButton->cursor = Cursor::POINTER;

			closeButton->eMouseClick = &removePlugin;

			Control* headerText = new Control();
			header->addControl(headerText);

			headerText->x = closeButton->x + closeButton->width + imgSpaceBetween;
			headerText->setY(0.0);
			headerText->setHeight(1.0);
			headerText->setWidth(1.0);
			headerText->width -= (imgSpaceBetween + headerText->height) * 4;
			headerText->fontSize = 10;
			headerText->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
			headerText->textAlignment = StringAlignment::LEFT;

			const wchar_t* pluginName = plugins[i]->name;
			const int pluginNameLen = wcslen(pluginName);
			char* buffer = (char*)malloc(pluginNameLen * sizeof(char));
			if (buffer != NULL) {

				wcstombs(buffer, pluginName, pluginNameLen);
				headerText->text = buffer;
				headerText->textLength = pluginNameLen;
			
			}

			Image* onOffSwitch = new Image();
			header->addControl(onOffSwitch);

			onOffSwitch->setPadding(imgPadding);
			onOffSwitch->paddingLeft = imgSpaceBetween;
			onOffSwitch->paddingRight = borderWidth;
			onOffSwitch->setHeight(1.0);
			onOffSwitch->width = onOffSwitch->height - 2 * imgPadding + onOffSwitch->paddingLeft + onOffSwitch->paddingRight;
			onOffSwitch->setX(1.0);
			onOffSwitch->x -= onOffSwitch->width;
			onOffSwitch->setY(0.0);
			onOffSwitch->setColor(MAIN_BACK_COLOR, 0xFFf92673);
			onOffSwitch->drawType = ImageDrawType::ONE_COLOR;
			onOffSwitch->selected = 1;
			onOffSwitch->setImage((Render::Bitmap*) Resources::powerButtonOff);
			onOffSwitch->cursor = Cursor::POINTER;

			onOffSwitch->eMouseClick = &togglePluginOnOff;

			Image* rightChannelSwitch = new Image();
			header->addControl(rightChannelSwitch);

			rightChannelSwitch->setPadding(imgPadding);
			rightChannelSwitch->paddingLeft = imgSpaceBetween;
			rightChannelSwitch->paddingRight = borderWidth;
			rightChannelSwitch->setHeight(1.0);
			rightChannelSwitch->width = rightChannelSwitch->height - 2 * imgPadding + rightChannelSwitch->paddingLeft + rightChannelSwitch->paddingRight;
			rightChannelSwitch->setX(1.0);
			rightChannelSwitch->x = onOffSwitch->x - rightChannelSwitch->width;
			rightChannelSwitch->setY(0.0);
			rightChannelSwitch->setColor(MAIN_BACK_COLOR, 0xFFf92673);
			rightChannelSwitch->drawType = ImageDrawType::ONE_COLOR;
			rightChannelSwitch->selected = 1;
			rightChannelSwitch->setImage((Render::Bitmap*) Resources::rightChannel);
			rightChannelSwitch->cursor = Cursor::POINTER;

			rightChannelSwitch->eMouseClick = &togglePluginRightChannel;

			Image* leftChannelSwitch = new Image();
			header->addControl(leftChannelSwitch);

			leftChannelSwitch->setPadding(imgPadding);
			leftChannelSwitch->paddingLeft = imgSpaceBetween;
			leftChannelSwitch->paddingRight = borderWidth;
			leftChannelSwitch->setHeight(1.0);
			leftChannelSwitch->width = leftChannelSwitch->height - 2 * imgPadding + leftChannelSwitch->paddingLeft + leftChannelSwitch->paddingRight;
			leftChannelSwitch->setX(1.0);
			leftChannelSwitch->x = rightChannelSwitch->x - leftChannelSwitch->width;
			leftChannelSwitch->setY(0.0);
			leftChannelSwitch->setColor(MAIN_BACK_COLOR, 0xFFf92673);
			leftChannelSwitch->drawType = ImageDrawType::ONE_COLOR;
			leftChannelSwitch->selected = 1;
			leftChannelSwitch->setImage((Render::Bitmap*) Resources::leftChannel);
			leftChannelSwitch->cursor = Cursor::POINTER;

			leftChannelSwitch->eMouseClick = &togglePluginLeftChannel;


			Image* showHideSwitch = new Image();
			header->addControl(showHideSwitch);

			showHideSwitch->setPadding(imgPadding);
			showHideSwitch->paddingLeft = imgSpaceBetween;
			showHideSwitch->paddingRight = 0;
			showHideSwitch->setHeight(1.0);
			showHideSwitch->width = showHideSwitch->height - 2 * imgPadding + showHideSwitch->paddingLeft + showHideSwitch->paddingRight;
			showHideSwitch->x = leftChannelSwitch->x - showHideSwitch->width;
			showHideSwitch->setY(0.0);
			showHideSwitch->setImage((Render::Bitmap*) Resources::minus);
			showHideSwitch->cursor = Cursor::POINTER;

			showHideSwitch->eMouseClick = &togglePluginUIVisibility;

			Image* movePlaceUp = new Image();
			header->addControl(movePlaceUp);

			movePlaceUp->setPadding(imgPadding);
			movePlaceUp->paddingLeft = imgSpaceBetween;
			movePlaceUp->paddingRight = 0;
			movePlaceUp->setHeight(1.0);
			movePlaceUp->width = movePlaceUp->height - 2 * imgPadding + movePlaceUp->paddingLeft + movePlaceUp->paddingRight;
			movePlaceUp->x = showHideSwitch->x - movePlaceUp->width;
			movePlaceUp->setY(0.0);
			movePlaceUp->setImage((Render::Bitmap*) Resources::upTriangle);
			movePlaceUp->cursor = Cursor::POINTER;

			movePlaceUp->eMouseClick = &movePluginPlaceUp;

			Image* movePlaceDown = new Image();
			header->addControl(movePlaceDown);

			movePlaceDown->setPadding(imgPadding);
			movePlaceDown->paddingLeft = imgSpaceBetween;
			movePlaceDown->paddingRight = 0;
			movePlaceDown->setHeight(1.0);
			movePlaceDown->width = movePlaceDown->height - 2 * imgPadding + movePlaceDown->paddingLeft + movePlaceDown->paddingRight;
			movePlaceDown->x = movePlaceUp->x - movePlaceDown->width;
			movePlaceDown->setY(0.0);
			movePlaceDown->setImage((Render::Bitmap*) Resources::downTriangle);
			movePlaceDown->cursor = Cursor::POINTER;

			movePlaceDown->eMouseClick = &movePluginPlaceDown;
		
			// add plugin ui
			pluginsUI[i] = container;

			// and calculate this plugin ui
			const wchar_t* test = plugins[i]->name;
			IPlugin* plugin = plugins[i];

			PluginUIHandler* uihnd = plugin->uihnd;
			uihnd->width = width - 2 * borderWidth;
			uihnd->y = lastY + headerHeight + borderWidth;
			uihnd->x = x + borderWidth;
			uihnd->maxTopY = y;
			uihnd->maxBottomY = y + height;
			uihnd->visible = 1;
			Plugin::initDrawPlugin(plugin);

			const int height = uihnd->height + headerHeight + 2 * borderWidth;
			lastY = uihnd->y + height + rowSpace;

			container->setHeight(height);

		}

		maxPluginCount = count;

	} else {

		//const int lastY = pluginsUI[count - 1]->y;

		int lastY;
		if (pluginCount <= 0) {
			lastY = this->getInBoxY() + headerHeight + borderWidth;
		} else {
			lastY = pluginsUI[pluginCount - 1]->y + pluginsUI[pluginCount - 1]->height + headerHeight + borderWidth + rowSpace;
		}

		for (int i = pluginCount; i < count; i++) {
			
			const wchar_t* pluginName = plugins[i]->name;
			const int pluginNameLen = wcslen(pluginName);
			
			pluginsUI[i]->childrens[0]->childrens[1]->setText((wchar_t*) pluginName, pluginNameLen);
			pluginsUI[i]->visible = 1;

			// and calculate this plugin ui
			IPlugin* plugin = plugins[i];

			PluginUIHandler* uihnd = plugin->uihnd;
			uihnd->width = width - 2 * borderWidth;
			uihnd->y = lastY;
			uihnd->x = x + borderWidth;
			uihnd->maxTopY = y;
			uihnd->maxBottomY = y + height;
			uihnd->visible = 1;
			Plugin::initDrawPlugin(plugin);

			const int height = uihnd->height + headerHeight + 2 * borderWidth;

			pluginsUI[i]->setHeight(height);
			pluginsUI[i]->setY(lastY - headerHeight - borderWidth);

			lastY = uihnd->y + height + rowSpace;
		
		}

	}

	pluginCount = count;


}

void PluginContainer::removeAll() {

	const int count = pluginCount;
	for (int i = 0; i < count; i++) {
		
		free(pluginsUI[i]->text);
		pluginsUI[i]->textLength = 0;
		pluginsUI[i]->visible = 0;

	}

	pluginCount = 0;

}

PluginContainer::~PluginContainer() {



}
