#include "plugin.hpp"
#include "BaseWidget.hpp"

struct CV0to10Module : BaseModule {
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

	CV0to10Module() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CV0to10Module::AMOUNT_PARAM, 0.0, 10.0, 0.0);
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CV0to10Module::step() {
	outputs[CV_OUTPUT].setChannels(1);
	outputs[CV_OUTPUT].setVoltage(params[AMOUNT_PARAM].value);
}

struct CV0to10ModuleWidget : BaseWidget {
    TextField *textField;

	CV0to10ModuleWidget(CV0to10Module *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CV0to10.svg")));

		addParam(createParam<Davies1900hLargeWhiteKnob>(Vec(10, 156.23), module, CV0to10Module::AMOUNT_PARAM));

		addOutput(createOutput<PJ301MPort>(Vec(26, 331), module, CV0to10Module::CV_OUTPUT));

		textField = createWidget<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
		((LedDisplayTextField*)textField)->color = componentlibrary::SCHEME_WHITE;
		addChild(textField);

		initColourChange(Rect(Vec(10, 10), Vec(50, 13)), module, 0.754f, 1.f, 0.58f);
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
Model *modelCV0to10Module = createModel<CV0to10Module, CV0to10ModuleWidget>("rcm-CV0to10");
