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

void InkStoryTracking::update_visit_count_variables(Knot* current_knot, Stitch* current_stitch, ExpressionParserV2::StoryVariableInfo& story_variable_info) {
	#define TK(what) static_cast<std::int64_t>(what)

	for (auto& knot : knot_stats) {
		story_variable_info.constants[knot.second.name] = TK(knot.second.times_visited);
		for (Uuid stitch_id : knot.second.stitches) {
			StitchStats& stitch = stitch_stats[stitch_id];
			std::string stitch_name = std::format("{}.{}", knot.second.name, stitch.name);
			story_variable_info.constants[stitch_name] = TK(stitch.times_visited);

			if (current_knot->uuid == knot.first) {
				story_variable_info.constants[stitch.name] = TK(stitch.times_visited);
			}

			for (Uuid gather_id : stitch.gather_points) {
				SubKnotStats& gather_point = gather_point_stats[gather_id];
				std::string gather_point_name = std::format("{}.{}.{}", knot.second.name, stitch.name, gather_point.name);
				story_variable_info.constants[gather_point_name] = TK(gather_point.times_visited);

				if (current_knot->uuid == knot.first) {
					std::string gather_point_name_2 = std::format("{}.{}", stitch.name, gather_point.name);
					story_variable_info.constants[gather_point_name_2] = TK(gather_point.times_visited);
				}

				if (current_stitch && current_stitch->uuid == stitch_id) {
					story_variable_info.constants[gather_point.name] = TK(gather_point.times_visited);
				}
			}
		}

		for (Uuid gather_id : knot.second.gather_points) {
			SubKnotStats& gather_point = gather_point_stats[gather_id];
			std::string gather_point_name = std::format("{}.{}", knot.second.name, gather_point.name);
			story_variable_info.constants[gather_point_name] = TK(gather_point.times_visited);

			if (current_knot->uuid == knot.first) {
				story_variable_info.constants[gather_point.name] = TK(gather_point.times_visited);
			}
		}
	}

	#undef TK
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
