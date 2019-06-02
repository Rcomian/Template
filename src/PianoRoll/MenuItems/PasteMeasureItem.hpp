#include "rack.hpp"

using namespace rack;

struct PasteMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(const event::Action &e) override {
  	APP->history->push(new PatternData::PatternAction("paste measure", module->patternData.moduleId, module->transport.currentPattern(), module->patternData));
    module->patternData.pasteMeasure(module->transport.currentPattern(), widget->rollAreaWidget->state.currentMeasure);
  }
};
