#include "GVerbWidget.hpp"
#include "ui.hpp"
#include "osdialog.h"
#include <string.h>
#include "window.hpp"


using namespace std;
using namespace rack;


struct ButtonTestWidget;


void setText(std::string text);
/*
void saveDialogReturnPath(string& pathStr) 
{
	std::string dir = assetLocal("presets");
	systemCreateDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcvm", filters);

	if (path) {
		std::string pathStr = path;
		free(path);
		std::string extension = stringExtension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcvm";
		}

		//widget->save(pathStr);
		ModuleWidget :: save(pathStr);
	}
	osdialog_filters_free(filters);
};
*/
struct ButtonTest : Module {
	enum ParamIds {
		SELECT_PRESET,
		Test1,
		Test2,
		Test3,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};



	ButtonTest() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	int preset_num;
	string path$ = "";
	string pstext$ = "";
	string pathStr = "";
	osdialog_filters *filters;
	string fileDesc = "";
	

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

/*
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

	*/
/////////step //////////////////////
void ButtonTest::step() 
{
	////////////  moving to onAction  //////////
	////////////////////////////////////////////
	preset_num = params[SELECT_PRESET].value;
	switch (preset_num)
	{
		///////////   Once I get name from save dialog, can set this up properly
		
		case 1:
			{
				pstext$ = "BS-1";
				path$ = "C:/Users/STS/Documents/Rack/presets/BS-1.vcvm";
				fileDesc = pstext$;
			}
		break;
		case 2:
			{
				pstext$ = "BS-2";
				path$ = "C:/Users/STS/Documents/Rack/presets/BS-2.vcvm";
				fileDesc = pstext$;
			}
		break;
		case 3:
			{
				pstext$ = "BS-3";
				path$ = "C:/Users//STS/Documents/Rack/presets/BS-3.vcvm";
				fileDesc = pstext$;
			}
		break;
	}	
 
};

//  Text Display
struct ButtonTestDisplay : TransparentWidget
{
	ButtonTest *module;
	int frame = 0;
	shared_ptr<Font> font;

	ButtonTestDisplay() 
	{
		font = Font::load(assetPlugin(plugin, "res/saxmono.ttf"));
	}
	
	void draw(NVGcontext *vg) override 
	{
		nvgFontSize(vg, 10);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -0);
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));	
		nvgTextBox(vg, 6, 6, 200, module->fileDesc.c_str(), NULL);
	}
};

///////////////////////////////////   Preset Select Knob ////////////////////////
struct MyActionPresetKnob : SVGKnob {
	MyActionPresetKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		snap = true;
		smooth = false;
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/Rogan1PSRed.svg")));
	}

  void onAction(EventAction &e) override;
  //void onChange(EventChange &e) override;

  ButtonTestWidget* widget;
	
};

//void MyActionPresetKnob::onChange(EventChange &e) {}
void MyActionPresetKnob::onAction(EventAction &e) {}

//////////////////////////////////// Load Preset button /////////////////////////////////////

struct MyActionLoadButton : SVGSwitch, MomentarySwitch {
	MyActionLoadButton() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_1.svg")));
	}

  void onAction(EventAction &e) override;

  ButtonTestWidget* widget;
};

///////////////////////////////////  Save Preset Button ////////////////////////////////////
struct MyActionSaveButton : SVGSwitch, MomentarySwitch {
	MyActionSaveButton() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_0.svg")));
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/BefacoPush_1.svg")));
	}

  void onAction(EventAction &e) override;

  ButtonTestWidget* widget;
};

struct ButtonTestWidget : ModuleWidget
{
	TextField *textField;
	ButtonTestWidget(Module *module) : ModuleWidget(module)
	{
		setPanel(SVG::load(assetPlugin(plugin, "res/ButtonTest.svg")));

		{
		textField = Widget::create<LedDisplayTextField>(Vec(8, 260.0));
		textField->box.size = Vec(150, 30);
		textField->multiline = true;
    ((LedDisplayTextField*)textField)->color = COLOR_YELLOW;
		addChild(textField);

		ButtonTestDisplay *display = new ButtonTestDisplay();
		display->module = static_cast<ButtonTest*>(module);
		display->box.pos = Vec(8, 100);
		display->box.size = Vec(200, 100);
		addChild(display);
		}

    MyActionPresetKnob *knbload = Widget::create<MyActionPresetKnob>(Vec(18, 300));
    knbload->widget = this;
    addChild(knbload);

    MyActionLoadButton *btnload = Widget::create<MyActionLoadButton>(Vec(6, 50));
    btnload->widget = this;
    addChild(btnload);

		MyActionSaveButton *btnsave = Widget::create<MyActionSaveButton>(Vec(40, 50));
    btnsave->widget = this;
    addChild(btnsave);
		

		//addParam(ParamWidget::create<rcm_Rogan1PSRed>(Vec(18, 300), module, ButtonTest::SELECT_PRESET, 1.0f, 3.0f, 1.0f));
		addParam(ParamWidget::create<LEDSliderBlue>(Vec(8, 150), module, ButtonTest::Test1, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<LEDSliderRed>(Vec(28, 150), module, ButtonTest::Test2, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<LEDSliderYellow>(Vec(48, 150), module, ButtonTest::Test3, 0.0f, 10.0f, 0.0f));
	}
};

void MyActionLoadButton::onAction(EventAction &e)
{

	/*
	pstext$ = "here it is" ;
	//fileDesc += to_string(textField->text.c_str()) + "\n";
	fileDesc += pstext$ + " ";

	std::string dir = assetLocal("presets");
	//systemCreateDirectory(dir);
	pathStr = dir + "BS-1a" + ".vcvm";
	fileDesc += pathStr + "\n";

*/
//   widget->load(path$);
};


void MyActionSaveButton::onAction(EventAction &e)

{
	std::string dir = assetLocal("presets");
	systemCreateDirectory(dir);
	//fileDesc = dir;
	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcvm", filters);

	if (path) {
		std::string pathStr = path;
		string presetname = path;
		free(path);
		std::string extension = stringExtension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcvm";
		}
		//fileDesc = presetname;
		//fileDesc += " WTF ";
		widget->save(pathStr);
		//ModuleWidget :: save(pathStr);
	}
	osdialog_filters_free(filters);
	
	//widget->saveDialog();
		
	// Need to get filename from saved string.  Haven't found a way to get it yet.
	// Once this is working, can get rid of hardcoding name & path...
	//////////////////////////////////////////////////////////////
	
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelButtonTest = Model::create<ButtonTest, ButtonTestWidget>("rcm", "ButtonTest", "Button Test", ENVELOPE_FOLLOWER_TAG);