#include <utility>
#include "window.hpp"
#include "RollAreaWidget.hpp"
#include "PianoRollModule.hpp"
#include "DragModes.hpp"
#include "../plugin.hpp"
#include "string.h"

using namespace rack;

RollAreaWidget::RollAreaWidget(PatternData* patternData, Transport* transport, Auditioner* auditioner) : patternData(patternData), transport(transport), auditioner(auditioner) {
  underlyingRollAreaWidget = new UnderlyingRollAreaWidget();
  underlyingRollAreaWidget->state = &state;
  underlyingRollAreaWidget->patternData = patternData;
  underlyingRollAreaWidget->transport = transport;
  underlyingRollAreaWidget->auditioner = auditioner;
  addChild(underlyingRollAreaWidget);
}

void RollAreaWidget::step() {
  underlyingRollAreaWidget->box = Rect(Vec(0,0), Vec(box.size.x, box.size.y));

  bool environmentDirty = dirty;
  bool stateDirty = state.consumeDirty();
  bool patternDirty = patternData->consumeDirty();
  bool transportDirty = transport->consumeDirty();

  dirty = environmentDirty || stateDirty || patternDirty || transportDirty;

  FramebufferWidget::step();
}

bool WidgetState::consumeDirty() {
  bool wasdirty = dirty;
  dirty = false;
  return wasdirty;
}

UnderlyingRollAreaWidget::UnderlyingRollAreaWidget() {
  fonthandle = nvgCreateFont(APP->window->vg, asset::system("res/fonts/DejaVuSans.ttf").c_str(), asset::system("res/fonts/DejaVuSans.ttf").c_str());
}
UnderlyingRollAreaWidget::~UnderlyingRollAreaWidget() {
}

std::vector<Key> UnderlyingRollAreaWidget::getKeys(const Rect& keysArea) {
  std::vector<Key> keys;

  int keyCount = (state->notesToShow) + 1;
  float keyHeight = keysArea.size.y / keyCount;

  int octave = state->lowestDisplayNote / 12;
  int offset = state->lowestDisplayNote % 12;

  for (int i = 0; i < keyCount; i++) {
    int n = (i+offset+12) % 12;
    keys.push_back(
      Key(
        Vec(keysArea.pos.x, (keysArea.pos.y + keysArea.size.y) - ( (1 + i) * keyHeight) ),
        Vec(keysArea.size.x, keyHeight ),
        n == 1 || n == 3 || n == 6 || n == 8 || n == 10,
        (i+offset) % 12,
        octave + ((i+offset) / 12)
      )
    );
  }

  return keys;
}

std::vector<BeatDiv> UnderlyingRollAreaWidget::getBeatDivs(const Rect &roll) {
  std::vector<BeatDiv> beatDivs;

  int totalDivisions = patternData->getStepsPerMeasure(transport->currentPattern());
  int divisionsPerBeat = patternData->getDivisionsPerBeat(transport->currentPattern());

  float divisionWidth = roll.size.x / totalDivisions;

  float top = roll.pos.y + topMargins;

  for (int i = 0; i < totalDivisions; i ++) {
    float x = roll.pos.x + (i * divisionWidth);

    BeatDiv beatDiv;
    beatDiv.pos.x = x;
    beatDiv.size.x = divisionWidth;
    beatDiv.pos.y = top;
    beatDiv.size.y = roll.size.y - (2 * topMargins);

    beatDiv.num = i;
    beatDiv.beat = (i % divisionsPerBeat == 0);

    beatDivs.push_back(beatDiv);
  }

  return beatDivs;
}

Rect UnderlyingRollAreaWidget::reserveKeysArea(Rect& roll) {
  Rect keysArea;
  keysArea.pos.x = roll.pos.x;
  keysArea.pos.y = roll.pos.y + topMargins;
  keysArea.size.x = 25.f;
  keysArea.size.y = roll.size.y - (2*topMargins);

  roll.pos.x = keysArea.pos.x + keysArea.size.x;
  roll.size.x = roll.size.x - keysArea.size.x;

  return keysArea;
}

std::tuple<bool, int> UnderlyingRollAreaWidget::findMeasure(Vec pos) {
  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  reserveKeysArea(roll);

  int numberOfMeasures = patternData->getMeasures(transport->currentPattern());
  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float boxHeight = topMargins * 0.75;

  for (int i = 0; i < numberOfMeasures; i++) {
    if (Rect(Vec(roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight), Vec(widthPerMeasure, boxHeight)).isContaining(pos)) {
      return std::make_tuple(true, i);
    }
  }

  return std::make_tuple(false, 0);
}

