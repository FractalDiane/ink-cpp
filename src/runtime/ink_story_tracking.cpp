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

cparse::TokenMap InkStoryTracking::add_visit_count_variables(const cparse::TokenMap& variables, Knot* current_knot, Stitch* current_stitch) {
	cparse::TokenMap result = variables;

	for (auto& knot : knot_stats) {
		result[knot.second.name] = knot.second.times_visited;
		for (std::uint32_t stitch_id : knot.second.stitches) {
			StitchStats& stitch = stitch_stats[stitch_id];
			std::string stitch_name = std::format("{}.{}", knot.second.name, stitch.name);
			result[stitch_name] = stitch.times_visited;

			if (current_knot->uuid == knot.first) {
				result[stitch.name] = stitch.times_visited;
			}

			for (std::uint32_t gather_id : stitch.gather_points) {
				SubKnotStats& gather_point = gather_point_stats[gather_id];
				std::string gather_point_name = std::format("{}.{}.{}", knot.second.name, stitch.name, gather_point.name);
				result[gather_point_name] = gather_point.times_visited;

				if (current_knot->uuid == knot.first) {
					std::string gather_point_name_2 = std::format("{}.{}", stitch.name, gather_point.name);
					result[gather_point_name_2] = gather_point.times_visited;
				}

				if (current_stitch && current_stitch->uuid == stitch_id) {
					result[gather_point.name] = gather_point.times_visited;
				}
			}
		}

		for (std::uint32_t gather_id : knot.second.gather_points) {
			SubKnotStats& gather_point = gather_point_stats[gather_id];
			std::string gather_point_name = std::format("{}.{}", knot.second.name, gather_point.name);
			result[gather_point_name] = gather_point.times_visited;

			if (current_knot->uuid == knot.first) {
				result[gather_point.name] = gather_point.times_visited;
			}
		}
	}

	return result;
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
