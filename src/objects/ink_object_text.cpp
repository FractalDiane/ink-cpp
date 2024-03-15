#include "objects/ink_object_text.h"

#include "runtime/ink_story.h"

#include "ink_utils.h"

std::vector<std::uint8_t> InkObjectText::to_bytes() const {
	Serializer<std::string> s;
	return s(text_contents);
}

InkObject* InkObjectText::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Deserializer<std::string> ds;
	text_contents = ds(bytes, index);
	return this;
}

void InkObjectText::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if ((story_state.selected_choice == SIZE_MAX && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::After)
	|| (story_state.selected_choice != SIZE_MAX && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::In)) {
		eval_result.result += text_contents;
	}
}

bool InkObjectText::has_any_contents(bool strip) const {
	return strip ? !strip_string_edges(text_contents, true, true, true).empty() : !text_contents.empty();
}

void InkObjectText::append_text(const std::string& text) {
	text_contents += text;
}
