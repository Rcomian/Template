#include "rack0.hpp"

using namespace rack;

struct PastePatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(const event::Action &e) override {
  	APP->history->push(new PatternData::PatternAction("paste pattern", module->patternData.moduleId, module->transport.currentPattern(), module->patternData));
    module->patternData.pastePattern(module->transport.currentPattern());
  }
};
