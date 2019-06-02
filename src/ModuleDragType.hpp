#include "rack.hpp"

struct ModuleDragType {
	ModuleDragType();
	virtual ~ModuleDragType();

	virtual void onDragMove(const rack::event::DragMove &e) = 0;
};
