#include "objects/ink_object_divert.h"

std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	Serializer<std::string> s;
	return s(target_knot);
}

InkObject* InkObjectDivert::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	target_knot = ds(bytes, index);
	return this;
}

void InkObjectDivert::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if (target_knot == "END" || target_knot == "DONE") {
		story_state.should_end_story = true;
	} else {
		eval_result.target_knot = target_knot;
	}
}
