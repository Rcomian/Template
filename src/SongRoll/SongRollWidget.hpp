#include <tuple>
#include <limits>

#include "rack.hpp"
#include "../BaseWidget.hpp"

using namespace rack;


namespace SongRoll {

  struct ModuleDragType;
  struct SongRollModule;

  struct SongRollWidget : BaseWidget {
    SongRollModule* module;

    SongRollWidget(SongRollModule *module);

    Rect getRollArea();

    void drawBackgroundColour(NVGcontext* ctx);
    void drawPatternEditors(NVGcontext* ctx);

    // Event Handlers

    void appendContextMenu(Menu* menu) override;
    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
    void onDragStart(EventDragStart& e) override;
    void baseDragMove(EventDragMove& e);
    void onDragMove(EventDragMove& e) override;
    void onDragEnd(EventDragEnd& e) override;

    json_t *dataToJson() override;
    void dataFromJson(json_t *rootJ) override;

  };
}