std::tuple<bool, bool> UnderlyingRollAreaWidget::findOctaveSwitch(Vec pos) {
  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  Rect keysArea = reserveKeysArea(roll);

  bool octaveUp = Rect(Vec(keysArea.pos.x, roll.pos.y), Vec(keysArea.size.x, keysArea.pos.y)).isContaining(pos);
  bool octaveDown = Rect(Vec(keysArea.pos.x, keysArea.pos.y + keysArea.size.y), Vec(keysArea.size.x, keysArea.pos.y)).isContaining(pos);
  
  return std::make_tuple(octaveUp, octaveDown);
}

std::tuple<bool, BeatDiv, Key> UnderlyingRollAreaWidget::findCell(Vec pos) {
  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  Rect keysArea = reserveKeysArea(roll);

  if (!roll.isContaining(pos)) {
    return std::make_tuple(false, BeatDiv(), Key());
  }

  auto keys = getKeys(keysArea);
  bool keyFound = false;
  Key cellKey;

  for (auto const& key: keys) {
    if (Rect(Vec(key.pos.x + key.size.x, key.pos.y), Vec(roll.size.x, key.size.y)).isContaining(pos)) {
      cellKey = key;
      keyFound = true;
      break;
    }
  }

  auto beatDivs = getBeatDivs(roll);
  bool beatDivFound = false;
  BeatDiv cellBeatDiv;

  for (auto const& beatDiv: beatDivs) {
    if (Rect(beatDiv.pos, beatDiv.size).isContaining(pos)) {
      cellBeatDiv = beatDiv;
      beatDivFound = true;
      break;
    }
  }

  return std::make_tuple(keyFound && beatDivFound, cellBeatDiv, cellKey);
}

void UnderlyingRollAreaWidget::drawKeys(const DrawArgs &args, const std::vector<Key> &keys) {
  for (auto const& key: keys) {
    nvgBeginPath(args.vg);
    nvgStrokeWidth(args.vg, 0.5f);
    nvgStrokeColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0));

    if (key.sharp) {
      nvgFillColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
    } else {
      nvgFillColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 1.0));
    }
    nvgRect(args.vg, key.pos.x, key.pos.y, key.size.x, key.size.y);

    nvgStroke(args.vg);
    nvgFill(args.vg);

    if (key.num == 0) {
      Vec textpos(key.pos.x + std::max(6.f, (float)(key.size.x * 0.5)), key.pos.y + (key.size.y * 0.5));

      nvgBeginPath(args.vg);
  		std::string coct = stringf("C%d", key.oct);
      nvgFontSize(args.vg,std::max(6.f, key.size.y));
      nvgFontFaceId(args.vg, fonthandle);
      nvgTextLetterSpacing(args.vg, 2.0);
      nvgFillColor(args.vg, nvgRGB(0.f, 0.f, 0.f));
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
      nvgText(args.vg, textpos.x, textpos.y, coct.c_str(), NULL);
    }
  }
}

void UnderlyingRollAreaWidget::drawSwimLanes(const DrawArgs &args, const Rect &roll, const std::vector<Key> &keys) {

  for (auto const& key: keys) {

    if (key.sharp) {
      nvgBeginPath(args.vg);
      nvgFillColor(args.vg, nvgRGBAf(0.f, 0.0f, 0.0f, 0.25f));
      nvgRect(args.vg, roll.pos.x, key.pos.y + 1, roll.size.x, key.size.y - 2);
      nvgFill(args.vg);
    }

    nvgBeginPath(args.vg);
    if (key.num == 11) {
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
      nvgStrokeWidth(args.vg, 1.0f);
    } else {
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
      nvgStrokeWidth(args.vg, 0.5f);
    }
    nvgMoveTo(args.vg, roll.pos.x, key.pos.y);
    nvgLineTo(args.vg, roll.pos.x + roll.size.x, key.pos.y);
    nvgStroke(args.vg);
  }

  nvgBeginPath(args.vg);
  nvgStrokeWidth(args.vg, 1.f);
  nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
  nvgMoveTo(args.vg, roll.pos.x, keys.back().pos.y);
  nvgLineTo(args.vg, roll.pos.x + roll.size.x, keys.back().pos.y);
  nvgStroke(args.vg);

  nvgBeginPath(args.vg);
  nvgStrokeWidth(args.vg, 1.f);
  nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
  nvgMoveTo(args.vg, roll.pos.x, keys[0].pos.y + keys[0].size.y);
  nvgLineTo(args.vg, roll.pos.x + roll.size.x, keys[0].pos.y + keys[0].size.y);
  nvgStroke(args.vg);
}


