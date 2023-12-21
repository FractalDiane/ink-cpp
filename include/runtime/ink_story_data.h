#pragma once

#include "objects/ink_object.h"
#include "serialization.h"
#include "runtime/ink_story_structs.h"

#include "shunting-yard.h"

#include <string>
#include <vector>
#include <unordered_map>

class InkStoryData {
private:
	std::unordered_map<std::string, Knot> knots;
	std::vector<std::string> knot_order;

	friend class InkStory;

public:
	InkStoryData(const std::vector<Knot>& story_knots);
	~InkStoryData();

	std::vector<std::uint8_t> get_serialized_bytes() const;

	void print_info() const;

	void increment_visit_count(Knot* knot, Stitch* stitch = nullptr, GatherPoint* gather_point = nullptr);
	void increment_turns_since();
	InkWeaveContent* get_content(const std::string& path, Knot* current_knot, Stitch* current_stitch);
	cparse::TokenMap add_visit_count_variables(const cparse::TokenMap& variables) const;
};
