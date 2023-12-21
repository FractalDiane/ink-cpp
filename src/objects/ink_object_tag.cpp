#include "objects/ink_object_tag.h"

std::vector<std::uint8_t> InkObjectTag::to_bytes() const {
	Serializer<std::string> s;
	return s(tag);
}

InkObject* InkObjectTag::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	tag = ds(bytes, index);
	return this;
}

void InkObjectTag::execute(InkStoryData* const story_data, InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.current_tags.push_back(tag);
}
