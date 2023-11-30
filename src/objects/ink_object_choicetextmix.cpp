#include "objects/ink_object_choicetextmix.h"

void InkObjectChoiceTextMix::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {

}

std::string InkObjectChoiceTextMix::to_string() const
{
	return !end ? "[" : "]";
}