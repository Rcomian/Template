#include "plugin.hpp"
#include "BaseWidget.hpp"

struct Follower {
	float level = 0.f;

	float step(float* left, float* right, float recovery) {
		auto value = max(abs(*left), abs(*right));

		if (value >= level) {
			level = value;
		} else {
			level -= (level - value) / (engineGetSampleRate() * recovery);
		}

		return clamp2(level / 10.f, 0.f, 1.f);
	}
};

struct DuckModule : BaseModule {
	enum ParamIds {
		AMOUNT_PARAM,
		RECOVERY_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LEFT_OVER_AUDIO,
		RIGHT_OVER_AUDIO,
		LEFT_UNDER_AUDIO,
		RIGHT_UNDER_AUDIO,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DuckModule() : BaseModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(AMOUNT_PARAM, 0.0, 2.0, 1.0);
		configParam(RECOVERY_PARAM, 0.0, 1.0, 0.5);
	}
	void step() override;

	Follower follower;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void DuckModule::step() {
	auto amount = clamp2(follower.step(&inputs[LEFT_OVER_AUDIO].value, &inputs[RIGHT_OVER_AUDIO].value, params[RECOVERY_PARAM].value) * pow(params[AMOUNT_PARAM].value, 2.0), 0.f, 1.f);

	outputs[LEFT_OUTPUT].value = crossfade(inputs[LEFT_UNDER_AUDIO].value, inputs[LEFT_OVER_AUDIO].value, amount);
	outputs[RIGHT_OUTPUT].value = crossfade(inputs[RIGHT_UNDER_AUDIO].value, inputs[RIGHT_OVER_AUDIO].value, amount);
}

struct DuckModuleWidget : BaseWidget {
	DuckModuleWidget(DuckModule *module) : BaseWidget(module) {
		setPanel(SVG::load(assetPlugin(pluginInstance, "res/Duck.svg")));

		addParam(createParam<Davies1900hLargeWhiteKnob>(Vec(10, 65), module, DuckModule::AMOUNT_PARAM));
		addParam(createParam<Davies1900hLargeWhiteKnob>(Vec(10, 166), module, DuckModule::RECOVERY_PARAM));

		addInput(createPort<PJ301MPort>(Vec(12, 257), PortWidget::INPUT, module, DuckModule::LEFT_OVER_AUDIO));
		addInput(createPort<PJ301MPort>(Vec(40, 257), PortWidget::INPUT, module, DuckModule::RIGHT_OVER_AUDIO));

		addInput(createPort<PJ301MPort>(Vec(12, 295), PortWidget::INPUT, module, DuckModule::LEFT_UNDER_AUDIO));
		addInput(createPort<PJ301MPort>(Vec(40, 295), PortWidget::INPUT, module, DuckModule::RIGHT_UNDER_AUDIO));

		addOutput(createPort<PJ301MPort>(Vec(12, 338), PortWidget::OUTPUT, module, DuckModule::LEFT_OUTPUT));
		addOutput(createPort<PJ301MPort>(Vec(40, 338), PortWidget::OUTPUT, module, DuckModule::RIGHT_OUTPUT));

		initColourChange(Rect(Vec(21.785, 10), Vec(37.278, 13)), module, 0.58f, 1.f, 0.58f);
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelDuckModule = createModel<DuckModule, DuckModuleWidget>("rcm-duck");