void UnderlyingRollAreaWidget::onButton(const event::Button &e) {
  if (e.action == GLFW_RELEASE) { return; }

  e.consume(this);

  lastMouseDown = e.pos;

  std::tuple<bool, bool> octaveSwitch = findOctaveSwitch(e.pos);
  std::tuple<bool, int> measureSwitch = findMeasure(e.pos);
  
  if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
    std::tuple<bool, BeatDiv, Key> cell = findCell(e.pos);
    if (!std::get<0>(cell)) { Widget::onButton(e); return; }

    int currentPattern = transport->currentPattern();

    int beatDiv = std::get<1>(cell).num;

  	APP->history->push(new PatternData::PatternAction("toggle retrigger", patternData->moduleId, transport->currentPattern(), *patternData));
    patternData->toggleStepRetrigger(currentPattern, state->currentMeasure, beatDiv);
  } else if (e.button == GLFW_MOUSE_BUTTON_LEFT && std::get<0>(octaveSwitch)) {
    state->lowestDisplayNote = clamp(state->lowestDisplayNote + 12, -1 * 12, 8 * 12);
    state->dirty = true;
  } else if (e.button == GLFW_MOUSE_BUTTON_LEFT && std::get<1>(octaveSwitch)) {
    state->lowestDisplayNote = clamp(state->lowestDisplayNote - 12, -1 * 12, 8 * 12);
    state->dirty = true;
  } else if (e.button == GLFW_MOUSE_BUTTON_LEFT && std::get<0>(measureSwitch)) {
    state->currentMeasure = std::get<1>(measureSwitch);
    state->dirty = true;
  }
  Widget::onButton(e);
}

void UnderlyingRollAreaWidget::onDragStart(const event::DragStart &e) {
  if (e.button != GLFW_MOUSE_BUTTON_LEFT) { return; }

  e.consume(this);

  Vec pos = lastMouseDown;
  std::tuple<bool, BeatDiv, Key> cell = findCell(pos);
  std::tuple<bool, int> measureSwitch = findMeasure(pos);

  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  Rect keysArea = reserveKeysArea(roll);
  bool inKeysArea = keysArea.isContaining(pos);

  Rect playDragArea(Vec(roll.pos.x, roll.pos.y), Vec(roll.size.x, topMargins));

  if (std::get<0>(cell) && (APP->window->getMods() & GLFW_MOD_SHIFT)) {
    currentDragType = new VelocityDragging(this, patternData, transport, state, transport->currentPattern(), state->currentMeasure, std::get<1>(cell).num);
  } else if (std::get<0>(cell)) {
    currentDragType = new NotePaintDragging(this, patternData, transport, auditioner);
  } else if (inKeysArea) {
    currentDragType = new KeyboardDragging(this->state);
  } else if (playDragArea.isContaining(pos)) {
    currentDragType = new PlayPositionDragging(auditioner, this, transport);
  } else if (std::get<0>(measureSwitch)) {
    currentDragType = new LockMeasureDragging(state, transport);
  }

  Widget::onDragStart(e);
}

void UnderlyingRollAreaWidget::onDragMove(const event::DragMove &e) {
  if (currentDragType != NULL) {
    currentDragType->onDragMove(e);
  } else {
    Widget::onDragMove(e);
  }
}

void UnderlyingRollAreaWidget::onDragEnd(const event::DragEnd &e) {
  if (currentDragType != NULL) {
    delete currentDragType;
    currentDragType = NULL;
  }
}

void UnderlyingRollAreaWidget::drawBeats(const DrawArgs &args, const std::vector<BeatDiv> &beatDivs) {
  bool first = true;
  for (const auto &beatDiv : beatDivs) {

    nvgBeginPath(args.vg);

    if (beatDiv.beat && !first) {
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
      nvgStrokeWidth(args.vg, 1.0f);
    } else if (beatDiv.triplet) {
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
      nvgStrokeWidth(args.vg, 0.5f);
    } else {
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5));
      nvgStrokeWidth(args.vg, 0.5f);
    }

    nvgMoveTo(args.vg, beatDiv.pos.x, beatDiv.pos.y);
    nvgLineTo(args.vg, beatDiv.pos.x, beatDiv.pos.y + beatDiv.size.y);

    nvgStroke(args.vg);

    first = false;
  }
}

