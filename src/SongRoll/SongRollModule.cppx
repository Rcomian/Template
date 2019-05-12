#include "../SongRoll/SongRollModule.hpp"

using namespace rack;
using namespace SongRoll;

SongRollModule::SongRollModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), runInputActive(false), transport(&songRollData) {
}

void SongRollModule::onReset() {
  transport.reset();
  songRollData.reset();
}

json_t *SongRollModule::dataToJson() {
  json_t *rootJ = Module::dataToJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  return rootJ;
}

void SongRollModule::dataFromJson(json_t *rootJ) {
  Module::dataFromJson(rootJ);

}

void SongRollModule::step() {
}
