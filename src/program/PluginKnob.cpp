#pragma once

#include "PluginKnob.h"
#include "Render.h"
#include "Color.h"
#include "Resources.h"

PluginKnob::PluginKnob() {

	const int size = 32;

	this->width = size;
	this->height = size;

	this->backgroundColor = Color::RED;

}

void PluginKnob::draw() {

	/*
	Render::color = this->backgroundColor;
	Render::fillRect(
		this->x,
		this->y,
		this->width,
		this->height
	);
	*/

	Render::Bitmap* knob = Resources::pluginKnob;
	Render::drawBitmap(
		knob->pixels,
		knob->width,
		knob->height,
		this->x,
		this->y,
		this->width,
		this->height
	);

};

PluginKnob::~PluginKnob() {

}