void UnderlyingRollAreaWidget::drawNotes(const DrawArgs &args, const std::vector<Key> &keys, const std::vector<BeatDiv> &beatDivs) {
  int lowPitch = keys.front().num + (keys.front().oct * 12);
  int highPitch = keys.back().num + (keys.back().oct * 12);

  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  reserveKeysArea(roll);

  int pattern = transport->currentPattern();

  for (const auto &beatDiv : beatDivs) {
    if (patternData->isStepActive(pattern, state->currentMeasure, beatDiv.num) == false ) { continue; }
    int pitch = patternData->getStepPitch(pattern, state->currentMeasure, beatDiv.num);

    if (pitch < lowPitch) {
      nvgBeginPath(args.vg);
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgStrokeWidth(args.vg, 1.f);
      nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgRect(args.vg, beatDiv.pos.x, roll.pos.y + roll.size.y - topMargins, beatDiv.size.x, 1);
      nvgStroke(args.vg);
      nvgFill(args.vg);
      continue;
    }

    if (pitch > highPitch) {
      nvgBeginPath(args.vg);
      nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgStrokeWidth(args.vg, 1.f);
      nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgRect(args.vg, beatDiv.pos.x, roll.pos.y + topMargins -1, beatDiv.size.x, 1);
      nvgStroke(args.vg);
      nvgFill(args.vg);
      continue;
    }

    for (auto const& key: keys) {
      if (key.num + (key.oct * 12) == pitch) {

        float velocitySize = (patternData->getStepVelocity(pattern, state->currentMeasure, beatDiv.num) * key.size.y * 0.9f) + (key.size.y * 0.1f);

        nvgBeginPath(args.vg);
        nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
        nvgStrokeWidth(args.vg, 0.5f);
        nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
        nvgRect(args.vg, beatDiv.pos.x, key.pos.y, beatDiv.size.x, (key.size.y - velocitySize));
        nvgStroke(args.vg);
        nvgFill(args.vg);

        nvgBeginPath(args.vg);
        nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
        nvgStrokeWidth(args.vg, 0.5f);
        nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
        nvgRect(args.vg, beatDiv.pos.x, key.pos.y + (key.size.y - velocitySize), beatDiv.size.x, velocitySize);
        nvgStroke(args.vg);
        nvgFill(args.vg);


        if (patternData->isStepRetriggered(pattern, state->currentMeasure, beatDiv.num)) {
          nvgBeginPath(args.vg);

          nvgStrokeWidth(args.vg, 0.f);
          nvgFillColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 1.f));

          nvgRect(args.vg, beatDiv.pos.x, key.pos.y, beatDiv.size.x / 4.f, key.size.y);
          nvgStroke(args.vg);
          nvgFill(args.vg);
        }

        break;
      }
    }

  }
}

void UnderlyingRollAreaWidget::drawMeasures(const DrawArgs &args) {
  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  reserveKeysArea(roll);

  int numberOfMeasures = patternData->getMeasures(transport->currentPattern());

  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float boxHeight = topMargins * 0.75;

  for (int i = 0; i < numberOfMeasures; i++) {
    bool drawingCurrentMeasure = i == state->currentMeasure;
    nvgBeginPath(args.vg);
    nvgStrokeColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 0.1f));
    nvgStrokeWidth(args.vg, 1.f);
    nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, drawingCurrentMeasure ? 1.f : 0.25f));
    nvgRect(args.vg, roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight, widthPerMeasure, boxHeight);
    nvgStroke(args.vg);
    nvgFill(args.vg);

    if (drawingCurrentMeasure && state->measureLockPressTime > 0.5f) {
      float barHeight = boxHeight * rescale(clamp(state->measureLockPressTime, 0.f, 1.f), 0.5f, 1.f, 0.f, 1.f);
      nvgBeginPath(args.vg);
      nvgStrokeColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.f));
      nvgStrokeWidth(args.vg, 0.f);
      nvgFillColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
      nvgRect(args.vg, roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - barHeight, widthPerMeasure, barHeight);
      nvgStroke(args.vg);
      nvgFill(args.vg);
    }
  }

  if (transport->isLocked()) {
    nvgBeginPath(args.vg);
    nvgStrokeColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
    nvgStrokeWidth(args.vg, 2.f);
    nvgRect(args.vg, roll.pos.x, roll.pos.y + roll.size.y - boxHeight, roll.size.x, boxHeight);
    nvgStroke(args.vg);
  }
}

