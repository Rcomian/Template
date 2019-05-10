#include "plugin.hpp"
#include "BaseWidget.hpp"

struct CVMmtModule : BaseModule {
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

	CVMmtModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(BUTTON_PARAM, 0.0, 10.0, 0.0);
	}
	void step() override;
};

void CVMmtModule::step() {
	outputs[CV_OUTPUT].value = params[BUTTON_PARAM].value;
}

struct PB61303White : SvgSwitch {
	PB61303White() {
		addFrame(SVG::load(assetPlugin(pluginInstance, "res/PB61303White.svg")));
	}
};

struct CVMmtModuleWidget : BaseWidget {
	TextField *textField;

	CVMmtModuleWidget(CVMmtModule *module) : BaseWidget(module) {
		setPanel(SVG::load(assetPlugin(pluginInstance, "res/CVMmt.svg")));

		auto pbswitch = createParam<PB61303White>(Vec(10, 156.23), module, CVMmtModule::BUTTON_PARAM);
		pbswitch->momentary = true;
		addParam(pbswitch);

		addOutput(createPort<PJ301MPort>(Vec(26, 331), PortWidget::OUTPUT, module, CVMmtModule::CV_OUTPUT));

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
Model *modelCVMmtModule = createModel<CVMmtModule, CVMmtModuleWidget>("rcm-CVMmt");
