#include "plugin.hpp"
#include "BaseWidget.hpp"

extern "C" 
{
	#include "gverb.h"
}

using namespace std;

struct Follower {
	float level = 0.f;

	void step(float* left, float* right) {
		auto value = max(abs(*left), abs(*right));

		if (value >= level) {
			level = value;
		} else {
			level -= (level - value) * 0.001;
		}

		if (level > 10.f) {
			*left /= (level / 10.f);
			*right /= (level / 10.f);
		}
	}
};

struct GVerbModule : BaseModule {
	enum ParamIds {
		ROOM_SIZE_PARAM,
		REV_TIME_PARAM,
		DAMPING_PARAM,
		SPREAD_PARAM,
		BANDWIDTH_PARAM,
		EARLY_LEVEL_PARAM,
		TAIL_LEVEL_PARAM,
		MIX_PARAM,
		RESET_PARAM,
		ROOM_SIZE_POT_PARAM,
		DAMPING_POT_PARAM,
		REV_TIME_POT_PARAM,
		BANDWIDTH_POT_PARAM,
		EARLY_LEVEL_POT_PARAM,
		TAIL_LEVEL_POT_PARAM,
		MIX_POT_PARAM,
		SPREAD_POT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LEFT_AUDIO,
		RIGHT_AUDIO,
		ROOM_SIZE_INPUT,
		DAMPING_INPUT,
		REV_TIME_INPUT,
		BANDWIDTH_INPUT,
		EARLY_LEVEL_INPUT,
		TAIL_LEVEL_INPUT,
		MIX_INPUT,
		SPREAD_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	ty_gverb* gverbL = NULL;
	ty_gverb* gverbR = NULL;

	GVerbModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(ROOM_SIZE_PARAM, 2.0, 300.0, 20.0);
		configParam(DAMPING_PARAM, 0.0, 1.0, 0.98);

		configParam(REV_TIME_PARAM, 0.0, 10.0, 1.0);
		configParam(BANDWIDTH_PARAM, 0.0, 1.0, 0.01);
		configParam(EARLY_LEVEL_PARAM, 0.0, 1.0, 0.8);
		configParam(TAIL_LEVEL_PARAM, 0.0, 1.0, 0.5);

		configParam(MIX_PARAM, 0.0, 1.0, 0.4);
		configParam(SPREAD_PARAM, 0.0, 1.0, 1.0);
		configParam(RESET_PARAM, 0.0, 1.0, 0.0);

		configParam(ROOM_SIZE_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(DAMPING_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(REV_TIME_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(BANDWIDTH_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(EARLY_LEVEL_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(TAIL_LEVEL_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(MIX_POT_PARAM, -1.f, 1.f, 0.f);
		configParam(SPREAD_POT_PARAM, -1.f, 1.f, 0.f);
	}
	void onSampleRateChange() override;
	void process(const ProcessArgs &args) override;
	void disposeGverbL();
	void disposeGverbR();

	float p_frequency = 0.f;
	float p_room_size = 0.f;
	float p_rev_time = 0.f;
	float p_damping = 0.f;
	float p_bandwidth = 0.f;
	float p_early_level = 0.f;
	float p_tail_level = 0.f;

	Follower follower;

	dsp::SchmittTrigger resetTrigger;

	float getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max);
	void handleParam(float value, float* store, void (*change)(ty_gverb*,float));

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void GVerbModule::disposeGverbL() {
	if (gverbL != NULL) {
		gverb_free(gverbL);
		gverbL = NULL;
	}
}

void GVerbModule::disposeGverbR() {
	if (gverbR != NULL) {
		gverb_free(gverbR);
		gverbR = NULL;
	}
}

float GVerbModule::getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max) {
	return clampSafe(params[param].value + (((clampSafe(inputs[mod].getVoltage(), -10.f, 10.f)/10) * max) * params[trim].value), min, max);
}

void GVerbModule::handleParam(float value, float* store, void (*change)(ty_gverb*,float)) {
	if (*store != value) {
		if (gverbL != NULL) {
			change(gverbL, value);
		}
		if (gverbR != NULL) {
			change(gverbR, value);
		}
		*store = value;
	}
}

void GVerbModule::onSampleRateChange() {
	disposeGverbL();
	disposeGverbR();
}

void GVerbModule::process(const rack::Module::ProcessArgs &args) {
	auto leftAudioIn = inputs[LEFT_AUDIO].getVoltageSum();
	auto rightAudioIn = inputs[RIGHT_AUDIO].getVoltageSum();

	auto reset = max(params[RESET_PARAM].value, inputs[RESET_INPUT].getVoltage());
	auto mix = getParam(MIX_PARAM, MIX_INPUT, MIX_POT_PARAM, 0.f, 1.f);

	if (resetTrigger.process(reset)) {
		disposeGverbL();
		disposeGverbR();
	}

	if (gverbL != NULL && inputs[LEFT_AUDIO].getChannels() == 0) {
		disposeGverbL();
	}

	if (gverbR != NULL && inputs[RIGHT_AUDIO].getChannels() == 0) {
		disposeGverbR();
	}

	if (gverbL == NULL) {
		if (inputs[LEFT_AUDIO].getChannels() > 0) {
			gverbL = gverb_new(
				args.sampleRate, // freq
				300,    // max room size
				getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f),    // room size
				getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10000.f),     // revtime
				getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f),   // damping
				90.0,   // spread
				getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f),     // input bandwidth
				getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f),   // early level
				getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f)    // tail level
			);

