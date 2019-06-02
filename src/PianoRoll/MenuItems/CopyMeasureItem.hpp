#include "rack.hpp"

using namespace rack;

struct CopyMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(const event::Action &e) override {
    module->patternData.copyMeasure(module->transport.currentPattern(), widget->rollAreaWidget->state.currentMeasure);
    widget->state = MEASURELOADED;
  }
};
