#include "rack.hpp"

#include "../BaseWidget.hpp"
#include "PatternData.hpp"
#include "Transport.hpp"
#include "Auditioner.hpp"

#include "../ValueChangeTrigger.hpp"

struct PianoRollModule : BaseModule {
	enum ParamIds {
		RUN_BUTTON_PARAM,
		RESET_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RUN_INPUT,
		RESET_INPUT,
		PATTERN_INPUT,
		RECORD_INPUT,
		VOCT_INPUT,
		GATE_INPUT,
		RETRIGGER_INPUT,
		VELOCITY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		RUN_OUTPUT,
		RESET_OUTPUT,
		PATTERN_OUTPUT,
		RECORD_OUTPUT,
		VOCT_OUTPUT,
		GATE_OUTPUT,
		RETRIGGER_OUTPUT,
		VELOCITY_OUTPUT,
		END_OF_PATTERN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	rack::dsp::SchmittTrigger clockInputTrigger;
	rack::dsp::SchmittTrigger resetInputTrigger;
	rack::dsp::SchmittTrigger runInputTrigger;

	rack::dsp::PulseGenerator retriggerOutputPulse;
	rack::dsp::PulseGenerator eopOutputPulse;
	rack::dsp::PulseGenerator gateOutputPulse;

	rack::dsp::ClockDivider processDivider;

	Auditioner auditioner;

	ValueChangeTrigger<bool> runInputActive;
	rack::dsp::RingBuffer<float, 16> clockBuffer;
	int clockDelay = 0;

	rack::dsp::SchmittTrigger recordingIn;
	rack::dsp::RingBuffer<float, 512> voctInBuffer;
	rack::dsp::RingBuffer<float, 512> gateInBuffer;
	rack::dsp::RingBuffer<float, 512> retriggerInBuffer;
	rack::dsp::RingBuffer<float, 512> velocityInBuffer;

  PatternData patternData;
  Transport transport;

	PianoRollModule();

	void process(const ProcessArgs &args) override;
	void onReset() override;
	void onAdd() override;

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;
};
