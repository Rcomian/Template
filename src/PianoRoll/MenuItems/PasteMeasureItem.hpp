#include "rack0.hpp"

using namespace rack;

struct PasteMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(const event::Action &e) override {
    module->patternData.pasteMeasure(module->transport.currentPattern(), widget->rollAreaWidget->state.currentMeasure);
  }
};
