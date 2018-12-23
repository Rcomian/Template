#include "GVerbWidget.hpp"
#include "../include/BaseWidget.hpp"

struct CV5to5Module : Module {
	enum ParamIds {
		AMOUNT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CV5to5Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CV5to5Module::step() {
	outputs[CV_OUTPUT].value = params[AMOUNT_PARAM].value;
}

struct CV5to5ModuleWidget : BaseWidget {
    TextField *textField;

	CV5to5ModuleWidget(CV5to5Module *module) : BaseWidget(module) {
		colourHotZone = Rect(Vec(10, 10), Vec(50, 13));
		backgroundHue = 0.754;
		backgroundSaturation = 1.f;
		backgroundLuminosity = 0.58f;

		setPanel(SVG::load(assetPlugin(plugin, "res/CV5to5.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(10, 156.23), module, CV5to5Module::AMOUNT_PARAM, -5.0, 5.0, 0.0));

		addOutput(Port::create<PJ301MPort>(Vec(26, 331), Port::OUTPUT, module, CV5to5Module::CV_OUTPUT));

        textField = Widget::create<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
        ((LedDisplayTextField*)textField)->color = COLOR_WHITE;
		addChild(textField);

	}

	json_t *toJson() override {
		json_t *rootJ = BaseWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		BaseWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCV5to5Module = Model::create<CV5to5Module, CV5to5ModuleWidget>("rcm", "rcm-CV5to5", "CV5to5", ENVELOPE_FOLLOWER_TAG);
