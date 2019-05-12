#include "rack0.hpp"

using namespace rack;

struct PianoRollWidget;
struct PianoRollModule;

struct PatternWidget : LedDisplay {
	/** Not owned */
	PianoRollWidget *widget = NULL;
	PianoRollModule *module = NULL;

	LedDisplayChoice *patternChoice;
	LedDisplaySeparator *patternSeparator;
	LedDisplayChoice *octaveChoice;
	LedDisplaySeparator *octaveSeparator;
	LedDisplayChoice *measuresChoice;
	LedDisplaySeparator *measuresSeparator;
	LedDisplayChoice *beatsPerMeasureChoice;
	LedDisplaySeparator *beatsPerMeasureSeparator;
	LedDisplayChoice *divisionsPerBeatChoice;
	LedDisplaySeparator *divisionsPerBeatSeparator;
	LedDisplayChoice *sequenceRunningChoice;
	PatternWidget();
};
