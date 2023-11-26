#include "runtime/ink_story.h"

InkStory::~InkStory() {
	for (const std::string& knot : knot_order) {
		const auto& knot_objects = knots[knot];
		for (InkObject* object : knot_objects) {
			delete object;
		}
	}
}
