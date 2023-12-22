#include "objects/ink_object_glue.h"

void InkObjectGlue::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if (!story_state.in_choice_text) {
		story_state.in_glue = true;
	}
}