			p_frequency = args.sampleRate;
		}
	}

	if (gverbR == NULL) {
		if (inputs[RIGHT_AUDIO].getChannels() > 0) {
			gverbR = gverb_new(
				args.sampleRate, // freq
				300,    // max room size
				getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f),    // room size
				getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10000.f),     // revtime
				getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f),   // damping
				90.0,   // spread
				getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f),     // input bandwidth
				getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f),   // early level
				getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f)    // tail level
			);

			p_frequency = args.sampleRate;
		}
	}


	if (gverbL != NULL || gverbR != NULL) {
		handleParam(getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f), &p_room_size, gverb_set_roomsize);
		handleParam(getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10.f), &p_rev_time, gverb_set_revtime);
		handleParam(getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f), &p_damping, gverb_set_damping);
		handleParam(getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f), &p_bandwidth, gverb_set_inputbandwidth);
		handleParam(getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f), &p_early_level, gverb_set_earlylevel);
		handleParam(getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f), &p_tail_level, gverb_set_taillevel);

		auto engineCount = gverbL != NULL && gverbR != NULL ? 2 : 1;
		auto spread = getParam(SPREAD_PARAM, SPREAD_INPUT, SPREAD_POT_PARAM, 0.f, 1.f);
		auto L_L = 0.f, L_R = 0.f;
		auto R_L = 0.f, R_R = 0.f;

		if (gverbL != NULL) {
			gverb_do(gverbL, leftAudioIn / 10.f, &L_L, &L_R);

			L_L = isfinite(L_L) ? L_L * 10.f : 0.f;
			L_R = isfinite(L_R) ? L_R * 10.f : 0.f;
		}

		auto L_L_S = (L_L + ((1-spread) * L_R)) / (2-spread);
		auto L_R_S = (L_R + ((1-spread) * L_L)) / (2-spread);

		if (gverbR != NULL) {
			gverb_do(gverbR, rightAudioIn / 10.f, &R_L, &R_R);

			R_L = isfinite(R_L) ? R_L * 10.f : 0.f;
			R_R = isfinite(R_R) ? R_R * 10.f : 0.f;
		}

		auto R_L_S = (R_L + ((1-spread) * R_R)) / (2-spread);
		auto R_R_S = (R_R + ((1-spread) * R_L)) / (2-spread);

		auto outputLeft = L_L_S + R_L_S / engineCount;
		auto outputRight = L_R_S + R_R_S / engineCount;

		follower.step(&outputLeft, &outputRight);

		outputs[LEFT_OUTPUT].setVoltage(
			crossfade(
				inputs[LEFT_AUDIO].getChannels() > 0 ? leftAudioIn : rightAudioIn,
				outputLeft,
				mix)
		);

		outputs[RIGHT_OUTPUT].setVoltage(
			crossfade(
				inputs[RIGHT_AUDIO].getChannels() > 0 ? rightAudioIn : leftAudioIn,
				outputRight,
				mix)
		);

	} else {
		outputs[LEFT_OUTPUT].setVoltage(0.f);
		outputs[RIGHT_OUTPUT].setVoltage(0.f);
	}
}

