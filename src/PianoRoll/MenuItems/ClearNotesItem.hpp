#include "rack.hpp"

using namespace rack;

struct ClearNotesItem : MenuItem {
  PianoRollModule *module = NULL;

  ClearNotesItem(PianoRollModule* module) {
    this->module = module;
    text = "Clear Notes";
  }

  void onAction(const event::Action &e) override {
  	APP->history->push(new PatternData::PatternAction("clear notes", module->patternData.moduleId, module->transport.currentPattern(), module->patternData));
    module->patternData.clearPatternSteps(module->transport.currentPattern());
  }
};
