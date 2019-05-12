#include "rack0.hpp"

using namespace rack;

struct ClockBufferItem : MenuItem {
  char buffer[100];
  PianoRollModule* module;
  int value;
  ClockBufferItem(PianoRollModule* module, int value) {
    this->module = module;
    this->value = value;

    snprintf(buffer, 10, "%d", value);
    text = buffer;
    if (value == module->clockDelay) {
      rightText = "✓";
    }
  }
  void onAction(const event::Action &e) override {
    module->clockDelay = value;
  }
};
