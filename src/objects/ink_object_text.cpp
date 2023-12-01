#include "objects/ink_object_text.h"

#include "runtime/ink_story.h"

#include "ink_utils.h"

std::vector<std::uint8_t> InkObjectText::to_bytes() const {
	Serializer<std::string> s;
	return s(text_contents);
}

InkObject* InkObjectText::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	text_contents = ds(bytes, index);
	return this;
}

void InkObjectText::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if ((story_state.selected_choice == -1 && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::After)
	|| (story_state.selected_choice != -1 && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::In)) {
		InkObject* next_object = story_state.get_current_object(1);
		bool strip_right = next_object && next_object->strip_previous_spaces();
		eval_result.result += strip_string_edges(text_contents, true, strip_right, true);
	}
}

void InkObjectText::append_text(const std::string& text) {
	text_contents += text;
}
