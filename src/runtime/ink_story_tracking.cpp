#include "runtime/ink_story_tracking.h"

#include <format>

void InkStoryTracking::increment_visit_count(Knot* knot, Stitch* stitch, GatherPoint* gather_point) {
	if (gather_point) {
		auto& entry = gather_point_stats[gather_point->uuid];
		++entry.times_visited;
		entry.turns_since_visited = 0;
	} else if (stitch) {
		auto& entry = stitch_stats[stitch->uuid];
		++entry.times_visited;
		entry.turns_since_visited = 0;
	} else if (knot) {
		auto& entry = knot_stats[knot->uuid];
		++entry.times_visited;
		entry.turns_since_visited = 0;
	}
}

void InkStoryTracking::increment_turns_since() {
	for (auto& knot : knot_stats) {
		++knot.second.turns_since_visited;
	}

	for (auto& stitch : stitch_stats) {
		++stitch.second.turns_since_visited;
	}

	for (auto& gather_point : gather_point_stats) {
		++gather_point.second.turns_since_visited;
	}
}

bool InkStoryTracking::get_content_stats(InkWeaveContent* content, InkStoryTracking::SubKnotStats& result) {
	if (auto knot = knot_stats.find(content->uuid); knot != knot_stats.end()) {
		result = knot->second;
		return true;
	} else if (auto stitch = stitch_stats.find(content->uuid); stitch != stitch_stats.end()) {
		result = stitch->second;
		return true;
	} else if (auto gather_point = gather_point_stats.find(content->uuid); gather_point != gather_point_stats.end()) {
		result = gather_point->second;
		return true;
	}

	return false;
}
