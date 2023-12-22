#include "objects/ink_object_choicetextmix.h"

void InkObjectChoiceTextMix::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.choice_mix_position = !end ? InkStoryState::ChoiceMixPosition::In : InkStoryState::ChoiceMixPosition::After;
}

std::string InkObjectChoiceTextMix::to_string() const
{
	return !end ? "[" : "]";
}