void UnderlyingRollAreaWidget::drawPlayPosition(const DrawArgs &args) {
  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
  reserveKeysArea(roll);

  int divisionsPerMeasure = patternData->getStepsPerMeasure(transport->currentPattern());
  int playingMeasure = transport->currentMeasure();
  int noteInMeasure = transport->currentStepInMeasure();
  int numberOfMeasures = patternData->getMeasures(transport->currentPattern());

  if (noteInMeasure == -1) {
    return;
  }

  if (playingMeasure == state->currentMeasure) {

    float divisionWidth = roll.size.x / divisionsPerMeasure;
    nvgBeginPath(args.vg);
    nvgStrokeColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 0.5f));
    nvgStrokeWidth(args.vg, 0.5f);
    nvgFillColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
    nvgRect(args.vg, roll.pos.x + (noteInMeasure * divisionWidth), roll.pos.y, divisionWidth, roll.size.y - topMargins);
    nvgStroke(args.vg);
    nvgFill(args.vg);
  }

  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float stepWidthInMeasure = widthPerMeasure / divisionsPerMeasure;	
  nvgBeginPath(args.vg);
  nvgStrokeColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
  nvgStrokeWidth(args.vg, 1.f);
  nvgFillColor(args.vg, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
  nvgRect(args.vg, roll.pos.x + (playingMeasure * widthPerMeasure) + (noteInMeasure * stepWidthInMeasure), roll.pos.y + roll.size.y - topMargins + 2, stepWidthInMeasure, topMargins - 2);
  nvgStroke(args.vg);
  nvgFill(args.vg);
}

void UnderlyingRollAreaWidget::drawVelocityInfo(const DrawArgs &args) {
  char buffer[100];

  if (state->displayVelocityHigh > -1 || state->displayVelocityLow > -1) {
    float displayVelocity = std::max(state->displayVelocityHigh, state->displayVelocityLow);

    Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));
    reserveKeysArea(roll);

    float posy;
    if (state->displayVelocityHigh > -1) {
      posy = roll.pos.y + ((roll.size.y * 0.25) * 1);
    } else {
      posy = roll.pos.y + ((roll.size.y * 0.25) * 3);				
    }

    nvgBeginPath(args.vg);
    snprintf(buffer, 100, "Velocity: %06.3fV (Midi %03d)", displayVelocity * 10.f, (int)(127 * displayVelocity));

    nvgFontSize(args.vg, roll.size.y / 12.f);
    float *bounds = new float[4];
    nvgTextBounds(args.vg, roll.pos.x, posy, buffer, NULL, bounds);

    nvgStrokeColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgStrokeWidth(args.vg, 5.f);
    nvgFillColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgRect(args.vg, roll.pos.x + (roll.size.x / 2.f) - ((bounds[2] - bounds[0]) / 2.f), bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
    nvgStroke(args.vg);
    nvgFill(args.vg);

    nvgBeginPath(args.vg);
    nvgStrokeColor(args.vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgFillColor(args.vg, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
    nvgTextAlign(args.vg, NVG_ALIGN_LEFT + NVG_ALIGN_MIDDLE);
    nvgText(args.vg, roll.pos.x + (roll.size.x / 2.f) - ((bounds[2] - bounds[0]) / 2.f), posy, buffer, NULL);

    delete bounds;

    nvgStroke(args.vg);
    nvgFill(args.vg);
  }
}

void UnderlyingRollAreaWidget::draw(const DrawArgs &args) {
  Widget::draw(args);

  Rect roll = Rect(Vec(0,0), Vec(box.size.x, box.size.y));

  int measure = transport->currentMeasure();
  if (measure != state->currentMeasure && state->lastDrawnStep != transport->currentStepInPattern()) {
    state->currentMeasure = measure;
  }
  state->lastDrawnStep = transport->currentStepInPattern();

  Rect keysArea = reserveKeysArea(roll);
  auto keys = getKeys(keysArea);
  drawKeys(args, keys);
  drawSwimLanes(args, roll, keys);
  auto beatDivs = getBeatDivs(roll);
  drawBeats(args, beatDivs);
  drawNotes(args, keys, beatDivs);
  drawMeasures(args);
  drawPlayPosition(args);
  drawVelocityInfo(args);
}	
