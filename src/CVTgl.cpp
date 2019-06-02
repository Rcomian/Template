#include "plugin.hpp"
#include "BaseWidget.hpp"

struct CVTglModule : BaseModule {
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

	CVTglModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(CVTglModule::BUTTON_PARAM, 0.0, 1.0, 0.0);
	}

	void step() override;

};

void CVTglModule::step() {
	outputs[CV_OUTPUT].value = params[BUTTON_PARAM].value * 10.f;
}

struct CKSSWhite : SvgSwitch {
	CKSSWhite() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CKSS_0_White.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CKSS_1_White.svg")));
	}
};

struct CVTglModuleWidget : BaseWidget {
	LedDisplayTextField *textField;

	CVTglModuleWidget(CVTglModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CVTgl.svg")));

		addParam(createParam<CKSSWhite>(Vec(31, 172), module, CVTglModule::BUTTON_PARAM));

		addOutput(createOutput<PJ301MPort>(Vec(26, 331), module, CVTglModule::CV_OUTPUT));

		textField = createWidget<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
		((LedDisplayTextField*)textField)->color = componentlibrary::SCHEME_WHITE;
		addChild(textField);

		initColourChange(Rect(Vec(10, 10), Vec(50, 13)), module, 0.754f, 1.f, 0.58f);
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
Model *modelCVTglModule = createModel<CVTglModule, CVTglModuleWidget>("rcm-CVTgl");
