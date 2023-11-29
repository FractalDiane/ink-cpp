#include "objects/ink_object_text.h"

#include "runtime/ink_story.h"

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
	eval_result.result += text_contents;

	if (story_state.get_current_object(1)->get_id() != ObjectId::Tag) {
		eval_result.should_continue = false;
	}
}

void InkObjectText::append_text(const std::string& text) {
	text_contents += text;
}
