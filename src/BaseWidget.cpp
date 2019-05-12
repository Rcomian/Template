#include "BaseWidget.hpp"
#include "window.hpp"

BaseWidget::BaseWidget(Module* module) : ModuleWidget(module) {
  box.size.y = RACK_GRID_HEIGHT;
}

static const float COLOURDRAG_SENSITIVITY = 0.0015f;

void ColourChangeWidget::onButton(const event::Button &e) {
  Widget::onButton(e);
  e.stopPropagating();
  if (e.button == GLFW_MOUSE_BUTTON_LEFT && (APP->window->getMods() & GLFW_MOD_SHIFT)) {
    // Consume if not consumed by child
    if (!e.isConsumed())
      e.consume(this);
  }
}

void ColourChangeWidget::onDragStart(const event::DragStart& e) {
  if ((APP->window->getMods() & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT) {
    dragging = true;
    APP->window->cursorLock();
  }

  Widget::onDragStart(e);
}

void ColourChangeWidget::onDragMove(const event::DragMove& e) {
  if (dragging) {
    float speed = 1.f;
    float range = 1.f;

    float delta = COLOURDRAG_SENSITIVITY * e.mouseDelta.y * speed * range;
    if (APP->window->getMods() & GLFW_MOD_CONTROL) {
      delta /= 16.f;
    }

    if (colourData) {
      colourData->backgroundHue = clamp(colourData->backgroundHue + delta, 0.f, 1.f);
    }
  } else {
    Widget::onDragMove(e);
  }
}

void ColourChangeWidget::onDragEnd(const event::DragEnd& e) {
  if (dragging) {
    dragging = false;
    APP->window->cursorUnlock();
  }

  Widget::onDragEnd(e);
}

json_t *BaseModule::dataToJson() {
  json_t *rootJ = Module::dataToJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  if (colourData) {
    json_object_set_new(rootJ, "backgroundHue", json_real(colourData->backgroundHue));
    json_object_set_new(rootJ, "backgroundSaturation", json_real(colourData->backgroundSaturation));
    json_object_set_new(rootJ, "backgroundLuminosity", json_real(colourData->backgroundLuminosity));
  }
  return rootJ;
}

void BaseModule::dataFromJson(json_t *rootJ) {
  Module::fromJson(rootJ);

  if (colourData) {
    json_t *backgroundHueJ = json_object_get(rootJ, "backgroundHue");
    if (backgroundHueJ) {
      colourData->backgroundHue = json_real_value(backgroundHueJ);
    }

    json_t *backgroundSaturationJ = json_object_get(rootJ, "backgroundSaturation");
    if (backgroundSaturationJ) {
      colourData->backgroundSaturation = json_real_value(backgroundSaturationJ);
    }

    json_t *backgroundLuminosityJ = json_object_get(rootJ, "backgroundLuminosity");
    if (backgroundLuminosityJ) {
      colourData->backgroundLuminosity = json_real_value(backgroundLuminosityJ);
    }
  }
}

void BaseWidget::initColourChange(Rect hotspot, BaseModule* baseModule, float hue, float saturation, float luminosity) {
  colourData.backgroundHue = hue;
  colourData.backgroundSaturation = saturation;
  colourData.backgroundLuminosity = luminosity;

  if (baseModule) {
    baseModule->colourData = &colourData;
  }

  auto colourChangeWidget = createWidget<ColourChangeWidget>(hotspot.pos);
  colourChangeWidget->box.size = hotspot.size;
  colourChangeWidget->colourData = &colourData;
  addChild(colourChangeWidget);
}

void BaseWidget::draw(const DrawArgs& args) {
  nvgBeginPath(args.vg);
  nvgFillColor(args.vg, nvgHSL(colourData.backgroundHue, colourData.backgroundSaturation, colourData.backgroundLuminosity));
  nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
  nvgFill(args.vg);

  ModuleWidget::draw(args);
}

