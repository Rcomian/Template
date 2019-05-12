#include "rack0.hpp"

using namespace rack;

namespace SongRoll {

  class SongRollData;

  class RollArea : public VirtualWidget {
  public:
    SongRollData& data;
    RollArea(Rect box, SongRollData& data);
  };
}
