#include "objects/ink_object_choicetextmix.h"

ByteVec InkObjectChoiceTextMix::to_bytes() const {
	Serializer<std::uint8_t> s;
	return s(static_cast<std::uint8_t>(end));
}

InkObject* InkObjectChoiceTextMix::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds;
	end = static_cast<bool>(ds(bytes, index));
	return this;
}

void InkObjectChoiceTextMix::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.choice_mix_position = !end ? InkStoryState::ChoiceMixPosition::In : InkStoryState::ChoiceMixPosition::After;
}

std::string InkObjectChoiceTextMix::to_string() const
{
	return !end ? "[" : "]";
}
