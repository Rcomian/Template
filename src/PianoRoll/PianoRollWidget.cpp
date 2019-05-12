#include "rack0.hpp"
#include "window.hpp"

#include "../plugin.hpp"
#include "PatternWidget.hpp"
#include "PianoRollWidget.hpp"
#include "PianoRollModule.hpp"
#include "MenuItems/CancelPasteItem.hpp"
#include "MenuItems/ClearNotesItem.hpp"
#include "MenuItems/ClockBufferItem.hpp"
#include "MenuItems/CopyMeasureItem.hpp"
#include "MenuItems/CopyPatternItem.hpp"
#include "MenuItems/NotesToShowItem.hpp"
#include "MenuItems/PasteMeasureItem.hpp"
#include "MenuItems/PastePatternItem.hpp"

using namespace rack;

extern Plugin* plugin;

// standalone module for the module browser
PianoRollModule browserModule;

PianoRollWidget::PianoRollWidget(PianoRollModule *module) : BaseWidget(module) {
  module = this->module = module == NULL ? &browserModule : module;
  setPanel(SVG::load(assetPlugin(pluginInstance, "res/PianoRoll.svg")));

  addInput(createPort<PJ301MPort>(Vec(50.114, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::CLOCK_INPUT));
  addInput(createPort<PJ301MPort>(Vec(85.642, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::RESET_INPUT));
  addInput(createPort<PJ301MPort>(Vec(121.170, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::PATTERN_INPUT));
  addInput(createPort<PJ301MPort>(Vec(156.697, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::RUN_INPUT));
  addInput(createPort<PJ301MPort>(Vec(192.224, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::RECORD_INPUT));

  addInput(createPort<PJ301MPort>(Vec(421.394, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::VOCT_INPUT));
  addInput(createPort<PJ301MPort>(Vec(456.921, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::GATE_INPUT));
  addInput(createPort<PJ301MPort>(Vec(492.448, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::RETRIGGER_INPUT));
  addInput(createPort<PJ301MPort>(Vec(527.976, 380.f-91-23.6), PortWidget::INPUT, module, PianoRollModule::VELOCITY_INPUT));

  addOutput(createPort<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::CLOCK_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::RESET_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::PATTERN_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::RUN_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::RECORD_OUTPUT));

  addOutput(createPort<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::VOCT_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(456.921, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::GATE_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(492.448, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::RETRIGGER_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(527.976, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::VELOCITY_OUTPUT));
  addOutput(createPort<PJ301MPort>(Vec(563.503, 380.f-25.9-23.6), PortWidget::OUTPUT, module, PianoRollModule::END_OF_PATTERN_OUTPUT));

  if (module != NULL) {
    rollAreaWidget = new RollAreaWidget(&module->patternData, &module->transport, &module->auditioner);
  } else {
    rollAreaWidget = new RollAreaWidget(NULL, NULL, NULL);
  }
  rollAreaWidget->box = getRollArea();
  addChild(rollAreaWidget);

  PatternWidget* patternWidget = createWidget<PatternWidget>(Vec(505.257, 380.f-224.259-125.586));
  patternWidget->module = module;
  patternWidget->widget = this;
  addChild(patternWidget);

  initColourChange(Rect(Vec(506, 10), Vec(85, 13)), module, 0.5f, 1.f, 0.25f);
}

void PianoRollWidget::appendContextMenu(Menu* menu) {

  menu->addChild(createMenuLabel(""));
  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Copy / Paste"));

  CopyPatternItem *copyPatternItem = new CopyPatternItem();
  copyPatternItem->widget = this;
  copyPatternItem->module = module;
  copyPatternItem->text = "Copy Pattern";
  menu->addChild(copyPatternItem);

  CopyMeasureItem *copyMeasureItem = new CopyMeasureItem();
  copyMeasureItem->widget = this;
  copyMeasureItem->module = module;
  copyMeasureItem->text = "Copy Measure";
  menu->addChild(copyMeasureItem);

  switch(state) {
    case COPYREADY:
      break;
    case PATTERNLOADED:
      {
        PastePatternItem *pastePatternItem = new PastePatternItem();
        pastePatternItem->widget = this;
        pastePatternItem->module = module;
        pastePatternItem->text = "Paste Pattern";
        menu->addChild(pastePatternItem);
      }
      break;
    case MEASURELOADED:
      {
        PasteMeasureItem *pasteMeasureItem = new PasteMeasureItem();
        pasteMeasureItem->widget = this;
        pasteMeasureItem->module = module;
        pasteMeasureItem->text = "Paste Measure";
        menu->addChild(pasteMeasureItem);
      }
      break;
    default:
      state = COPYREADY;
      break;
  }

  menu->addChild(createMenuLabel(""));
    menu->addChild(new ClearNotesItem(this->module));

  menu->addChild(createMenuLabel(""));
  menu->addChild(createMenuLabel("Notes to Show"));
    menu->addChild(new NotesToShowItem(this, 12));
    menu->addChild(new NotesToShowItem(this, 18));
    menu->addChild(new NotesToShowItem(this, 24));
    menu->addChild(new NotesToShowItem(this, 36));
    menu->addChild(new NotesToShowItem(this, 48));
    menu->addChild(new NotesToShowItem(this, 60));
  menu->addChild(createMenuLabel(""));
  menu->addChild(createMenuLabel("Clock Delay (samples)"));
    menu->addChild(new ClockBufferItem(module, 0));
    menu->addChild(new ClockBufferItem(module, 1));
    menu->addChild(new ClockBufferItem(module, 2));
    menu->addChild(new ClockBufferItem(module, 3));
    menu->addChild(new ClockBufferItem(module, 4));
    menu->addChild(new ClockBufferItem(module, 5));
    menu->addChild(new ClockBufferItem(module, 10));
}

Rect PianoRollWidget::getRollArea() {
  Rect roll;
  roll.pos.x = 15.f;
  roll.pos.y = 380-365.f;
  roll.size.x = 480.f;
  roll.size.y = 220.f;
  return roll;
}


json_t *PianoRollWidget::toJson() {
  json_t *rootJ = BaseWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  json_object_set_new(rootJ, "lowestDisplayNote", json_integer(this->rollAreaWidget->state.lowestDisplayNote));
  json_object_set_new(rootJ, "notesToShow", json_integer(this->rollAreaWidget->state.notesToShow));
  json_object_set_new(rootJ, "currentMeasure", json_integer(this->rollAreaWidget->state.currentMeasure));

  return rootJ;
}

void PianoRollWidget::fromJson(json_t *rootJ) {
  BaseWidget::fromJson(rootJ);

  json_t *lowestDisplayNoteJ = json_object_get(rootJ, "lowestDisplayNote");
  if (lowestDisplayNoteJ) {
    rollAreaWidget->state.lowestDisplayNote = json_integer_value(lowestDisplayNoteJ);
  }

  json_t *notesToShowJ = json_object_get(rootJ, "notesToShow");
  if (notesToShowJ) {
    rollAreaWidget->state.notesToShow = json_integer_value(notesToShowJ);
  }

  json_t *currentMeasureJ = json_object_get(rootJ, "currentMeasure");
  if (currentMeasureJ) {
    rollAreaWidget->state.currentMeasure = json_integer_value(currentMeasureJ);
  }

  rollAreaWidget->state.dirty = true;
}
