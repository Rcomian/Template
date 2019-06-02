#include "plugin.hpp"
#include "BaseWidget.hpp"

struct SchmittResetTrigger {
	enum State {
		UNKNOWN,
		LOW,
		HIGH
	};
	State state;

	SchmittResetTrigger() {
		reset();
	}
	void reset() {
		state = UNKNOWN;
	}
	/** Updates the state of the Schmitt Trigger given a value.
	Returns true if triggered, i.e. the value increases from 0 to 1.
	If different trigger thresholds are needed, use
		process(rescale(in, low, high, 0.f, 1.f))
	for example.
	*/
	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= 1.f) {
					state = HIGH;
				}
				break;
			case HIGH:
				if (in <= 0.f) {
					state = LOW;
					return true;
				}
				break;
			default:
				if (in >= 1.f) {
					state = HIGH;
				}
				else if (in <= 0.f) {
					state = LOW;
				}
				break;
		}
		return false;
	}
	bool isHigh() {
		return state == HIGH;
	}
};


struct SEQAdapterModule : BaseModule {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    RUN_INPUT,
    CLK_INPUT,
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    RUN_OUTPUT,
		CLK_OUTPUT,
    RESET_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RUN_LIGHT,
		ARMED_LIGHT,
		NUM_LIGHTS
	};

	SEQAdapterModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	void step() override;
  rack::dsp::SchmittTrigger runTrigger;
	SchmittResetTrigger clkResetTrigger;
  rack::dsp::SchmittTrigger resetTrigger;

  bool running = false;
  bool redirectNextClk = false;
	bool receivedClockWhenStopped = false;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void SEQAdapterModule::step() {
  
  bool runTriggered = runTrigger.process(inputs[RUN_INPUT].value);
  bool resetTriggered = resetTrigger.process(inputs[RESET_INPUT].value);
  bool clkReset = clkResetTrigger.process(inputs[CLK_INPUT].value);

  if (runTriggered) {
    running = !running;
		receivedClockWhenStopped = false;
  }
  else if (clkReset && !resetTriggered) {
		if (receivedClockWhenStopped) {
	    running = true;
		} else {
			receivedClockWhenStopped = true;
		}
    redirectNextClk = false;
  }

  if (resetTriggered && !running) {
    redirectNextClk = true;
  }

  float redirectedResetValue = redirectNextClk ? inputs[CLK_INPUT].value : inputs[RESET_INPUT].value;
  float passThroughResetValue = running ? redirectedResetValue : 0;

  outputs[RUN_OUTPUT].value = inputs[RUN_INPUT].value;
  outputs[RESET_OUTPUT].value = passThroughResetValue;
  outputs[CLK_OUTPUT].value = inputs[CLK_INPUT].value;

	lights[RUN_LIGHT].value = running ? 1.0 : 0.0;
	lights[ARMED_LIGHT].value = redirectNextClk ? 1.0 : 0.0;
}

struct SEQAdapterModuleWidget : BaseWidget {
    TextField *textField;

	SEQAdapterModuleWidget(SEQAdapterModule *module) {
		setModule(module);
		initColourChange(Rect(Vec(10, 10), Vec(100, 13)), module, 0.528f, 0.6f, 0.4f);

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/seqadapter.svg")));

		addInput(createInput<PJ301MPort>(Vec(12, 380-22.977-92), module, SEQAdapterModule::RESET_INPUT));
		addInput(createInput<PJ301MPort>(Vec(48, 380-22.977-92), module, SEQAdapterModule::RUN_INPUT));
		addInput(createInput<PJ301MPort>(Vec(83, 380-22.977-92), module, SEQAdapterModule::CLK_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(30.5, 380-22.977-20), module, SEQAdapterModule::RESET_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(65.5, 380-22.977-20), module, SEQAdapterModule::CLK_OUTPUT));

		addChild(createLight<MediumLight<GreenLight>>(Vec(86, 70), module, SEQAdapterModule::RUN_LIGHT));
		addChild(createLight<MediumLight<YellowLight>>(Vec(86, 85), module, SEQAdapterModule::ARMED_LIGHT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelSEQAdapterModule = createModel<SEQAdapterModule, SEQAdapterModuleWidget>("rcm-seq-adapter");
