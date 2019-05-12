#include "rack0.hpp"

using namespace rack;

struct PastePatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(const event::Action &e) override {
    module->patternData.pastePattern(module->transport.currentPattern());
  }
};
