#pragma once

#include "PluginBackground.h"
#include "Render.h"
#include "Color.h"

PluginBackground::PluginBackground() {
	
	this->backgroundColor = Color::RED;

}

void PluginBackground::draw() {

	Render::color = this->backgroundColor;
	Render::fillRect(
		this->x,
		this->y,
		this->width,
		this->height
	);

}
