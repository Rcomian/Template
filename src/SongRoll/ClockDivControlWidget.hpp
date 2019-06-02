#include "rack.hpp"

using namespace rack;

namespace SongRoll {

  struct SongRollData;

  class ClockDivControlWidget : public VirtualWidget {
  public:
    int clock_div=1;

    ClockDivControlWidget();

    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
  };

}
