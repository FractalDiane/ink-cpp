#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "runtime/ink_story_structs.h"

#include "shunting-yard.h"

struct InkStoryTracking {
	struct SubKnotStats {
		std::string name;
		std::size_t times_visited = 0;
		std::int64_t turns_since_visited = -1;

		SubKnotStats() : name{std::string()}, times_visited{0}, turns_since_visited{-1} {}
		SubKnotStats(const std::string& name) : name{name}, times_visited{0}, turns_since_visited{-1} {}
	};

	struct KnotStats : public SubKnotStats {
		std::vector<std::uint32_t> stitches;
		std::vector<std::uint32_t> gather_points;
	};

	struct StitchStats : public SubKnotStats {
		std::vector<std::uint32_t> gather_points;
	};

	std::unordered_map<std::uint32_t, KnotStats> knot_stats;
	std::unordered_map<std::uint32_t, StitchStats> stitch_stats;
	std::unordered_map<std::uint32_t, SubKnotStats> gather_point_stats;

	void increment_visit_count(Knot* knot, Stitch* stitch = nullptr, GatherPoint* gather_point = nullptr);
	void increment_turns_since();
	cparse::TokenMap add_visit_count_variables(const cparse::TokenMap& variables);
	bool get_content_stats(InkWeaveContent* content, InkStoryTracking::SubKnotStats& result);
};
