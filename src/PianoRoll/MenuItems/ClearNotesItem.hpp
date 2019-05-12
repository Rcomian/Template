#include "rack0.hpp"

using namespace rack;

struct ClearNotesItem : MenuItem {
  PianoRollModule *module = NULL;

  ClearNotesItem(PianoRollModule* module) {
    this->module = module;
    text = "Clear Notes";
  }

  void onAction(const event::Action &e) override {
    module->patternData.clearPatternSteps(module->transport.currentPattern());
  }
};
