#include "plugin.hpp"
#include "BaseWidget.hpp"



struct PolyNosModule : BaseModule {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    SRC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NOIS_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	PolyNosModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	void process(const ProcessArgs &args) override;
  rack::dsp::SchmittTrigger clkTrigger;
  rack::dsp::SchmittTrigger resetTrigger;

	rack::dsp::PulseGenerator resetPulse;

	bool armed = false;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void PolyNosModule::process(const ProcessArgs &args) {
  
	auto channels = std::max(1, inputs[SRC_INPUT].getChannels());

	outputs[NOIS_OUTPUT].setChannels(channels);
	for (int i = 0; i < channels; i++) {
	  outputs[NOIS_OUTPUT].setVoltage((rack::random::uniform() - 0.5) * 10, i);
	}
}

struct PolyNosModuleWidget : BaseWidget {
    TextField *textField;

	PolyNosModuleWidget(PolyNosModule *module) {
	  initColourChange(Rect(Vec(10, 10), Vec(100, 13)), module, 0.125f, 0.25f, 0.5f);
		setModule(module);

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/polynos.svg")));

		addInput(createInputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(60.5)), module, PolyNosModule::SRC_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(103.8)), module, PolyNosModule::NOIS_OUTPUT));

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelPolyNosModule = createModel<PolyNosModule, PolyNosModuleWidget>("rcm-polynos");
