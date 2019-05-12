#pragma once

#include "rack0.hpp"

using namespace rack;

struct ModuleDragType;

struct ColourData {
	float backgroundHue = 1.f;
	float backgroundSaturation = 1.f;
	float backgroundLuminosity = 0.25f;
};

struct ColourChangeWidget : Widget {
	bool dragging = false;
	ColourData *colourData = NULL;

	void onButton(const event::Button &e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragMove(const event::DragMove& e) override;
	void onDragEnd(const event::DragEnd& e) override;
};

struct BaseModule : Module {
	ColourData *colourData = NULL;

	json_t *dataToJson() override;
	void dataFromJson(json_t *root) override;
};

struct BaseWidget : ModuleWidget {
	ColourData colourData;

  BaseWidget(Module *module);

	void initColourChange(Rect hotspot, BaseModule* baseModule, float hue, float saturation, float value); 

  void draw(const DrawArgs& args) override;
};


