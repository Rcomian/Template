#include "GVerbWidget.hpp"

struct CVMmtModule : Module {
	enum ParamIds {
		BUTTON_PARAM,
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

	CVMmtModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CVMmtModule::step() {
	outputs[CV_OUTPUT].value = params[BUTTON_PARAM].value;
}

struct PB61303White : SVGSwitch, MomentarySwitch {
	PB61303White() {
		addFrame(SVG::load(assetPlugin(plugin, "res/PB61303White.svg")));
	}
};

struct CVMmtModuleWidget : ModuleWidget {
    TextField *textField;

	CVMmtModuleWidget(CVMmtModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CVMmt.svg")));

		addParam(ParamWidget::create<PB61303White>(Vec(10, 156.23), module, CVMmtModule::BUTTON_PARAM, 0.0, 10.0, 0.0));

		addOutput(Port::create<PJ301MPort>(Vec(26, 331), Port::OUTPUT, module, CVMmtModule::CV_OUTPUT));

        textField = Widget::create<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
        ((LedDisplayTextField*)textField)->color = COLOR_WHITE;
		addChild(textField);

	}

	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

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
Model *modelCVMmtModule = Model::create<CVMmtModule, CVMmtModuleWidget>("rcm", "rcm-CVMmt", "CVMmt", ENVELOPE_FOLLOWER_TAG);
