#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "runtime/ink_story_structs.h"

#include "expression_parser/expression_parser.h"

struct InkStoryTracking {
	struct SubKnotStats {
		std::string name;
		std::size_t times_visited = 0;
		std::int64_t turns_since_visited = -1;

		SubKnotStats() : name{std::string()}, times_visited{0}, turns_since_visited{-1} {}
		SubKnotStats(const std::string& name) : name{name}, times_visited{0}, turns_since_visited{-1} {}
	};

	struct KnotStats : public SubKnotStats {
		std::vector<Uuid> stitches;
		std::vector<Uuid> gather_points;
	};

	struct StitchStats : public SubKnotStats {
		std::vector<Uuid> gather_points;
	};

	std::unordered_map<Uuid, KnotStats> knot_stats;
	std::unordered_map<Uuid, StitchStats> stitch_stats;
	std::unordered_map<Uuid, SubKnotStats> gather_point_stats;

	void increment_visit_count(Knot* knot, Stitch* stitch = nullptr, GatherPoint* gather_point = nullptr);
	void increment_turns_since();
	//ExpressionParser::VariableMap get_visit_count_variables(Knot* current_knot, Stitch* current_stitch);
	void update_visit_count_variables(Knot* current_knot, Stitch* current_stitch, ExpressionParserV2::StoryVariableInfo& story_variable_info);
	bool get_content_stats(InkWeaveContent* content, InkStoryTracking::SubKnotStats& result);
};
