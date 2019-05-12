#include "plugin.hpp"

#include "PianoRoll/PianoRollModule.hpp"
#include "PianoRoll/PianoRollWidget.hpp"

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelPianoRollModule = createModel<PianoRollModule, PianoRollWidget>("rcm-pianoroll");
