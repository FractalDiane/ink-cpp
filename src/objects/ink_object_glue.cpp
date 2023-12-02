#include "objects/ink_object_glue.h"

void InkObjectGlue::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.in_glue = true;
}
