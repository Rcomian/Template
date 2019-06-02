#include "plugin.hpp"
#include "BaseWidget.hpp"



struct SyncModule : BaseModule {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    CLK_INPUT,
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLK_OUTPUT,
    RESET_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLK_LIGHT,
		ARMED_LIGHT,
		NUM_LIGHTS
	};

	SyncModule() : BaseModule() {
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

void SyncModule::process(const ProcessArgs &args) {
  
	auto clk = inputs[CLK_INPUT].getVoltage();
	auto reset = inputs[RESET_INPUT].getVoltage();

  bool clkTriggered = clkTrigger.process(clk);
  bool resetTriggered = resetTrigger.process(reset);
  
	if (clkTriggered && armed) {
		armed = false;
		resetPulse.trigger();
	}

	if (resetTriggered) {
		if (clk == 0.f) {
			armed = true;
		} else {
			resetPulse.trigger();
		}
	}

	bool resetPulseHigh = resetPulse.process(args.sampleTime);

  outputs[RESET_OUTPUT].setVoltage(resetPulseHigh ? 10.f : 0.f);
  outputs[CLK_OUTPUT].setVoltage(inputs[CLK_INPUT].getVoltage());

	lights[CLK_LIGHT].value = inputs[CLK_INPUT].getVoltage() ? 1.0 : 0.0;
	lights[ARMED_LIGHT].value = armed ? 1.0 : 0.0;
}

struct SyncModuleWidget : BaseWidget {
    TextField *textField;

	SyncModuleWidget(SyncModule *module) {
		setModule(module);
		initColourChange(Rect(Vec(10, 10), Vec(100, 13)), module, 1.f, 0.6f, 0.4f);

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sync.svg")));

		addInput(createInputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(60.5)), module, SyncModule::CLK_INPUT));
		addInput(createInputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(74.02)), module, SyncModule::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(103.8)), module, SyncModule::CLK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(Vec(mm2px(5.08), mm2px(117.5)), module, SyncModule::RESET_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(Vec(mm2px(5.08), mm2px(18.5)), module, SyncModule::CLK_LIGHT));
		addChild(createLightCentered<MediumLight<YellowLight>>(Vec(mm2px(5.08), mm2px(36)), module, SyncModule::ARMED_LIGHT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelSyncModule = createModel<SyncModule, SyncModuleWidget>("rcm-sync");
