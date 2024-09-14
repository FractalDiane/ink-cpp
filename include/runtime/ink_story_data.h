#pragma once

#include "objects/ink_object.h"
#include "serialization.h"
#include "runtime/ink_story_structs.h"

#include <string>
#include <vector>
#include <unordered_map>

struct GetContentResult {
	WeaveContentType result_type = WeaveContentType::Knot;
	Knot* knot = nullptr;
	Stitch* stitch = nullptr;
	GatherPoint* gather_point = nullptr;
	bool found_any = false;
	bool is_choice_label = false;
	ChoiceLabelData choice_label;

	InkWeaveContent* get_target() {
		switch (result_type) {
			case WeaveContentType::Knot:
				return knot;
			case WeaveContentType::Stitch:
				return stitch;
			case WeaveContentType::GatherPoint:
			default:
				return gather_point;
		}
	}
};

class InkStoryData {
private:
	std::unordered_map<std::string, Knot> knots;
	std::vector<std::string> knot_order;
	ExpressionParserV2::StoryVariableInfo variable_info;

	friend class InkStory;
	friend class InkCompiler;

public:
	InkStoryData(const std::vector<Knot>& story_knots, ExpressionParserV2::StoryVariableInfo&& variable_info);
	~InkStoryData();

	std::vector<std::uint8_t> get_serialized_bytes() const;

	void print_info() const;

	GetContentResult get_content(const std::string& path, const std::vector<KnotStatus>& knots_stack, Stitch* current_stitch);
};