struct GVerbModuleWidget : BaseWidget {
	GVerbModuleWidget(GVerbModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Reverb.svg")));

		addParam(createParam<Davies1900hLargeWhiteKnob>(Vec(50, 44), module, GVerbModule::ROOM_SIZE_PARAM));
		addParam(createParam<Davies1900hLargeWhiteKnob>(Vec(50, 115), module, GVerbModule::DAMPING_PARAM));

		addParam(createParam<Davies1900hWhiteKnob>(Vec(127, 60), module, GVerbModule::REV_TIME_PARAM));
		addParam(createParam<Davies1900hWhiteKnob>(Vec(127, 120), module, GVerbModule::BANDWIDTH_PARAM));
		addParam(createParam<Davies1900hWhiteKnob>(Vec(185, 60), module, GVerbModule::EARLY_LEVEL_PARAM));
		addParam(createParam<Davies1900hWhiteKnob>(Vec(185, 120), module, GVerbModule::TAIL_LEVEL_PARAM));

		addParam(createParam<RoundBlackKnob>(Vec(84, 189), module, GVerbModule::MIX_PARAM));
		addParam(createParam<RoundBlackKnob>(Vec(135, 189), module, GVerbModule::SPREAD_PARAM));
		addParam(createParam<PB61303>(Vec(186, 189), module, GVerbModule::RESET_PARAM));

		addParam(createParam<Trimpot>(Vec(15, 263), module, GVerbModule::ROOM_SIZE_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(42, 263), module, GVerbModule::DAMPING_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(70, 263), module, GVerbModule::REV_TIME_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(97, 263), module, GVerbModule::BANDWIDTH_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(124, 263), module, GVerbModule::EARLY_LEVEL_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(151, 263), module, GVerbModule::TAIL_LEVEL_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(178, 263), module, GVerbModule::MIX_POT_PARAM));
		addParam(createParam<Trimpot>(Vec(205, 263), module, GVerbModule::SPREAD_POT_PARAM));

		addInput(createInput<PJ301MPort>(Vec(14, 286), module, GVerbModule::ROOM_SIZE_INPUT));
		addInput(createInput<PJ301MPort>(Vec(41, 286), module, GVerbModule::DAMPING_INPUT));
		addInput(createInput<PJ301MPort>(Vec(68, 286), module, GVerbModule::REV_TIME_INPUT));
		addInput(createInput<PJ301MPort>(Vec(95, 286), module, GVerbModule::BANDWIDTH_INPUT));
		addInput(createInput<PJ301MPort>(Vec(123, 286), module, GVerbModule::EARLY_LEVEL_INPUT));
		addInput(createInput<PJ301MPort>(Vec(150, 286), module, GVerbModule::TAIL_LEVEL_INPUT));
		addInput(createInput<PJ301MPort>(Vec(177, 286), module, GVerbModule::MIX_INPUT));
		addInput(createInput<PJ301MPort>(Vec(204, 286), module, GVerbModule::SPREAD_INPUT));
		addInput(createInput<PJ301MPort>(Vec(232, 286), module, GVerbModule::RESET_INPUT));

		addInput(createInput<PJ301MPort>(Vec(14, 332), module, GVerbModule::LEFT_AUDIO));
		addInput(createInput<PJ301MPort>(Vec(41, 332), module, GVerbModule::RIGHT_AUDIO));

		addOutput(createOutput<PJ301MPort>(Vec(204, 332), module, GVerbModule::LEFT_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(232, 332), module, GVerbModule::RIGHT_OUTPUT));

		initColourChange(Rect(Vec(111.572, 10), Vec(46.856, 13)), module, 0.06667f, 1.f, 0.58f);
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelGVerbModule = createModel<GVerbModule, GVerbModuleWidget>("rcm-gverb");
