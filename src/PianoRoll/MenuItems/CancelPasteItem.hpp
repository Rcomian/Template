#include "rack0.hpp"

using namespace rack;

struct CancelPasteItem : MenuItem {
  PianoRollWidget *widget = NULL;
  void onAction(const event::Action &e) override {
    widget->state = COPYREADY;
  }
};
