#include "plugin.hpp"
#include "BaseWidget.hpp"

struct CVS0to10Module : BaseModule {
	enum ParamIds {
		AMOUNT_PARAM_A,
		AMOUNT_PARAM_B,
		AMOUNT_PARAM_C,
		AMOUNT_PARAM_D,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT_A,
		CV_OUTPUT_B,
		CV_OUTPUT_C,
		CV_OUTPUT_D,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CVS0to10Module() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(AMOUNT_PARAM_A, 0.0, 10.0, 0.0);
		configParam(AMOUNT_PARAM_B, 0.0, 10.0, 0.0);
		configParam(AMOUNT_PARAM_C, 0.0, 10.0, 0.0);
		configParam(AMOUNT_PARAM_D, 0.0, 10.0, 0.0);
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CVS0to10Module::step() {
	outputs[CV_OUTPUT_A].value = params[AMOUNT_PARAM_A].value;
	outputs[CV_OUTPUT_B].value = params[AMOUNT_PARAM_B].value;
	outputs[CV_OUTPUT_C].value = params[AMOUNT_PARAM_C].value;
	outputs[CV_OUTPUT_D].value = params[AMOUNT_PARAM_D].value;
}

struct CVS0to10ModuleWidget : BaseWidget {
    TextField *textField;

	CVS0to10ModuleWidget(CVS0to10Module *module) : BaseWidget(module) {
		setPanel(SVG::load(assetPlugin(pluginInstance, "res/CVS0to10.svg")));

		auto x = 6.f;
		addParam(createParam<LEDSliderWhite>(Vec(11.5-x, 135), module, CVS0to10Module::AMOUNT_PARAM_A));
		addParam(createParam<LEDSliderWhite>(Vec(26.0-x, 135), module, CVS0to10Module::AMOUNT_PARAM_B));
		addParam(createParam<LEDSliderWhite>(Vec(40.5-x, 135), module, CVS0to10Module::AMOUNT_PARAM_C));
		addParam(createParam<LEDSliderWhite>(Vec(55.0-x, 135), module, CVS0to10Module::AMOUNT_PARAM_D));

		addOutput(createPort<PJ301MPort>(Vec(12.5, 278), PortWidget::OUTPUT, module, CVS0to10Module::CV_OUTPUT_A));
		addOutput(createPort<PJ301MPort>(Vec(42, 278), PortWidget::OUTPUT, module, CVS0to10Module::CV_OUTPUT_B));
		addOutput(createPort<PJ301MPort>(Vec(12.5, 317), PortWidget::OUTPUT, module, CVS0to10Module::CV_OUTPUT_C));
		addOutput(createPort<PJ301MPort>(Vec(42, 317), PortWidget::OUTPUT, module, CVS0to10Module::CV_OUTPUT_D));

		textField = createWidget<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
		((LedDisplayTextField*)textField)->color = COLOR_WHITE;
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
Model *modelCVS0to10Module = createModel<CVS0to10Module, CVS0to10ModuleWidget>("rcm-CVS0to10");
